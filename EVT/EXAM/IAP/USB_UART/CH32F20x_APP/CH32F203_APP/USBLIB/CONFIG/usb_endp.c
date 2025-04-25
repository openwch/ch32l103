/********************************** (C) COPYRIGHT *******************************
* File Name          : usb_endp.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/08/08
* Description        : Endpoint routines
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/ 
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"

#define IN_FRAME_INTERVAL             5

/* Private variables */
//RX
u8 EP2_Rx_Buffer[USBD_DATA_SIZE+4];
u16 EP2_Rx_Cnt=0; 

//TX;
u8 EP2_Tx_Buffer[2];
u16 EP2_Tx_Cnt=0; 

u8 EP2_IN_Flag = 0;
u8 EP2_OUT_Flag = 0;

/*******************************************************************************
* Function Name  : EP2_IN_Callback
* Description    : Endpoint 2 IN.
* Input          : None.
* Return         : None.
*******************************************************************************/
void EP2_IN_Callback (void)
{ 
	EP2_IN_Flag = 0;
	SetEPTxStatus(ENDP2, EP_TX_NAK);	
}

/*******************************************************************************
* Function Name  : EP2_OUT_Callback
* Description    : Endpoint 2 IN.
* Input          : None.
* Return         : None.
*******************************************************************************/
void EP2_OUT_Callback(void)
{
	EP2_OUT_Flag = 1;														
}

/*******************************************************************************
* Function Name  : SOF_Callback
* Description    : SOF call back.
* Input          : None.
* Return         : None.
*******************************************************************************/
void SOF_Callback(void)
{
	static uint32_t FrameCount = 0;

	if(bDeviceState == CONFIGURED)
	{
		if (FrameCount++ == IN_FRAME_INTERVAL)
		{		
			FrameCount = 0;
			//Handle_USBAsynchXfer();
		}
	}  
}






