/********************************** (C) COPYRIGHT *******************************
 * File Name          : M6_CurrentLoop.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2026/10/08
 * Description        : ADC sampling result reading, current loop processing, FOC calculation, overcurrent phaseloss protection processing

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
void ADC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
extern void FOC_Cal(Motor_Sructure_Type *MStruc,SVPWM_2R_Type *SVPWM_Cal,Motor_PID_Type *Mpid,\
        ADCStruc_Type *ADCStruc,Motor_LBG_Type *MLBG,int8_t Direction);
//extern void Waveform_Display (void);
/* Private variables --------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
/*********************************************************************
 * @fn      ADC_IRQHandler
 *
 * @brief   ADC interrupts the response function to read the current conversion result
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void ADC_IRQHandler(void)
{
        int16_t ADsample_int1=0;
        int16_t ADsample_int2=0;

        ADsample_int1 = (int32_t)(ADC_M.OffsetValue1) - (int32_t)((ADC1->IDATAR1)<<1);
        ADsample_int2 = (int32_t)(ADC_M.OffsetValue2) - (int32_t)((ADC1->IDATAR2)<<1);

        //amplitude limiting
        ADsample_int1=( (ADsample_int1) < (S16_MIN) ? (S16_MIN) : \
                                      ( (ADsample_int1) > (S16_MAX) ? (S16_MAX) : (ADsample_int1)));

        ADsample_int2=( (ADsample_int2) < (S16_MIN) ? (S16_MIN) : \
                                      ( (ADsample_int2) > (S16_MAX) ? (S16_MAX) : (ADsample_int2)));


        MStruc_M.C3S.PhaseC  = (_iq24)((_IQ16MPY_MACRO(_IQ16(ADsample_int1),\
                                        ADC_M.RatedCurr_ConvInv))<<1);

        MStruc_M.C3S.PhaseB = (_iq24)((_IQ16MPY_MACRO(_IQ16(ADsample_int2),\
                                        ADC_M.RatedCurr_ConvInv))<<1);

        MStruc_M.C3S.PhaseA= -(MStruc_M.C3S.PhaseB + MStruc_M.C3S.PhaseC);
        switch(RunningStatus_M)
        {
            case DIRCHECK:
                if(Flystart_M.Process_mark==FALSE)
                {
                    MLBG_M.Cal_Struc.Inimark=TRUE;

                    Flystart_Process(&Flystart_M,Ramp_M.Direction,&MStruc_M,&SVPWM_Cal_M,&MLBG_M,&ADC_M,&Mpid_M);
                }

                if((Flystart_M.Process_mark==TRUE)&&(Flystart_M.Switch_mark==FALSE))
                 {
                     Flystart_Switch(&Flystart_M,&Ramp_M);
                     BUZZ_M.Effective = TRUE;
                     BUZZ_M.BuzzMark = 1;
                 }
                break;
            case RUN:
                FOC_Cal(&MStruc_M,&SVPWM_Cal_M,&Mpid_M,&ADC_M,&MLBG_M,Ramp_M.Direction);
                break;
            default:
              break;
                }

    if ( (((RunningStatus_M == RUN)&&(Ramp_M.Decemark==FALSE)) )\
            &&(Phaseloss_M.Detectphase==TRUE) )
    {
        if((Phaseloss_Verify2R3R(&Phaseloss_M,&MStruc_M)))
            Protection_SetFault(PHASELOSS_FALUT);
    }

    if ((RunningStatus_M == DIRCHECK)||(RunningStatus_M == RUN))
    {
        if(Phase_Ins_OverCurrent_Verify(&(MStruc_M.C3S),&InstPhaseOC_M) == TRUE)
            Protection_SetFault(MOTOR_OVER_CURR_SOFT_M);
    }

    TIM1->CH1CVR = SVPWM_Cal_M.cc1Load;
    TIM1->CH2CVR = SVPWM_Cal_M.cc2Load;
    TIM1->CH3CVR = SVPWM_Cal_M.cc3Load;

    OPA->CFGR1 &= ~(uint16_t)(1<<0);
    ADC1->STATR = ~(uint32_t)ADC_FLAG_JEOC;
}

