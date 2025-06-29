/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2025/04/11
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
  CAN_Wakeup routine:
  Configure a particular filter to match the address of the packet that wakes
  up SLEEP mode and bind the filter to Mailbox 1, which will automatically
  wake up from SLEEP mode when a packet is received from that address.
 */

#include "debug.h"

/* Global define */
/* Frame Format Definition */
#define Standard_Frame   0
#define Extended_Frame   1

/* Frame Formate Selection */
#define Frame_Format   Standard_Frame
// #define Frame_Format   Extended_Frame
/* Global Variable */


#define CANSOFTFILTER_MAX_GROUP_NUM 2           // The maximum recommended configuration is 14. 
                                                //Configure only what you need to prevent excessive RAM usage or an increase in the software's filtering time.

#define CANSOFTFILER_PREDEF_CTRLBYTE_MASK32 ((CAN_FilterScale_32bit << 5) | (CAN_FilterMode_IdMask << 1))
#define CANSOFTFILER_PREDEF_CTRLBYTE_ID32   ((CAN_FilterScale_32bit << 5) | (CAN_FilterMode_IdList << 1))

/* 
This is the structure of the software filtering table. It can be configured through the CAN_SoftFilterInit function,
or you can directly set the configuration values. The configured values can be modified directly during runtime.
However, when using the interrupt mode for reception, you need to be aware that if the modification is interrupted, 
it may affect the filtering results during this period. 
*/
struct CANFilterStruct_t
{
    union
    {
        union
        {
            struct
            {
                uint32_t :1;
                uint32_t RTR :1;
                uint32_t IDE :1;
                uint32_t ExID :29;
            }Access_Ex;
            struct
            {
                uint32_t :1;
                uint32_t RTR :1;
                uint32_t IDE :1;
                uint32_t :18;
                uint32_t StID :11;
            }Access_St;
        };
        union{
            struct {
                uint16_t FR_16_L;
                uint16_t FR_16_H;
            };
            uint32_t FR_32;
        };
    }FR[2];
    union
    {
        struct
        {
            uint16_t en :1;
            uint16_t mode :4;
            uint16_t scale :3;
        };
        uint16_t ctrl_byte;
    };
}CANFilterStruct[CANSOFTFILTER_MAX_GROUP_NUM];

uint8_t interrupt_rx_flag = 0;
volatile u8 canexbuf_interrupt[8];

void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void CAN_SoftFilterInit(CAN_FilterInitTypeDef* CAN_FilterInitStruct);
void CAN_Test_Mode_Init(u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode);
void CAN_ReceiveViaSoftFilter(CAN_TypeDef* CANx, uint8_t FIFONumber, CanRxMsg* RxMessage);
u8 CAN_Send_Msg(u8 *msg, u8 len);
u8 CAN_Receive_Msg(u8 *buf);

/*********************************************************************
 * @fn      CAN_SoftFilterInit
 *
 * @brief   Initializes the CAN peripheral according to the specified
 *        parameters in the CAN_FilterInitStruct.
 *
 * @param   CAN_FilterInitStruct - pointer to a CAN_FilterInitTypeDef
 *        structure that contains the configuration information.
 *
 * @return  none
 */
void CAN_SoftFilterInit(CAN_FilterInitTypeDef* CAN_FilterInitStruct)
{
    if(CAN_FilterInitStruct->CAN_FilterActivation)
    {
        CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].en = 1;
    }else
    {
        CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].en = 0;
    }
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[0].FR_16_H = CAN_FilterInitStruct->CAN_FilterIdHigh;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[0].FR_16_L = CAN_FilterInitStruct->CAN_FilterIdLow;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[1].FR_16_H = CAN_FilterInitStruct->CAN_FilterMaskIdHigh;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[1].FR_16_L = CAN_FilterInitStruct->CAN_FilterMaskIdLow;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].mode = CAN_FilterInitStruct->CAN_FilterMode;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].scale = CAN_FilterInitStruct->CAN_FilterScale;
}


/*********************************************************************
 * @fn      CAN_ReceiveViaSoftFilter
 *
 * @brief   Receives a message via soft filter.
 *
 * @param   CANx - where x can be 1 to select the CAN peripheral.
 *          FIFONumber - Receive FIFO number.
 *            CAN_FIFO0.
 *          RxMessage -  pointer to a structure receive message which contains
 *        CAN Id, CAN DLC, CAN datas and FMI number.
 *
 * @return  none
 */
