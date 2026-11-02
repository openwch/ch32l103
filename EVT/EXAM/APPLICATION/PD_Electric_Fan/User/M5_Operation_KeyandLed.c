/********************************** (C) COPYRIGHT *******************************
 * File Name          : M5_Operation_KeyandLed.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/11/04
 * Description        : Key and LED operation
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/* Includes -----------------------------------------------------------------*/
#include "M0_Control_Library.h"
/* Private typedef ----------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/

/*********************************************************************
 * @fn      KEY_AND_LEDIO_Init
 *
 * @brief   Button and LED indicator I/O configuration
 *
 * @return  none
 */
void KeyandLED_IOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_AFIO|RCC_PB2Periph_GPIOA|RCC_PB2Periph_GPIOB|RCC_PB2Periph_GPIOC|RCC_PB2Periph_GPIOD, ENABLE);

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//BEEP
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*********************************************************************
 * @fn      KeyandLED_Parameter_Define
 *
 * @brief   Key detection operation parameter definition
 *
 *  @para   KeyM: Key management structure pointer
 *
 * @return  none
 */
void KeyandLED_Parameter_Define(Key_Manager_Type *KeyM)
{
    KeyM->RUN.Effectivetime = (uint16_t)(SRKEY_EFFECTIVE_TIME*KEY_CHECK_FREQ);
    KeyM->RUN.Ineffectivetime = (uint16_t)(SRKEY_INEFFECTIVE_TIME*KEY_CHECK_FREQ);
    KeyM->RUN.Group = GPIOA;
    KeyM->RUN.Pin = GPIO_Pin_11;
    KeyM->RUN.KeyPress=PRESSUP;

    KeyM->Speedadjust.Effectivetime = (uint16_t)(SRKEY_EFFECTIVE_TIME*KEY_CHECK_FREQ);
    KeyM->Speedadjust.Ineffectivetime = (uint16_t)(SRKEY_INEFFECTIVE_TIME*KEY_CHECK_FREQ);
    KeyM->Speedadjust.Group = GPIOA;
    KeyM->Speedadjust.Pin = GPIO_Pin_12;
    KeyM->Speedadjust.KeyPress=PRESSUP;
}

/*********************************************************************
 * @fn      KeyandLED_Status_Init
 *
 * @brief   Key detection operation status initialization
 *
 *  @para   KeyM: Key management structure pointer
 *
 * @return  none
 */
void KeyandLED_Status_Init(Key_Manager_Type *KeyM)
{
    KeyM->Speedadjust.Effectivecnt = 0;
    KeyM->Speedadjust.Ineffectivecnt = 0;
    KeyM->Speedadjust.Status = FALSE;

    KeyM->RUN.Effectivecnt = 0;
    KeyM->RUN.Ineffectivecnt = 0;
    KeyM->RUN.Status = FALSE;

    KeyM->oscillate.Effectivecnt = 0;
    KeyM->oscillate.Ineffectivecnt = 0;
    KeyM->oscillate.Status = FALSE;

    KeyM->Timeoff.Effectivecnt = 0;
    KeyM->Timeoff.Ineffectivecnt = 0;
    KeyM->Timeoff.Status = FALSE;
}

/*********************************************************************
 * @fn      Skeycheck_Recover
 *
 * @brief   Recover type Speed key detection
 *
 *  @para   Keyrec: Recove Key structure pointer
 *
 * @return  none
 */
void Skeycheck_Recover(Keyrecover_Type *Keyrec)
{
    if(GPIO_ReadInputDataBit(Keyrec->Group, Keyrec->Pin) == 0)
    {
        if(Keyrec->Effectivecnt <= U16_MAX)
            Keyrec->Effectivecnt++;

        Keyrec->Ineffectivecnt=0;
    }
    else
    {
        if(Keyrec->Ineffectivecnt <= U16_MAX)
            Keyrec->Ineffectivecnt++;

            Keyrec->Effectivecnt=0;
    }
    if((Keyrec->Effectivecnt >= Keyrec->Effectivetime)&&(Keyrec->KeyPress==PRESSUP))
    {
        Keyrec->Status=TRUE;
        Keyrec->Effectivecnt=0;
        Keyrec->KeyPress=PRESSDOWN;
    }
    else if((Keyrec->Ineffectivecnt>=Keyrec->Ineffectivetime))
    {
        Keyrec->Effectivecnt=0;
        Keyrec->Ineffectivecnt=0;
        Keyrec->KeyPress=PRESSUP;
    }

}
/*********************************************************************
 * @fn      KeyCheck_Process
 *
 * @brief   Key detection and Process
 *
 *  @para   KeyM: Key management structure pointer
 *          Speed_Ramp: acceleration and deceleration control structure
 *
 * @return  none
 */
