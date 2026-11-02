/********************************** (C) COPYRIGHT *******************************
 * File Name          : M5_Operation_and_Display.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/11/04
 * Description        : Motor operation related processing
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
/* Private Functions --------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
static volatile uint16_t Timewait = 0;

Rampctr_Type   Ramp_M;
Operation_Type      Operation_M;
Powercontrol_Type   Powerctr_M;
Key_Manager_Type    Key_M;
Buzzctrl_Type       BUZZ_M;
/*********************************************************************
 * @fn      Motor_Operation_Process
 *
 * @brief   Motor operation
 *
 * @return  none
 */
void Motor_Operation_Process(void)
{
    switch (RunningStatus_M)
    {
        case IDLE:
            if (Powerctr_M.Poweron == TRUE)
            {
                Ramp_Set(&Ramp_M);
            }
//            RunningStatus_M = INIT;
            break;

        case INIT:
            Motor_Start_Init_M();
            RunningStatus_M = PRESTART;

            BUZZ_M.Effective = TRUE;
            BUZZ_M.BuzzMark = 1;
            break;

        case PRESTART:
            break;

        case POSITION:
            break;

        case START:
            break;

        case RUN:
            break;

        case STOP:
            TIM_CtrlPWMOutputs(TIM1, DISABLE);
            ADC_Start(DISABLE);
            RunningStatus_M = WAIT;
            Time_Delay_Set(Operation_M.StopwaitSet,&Operation_M.StopwaitCnt);
		   break;

        case WAIT:

            if (Time_Delay_Elapse(Operation_M.StopwaitCnt) == TRUE)
            {
                RunningStatus_M=IDLE;
                Operation_M.Checkmode = 0;
            }

            break;
        case FAULT:
            TIM_CtrlPWMOutputs(TIM1, DISABLE);
            ADC_Start(DISABLE);
            break;
        default:
            break;
    }
}
/*********************************************************************
 * @fn      Poweroperation_Initial
 *
 * @brief   Power-on and power-off parameters and status initialization
 *
 *  @para   PowerOper: Power-on and power-off control structure pointer
 *
 * @return  none
 */
void Poweroperation_Initial(Powercontrol_Type *PowerOper)
{
    PowerOper->Poweron_CntThresh = (uint16_t)(POWERVERY_FREQ*POWERON_TIME);
    PowerOper->Poweron = FALSE;
    PowerOper->Poweron_Cnt = 0;
}

/*********************************************************************
 * @fn      Operation_Parameter_Define
 *
 * @brief   Operation parameter definition
 *
 *  @para   Operation: Operation control structure pointer
 *
 * @return  none
 */
void Operation_Parameter_Define(Operation_Type *Operation)
{
    Operation->StopwaitSet = (uint16_t)(STOP_WAITING_TIME * OPERATION_CAL_FREQ);
    Operation->FaultHoldingSet = (uint16_t)(FAULT_HOLDING_TIME * OPERATION_CAL_FREQ);
    Operation->Startmark = FALSE;
//    Operation->Restartmark = FALSE;
    Operation->Checkmode = 0;
}

/*********************************************************************
 * @fn      Operation_Status_Init
 *
 * @brief   Operation status initialization
 *
 *  @para   Operation: Operation control structure pointer
 *
 * @return  none
 */
void Operation_Status_Init(Operation_Type *Operation)
{
    Operation->StopwaitCnt = 0;
    Operation->FaultHoldingCnt = 0;
}
/*********************************************************************
 * @fn      Speedrampctr_Parameter_Define
 *
 * @brief   Speed control parameter definition
 *
 *  @para   Speed_Ramp: Acceleration and deceleration control structure pointer
 *
 * @return  none
 */
void rampctr_Parameter_Define(Rampctr_Type *Ramp_M)
{
    Ramp_M->Direction = STARTUP_DIRECTION;
    Ramp_M->Point_adncnt = POINT_DEFAULT;

    Ramp_M->AccStep = _IQ(1.0)/(SPEED_LOOP_CAL_FREQ*ACCE_TIME_M);
    Ramp_M->DecStep = _IQ(1.0)/(SPEED_LOOP_CAL_FREQ*DECE_TIME_M);
}

/*********************************************************************
 * @fn      Speedrampctr_Initial
 *
 * @brief   Speed control initialization
 *
 *  @para   Speed_Ramp: Acceleration and deceleration control structure pointer
 *
 * @return  none
 */
