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
 *Comparator routines:
 * CMP1_P0--PA2
 * CMP1_N0--PB1
 * CMP1_OUT--PB0
 * In this example, PA2 and PB1 are postive and negative input, PB0 is the output of the comparator.
 * When the voltage of PA2 is greater than the voltage of PB1, the output of PB0 is high level,
 * otherwise the output of PB0 is low level.
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

    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOA|RCC_PB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOC, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOA, &GPIO_InitStructure );

    CMP_InitTypeDef.CMP_NUM = CMP1;
    CMP_InitTypeDef.Mode = OUT_IO0;
    CMP_InitTypeDef.NSEL = CMP_CHN0;
    CMP_InitTypeDef.PSEL = CMP_CHP_0;

    OPA_CMP_Init(&CMP_InitTypeDef);
    OPA_CMP_Cmd(CMP1,ENABLE);
}

/*********************************************************************
 * @fn      CMP_OUT
 *
 * @brief   CMP out program.
 *
 * @return  none
 */
void CMP_OUT( void )
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOB, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOB, &GPIO_InitStructure );
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
    USART_Printf_Init( 115200 );

    printf( "SystemClk:%d\r\n", SystemCoreClock );
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    OPCM_Unlock();
    CMP_Init();
    CMP_OUT();
	while(1);
}



