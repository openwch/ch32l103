/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/05/23
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 * CMP1_P0--PA2
 * CMP1_N0--PB1
 * CMP1_OUT--TIM2_CH1
 * In this example, PA2 and PB1 are postive and negative input, when output is high
 * sleep wake will be performed.
 *
 */

#include "debug.h"

/*********************************************************************
 * @fn      CMP_Init
 *
 * @brief   Initializes CMP.
 *
 * @return  none
 */
void CMP_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    CMP_InitTypeDef  CMP_InitTypeDef = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOA|RCC_PB2Periph_GPIOB, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOA, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    EXTI_InitStructure.EXTI_Line = EXTI_Line22;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = CMPWakeUp_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    CMP_InitTypeDef.CMP_NUM = CMP1;
    CMP_InitTypeDef.Mode = OUT_IO_TIM2;
    CMP_InitTypeDef.NSEL = CMP_CHN0;
    CMP_InitTypeDef.PSEL = CMP_CHP_0;
    OPA_CMP_Init(&CMP_InitTypeDef);
    OPA_CMP_Cmd(CMP1,ENABLE);
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
	SystemCoreClockUpdate();
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );

	OPCM_Unlock();
    CMP_Init();
	OPA_CMP_WakeUp_ModeConfig(CMP_WakeUp_Rising);

    printf("\r\n ********** \r\n");
    __WFI();
    printf("\r\n ########## \r\n");
	while(1);
}

void CMPWakeUp_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      CMPWakeUp_IRQHandler
 *
 * @brief   This function handles CMP exception.
 *
 * @return  none
 */
void CMPWakeUp_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line22)!=RESET)
    {
        printf("Run at EXTI\r\n");
        EXTI_ClearITPendingBit(EXTI_Line22);     /* Clear Flag */
    }
}