void rampctr_Initial(Rampctr_Type *Ramp_M)
{
    Ramp_Set(Ramp_M);

#if(CLOSE_LOOP==SPEED_LOOP)
    Ramp_M->Aftramp = SPEEDPOINT1*Ramp_M->Direction;
#elif (CLOSE_LOOP==POWER_LOOP)
    Ramp_M->Aftramp =  POWERPOINT1;
#endif
    Ramp_M->Startfinish =  FALSE;
    Ramp_M->Decemark =FALSE;

    System_Status_Global = SPEED_CONTROL_M;
}

/*********************************************************************
 * @fn      Ramp_Set
 *
 * @brief   Set speed according speed rank
 *
 *  @para   Speed_Ramp: Acceleration and deceleration control structure pointer
 *
 * @return  none
 */
void Ramp_Set(Rampctr_Type *Ramp_M)
{
#if     (CLOSE_LOOP==SPEED_LOOP)
    switch(Ramp_M->Point_adncnt)
    {
        case 1:
           Ramp_M->Preramp =  SPEEDPOINT1*Ramp_M->Direction;
           break;
       case 2:
           Ramp_M->Preramp =  SPEEDPOINT2*Ramp_M->Direction;
           break;
       case 3:
           Ramp_M->Preramp =  SPEEDPOINT3*Ramp_M->Direction;
           break;
       default:
           break;
    }
#elif   (CLOSE_LOOP==POWER_LOOP)
    switch(Ramp_M->Point_adncnt)
    {
        case 1:
           Ramp_M->Preramp =  POWERPOINT1;
           break;
       case 2:
           Ramp_M->Preramp =  POWERPOINT2;
           break;
       case 3:
           Ramp_M->Preramp =  POWERPOINT3;
           break;
       default:
           break;
    }
#endif
}
/******************************************************************************
* Function Name  : Speed_Ramp_Process
* Description    : Speed ramp processing, executed in the loop; the acceleration and deceleration calculation frequency must match the interrupt frequency;
* Input          : Speed ramp structure pointer
* Output         : Speed after ramp processing
* Return         : None
******************************************************************************/
void Speed_Ramp_Process(Rampctr_Type *Speed_Ramp)
{
    _iq24 Speed_Ref_Output;
    Speed_Ref_Output = Speed_Ramp->Aftramp;

    //Both the new speed and the current speed are >= 0
    if((Speed_Ref_Output >=_IQ24(0.0)) && (Speed_Ramp->Preramp >=_IQ24(0.0)))
    {
        if(Speed_Ref_Output < Speed_Ramp->Preramp)//Acceleration processing
        {
            Speed_Ref_Output += Speed_Ramp->AccStep;
            if(Speed_Ref_Output > Speed_Ramp->Preramp)//Final value processing
            {
               Speed_Ref_Output = Speed_Ramp->Preramp;
            }
            else;
        }
        else if(Speed_Ref_Output > Speed_Ramp->Preramp)//Deceleration processing
        {
            Speed_Ref_Output -= Speed_Ramp->DecStep;
            if(Speed_Ref_Output < Speed_Ramp->Preramp)//Final value processing
            {
               Speed_Ref_Output = Speed_Ramp->Preramp;
            }
            else;
        }
        else;
    }

    //New speed < 0, current speed > 0
    else if((Speed_Ref_Output >=_IQ24(0.0)) && (Speed_Ramp->Preramp < _IQ24(0.0)))
    {
        Speed_Ref_Output -= Speed_Ramp->DecStep;
    }

    //Both the new speed and the current speed are < 0
    else if((Speed_Ref_Output < _IQ24(0.0)) && (Speed_Ramp->Preramp < _IQ24(0.0)))
     {
         if(Speed_Ref_Output < Speed_Ramp->Preramp)//Deceleration processing
         {
            Speed_Ref_Output += Speed_Ramp->DecStep;
            if(Speed_Ref_Output > Speed_Ramp->Preramp)
            {
                Speed_Ref_Output = Speed_Ramp->Preramp;//Final value processing
            }
            else;
         }
         else if(Speed_Ref_Output > Speed_Ramp->Preramp)//Acceleration processing
         {
            Speed_Ref_Output -= Speed_Ramp->AccStep;
            if(Speed_Ref_Output < Speed_Ramp->Preramp)//Final value processing
            {
                Speed_Ref_Output = Speed_Ramp->Preramp;
            }
            else;
         }
         else;
     }

    //New speed >= 0, current speed < 0
    else if((Speed_Ref_Output < _IQ24(0.0)) && (Speed_Ramp->Preramp >= _IQ24(0.0)))
    {
        Speed_Ref_Output += Speed_Ramp->DecStep;
    }

    else;
    Speed_Ramp->Aftramp = Speed_Ref_Output;
}
/******************************************************************************
* Function Name  : Power_Ramp_Process
* Description    : Power ramp processing, executed in the loop; the acceleration and deceleration calculation frequency must match the interrupt frequency;
* Input          : Ramp structure pointer
* Output         : Power after ramp processing
* Return         : None
******************************************************************************/
void Power_Ramp_Process(Rampctr_Type *Power_Ramp)
{
    _iq24 Power_Ref_Output;
    Power_Ref_Output = Power_Ramp->Aftramp;
        if(Power_Ref_Output < Power_Ramp->Preramp)//Ramp-up processing
        {
            Power_Ref_Output += Power_Ramp->AccStep;
            if(Power_Ref_Output > Power_Ramp->Preramp)//Final value processing
            {
                Power_Ref_Output = Power_Ramp->Preramp;
            }
            else;
        }
        else if(Power_Ref_Output > Power_Ramp->Preramp)//Ramp-down processing
        {
            Power_Ref_Output -= Power_Ramp->DecStep;
            if(Power_Ref_Output < Power_Ramp->Preramp)//Final value processing
            {
                Power_Ref_Output = Power_Ramp->Preramp;
            }
            else;
        }
        Power_Ramp->Aftramp = Power_Ref_Output;
}
/*********************************************************************
 * @fn      Poweroperation_Process
 *
 * @brief   Power-on control logic
 *
 *  @para   Speed_Ramp: Acceleration and deceleration control structure pointer
 *          ADCStruc:ADC sampling structure pointer
 *          PowerOper:power-on and power-off control structure pointer
 *          Runningstatus: running status
 *          Speedact actual speed;
 *
 * @return  none
 */