void CAN_ReceiveViaSoftFilter(CAN_TypeDef* CANx, uint8_t FIFONumber, CanRxMsg* RxMessage)
{
    for (int group = 0; group < sizeof(CANFilterStruct)/sizeof(*CANFilterStruct); group++) 
    {
        if (CANFilterStruct[group].en) 
        {
            uint32_t temp = CANx->sFIFOMailBox[0].RXMIR & (~0x1);
            switch ((uint8_t)CANFilterStruct[group].ctrl_byte & ~0x1) 
            {

                case CANSOFTFILER_PREDEF_CTRLBYTE_ID32:
                    if((CANFilterStruct[group].FR[0].FR_32 != temp) && (CANFilterStruct[group].FR[1].FR_32 != temp))
                    {
                        continue;
                    }
                    else
                    {
                        CAN_Receive(CANx, CAN_FIFO0, RxMessage);
                        return;
                    }
                    break;

                case CANSOFTFILER_PREDEF_CTRLBYTE_MASK32:
                    if((CANFilterStruct[group].FR[0].FR_32 & CANFilterStruct[group].FR[1].FR_32) ^ (temp & CANFilterStruct[group].FR[1].FR_32))
                    {
                        continue;
                    }
                    else 
                    {
                        CAN_Receive(CANx, CAN_FIFO0, RxMessage);
                        return;
                    }
                    break;

                default:
                    return;
                    break;
            }
        }
    }
    CAN_FIFORelease(CANx,CAN_FIFO0);
}

/*********************************************************************
 * @fn      CAN_Mode_Init
 *
 * @brief   Initializes CAN communication test mode.
 *          Bps =Fpclk1/((tpb1+1+tbs2+1+1)*brp)
 *
 * @param   tsjw - CAN synchronisation jump width.
 *          tbs2 - CAN time quantum in bit segment 1.
 *          tbs1 - CAN time quantum in bit segment 2.
 *          brp - Specifies the length of a time quantum.
 *          mode - Test mode.
 *            CAN_Mode_Normal.
 *            CAN_Mode_LoopBack.
 *            CAN_Mode_Silent.
 *            CAN_Mode_Silent_LoopBack.
 *
 * @return  none
 */
void CAN_Mode_Init(u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode)
{
	GPIO_InitTypeDef GPIO_InitStructure =
	{
			0
	};
	CAN_InitTypeDef CAN_InitStructure =
	{
			0
	};
	CAN_FilterInitTypeDef CAN_FilterInitStructure =
	{
			0
	};
	NVIC_InitTypeDef NVIC_InitStructure =
	{
			0
	};

	RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOA , ENABLE );
	RCC_PB1PeriphClockCmd( RCC_PB1Periph_CAN1 , ENABLE );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOA , &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init( GPIOA , &GPIO_InitStructure );

	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = ENABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = mode;
	CAN_InitStructure.CAN_SJW = tsjw;
	CAN_InitStructure.CAN_BS1 = tbs1;
	CAN_InitStructure.CAN_BS2 = tbs2;
	CAN_InitStructure.CAN_Prescaler = brp;
	CAN_Init( CAN1 , &CAN_InitStructure );

	CAN_FilterInitStructure.CAN_FilterNumber = 0;

#if (Frame_Format == Standard_Frame)
	/* identifier/mask mode, One 32-bit filter, StdId: 0x317 */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x62E0;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xFFE0;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0004;

#elif (Frame_Format == Extended_Frame)
	/* identifier/mask mode, One 32-bit filter, ExtId: 0x12124567 */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x9092;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x2B3C;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xFFFC;

#endif

	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_SoftFilterInit( &CAN_FilterInitStructure );

	CAN_ITConfig( CAN1 , CAN_IT_FMP0 , ENABLE );

	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init( &NVIC_InitStructure );
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
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_1 );
	SystemCoreClockUpdate();
	Delay_Init();
	USART_Printf_Init( 115200 );
	printf( "SystemClk:%d\r\n" , SystemCoreClock );
	printf( "ChipID:%08x\r\n" , DBGMCU_GetCHIPID() );

	printf( "Wake up by CAN Rourine\r\n" );

	/* Bps = 500Kbps */
	CAN_Mode_Init( CAN_SJW_1tq , CAN_BS2_5tq , CAN_BS1_6tq , 12 , CAN_Mode_Normal );

	printf( "System will fall asleep immediately\r\n" );
    __WFI();

	while (1)
	{
		printf("Run in main\r\n");
		Delay_Ms(1000);
	}
}

void __attribute__((interrupt("WCH-Interrupt-fast"))) USB_LP_CAN1_RX0_IRQHandler()
{
	CanRxMsg CAN_buf;
	printf( "Woke up!!\r\n" );
	if ( CAN_GetITStatus( CAN1 , CAN_IT_FMP0 ) == SET )
	{
		printf( "Entered the CAN interrupt\r\n" );
		CAN_ClearITPendingBit( CAN1 , CAN_IT_FMP0 );
		CAN_ReceiveViaSoftFilter( CAN1 , CAN_FIFO0 , &CAN_buf );
		if(CAN_buf.DLC)
		{
#if Frame_Format == Standard_Frame
			printf( "Received freame's address is %#x\r\n" , CAN_buf.StdId );
#else
			printf("Received freame's address is %#x\r\n", CAN_buf.ExtId);
#endif
		}
	}
}
