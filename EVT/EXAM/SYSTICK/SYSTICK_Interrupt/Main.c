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

/*
 *@Note
 *Systick interrupt:
 *USART1_Tx(PA9).
 *
 */

#include "debug.h"

/* Global define */
int a=0;
u32 counter=0;

/* Global Variable */

/*********************************************************************
 * @fn      SYSTICK_Init_Config
 *
 * @brief   SYSTICK_Init_Config
 *
 * @return  none
 */
void SYSTICK_Init_Config( u32 ticks )
{
    SysTick->VAL = 0;
    SysTick->LOAD = ticks;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk |
					SysTick_CTRL_TICKINT_Msk |
					SysTick_CTRL_CLKSOURCE_Msk;
    NVIC_SetPriority(SysTick_IRQn , 3);
}

/*********************************************************************
 * @fn      Systick_Handler
 *
 * @brief   Systick_Handler
 *
 * @return  none
 */
void SysTick_Handler(void)
{
	printf("welcome to WCH\r\n"); 
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main( void )
{
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_2 );
    USART_Printf_Init( 115200 );
    printf( "SystemClk:%d\r\n", SystemCoreClock );
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    SYSTICK_Init_Config( 16500000-1);//Max count:16777215
    
    while( 1 )
    {
        a=SysTick->CTRL>>16;
       if(a == 1)
		{
			counter++;
			printf( "counter:%d\r\n",counter);
		}
    }
}
