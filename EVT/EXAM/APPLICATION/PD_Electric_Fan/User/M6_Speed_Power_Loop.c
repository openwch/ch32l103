/* Includes -----------------------------------------------------------------*/
#include "M0_Control_Library.h"

/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private functions --------------------------------------------------------*/
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/* Private variables --------------------------------------------------------*/
static uint8_t PD_CNT = PD_PERIOD;  //PD processing counter
/* Variables ----------------------------------------------------------------*/
/******************************************************************************
* Function Name  : SysTick_Handler
* Description    : Speed loop processing interrupt.
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void SysTick_Handler(void)
{
    _iq CRef2R_cq=0;
    PD_CNT--;
    //Variable decrement processing based on the interrupt time (0.5ms)
    Time_Delay_Process();

    if(PD_CNT == 0)
    {
        //PD processing
        PD_Process();
        PD_CNT = PD_PERIOD;  //PD processing counter
    }
    //Power-on/off control processing logic
    Poweroperation_Process(&Ramp_M,&ADC_M,&Powerctr_M,&RunningStatus_M,MLBG_M.Cal_Struc.Angspeed_Aver1);

    //Buzzer control processing
    BUZZ_PROCESS(&BUZZ_M);

    //Keys can be pressed only after power-on is valid
    if(Powerctr_M.Poweron==TRUE)
    {
        //Key detection
        KeyCheck_Process(&Key_M,&Ramp_M);
    }
    switch(RunningStatus_M)
    {
    case RUN:
               //LBG observed speed array update
               Buffer_Update(&(LBG_SpeedBuffer_M[0]), &(MLBG_M.SpeedBuffPara), MLBG_M.Cal_Struc.Angspeed_Inst);

               //LBG observed average speed calculation
               MLBG_M.Cal_Struc.Angspeed_Aver1 = LPF_1st_Process(&(MLBG_M.Cal_Struc.SpeedLPF1st_M),\
                       MLBG_M.Cal_Struc.Angspeed_Inst);//Low-pass filtering of the observed speed

               LBG_Status_M.Reliable =  Speed_Reliable_Verify(&(MLBG_M.SpeedBuffPara),&LBG_SpeedBuffer_M[0],\
                       MLBG_M.Cal_Struc.Angspeed_Aver1,LBG_Status_M.VarianceThresh);

   //            //Speed stability determination
               Speed_Stable_Verify(&LBG_Status_M, MLBG_M.Cal_Struc.Angspeed_Aver1);
               if(LBG_Status_M.Stable==FALSE)
                   Protection_SetFault(SPEED_ABNORMAL_M);
   //
   //             //Stall determination
               if((Ramp_M.Decemark==FALSE))
               {
                   Motor_Stall_Verify(Ramp_M.Aftramp, MLBG_M.Cal_Struc.EalphaNew , MLBG_M.Cal_Struc.EbetaNew, MStruc_M.CRef2R.cq,&LBG_Status_M,MLBG_M.Cal_Struc.Angspeed_Aver1);
                   if(LBG_Status_M.Stall==TRUE)
                    Protection_SetFault(MOTOR_STALL_M);

                   Speed_Outrange_Verify(_IQabs(MLBG_M.Cal_Struc.Angspeed_Aver2),&LBG_Status_M);
                   //Underspeed protection triggers stall protection
                       if(LBG_Status_M.Underspeed==TRUE)
                           Protection_SetFault(MOTOR_STALL_M);
               }


               //Speed loop regulation
   #if     (CLOSE_LOOP==SPEED_LOOP)
                   Speed_Ramp_Process(&Ramp_M);
                   CRef2R_cq = PI_Calculaion_Delta (Ramp_M.Aftramp, MLBG_M.Cal_Struc.Angspeed_Aver2, &(Mpid_M.Speed));
   #elif   (CLOSE_LOOP==POWER_LOOP)
                   Power_Cal.AngPower_Inst= _IQabs(_IQMPY_MACRO(MStruc_M.VRef2RC.cd,MStruc_M.C2R.cd))+_IQabs(_IQMPY_MACRO(MStruc_M.VRef2RC.cq,MStruc_M.C2R.cq));
                   Power_Cal.AngPower_Aver1 = LPF_1st_Process(&(Power_Cal.PowerLPF1st),Power_Cal.AngPower_Inst);//Low-pass filtering of the power
                   Power_Ramp_Process(&Ramp_M);
                   CRef2R_cq = PI_Calculaion_Delta (Ramp_M.Aftramp, Power_Cal.AngPower_Aver1, &(Power_Cal.Power));
                   CRef2R_cq=CRef2R_cq*Ramp_M.Direction;
   #endif

   //            //The reference speed matches the underspeed; staying too long triggers the underspeed protection
               if((_IQabs(MLBG_M.Cal_Struc.Angspeed_Aver2)<LBG_UNDERSPEED_THRESH_M)&&(Ramp_M.Decemark==FALSE))
               {
                   //IQ is fixed here; for the customer's motor with increased current, the output angle cannot hold
                   MStruc_M.CRef2R.cq=_IQ(0.35)*Ramp_M.Direction;

   #if     (CLOSE_LOOP==SPEED_LOOP)
                   Mpid_M.Speed.Output_New=_IQ(0.35)*Ramp_M.Direction;
                   Mpid_M.Speed.Output_Prev=_IQ(0.35)*Ramp_M.Direction;
   #elif   (CLOSE_LOOP==POWER_LOOP)
                   Power_Cal.Power.Output_New=_IQ(0.25);
                   Power_Cal.Power.Output_Prev=_IQ(0.25);
   #endif
               }
               else
                   MStruc_M.CRef2R.cq=CRef2R_cq;

               //D-axis current switching processing
               if((DCurrProcess_M.Stage == 1)&&(_IQabs(MStruc_M.CRef2R.cd) <= DCurrProcess_M.Target1)&&(_IQabs(MLBG_M.Cal_Struc.Angspeed_Aver1)>=(_IQ(0.1))))
                   Dcurr_Process_Init(&DCurrProcess_M,2);

               MStruc_M.CRef2R.cd = Dcurr_Process(&DCurrProcess_M, MLBG_M.Cal_Struc.Angspeed_Aver1, MStruc_M.CRef2R.cd);

               break;
           default:
                   break;
       }

       //Bus voltage overvoltage
       DCOvervolt_Protection(ADC_M.DCVoltAver, &DCBusProtecion_M);

       //Bus voltage undervoltage
//       DCUndervolt_Protection(ADC_M.DCVoltAver, &DCBusProtecion_M);

       SysTick->SR &= ~(1 << 0);
   }

/************************************************************************************************************
* Function Name  : Systick_Init
* Description    : SYS timer initialization
* Input          : NewState - ENABLE or DISABLE.
* Output         : None
* Return         : None
*************************************************************************************************************/
void Systick_Init(void)
{
    SysTick->CTLR  =0;
    SysTick->CMP   =0;
    SysTick->SR   &= ~(1 << 0);
    SysTick->CNT   = 0;
    SysTick->CMP   = TIMER_PERIDO_SYSTICK;  //Compare value
    SysTick->CTLR |=(1 << 1)|(1 << 3)|(1 << 5);//Enable interrupt, auto-reload enable, up counting, update initial value
}
/************************************************************************************************************
* Function Name  : SYS_TIM_Cmd
* Description    : SYS timer enable
* Input          : NewState - ENABLE or DISABLE.
* Output         : None
* Return         : None
*************************************************************************************************************/
void SYS_TIM_Cmd(FunctionalState NewState)
{
    if(NewState != DISABLE)
    {
        SysTick->CTLR |= TIM_CEN;
    }
    else
    {
        SysTick->CTLR &= (uint16_t)(~((uint16_t)TIM_CEN));
    }
}
