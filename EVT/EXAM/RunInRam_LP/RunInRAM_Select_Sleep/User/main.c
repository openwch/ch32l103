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
 *RunInRAM_Select_Sleep:
 * This routine demonstrates that some programs run in RAM, which is faster than running in FLASH.
 * Sleep mode and stop regulator on run in RAM.
 * EXTI_Line0(PA0)
 * If you want the function to run in RAM you need to add prefix __attribute__((section(".highcode")))
 * Note-
 *  1.printf functions and C function library is not applicable to run in RAM.
 *  2.The wake-up interrupt service function requires run in RAM.
 */

#include "debug.h"

/* Global define */
#define Enter_LP_WFI_Mode    0x00
#define Enter_LP_WFE_Mode    0x01

#define LP_Sleep_Mode    0x00
#define LP_Stop_Mode     0x01

#define Enter_LP_Mode  Enter_LP_WFI_Mode
//#define Enter_LP_Mode  Enter_LP_WFE_Mode

#ifndef Enter_LP_Mode
    #error  Missing definition: "Enter_LP_Mode undefined"
#endif

//#define LP_Mode  LP_Sleep_Mode
#define LP_Mode  LP_Stop_Mode

#ifndef LP_Mode
    #error Missing definition: "LP_Mode undefined"
#endif

/* Global Variable */
uint32_t ulldo_val;

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
 * @fn      EXTI0_INT_INIT
 *
 * @brief  Initializes EXTI0 collection.
 *
 * @return  none
 */
void EXTI0_INT_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_AFIO | RCC_PB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* GPIOA.0 ----> EXTI_Line0 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;

#if (Enter_LP_Mode == Enter_LP_WFI_Mode)
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
#else
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
#endif
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

#if (Enter_LP_Mode == Enter_LP_WFI_Mode)
    NVIC_SetPriority(EXTI0_IRQn, 0x80);
    NVIC_EnableIRQ(EXTI0_IRQn);

#endif
    FLASH_Unlock();
    FLASH->CTLR |= (1<<13);
    FLASH_Lock();

    NVIC_SetPriority(FLASH_IRQn, 0x00);
    NVIC_EnableIRQ(FLASH_IRQn);
}

/*********************************************************************
 * @fn      LDO_Deal
 *
 * @brief   Deal LDO ULLDO mode(reduced power 80uA).
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void LDO_Deal(void)
{
    //reduced power 80uA
    /* LDOTRIM[1:0] == 01b */
    uint32_t tmp = 0;
    tmp = EXTEN->EXTEN_CTR;
    tmp &= ~EXTEN_LDO_TRIM;
    tmp |= EXTEN_LDO_TRIM0;
    EXTEN->EXTEN_CTR = tmp;

#if (LP_Mode == LP_Stop_Mode)
    RCC->PB1PCENR |= RCC_PB1Periph_PWR;
    PWR->CTLR |= (1 << 12);
#endif
}

