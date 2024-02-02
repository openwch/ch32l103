/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/07/08
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *OPA interrupt routine:
 *OPA1_CHP0--PB15
 *OPA1_CHN1--PB11
 *OPA1_OUT--PA3
 *
 *In this example, PA5 and PA4 are short-circuited, and the external voltage is input from PA7,
 *when PA7 high voltage enter the OPA interrupt when the OPA in query function
 */

#include "debug.h"

/*********************************************************************
 * @fn      OPA1_Init
 *
 * @brief   Initializes OPA1.
 *
 * @return  none
 */
void OPA1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    OPA_InitTypeDef  OPA_InitStructure={0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = OPA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    OPA_InitStructure.OPA_NUM = OPA1;
    OPA_InitStructure.PSEL = CHP0;
    OPA_InitStructure.NSEL = CHN0;
    OPA_InitStructure.Mode = OUT_IO_OUT0;
    OPA_InitStructure.OPA_POLL_Interval = 0x50;
    OPA_InitStructure.PSEL_POLL = CHP_OPA1_ON;
    OPA_InitStructure.PSEL_POLL = CHP_OPA1_ON;
    OPA_InitStructure.OUT_IE = OUT_IE_OPA1_ON;

    OPA_Init(&OPA_InitStructure);
    OPA_Cmd(OPA1,ENABLE);
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

    while(1)
    {

    }
}

void OPA_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/*********************************************************************
 * @fn      OPA_IRQHandler
 *
 * @brief   This function handles OPA exception.
 *
 * @return  none
 */
void OPA_IRQHandler(void)
{
    if(OPA_GetFlagStatus(OPA_FLAG_OUT_OPA1)!=RESET)
    {
        printf("Run at EXTI\r\n");
        OPA_ClearFlag(OPA_FLAG_OUT_OPA1);     /* Clear Flag */
    }
}
