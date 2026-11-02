/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.2
 * Date               : 2026/10/16
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
#define RAM_BASE            0x20000000U
#define RAM2K_BASE          (RAM_BASE)          
#define RAM2K_SIZE          (2U  * 1024U)
#define RAM18K_BASE         (RAM_BASE + 2U * 1024U)    
#define RAM18K_SIZE         (18U * 1024U)

#define RAM18K_TEST_ADDR    (RAM_BASE + 10U * 1024U)      
#define RAM18K_TEST_WORDS   1024U                          
#define RAM2K_TEST_ADDR     (RAM_BASE + 1U * 1024U + 512U)  
#define RAM2K_TEST_WORDS    10U  

#define RAM_TEST_PATTERN    0x3333AAAAU

#define RAM_TEST_OK         0U
#define RAM_TEST_FAIL       1U

/* Global Variable */

/*********************************************************************
 * @fn      TestDataWrite
 *
 * @brief   Write data to certain address of 2K RAM and 18K RAM.
 *
 * @return  none
 */
uint32_t  TestDataWrite(void)
{
    uint32_t myAddr = RAM18K_TEST_ADDR;
    uint32_t myAddr2 = RAM2K_TEST_ADDR;
    uint32_t i = 0;
    for(i = 0; i < RAM18K_TEST_WORDS; i++) //Write 18KRAM
    {
        *(uint32_t volatile *)(myAddr + (i << 2)) = RAM_TEST_PATTERN;
    }
    for(i = 0; i < RAM18K_TEST_WORDS; i++) //Check 18KRAM
    {
        if((*(uint32_t volatile *)(myAddr + (i << 2))) != RAM_TEST_PATTERN)
        {
            printf("18K RAM write error at 0x%08x\r\n", (unsigned)(myAddr + (i << 2)));
            return RAM_TEST_FAIL;
        }
    }
    printf("Write 18K RAM Successfully!\r\n");

    for(i = 0; i < RAM2K_TEST_WORDS; i++) //Write 2KRAM
    {
        *(uint32_t volatile *)(myAddr2 + (i << 2)) = RAM_TEST_PATTERN;
    }
    for(i = 0; i < RAM2K_TEST_WORDS; i++) //Check 2KRAM
    {
        if((*(uint32_t volatile *)(myAddr2 + (i << 2))) != RAM_TEST_PATTERN)
        {
            printf("2K RAM write error at 0x%08x\r\n", (unsigned)(myAddr2 + (i << 2)));
            return RAM_TEST_FAIL;
        }
    }
    printf("Write 2K RAM Successfully!\r\n");
    return RAM_TEST_OK;
}

/*********************************************************************
 * @fn      TestDataRead
 *
 * @brief   Print the data of certain address of 2K RAM and 18K RAM.
 *
 * @return  none
 */
void TestDataRead(void)
{

    uint32_t i = 0;
    printf("2K RAM:\r\n");
    for(i = 0; i < RAM2K_TEST_WORDS; i++){
        printf("0x%08x=0x%08x\r\n", (unsigned)(RAM2K_TEST_ADDR + (i << 2)),
               (unsigned)*(uint32_t volatile *)(RAM2K_TEST_ADDR + (i << 2)));
    }
    printf("18K RAM:\r\n");
    printf("0x%08x=0x%08x\r\n", (unsigned)(RAM18K_TEST_ADDR + (i << 2)),
               (unsigned)*(uint32_t volatile *)(RAM18K_TEST_ADDR + (i << 2)));
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
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    /* Configure unused GPIO as IPD to reduce power consumption */
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA|RCC_PB2Periph_GPIOB|
            RCC_PB2Periph_GPIOC|RCC_PB2Periph_GPIOD|RCC_PB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);

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
    printf("SystemClk:%u\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );

    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);

    /* Delay 1.5s to avoid entering low power mode immediately after reset*/
    Delay_Ms(1500);
    while(1)
    {
        TestDataRead();
        printf("Standby Mode RAM Test\r\n");
        if(TestDataWrite() != RAM_TEST_OK)
        {
           printf("RAM write/verify failed\r\n"); 
        }
        PWR_WakeUpPinCmd(ENABLE);
        PWR_EnterSTANDBYMode_RAM();
    }
}
