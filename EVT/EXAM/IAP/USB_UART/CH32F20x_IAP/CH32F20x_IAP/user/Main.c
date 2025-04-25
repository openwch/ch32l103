/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.1
* Date               : 2025/01/09
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 * IAP routine: this routine support USB and USART mode,
 * and you can choose the command method or the IO method to jump to the APP .   
 * Key  parameters: CalAddr - address in flash (same in APP), note that this address needs to be unused.
 *                  CheckNum - The value of 'CalAddr' that needs to be modified.
 * Tips :the routine need IAP software version 1.50.
 */
#include "debug.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "iap.h"
#include "usb_istr.h"
#include "usb_desc.h"
#include "ch32f20x_usbfs_device.h"
#include "ch32f20x_usbhs_device.h"

extern u8 End_Flag;

#define UPGRADE_MODE_COMMAND   0
#define UPGRADE_MODE_IO        1

#define UPGRADE_MODE   UPGRADE_MODE_COMMAND
/* Global define */

/* Global Variable */    

/*********************************************************************
 * @fn      IAP_2_APP
 *
 * @brief   IAP_2_APP program.
 *
 * @return  none
 */
void IAP_2_APP(void) {
    USB_Port_Set(DISABLE, DISABLE);
    USBFSH->HOST_CTRL = 0x00;
    USBFSD->BASE_CTRL = 0x00;
    USBFSD->INT_EN = 0x00;
    USBFSD->UEP2_3_MOD = 0x00;
    USBFSD->BASE_CTRL |= (1 << 1) | (1 << 2);
	#if defined (CH32F20x_D8C) 
	  NVIC_DisableIRQ( USBHS_IRQn );
	#endif
    Delay_Ms(50);
	  iap_load_app(0x08000000 + User_APP_Addr_offset);
    Delay_Ms(50);
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
	u8 usbstatus=0;	
	SystemCoreClockUpdate();
	USART_Printf_Init(115200); 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init(); 
	printf("SystemClk:%d\r\n", SystemCoreClock);
	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
	printf("IAP\r\n");
	
#if UPGRADE_MODE == UPGRADE_MODE_COMMAND
    if(*(uint32_t*)FLASH_Base  != 0xe339e339 )
    {
        if(*(uint32_t*)CalAddr != CheckNum)
        {
            IAP_2_APP();
            while(1);
        }
    }
#elif UPGRADE_MODE == UPGRADE_MODE_IO
    if(PA0_Check() == 0)
    {
        IAP_2_APP();
        while(1);
    }
#endif
		
	USART3_CFG(460800);
#if defined (CH32F20x_D6) || defined (CH32F20x_D8) || defined (CH32F20x_D8W)	
	USBD_CFG();
#endif	
	USBFS_Init( );  
#if defined (CH32F20x_D8C) 
	USBHS_RCC_Init( );
	USBHS_Device_Init( ENABLE );
	NVIC_EnableIRQ( USBHS_IRQn );

#endif

	while(1)
	{
#if defined (CH32F20x_D6) || defined (CH32F20x_D8) || defined (CH32F20x_D8W)	
		if(usbstatus!=bDeviceState)
		{
			usbstatus=bDeviceState;
			
			if(usbstatus==CONFIGURED)
			{	
				
			}else
			{
				
			}
		}
		
		EP2_RecData_Deal();
#endif	

		if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != RESET){
			UART_Rx_Deal();
		}		
#if UPGRADE_MODE == UPGRADE_MODE_COMMAND
		if (End_Flag)
		{
			IAP_2_APP();
			while(1);
		}
#endif
		IWDG_ReloadCounter();
	}
}







