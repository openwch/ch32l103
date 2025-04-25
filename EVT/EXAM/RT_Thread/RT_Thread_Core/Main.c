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
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"

/* Global define */
#define LED0_PIN  18   //PC3

/* Global Variable */


/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOA.0
 *
 * @return  none
 */
void GPIO_Toggle_INIT( void )
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOA, &GPIO_InitStructure );
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *  
 * @note    main is one of the tasks,other tasks include idle and tshell
 *
 * @return  none
 */
int main( void )
{
    u8 i = 0;
    SystemCoreClockUpdate();
    rt_kprintf( "SystemClk:%d\r\n", SystemCoreClock );
    rt_kprintf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    GPIO_Toggle_INIT();

	
    while( 1 )
    {
        rt_thread_mdelay( 500 );
        GPIO_WriteBit( GPIOA, GPIO_Pin_0, ( i == 0 ) ? ( i = Bit_SET ) : ( i = Bit_RESET ) );
    }
}



/*********************************************************************
 * @fn      led
 *
 * @brief   led blink by cmd of shell
 *
 * @return  err
 */
int led(void)
{
    rt_uint8_t count;

    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    for(count = 0 ; count < 10 ;count++)
    {
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_kprintf("led on, count : %d\r\n", count);
        rt_thread_mdelay(500);

        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_kprintf("led off\r\n");
        rt_thread_mdelay(500);
    }
    return 0;
}
MSH_CMD_EXPORT(led,  led sample by using I/O drivers);

