/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/04/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *OPA reset routune:
 *OPA1_CHP0--PB15
 *OPA1_CHN0--PB11
 *OPA1_OUT--PA3
 *
 *In this example, PA3 and PB11 are short-circuited,PB15 (OPA input terminal is high level), system reset.
 */

#include "debug.h"

/*********************************************************************
 * @fn      OPA1_Init
 *
 * @brief   Initializes OPA2.
 *
 * @return  none
 */
void OPA1_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    OPA_InitTypeDef  OPA_InitStructure = {0};

    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOA|RCC_PB2Periph_GPIOB, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOB, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOA, &GPIO_InitStructure );

    OPA_InitStructure.OPA_NUM = OPA1;
    OPA_InitStructure.NSEL = CHN0;
    OPA_InitStructure.PSEL = CHP0;
    OPA_InitStructure.RST_EN = RST_OPA1_ON;
    OPA_InitStructure.PSEL_POLL = CHP_OPA1_ON;;
    OPA_InitStructure.OPA_POLL_Interval = 0x50;
    OPA_InitStructure.Mode = OUT_IO_OUT0;
    OPA_Init( &OPA_InitStructure );
    OPA_Cmd(OPA1, ENABLE );
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
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );

    OPCM_Unlock();
    OPA1_Init();

    while(1);
}
