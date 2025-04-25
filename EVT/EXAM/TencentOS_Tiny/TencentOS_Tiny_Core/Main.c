/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/08/08
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "debug.h"
#include "tos_k.h"

/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOA.0/1
 *
 * @return  none
 */
void GPIO_Toggle_INIT(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure={0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}


#define TASK1_STK_SIZE       512
k_task_t task1;
__ALIGNED__(4) uint8_t task1_stk[TASK1_STK_SIZE];


#define TASK2_STK_SIZE       512
k_task_t task2;
__ALIGNED__(4) uint8_t task2_stk[TASK2_STK_SIZE];


/*********************************************************************
 * @fn      task1_entry
 *
 * @brief   task1 program.
 *
 * @param  *arg - Parameters point of task1
 *
 * @return  none
 */
void task1_entry(void *arg)
{
    while(1)
    {
        printf("###I am task1\r\n");
        GPIO_SetBits(GPIOA, GPIO_Pin_0);
        tos_task_delay(500);
        GPIO_ResetBits(GPIOA, GPIO_Pin_0);
        tos_task_delay(500);
    }
}

/*********************************************************************
 * @fn      task2_entry
 *
 * @brief   task2 program.
 *
 * @param  *arg - Parameters point of task2
 *
 * @return  none
 */
void task2_entry(void *arg)
{		
    while(1)
    {
        printf("***I am task2\r\n");
        GPIO_ResetBits(GPIOA, GPIO_Pin_1);
        tos_task_delay(1000);
        GPIO_SetBits(GPIOA, GPIO_Pin_1);
        tos_task_delay(1000);
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
	USART_Printf_Init(115200);
    SystemCoreClockUpdate();
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
	printf("Welcome to TencentOS tiny(%s)\r\n", TOS_VERSION);

	GPIO_Toggle_INIT();
	tos_knl_init();
    tos_task_create(&task1, "task1", task1_entry, NULL, 3, task1_stk, TASK1_STK_SIZE, 0); // Create task1
    tos_task_create(&task2, "task2", task2_entry, NULL, 3, task2_stk, TASK2_STK_SIZE, 0); // Create task2
    tos_knl_start();

    printf("should not run at here!\r\n");

    while(1)
	{
	    
	}
}



