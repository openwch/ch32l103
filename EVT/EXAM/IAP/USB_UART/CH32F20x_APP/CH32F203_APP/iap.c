/********************************** (C) COPYRIGHT  *******************************
* File Name          : iap.c
* Author             : WCH
* Version            : V1.0.1
* Date               : 2025/01/09
* Description        : IAP
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "iap.h"
#include "usb_istr.h"
#include "usb_lib.h"
#include "string.h"
#include "flash.h"

/******************************************************************************/  
iapfun jump2app; 
u32 Program_addr = FLASH_Base; 
u32 Verify_addr = FLASH_Base; 
u32 User_APP_Addr_offset = 0x5000;  
u8 Verify_Star_flag = 0;   
u8 Fast_Program_Buf[390];
u16 CodeLen = 0;
u8 End_Flag = 0;  
#define  isp_cmd_t   ((isp_cmd  *)EP2_Rx_Buffer)

/*********************************************************************
 * @fn      RecData_Deal
 *
 * @brief   UART-USB deal data
 *
 * @return  ERR_ERROR - ERROR
 *          ERR_SUCCESS - SUCCESS
 *          ERR_End - End
 */
u8 RecData_Deal(void)
{
    u8 s;

    switch ( isp_cmd_t->other.buf[0]) {
			case CMD_IAP_ERASE:
					s = ERR_ERROR;
					break;

			case CMD_IAP_PROM:
					s = ERR_ERROR;
					break;

			case CMD_IAP_VERIFY:
					s = ERR_ERROR;
					break;

			case CMD_IAP_END:				
					s = ERR_ERROR;
					break;
			
    case CMD_JUMP_IAP:
				 FLASH_Unlock_Fast();
				 FLASH_ErasePage_Fast(CalAddr & 0xFFFFFF00);
         FLASH_ProgramWord(CalAddr, 0x5aa55aa5);
         FLASH->CTLR |= ((uint32_t)0x00008000);
         FLASH->CTLR |= ((uint32_t)0x00000080);

        s = ERR_SUCCESS;
        break;
		
			default:
					s = ERR_ERROR;
					break;
    }

    return s;
}

/*********************************************************************
 * @fn      EP2_IN_Tx_Deal
 *
 * @brief   Deal device Endpoint 2 IN.
 *
 * @return  none
 */
void EP2_IN_Tx_Deal(void)
{
	UserToPMABufferCopy(EP2_Tx_Buffer, ENDP2_TXADDR, EP2_Tx_Cnt);
	SetEPTxCount(ENDP2, EP2_Tx_Cnt);
	
	SetEPTxValid(ENDP2); 
}

/*********************************************************************
 * @fn      EP2_RecData_Deal
 *
 * @brief   Deal device Endpoint 2 OUT.
 *
 * @return  none
 */
void EP2_RecData_Deal(void)  
{
	u8 s;

	if(EP2_OUT_Flag){
		EP2_OUT_Flag = 0;
		EP2_Rx_Cnt = USB_SIL_Read(EP2_OUT, EP2_Rx_Buffer);
			
		s = RecData_Deal();
		
		if(s!=ERR_End){		
				EP2_Tx_Buffer[0] = 0x00;
				if(s==ERR_ERROR) EP2_Tx_Buffer[1] = 0x01;   
				EP2_Tx_Cnt = 2;
				EP2_IN_Tx_Deal();
		}
		
		SetEPRxStatus(ENDP2, EP_RX_VALID);
	}
}

/*********************************************************************
 * @fn      USBD_CFG
 *
 * @brief   USB config
 *
 * @return  none
 */
void USBD_CFG(void)
{
	Set_USBConfig(); 
	USB_Init();	    
   
	USB_Port_Set(DISABLE, DISABLE);	
	Delay_Ms(700);
	USB_Port_Set(ENABLE, ENABLE);
	
	USB_Interrupts_Config();   	
}

/*********************************************************************
 * @fn      USART3_CFG
 *
 * @brief   baudrate:UART1 baudrate
 *
 * @return  none
 */
void USART3_CFG(u32 baudrate)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	
	USART_Init(USART3, &USART_InitStructure); 
	
	USART_Cmd(USART3, ENABLE); 
}

/*********************************************************************
 * @fn      UART3_SendMultiyData
 *
 * @brief   USART3 send data
 *
 * @param   pbuf - Packet to be sent
 *          num - Number of data sent
 *
 * @return  none
 */
void UART3_SendData(u8 data)
{
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);  
	USART_SendData(USART3, data);
}

/*********************************************************************
 * @fn      Uart3_Rx
 *
 * @brief   Uart1 receive date
 *
 * @return  none
 */
u8 Uart3_Rx( void )
{
	while( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);
	return USART_ReceiveData( USART3 );
}

/*********************************************************************
 * @fn      UART_Rx_Deal
 *
 * @brief   UART Rx data deal
 *
 * @return  none
 */
void UART_Rx_Deal(void)
{
    u8 i, s;
    u16 Data_add = 0;

    if (Uart3_Rx() == Uart_Sync_Head1)
    {
        if (Uart3_Rx() == Uart_Sync_Head2)
        {
            isp_cmd_t->UART.Cmd = Uart3_Rx();
            Data_add += isp_cmd_t->UART.Cmd;
            isp_cmd_t->UART.Len = Uart3_Rx();
            Data_add += isp_cmd_t->UART.Len;

            if(isp_cmd_t->UART.Cmd == CMD_IAP_ERASE ||isp_cmd_t->UART.Cmd == CMD_IAP_VERIFY)
            {
                isp_cmd_t->other.buf[2] = Uart3_Rx();
                Data_add += isp_cmd_t->other.buf[2];
                isp_cmd_t->other.buf[3] = Uart3_Rx();
                Data_add += isp_cmd_t->other.buf[3];
                isp_cmd_t->other.buf[4] = Uart3_Rx();
                Data_add += isp_cmd_t->other.buf[4];
                isp_cmd_t->other.buf[5] = Uart3_Rx();
                Data_add += isp_cmd_t->other.buf[5];
            }
            if ((isp_cmd_t->other.buf[0] == CMD_IAP_PROM) || (isp_cmd_t->other.buf[0] == CMD_IAP_VERIFY))
            {
                for (i = 0; i < isp_cmd_t->UART.Len; i++) {
                    isp_cmd_t->UART.data[i] = Uart3_Rx();
                    Data_add += isp_cmd_t->UART.data[i];
                }
            }
            if (Uart3_Rx() == (uint8_t)(Data_add & 0xFF))
            {
                if(Uart3_Rx() == (uint8_t)(Data_add >>8))
                {
                    if (Uart3_Rx() == Uart_Sync_Head2)
                    {
                        if (Uart3_Rx() == Uart_Sync_Head1)
                        {
                            s = RecData_Deal();

                            if (s != ERR_End)
                            {
                                UART3_SendData(Uart_Sync_Head1);
                                UART3_SendData(Uart_Sync_Head2);
                                UART3_SendData(0x00);
                                if (s == ERR_ERROR)
                                {
                                    UART3_SendData(0x01);
                                }
                                else
                                {
                                    UART3_SendData(0x00);
                                }
                                UART3_SendData(Uart_Sync_Head2);
                                UART3_SendData(Uart_Sync_Head1);
                            }
                        }
                    }
                }
            }
        }
    }
}



