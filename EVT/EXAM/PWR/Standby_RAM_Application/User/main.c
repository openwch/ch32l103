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
 *@Note
 *when LV is not enabled in standby mode, RAM 2k and 18K low-power data holding routines:
 *This routine demonstrates writing data at the specified location of 2K RAM and 18K RAM,
 *and then WFI enters STANDBY sleep mode, through the PA0 (wakeup) pin Input high level
 *to exit standby mode, print RAM data after waking up, and test whether RAM is maintained
 *data.
 *
 */

#include "debug.h"

/* Global define */
#define _KEEP_RAM   __attribute__((section(".keep_ram"))) //space for keep RAM.

/* Global Variable */




_KEEP_RAM   u32 DataBuf[256];
/*********************************************************************
 * @fn      TestDataWrite
 *
 * @brief   Write 0x22222222 to certain address of RAM.
 *
 * @return  none
 */
void TestDataWrite(void)
{
    uint32_t i=0;
    /* Write data in RAM */
    for( i=0; i<256; i++ )
    {
       DataBuf[i] = 0x22222222;
    }
    printf("Write data in RAM Successfully!\r\n");

}

/*********************************************************************
 * @fn      TestDataRead
 *
 * @brief   Print the data of certain address of RAM.
 *
 * @return  none
 */
void TestDataRead(void)
{
    uint32_t i=0;
    printf("Read data in RAM:\r\n");
    for( i=0; i<256; i++ )
    {
        printf("DataBuf#%d = 0x%08lx\r\n",i,DataBuf[i]);
    }
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
    GPIO_InitTypeDef GPIO_InitStructure={0};

    /* Configure unused GPIO as IPD to reduce power consumption */
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA|RCC_PB2Periph_GPIOB|
            RCC_PB2Periph_GPIOC|RCC_PB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );

    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);

    /* Delay 1.5s to avoid entering low power mode immediately after reset*/
    Delay_Ms(1500);
    while(1)
    {
        TestDataRead();
        printf("3.Stop RAM LV Mode Test\r\n");
        TestDataWrite();
        PWR_WakeUpPinCmd(ENABLE);
        PWR_EnterSTANDBYMode_RAM();
        printf("\r\n3.Out \r\n");
        printf("\r\n ########## \r\n");

    }
}
