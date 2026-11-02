/********************************** (C) COPYRIGHT *******************************
 * File Name          : M8_Protection.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2026/10/08
 * Description        : Protection parameter initialization, protection state setting

*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/* Includes -----------------------------------------------------------------*/
#include "M0_Control_Library.h"

/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private functions --------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
InstPhaseOC_Type  InstPhaseOC_M;        // Instantaneous over current protection structure
DCbusProtection_Type DCBusProtecion_M;  // Bus voltage protection structure
Phaseloss_Type Phaseloss_M;             // Phase loss protection structure
/*********************************************************************
 * @fn      Protection_and_Emergency_Parameter_Define
 *
 * @brief   Protection parameters are initialization
 *
 * @return  none
 */
void Protection_and_Emergency_Define (void)
{
    InstPhaseOC_M.Cnt = 0;
    InstPhaseOC_M.CurrThreshold = INST_PHASEOC_THRE_M;
    InstPhaseOC_M.CntThreshold = INST_PHASEOC_CNT_THRE_M;

    DCBusProtecion_M.OverThresh = DCBUS_OVER_VOLT_THRE;
    DCBusProtecion_M.UnderThresh = DCBUS_UNDER_VOLT_THRE;
    DCBusProtecion_M.OverCntThresh = DCBUS_OVER_VOLT_CNT;
    DCBusProtecion_M.UnderCntThresh = DCBUS_UNDER_VOLT_CNT;
    DCBusProtecion_M.OverCnt = 0;
    DCBusProtecion_M.UnderCnt = 0;
    DCBusProtecion_M.Overvolt = FALSE;
    DCBusProtecion_M.Undervolt = FALSE;

    Phaseloss_M.CurrThresh = PHASELOSS_CURR_THRE;
    Phaseloss_M.CntThresh = (uint16_t)(PHASELOSS_TIME_THRE * PHASELOSS_CAL_FREQ);
    Phaseloss_M.MultiphaseCnt = 0;
    Phaseloss_M.SiglephaseCntA = 0;
    Phaseloss_M.SiglephaseCntB = 0;
    Phaseloss_M.SiglephaseCntC = 0;
    Phaseloss_M.CurrBuffPara.Length = PHASELOSS_CAL_BUFFSIZE;
    Phaseloss_M.CurrBuffPara.Inv1 = _IQ(1.0)/PHASELOSS_CAL_BUFFSIZE;
    Phaseloss_M.CurrBuffPara.Inv2 = _IQ(1.0) - Phaseloss_M.CurrBuffPara.Inv1;

    Phaseloss_M.DetectphaseSet = (uint16_t)(PHASELOSS_CAL_FREQ*PHASELOSS_DETECT_TIME);
}
/******************************************************************************
* Function Name  : Protection_and_Emergency_Init
* Description    : Fault protection parameter initialization
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void Protection_and_Emergency_Init (void)
{
    InstPhaseOC_M.Cnt = 0;
    DCBusProtecion_M.OverCnt = 0;
    DCBusProtecion_M.UnderCnt = 0;
    DCBusProtecion_M.Overvolt = FALSE;
    DCBusProtecion_M.Undervolt = FALSE;

    Phaseloss_M.MultiphaseCnt = 0;
    Phaseloss_M.SiglephaseCntA = 0;
    Phaseloss_M.SiglephaseCntB = 0;
    Phaseloss_M.SiglephaseCntC = 0;
    Phaseloss_M.DetectphaseSet = (uint16_t)(PHASELOSS_CAL_FREQ*PHASELOSS_DETECT_TIME);
    Phaseloss_M.Detectphase=FALSE;


}
/******************************************************************************
* Function Name  : Protection_SetFault
* Description    : Fault setting
* Input          : Fault type
* Output         : None
* Return         : None
******************************************************************************/
void Protection_SetFault(uint16_t Fault_type)
{
    //Set the fault hold time; checked before fault clearing (clearing too fast may make it unreadable)
    Time_Delay_Set(Operation_M.FaultHoldingSet,&(Operation_M.FaultHoldingCnt));

    TIM_CtrlPWMOutputs(TIM1, DISABLE);  //Stop PWM output

    ADC_Start(DISABLE);//ADC start

    Operation_M.Checkmode =0;

    RunningStatus_M = FAULT;            //Set the motor running status to FAULT

    System_Status_Global |= Fault_type; //Set the fault bit of the global variable
}

