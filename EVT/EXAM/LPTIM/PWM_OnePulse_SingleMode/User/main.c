/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/26
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 * PWM_OnePulse_SingleMode routine:
 * PB15 is output pin .
 * ETR trigger is PB14
 * TAMP trigger is PC13
 * NOTE:If you do not need to generate PWM waveforms by triggering, you need to set the LPTIM External Trigger Polarity parameter to off.
 *      for example,LPTIM_TimeBaseInitStruct.LPTIM_ExTriggerPolarity = LPTIM_ExTriggerPolarity_Disable.
 */

#include "debug.h"

/* Global define */
/* LPTIM work mode */
#define OnePulseMode             1
#define SingleMode               2
#define PWM                      3

/* LPTIM Trigger Source */
#define ETR                      1//PB14
#define RTC_ALARM                2
#define TAMP                     3//PC13

/* LPTIM work mode select */
#define MODE       OnePulseMode
//#define MODE       SingleMode
//#define MODE       PWM

/* LPTIM Trigger Source select */
#define Trigger    ETR
//#define Trigger    RTC_ALARM
//#define Trigger    TAMP

/* Global Variable */

#if(Trigger == RTC_ALARM)
/*********************************************************************
 * @fn      RTC_NVIC_Config
 *
 * @brief   RTC_NVIC_Config.
 *
 * @return  none
 */
static void RTC_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*********************************************************************
 * @fn      RTC_NVIC_Init
 *
 * @brief   RTC_NVIC_Init.
 *
 * @return  none
 */
u8 RTC_NVIC_Init( u32 SetCnt, u32 SetAlarm )
{

    u8 temp=0;
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR | RCC_PB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    BKP_DeInit();

    RCC_HSEConfig(RCC_HSE_ON);
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
  {
      temp++;
      Delay_Ms(10);
  }
  if(temp>=250)return 1;

    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForLastTask();
    RTC_WaitForSynchro();
    RTC_ITConfig( RTC_IT_ALR, ENABLE );
    RTC_WaitForLastTask();
    RTC_EnterConfigMode();
    RTC_SetAlarm(SetAlarm);
    RTC_WaitForLastTask();
    RTC_SetCounter(SetCnt);
    RTC_WaitForLastTask();
    RTC_ExitConfigMode();
    BKP_WriteBackupRegister(BKP_DR1, 0X5050);
    RTC_NVIC_Config();
    return 0;
}
#endif

/*********************************************************************
 * @fn      LPTIM_Init
 *
 * @brief   LPTIM_Init.
 *
 * @return  none
 */
void LPTIM_Init(u16 cmp,u16 arr)
{
    GPIO_InitTypeDef        GPIO_InitStructure = {0};
    LPTIM_TimeBaseInitTypeDef   LPTIM_TimeBaseInitStruct = {0};

#if(Trigger == TAMP)
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

    RCC_PB1PeriphClockCmd(RCC_PB1Periph_LPTIM, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    LPTIM_Cmd(ENABLE);

    LPTIM_TimeBaseInitStruct.LPTIM_ClockSource = LPTIM_ClockSource_In;
    LPTIM_TimeBaseInitStruct.LPTIM_ClockPolarity = LPTIM_ClockPolarity_Rising;
    LPTIM_TimeBaseInitStruct.LPTIM_ClockSampleTime = LPTIM_ClockSampleTime_0T;
    LPTIM_TimeBaseInitStruct.LPTIM_TriggerSampleTime = LPTIM_TriggerSampleTime_0T;
    LPTIM_TimeBaseInitStruct.LPTIM_ClockPrescaler = LPTIM_TClockPrescaler_DIV1;
#if(Trigger == ETR)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    LPTIM_TimeBaseInitStruct.LPTIM_TriggerSource = LPTIM_TriggerSource_ETR;
#elif(Trigger == RTC_ALARM)
    LPTIM_TimeBaseInitStruct.LPTIM_TriggerSource = LPTIM_TriggerSource_RTC_ALARM;
#elif(Trigger == TAMP)
    LPTIM_TimeBaseInitStruct.LPTIM_TriggerSource = LPTIM_TriggerSource_TAMP;
#endif
    LPTIM_TimeBaseInitStruct.LPTIM_ExTriggerPolarity = LPTIM_ExTriggerPolarity_Rising;
    LPTIM_TimeBaseInitStruct.LPTIM_TimeOut = DISABLE;
    LPTIM_TimeBaseInitStruct.LPYIM_OutputPolarity = LPYIM_OutputPolarity_High;
    LPTIM_TimeBaseInitStruct.LPYIM_UpdateMode = LPYIM_UpdateMode0;
    LPTIM_TimeBaseInitStruct.LPTIM_CountSource = LPTIM_CountSource_Internal;
    LPTIM_TimeBaseInitStruct.LPTIM_Encoder = DISABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_InClockSource = LPTIM_InClockSource_PCLK1;
    LPTIM_TimeBaseInitStruct.LPTIM_ForceOutHigh = DISABLE;
#if(MODE == OnePulseMode)
    LPTIM_TimeBaseInitStruct.LPTIM_OnePulseMode = ENABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_SingleMode = ENABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_PWMOut = ENABLE;
#elif(MODE == SingleMode)
    LPTIM_TimeBaseInitStruct.LPTIM_SingleMode = ENABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_PWMOut = ENABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_ContinuousMode = DISABLE;
#elif(MODE == PWM)
    LPTIM_TimeBaseInitStruct.LPTIM_PWMOut = ENABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_ContinuousMode = ENABLE;
#endif

    LPTIM_TimeBaseInitStruct.LPTIM_CounterDirIndicat = DISABLE;
    LPTIM_TimeBaseInitStruct.LPTIM_Pulse = cmp;
    LPTIM_TimeBaseInitStruct.LPTIM_Period = arr;

    LPTIM_TimeBaseInit( & LPTIM_TimeBaseInitStruct);
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
#if(Trigger == RTC_ALARM)
    RTC_NVIC_Init( 2222222, 2222228 );
#endif

    LPTIM_Init(50,100);

    while(1)
    {
    }
}

#if(Trigger == RTC_ALARM)
void RTC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void RTC_IRQHandler()
{
     if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)//Alarm INT
     {

      RTC_EnterConfigMode();                // ETR allow Configure
      RTC_SetCounter(2222222);              // Set RTC count
      RTC_WaitForLastTask();                // Wait task finish
      RTC_ExitConfigMode();                 // Exit Configure

      }
      RTC_ClearITPendingBit(RTC_IT_ALR);    //Clear INT
      RTC_WaitForLastTask();
}
#endif
