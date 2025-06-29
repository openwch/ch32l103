/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/02/27
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *RunInRAM_Select:
 * This routine demonstrates that some programs run in RAM, which is faster than running in FLASH.
 * If you want the function run in RAM,you need to add prefix __attribute__((section(".highcode")))
 */

#include "debug.h"

/* Global define */

/* Global Variable */

const u8 tp[10] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};

/*********************************************************************
 * @fn      FLASH_LP_Enter
 *
 * @brief   Enter FLASH Low power mode 0
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void FLASH_LP_Enter(void)
{
    /* FLASH low power mode reduced power 1000uA */
    RCC->PB1PCENR |= RCC_PB1Periph_PWR;
    PWR->CTLR |= (7 << 9);
}

/*********************************************************************
 * @fn      FLASH_LP_Exit
 *
 * @brief   Exit FLASH Low power mode 0
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void FLASH_LP_Exit(void)
{
    //restore FLASH, and restore clock
    RCC->PB1PCENR |= RCC_PB1Periph_PWR;
    PWR->CTLR &= ~(1 << 9);
}

/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOA.0
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void GPIO_Toggle_INIT(void)
{
    FLASH_LP_Enter();

    //PA0
    RCC->PB2PCENR |= RCC_PB2Periph_GPIOA;
    GPIOA->CFGLR &= ~0x0000000F;
    GPIOA->CFGLR |= 3;

    FLASH_LP_Exit();
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
    vu8 i = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("TRIM-0-%08x\r\n", EXTEN->EXTEN_CTR);

    GPIO_Toggle_INIT();

    for(i=0; i<10; i++){
        printf("-%02x\r\n", tp[i]);
    }

    while(1)
    {
        Delay_Ms(1000);
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, (i == 0) ? (i = Bit_SET) : (i = Bit_RESET));
    }
}
