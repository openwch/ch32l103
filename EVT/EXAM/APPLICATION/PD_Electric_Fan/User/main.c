/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2022/09/01
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/*
 * @Note
 * The purpose of this description and routine is to provide customers with the application framework of electric fan cartridge
 * permanent magnet synchronous motor based on WCH MCU,including MCU peripherals, FOC control, observer, loop control, protection,
 * and other basic solutions and modules, to assist customers in shortening the product development cycle.
*/
/* Includes -----------------------------------------------------------------*/
#include "M0_Control_Library.h"
#include "ch32l103_conf.h"
#include "IQmath_RV32.h"
#include "debug.h"
//extern void Waveform_Display();
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private functions --------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/

/*******************************************************************************
* Function Name  : main
* Description    : main
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
    Delay_Init();

    Delay_Ms(100);

    PD_Init();

    KeyandLED_IOInit();
    KeyandLED_Parameter_Define(&Key_M);

    SPI_FullDuplex_Init();

    Global_Status_Initial();

    Operation_Parameter_Define(&Operation_M);

    Motor_Prameter_Define(&MBase_M);

    Motor_FilerPara_Cal_M(&MStruc_M);

    Motor_BaseCal_M(&MBase_M);

    PID_Prameter_Define(&Mpid_M);

    DCurrProcess_Parameter_Define(&DCurrProcess_M);

    PID_ParaBase_Cal_M(&MBase_M,&Mpid_M);

    PowerCal_Prameter_Define(&Power_Cal);

    Protection_and_Emergency_Define();

    rampctr_Parameter_Define(&Ramp_M);

    SVPWM_2R_Paramter_Init(&SVPWM_Cal_M ,&MBase_M);

    SVPWM_2R_Timer_Configure();

    ADC_Parameter_Init(&ADC_M,&MBase_M);

    ADC_OPA_Configure(&ADC_M);

    LBG_Parameter_Define(&MLBG_M,&LBG_Status_M);

    LBG_Paramter_Cal (&MBase_M,&MLBG_M);

    Flystart_Prameter_Define(&Flystart_M);

    Flystart_Paramter_Cal(&Flystart_M,&MBase_M);

    Poweroperation_Initial(&Powerctr_M);

    Systick_Init();

//    BUZZ_TIMER_Init();
//    BUZZ_Parameter_define(&BUZZ_M);

    Set_DevChk( DevRole_Sink );

    Interrupt_Configuration();

    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    SYS_TIM_Cmd(ENABLE);

    while(1)
    {
        Motor_Operation_Process();
        SPI_Deal_sendMonitorData();
     }
}