void KeyCheck_Process(Key_Manager_Type *KeyM, Rampctr_Type *Ramp_M)
{

        Skeycheck_Recover(&(KeyM->RUN));

        if(((RunningStatus_M == RUN)&&(Ramp_M->Decemark == FALSE))&&(KeyM->RUN.KeyPress==PRESSUP))
        {
            Skeycheck_Recover(&(KeyM->Speedadjust));
        }

     if(KeyM->RUN.Status == TRUE)
     {
         switch(Operation_M.Checkmode)
         {
             case 0:
                 if(( (RunningStatus_M == IDLE)||((RunningStatus_M == FAULT)) )&&(Operation_M.Checkmode ==0))
                 {
                       RunningStatus_M = INIT;

                       Ramp_Set(Ramp_M);

                       Operation_M.Checkmode =1;
                 }
                 if((Operation_M.Checkmode ==0)&&(Ramp_M->Decemark ==TRUE))
                 {
                       Ramp_Set(Ramp_M);

                       Ramp_M->Decemark = FALSE;
                       Operation_M.Checkmode = 1;

                       BUZZ_M.Effective = TRUE;
                       BUZZ_M.BuzzMark = 1;
                 }
                break;
            case 1:
                if(Operation_M.Checkmode ==1)
                {
#if     (CLOSE_LOOP==SPEED_LOOP)
                    Ramp_M->Preramp = BREAKSPEED;
#elif   (CLOSE_LOOP==POWER_LOOP)
                    Ramp_M->Preramp = BREAKPOWER;
#endif
                    Ramp_M->Decemark =TRUE;

                    Operation_M.Checkmode =0;

                    BUZZ_M.Effective = TRUE;
                    BUZZ_M.BuzzMark = 2;
                }
                break;

            default:
                break;
             }

         KeyM->RUN.Status = FALSE;
     }
     if((KeyM->Speedadjust.Status == TRUE) &&(Ramp_M->Decemark == FALSE))
     {
         Ramp_M->Point_adncnt++;

         if(Ramp_M->Point_adncnt > POINT_RANK)
             Ramp_M->Point_adncnt = 1;

         KeyM->Speedadjust.Status = FALSE;

         Ramp_Set(Ramp_M);

         BUZZ_M.Effective = TRUE;
         BUZZ_M.BuzzMark = 1;
     }

     //if((Ramp_M->Startfinish ==TRUE)&&(Ramp_M->Decemark ==TRUE))
     if(Ramp_M->Decemark ==TRUE)
     {
#if     (CLOSE_LOOP==SPEED_LOOP)
       if(_IQabs(MLBG_M.Cal_Struc.Angspeed_Aver1)<=FAST_DECE_SPEED)
         {
                 if(RunningStatus_M!=FAULT)
                     RunningStatus_M = STOP;
                 Ramp_M->Decemark = FALSE;
         }
#elif   (CLOSE_LOOP==POWER_LOOP)
         if(Power_Cal.AngPower_Aver1<=FAST_DECE_POWER)
         {
             if(RunningStatus_M!=FAULT)
                 RunningStatus_M = STOP;
             Ramp_M->Decemark = FALSE;
         }
#endif
     }
}

/*******************************************************************************
* Function Name  : PD_Seek
* Description    : PD communication
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************/
//__attribute__((section(".highcode")))
void PD_Process(void)
{
    if(!PD_PHY.WaitTxGcrc)
    {
      if ( PD_PHY.WaitRxGcrc ) {
          PD_PHY.WaitRxGcrc++;
          if(PD_PHY.WaitRxGcrc>2)
          {
              if ( PD_PHY.RetryCnt ) {
                  PD_PHY.RetryCnt --;
                  PD_PHY.WaitRxGcrc = 0;
                  PD_TX_MSG();
                  //DEBUG_Print("Re%d\r\n",PD_PHY.RetryCnt);
              }
              else {
                  //DEBUG_Print("RX GCRC Timeout\r\n");
                  PD_PHY.WaitRxGcrc = 0;
                  (PD_PHY.TxMsgID<7)?(PD_PHY.TxMsgID++):(PD_PHY.TxMsgID=0);
                  PD_PHY.pTxTimeout();
                  //TIM1->CNT = PD_SYS_TIMER;
              }
          }
      }

      if ( PD_PHY.WaitMsgTx ) {

          if ( PD_PHY.MsgTxCnt ) PD_PHY.MsgTxCnt--;
          if ( !PD_PHY.MsgTxCnt ) {
              PD_PHY.WaitMsgTx = 0;
              PD_TX_MSG();
          }
      }
      if ( PD_PHY.WaitMsgRx ) {
          if ( PD_PHY.MsgRxCnt ) PD_PHY.MsgRxCnt--;
          else {
              PD_PHY.WaitMsgRx = 0;
              PD_PHY.pRxTimeout();
          }
      }

      if ( PD_PHY.IdleCnt < 0xFFFF ) PD_PHY.IdleCnt ++;       //Idle counter increment

      PD_DEVICE.pDevChk();        //Check connection status
 }
}
/*******************************************************************************
* Function Name  : PD_Process
* Description    : PD processing
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************/
//__attribute__((section(".highcode")))
void PD_Seek(void)
{
    if ( PD_PHY.WaitTxGcrc )
       {
           PD_PHY.WaitTxGcrc++;
           if(PD_PHY.WaitTxGcrc>2 )
           {
               PD_TX_GCRC();
           }
       }
           if(PD_PHY.RxHRST)
           {
               PD_PHY.RxHRST=0;
               PD_PHY.pRxHRST();
           }

           if(PD_PHY.TxFinish)
           {
               PD_PHY.TxFinish=0;
               PD_PHY.pTxFinish();
           }

           if(PD_PHY.RxFinish)
           {
               PD_PHY.RxFinish=0;
               PD_PHY.pRxFinish();
           }
}