void Poweroperation_Process(Rampctr_Type *SpeedRamp,ADCStruc_Type *ADCStruc,Powercontrol_Type *PowerOper,\
                    Runningstatus_Type *Runningstatus,_iq24 Speedact)
{

    if((_IQabs(SpeedRamp->Aftramp) >= START_FINISH_SPEED)&&(_IQabs(Speedact) >= START_FINISH_SPEED))
        SpeedRamp->Startfinish = TRUE;

//    if( (PowerOper->DCVoltAver <= PowerOper->Poweroff_Volt)||(PD_DEVICE.ConnectStat==0))
    if( (ADC_M.DCVoltAver  <= POWEROFF_VOLT))
    {
        switch(*Runningstatus)
        {
            case INIT:
            case DIRCHECK:
            case PRESTART:
            case POSITION:
            case START:
            case RUN:
                if(*Runningstatus != FAULT)
                *Runningstatus = STOP;
                break;
        }

        PowerOper->Poweron = FALSE;
        BUZZ_M.VdcFirstPoweron =FALSE;
        BUZZ_M.VdcPoweron =FALSE;
    }


//    if((PowerOper->Poweron == FALSE)&&(PD_DEVICE.ConnectStat==1))
    if((PowerOper->Poweron == FALSE))
    {
        if( (ADC_M.DCVoltAver > POWERON_VOLT))
        {
            PowerOper->Poweron_Cnt++;
            if(PowerOper->Poweron_Cnt >= PowerOper->Poweron_CntThresh)
            {
                PowerOper->Poweron = TRUE;
                PowerOper->Poweron_Cnt = 0;


                if(BUZZ_M.VdcFirstPoweron==FALSE)
                {
                    BUZZ_M.VdcFirstPoweron =TRUE;
                    BUZZ_M.Effective = TRUE;
                    BUZZ_M.BuzzMark=1;
                }
            }
        }
    }
}
/******************************************************************************
* Function Name  : BUZZ_TIMER_Init
* Description    : Buzzer control timer initialization
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void BUZZ_TIMER_Init(void)
{
    TIM_TimeBaseInitTypeDef TimeBaseInitStructure={0};  //Timer initialization structure
    TIM_OCInitTypeDef TIM_OCInitStructure={0};
    GPIO_InitTypeDef GPIO_InitStructure={0};

    //Enable clock
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_TIM2, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA, ENABLE);

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //Capture timer configuration
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructInit(&TimeBaseInitStructure);
    TimeBaseInitStructure.TIM_Prescaler = BUZZ_TIMER_PRESCALER;
    TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TimeBaseInitStructure.TIM_Period = BUZZ_TIMER_PERIOD;
    TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2,&TimeBaseInitStructure);

    //Channel 2 configuration, used to generate the PWM wave
    TIM_OCStructInit(&TIM_OCInitStructure);
    //Valid when greater than the compare register value, otherwise invalid
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    //Enable the forward channel compare output
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    //Capture/compare register value is set to the timer period
    TIM_OCInitStructure.TIM_Pulse = BUZZ_TIMER_PERIOD/2;
    //Forward channel compare output is active high
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
   //Complementary channel idle state is low level
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);//Initialize capture/compare channel 2
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);//Enable preload of capture/compare channel 2
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Disable); //Disable the PWM output of channel 2

    //Clear the timer interrupt flags
    TIM_ClearFlag(TIM2, TIM_FLAG_Update | TIM_FLAG_CC1 | TIM_FLAG_CC2 | \
                  TIM_FLAG_CC3 | TIM_FLAG_CC4 | TIM_FLAG_Trigger | TIM_FLAG_CC1OF | \
                  TIM_FLAG_CC2OF | TIM_FLAG_CC3OF | TIM_FLAG_CC4OF);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);      //Clear the update event interrupt flag
    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
    TIM_SetCounter(TIM2,0);                    //Clear the timer counter
}

/******************************************************************************
* Function Name  : BUZZ_Parameter_define
* Description    : Buzzer control parameter definition
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void BUZZ_Parameter_define(Buzzctrl_Type *BUZZCtr)
{
    BUZZCtr->Effective = FALSE;
    BUZZCtr->Effectivecntset1 = (uint16_t)(BUZZ_PROCESS_FREQ*BUZZ_EFFECTIVE_TIME1);
    BUZZCtr->Effectivecntset2 = (uint16_t)(BUZZ_PROCESS_FREQ*BUZZ_EFFECTIVE_TIME2);
    BUZZCtr->Effectivecnt =0;
    BUZZCtr->BuzzMark=0;
    BUZZCtr->VdcFirstPoweron =FALSE;
    BUZZCtr->VdcPoweron =FALSE;
}

/******************************************************************************
* Function Name  : BUZZ_PROCESS
* Description    : Buzzer
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void BUZZ_PROCESS(Buzzctrl_Type *BUZZCtr)
{
    if(BUZZCtr->Effective ==TRUE)
   {
        TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Enable); //Enable the PWM output of channel 2

        if(BUZZCtr->BuzzMark==1)
        {
            BUZZCtr->Effectivecnt = BUZZCtr->Effectivecntset1;
        }
        else if(BUZZCtr->BuzzMark==2)
        {
            BUZZCtr->Effectivecnt = BUZZCtr->Effectivecntset2;
        }
        BUZZCtr->BuzzMark=0;
        BUZZCtr->Effective = FALSE;
   }

   if(BUZZCtr->Effectivecnt == 1)
   {
        TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Disable); //Disable the PWM output of channel 2
        if((BUZZCtr->VdcFirstPoweron==TRUE)&&(BUZZCtr->VdcPoweron==FALSE))
        {
            BUZZCtr->VdcPoweron=TRUE;
        }
   }
}

/*********************************************************************
 * @fn      Time_Delay_Process
 *
 * @brief   Variables decrement processing based on interrupt time (0.5ms)
 *
 * @return  none
 */
