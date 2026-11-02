/********************************** (C) COPYRIGHT *******************************
 * File Name          : M5_MotorStart.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/11/04
 * Description        : Initialization of the motor before starting
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
/*********************************************************************
 * @fn      Motor_Start_Init_M
 *
 * @brief   Initialization of the motor before starting
 *
 * @return  none
 */
void Motor_Start_Init_M(void)
{
    rampctr_Initial(&Ramp_M);//ramp initialization, placed first, contains the direction definition

    KeyandLED_Status_Init(&Key_M);//key and LED status initialization

    Operation_Status_Init(&Operation_M);//operation status initialization

    Protection_and_Emergency_Init();//protection parameters and status initialization

    PID_ParaBase_Cal_M(&MBase_M,&Mpid_M);//recalculate PI parameters
    PID_Output_Init(&Mpid_M,Ramp_M.Direction);//PID regulator output initialization

#if (CLOSE_LOOP==POWER_LOOP)
    PowerCal_Init(&Power_Cal);//power loop initialization
#endif

    Motor_Status_Initial_M(&MStruc_M);//motor status initialization

    Dcurr_Process_Init(&DCurrProcess_M,1);//D-axis current process initialization

    SVPWM_2R_Status_Init_M(&SVPWM_Cal_M);//SVPWM calculation status initialization

    LBG_Status_Init(&MLBG_M,&LBG_Status_M,Ramp_M.Direction);//LBG observer initialization

    Buffer_Clear(&(LBG_SpeedBuffer_M[0]),&(MLBG_M.SpeedBuffPara));//LBG observed speed array initialization

    ADC_Offset_Reading(ADC1,&ADC_M,1);//ADC1 sampling offset reading

    Flystart_Status_Init(&Flystart_M,Ramp_M.Direction); //back-EMF detection

    SVPWM_Zerovolt_Output(&SVPWM_Cal_M);

    TIM_CtrlPWMOutputs(TIM1,ENABLE);//enable PWM output
}