/******************************************************************************
* Function Name  : Protction_Fault_Reset_M
* Description    : Fault reset
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void Protction_Fault_Reset_M(void)
{
    if (Time_Delay_Elapse(Operation_M.FaultHoldingCnt))         //Delay time reached
    {
        System_Status_Global &= ~STARTUP_FAILURE_M;         //Clear the startup failure fault

        System_Status_Global &= ~SPEED_ABNORMAL_M;          //Clear the speed detection abnormal fault
        LBG_Status_M.Unstable_CNT = 0;

        System_Status_Global &= ~MOTOR_OVER_CURR_SOFT_M;    //Clear the software overcurrent fault
        InstPhaseOC_M.Cnt = 0;                              //Clear software overcurrent detection count

        if (GPIO_ReadInputDataBit(BRK_GPIO_M, BRK_PIN_M))   //No hardware overcurrent fault
          System_Status_Global &= ~DC_OVER_CURR_HARD_M;     //Clear fault flag

        System_Status_Global &= ~MOTOR_STALL_M;             //Clear stall abnormal fault
        LBG_Status_M.Stall_CNT = 0;

        System_Status_Global &= ~PHASELOSS_FALUT;           //Clear the phase loss abnormal fault
        Phaseloss_M.MultiphaseCnt = 0;
        Phaseloss_M.SiglephaseCntA = 0;
        Phaseloss_M.SiglephaseCntB = 0;
        Phaseloss_M.SiglephaseCntC = 0;

        if((System_Status_Global&DC_OVER_VOLT)==DC_OVER_VOLT)       //For DC bus overvoltage fault
        {
            if(ADC_M.DCVoltAver < DCBusProtecion_M.OverThresh)   //Check whether it is below the DC bus overvoltage threshold at this time
            {
                System_Status_Global&= ~DC_OVER_VOLT;               //Clear the DC bus overvoltage fault flag
                DCBusProtecion_M.OverCnt = 0;
            }
        }
        if((System_Status_Global&DC_UNDER_VOLT)==DC_UNDER_VOLT)      //For DC bus undervoltage fault
        {
            if(ADC_M.DCVoltAver > DCBusProtecion_M.UnderThresh)  //Check whether it is above the DC bus undervoltage threshold at this time
            {
                System_Status_Global&= ~DC_UNDER_VOLT;              //Clear the DC bus undervoltage fault flag
                DCBusProtecion_M.UnderCnt = 0;
            }
        }
    }

    //Re-judge the fault
    if ( (System_Status_Global & (STARTUP_FAILURE_M |SPEED_ABNORMAL_M | MOTOR_OVER_CURR_SOFT_M | MOTOR_OVERLOAD_M\
             | DC_OVER_CURR_HARD_M | MOTOR_STALL_M |PHASELOSS_FALUT | DC_OVER_VOLT | DC_UNDER_VOLT)) == 0 )
    {
      RunningStatus_M = IDLE;                   //Set IDLE state
    }
}

/******************************************************************************
* Function Name  : DCOvervolt_Protection
* Description    : DC bus overvoltage protection
* Input          : DC bus voltage to be detected; voltage protection structure pointer
* Output         : None
* Return         : None
******************************************************************************/
void DCOvervolt_Protection(_iq24 DCVolt, DCbusProtection_Type *DCbusProtec)
{
    if (DCVolt >= DCbusProtec->OverThresh)
        DCbusProtec->OverCnt++;
    else
        DCbusProtec->OverCnt = 0;

    if(DCbusProtec->OverCnt > DCbusProtec->OverCntThresh)
    {
        Protection_SetFault(DC_OVER_VOLT);//DC bus overvoltage
        DCBusProtecion_M.Overvolt = TRUE;
    }
}
/*********************************************************************
 * @fn      DCUndervolt_Protection
 *
 * @brief   DC bus under voltage protection
 *
 *  @para   DCVolt: DC bus voltage
 *          DCbusProtec: Bus voltage protection structure pointer
 *
 * @return  none
 */
void DCUndervolt_Protection(_iq24 DCVolt, DCbusProtection_Type *DCbusProtec)
{
    if (DCVolt <= DCbusProtec->UnderThresh)
         DCbusProtec->UnderCnt++;
    else
        DCbusProtec->UnderCnt = 0;

    if(DCbusProtec->UnderCnt > DCbusProtec->UnderCntThresh)
    {
        Protection_SetFault(DC_UNDER_VOLT);//DC bus undervoltage
        DCBusProtecion_M.Overvolt = TRUE;
    }
}