void Time_Delay_Process(void)
{
    if (Timewait != 0)
        Timewait --;

    if (Operation_M.StopwaitCnt != 0)
        Operation_M.StopwaitCnt --;

    if (Operation_M.FaultHoldingCnt != 0)
        Operation_M.FaultHoldingCnt--;

    if (BUZZ_M.Effectivecnt != 0)
        BUZZ_M.Effectivecnt --;

    if ((Phaseloss_M.DetectphaseCnt!=0)&&(RunningStatus_M==RUN))
    {
        Phaseloss_M.DetectphaseCnt--;
        if(Phaseloss_M.DetectphaseCnt==0)
        {
            Phaseloss_M.Detectphase=TRUE;
        }
    }
}

/*********************************************************************
 * @fn      Time_Wait
 *
 * @brief   The function wait for a delay to be over.
 *
 *  @para   time: Time Wait set value
 *
 * @return  none
 */
void Time_Wait(uint16_t time)
{
    Timewait = time;
    while (Timewait != 0)
    {;}
}
/*********************************************************************
 * @fn      Time_Delay_Set
 *
 * @brief   The function wait for a delay to be over.
 *
 *  @para   Delayset: Time delay set value
 *          Delaycounter:timing count variable pointer
 *
 * @return  none
 */
void Time_Delay_Set(uint16_t Delayset, uint16_t *Delaycounter)
{
    *Delaycounter = Delayset;
}
/*********************************************************************
 * @fn      Time_Delay_Elapse
 *
 * @brief   Time delay elapse check
 *
 *  @para   Delaycounter: Timing delay counter variable
 *
 * @return  Enumeration of true and false
 */
Truth_Verify_Type Time_Delay_Elapse(uint16_t Delaycounter)
{
     if (Delaycounter == 0)
       return (TRUE);
     else
       return (FALSE);
}
