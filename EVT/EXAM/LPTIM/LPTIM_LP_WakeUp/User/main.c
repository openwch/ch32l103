
/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/11/05
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *LPTIM low Mode rountine:
 *PB12 is external clock input pin.
 *EXWAKEUP Mode:PB12 Provide clock for LPTIM in low power mode.
 *LPTIM PB12 need connnect external clock in EXWAKEUP Mode.
 *INWAKEUP: LSI provide clock for LPTIM in low power mode.
 *When LPTIM counts to the target value,enter interrupt wake-up chip.
 *
 */

#include "debug.h"


/* Global define */
#define Sleep             1
#define Stop              2

#define EXWAKEUP          1
#define INWAKEUP          2

#define MODE            Sleep
//#define MODE          Stop

#define WKMODE       EXWAKEUP
//#define WKMODE        INWAKEUP

/* Global Variable */


/*********************************************************************
 * @fn      LPTIM_Init
 *
 * @brief   LPTIM_Init.
 *
 * @return  none
 */
void LPTIM_Init(u16 arr)
{
    NVIC_InitTypeDef NVIC_InitStructure = {0};
	EXTI_InitTypeDef EXTI_InitStructure = {0};
    LPTIM_TimeBaseInitTypeDef   LPTIM_TimeBaseInitStruct = {0};
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB|RCC_PB2Periph_AFIO, ENABLE);
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_LPTIM, ENABLE);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line21;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = LPTIM_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    LPTIM_Cmd(ENABLE);

#if(WKMODE==INWAKEUP)
    RCC_LSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)!=SET);
#endif

#if(WKMODE==EXWAKEUP)
    LPTIM_TimeBaseInitStruct.LPTIM_ClockSource = LPTIM_ClockSource_Ex;
    LPTIM_TimeBaseInitStruct.LPTIM_CountSource = LPTIM_CountSource_External;
    LPTIM_TimeBaseInitStruct.LPTIM_ClockPrescaler = LPTIM_TClockPrescaler_DIV8;
    LPTIM_TimeBaseInitStruct.LPTIM_InClockSource = LPTIM_InClockSource_PCLK1;
#elif(WKMODE==INWAKEUP)
    LPTIM_TimeBaseInitStruct.LPTIM_ClockSource = LPTIM_ClockSource_In;
    LPTIM_TimeBaseInitStruct.LPTIM_CountSource = LPTIM_CountSource_Internal;
    LPTIM_TimeBaseInitStruct.LPTIM_ClockPrescaler = LPTIM_TClockPrescaler_DIV128;
    LPTIM_TimeBaseInitStruct.LPTIM_InClockSource = LPTIM_InClockSource_LSI;
#endif
    LPTIM_TimeBaseInitStruct.LPTIM_ClockPolarity = LPTIM_ClockPolarity_Falling;
    LPTIM_TimeBaseInitStruct.LPTIM_ClockSampleTime = LPTIM_ClockSampleTime_0T;
    LPTIM_TimeBaseInitStruct.LPTIM_TriggerSampleTime = LPTIM_TriggerSampleTime_0T;
    LPTIM_TimeBaseInitStruct.LPTIM_ExTriggerPolarity = LPTIM_ExTriggerPolarity_Disable;
    LPTIM_TimeBaseInitStruct.LPTIM_TimeOut = ENABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_OutputPolarity = LPTIM_OutputPolarity_High;
    LPTIM_TimeBaseInitStruct.LPTIM_UpdateMode = LPTIM_UpdateMode0;
    LPTIM_TimeBaseInitStruct.LPTIM_Encoder = DISABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_ForceOutHigh = DISABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_SingleMode = DISABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_ContinuousMode = ENABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_PWMOut = DISABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_CounterDirIndicat = DISABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_Pulse = 0;
    LPTIM_TimeBaseInitStruct.LPTIM_Period = arr;

    LPTIM_TimeBaseInit( & LPTIM_TimeBaseInitStruct);
    LPTIM_ITConfig(LPTIM_IT_ARRM, ENABLE);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);

#if(WKMODE==EXWAKEUP)
    LPTIM_Init(100);
#elif(WKMODE==INWAKEUP)
    LPTIM_Init(400);
#endif

#if(MODE==Sleep)
    __WFI();
#elif(MODE==Stop)
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
#endif
    printf("wake up\r\n");
    while(1)
    {
       #if(WKMODE==EXWAKEUP)
        printf("Welcome to WCH\r\n");
        Delay_Ms(1000);
       #endif
    }
}

void LPTIM_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void LPTIM_IRQHandler(void)
{
    if((LPTIM_GetFlagStatus(LPTIM_FLAG_ARRM)==SET))
    {
        LPTIM_ClearFlag(LPTIM_FLAG_ARRM);
#if(MODE==Stop)
        SystemInit();
#endif
     }

}