/*********************************************************************
 * @fn      LDO_Restore
 *
 * @brief   Restore LDO Value.
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void LDO_Restore(void)
{
    /* Restore reset value-
       LDOTRIM[1:0] == 10b */
    uint32_t tmp = 0;
    tmp = EXTEN->EXTEN_CTR;
    tmp &= ~EXTEN_LDO_TRIM;
    tmp |= EXTEN_LDO_TRIM1;
    EXTEN->EXTEN_CTR = tmp;
}
/*********************************************************************
 * @fn      Exit_LowPower
 *
 * @brief   Exit low power.
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void Exit_LowPower(void)
{
    LDO_Restore();

    FLASH_LP_Exit();
}

/*********************************************************************
 * @fn      Enter_LowPower
 *
 * @brief   Enter low power.
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void Enter_LowPower(void)
{
    LDO_Deal();
    FLASH_LP_Enter();

#if (LP_Mode == LP_Stop_Mode)
#if (Enter_LP_Mode == Enter_LP_WFI_Mode)
    {
        uint32_t tmpreg = 0;
        tmpreg = PWR->CTLR;
        tmpreg &= 0xFFFFFFFC;
        PWR->CTLR = tmpreg;

        NVIC->SCTLR |= (1 << 2);
        NVIC->SCTLR &= ~(1<<3);   // wfi
        asm volatile ("wfi");
        NVIC->SCTLR &= ~(1 << 2);
    }

#else
    {
        uint32_t tmpreg = 0, t=0;
        tmpreg = PWR->CTLR;
        tmpreg &= 0xFFFFFFFC;
        PWR->CTLR = tmpreg;

        NVIC->SCTLR |= (1 << 2);
        t = NVIC->SCTLR;
        NVIC->SCTLR |= (1<<3)|(1<<5);
        NVIC->SCTLR = (NVIC->SCTLR & ~(1<<5)) | ( t & (1<<5));

        NVIC->SCTLR |= (1<<3);
        asm volatile ("wfi");

        NVIC->SCTLR |= (1<<3);
        asm volatile ("wfi");
        NVIC->SCTLR &= ~(1 << 2);
    }

#endif

#elif (LP_Mode == LP_Sleep_Mode)
#if (Enter_LP_Mode == Enter_LP_WFI_Mode)
    NVIC->SCTLR &= ~(1<<3);   // wfi
    asm volatile ("wfi");
#else
    {
        uint32_t t;

        t = NVIC->SCTLR;
        NVIC->SCTLR |= (1<<3)|(1<<5);
        NVIC->SCTLR = (NVIC->SCTLR & ~(1<<5)) | ( t & (1<<5));

        NVIC->SCTLR |= (1<<3);
        asm volatile ("wfi");

        NVIC->SCTLR |= (1<<3);
        asm volatile ("wfi");
    }
#endif
#endif

#if (Enter_LP_Mode == Enter_LP_WFE_Mode)
    Exit_LowPower();
#endif
}

/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOB pin6,pin7.
 *
 * @return  none
 */
void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
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

#if (LP_Mode == LP_Sleep_Mode)
#if (Enter_LP_Mode == Enter_LP_WFI_Mode)
    printf("Sleep Mode Test-WFI\r\n");
#else
    printf("Sleep Mode Test-WFE\r\n");
#endif
#elif (LP_Mode == LP_Stop_Mode)
#if (Enter_LP_Mode == Enter_LP_WFI_Mode)
    printf("Stop Mode Test-WFI\r\n");
#else
    printf("Stop Mode Test-WFE\r\n");
#endif

#endif

    EXTI0_INT_INIT();

    /*set cc1,cc2 pin mode to pull-up */
    GPIO_Toggle_INIT();

    /*close cc1,cc2 pull-down resistors */
    USBPD->PORT_CC1 &= ~(0x01<<1);
    USBPD->PORT_CC2 &= ~(0x01<<1);

    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);
    printf("\r\n ********** \r\n");

    /* Run in RAM */
    Enter_LowPower();

    printf("\r\n ########## \r\n");

    while(1)
    {
        Delay_Ms(1000);
        printf("Run in main\r\n");
    }
}


#if (Enter_LP_Mode == Enter_LP_WFI_Mode)
void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/*********************************************************************
 * @fn      EXTI0_IRQHandler
 *
 * @brief   This function handles EXTI0 exception.
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void EXTI0_IRQHandler(void)
{
    Exit_LowPower();

#if (LP_Mode == LP_Stop_Mode)
    SystemInit();
#endif

    if((EXTI->INTFR & EXTI_Line0))
    {
        EXTI->INTFR = EXTI_Line0; /* Clear Flag */
    }
}

#endif

void FLASH_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/*********************************************************************
 * @fn      FLASH_IRQHandler
 *
 * @brief   This function handles EXTI0 exception.
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void FLASH_IRQHandler(void)
{
    Exit_LowPower();

#if (LP_Mode == LP_Stop_Mode)
    SystemInit();
#endif

    FLASH->STATR &= ~(1<<6);
}

