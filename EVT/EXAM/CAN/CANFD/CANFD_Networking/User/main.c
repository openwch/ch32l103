/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2025/04/17
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *CANFD Normal mode, standard frame and expanded frame data transceiver:
 *CAN_Tx(PA12),CAN_Rx(PA11)
 *This routine requires an external CAN transceiver to demonstrate
 *Standard_Frame and Extended_Frame.
 *Standard_Frame: including 1 32bit filter mask bit pattern, 2 16bit filter
 *  mask bit patterns,one 32bit identifier list mode, 2 16bit identifier list modes;
 *Extended_Frame: 1 32bit filter mask bit pattern.
 *
 *note-
 *1.PHY is TJA1042T.
 *2.When CANFD communication speed is fast,the following configuration is required:
 *  a.CAN->CANFD_BTR TDCE(bit[23]) = 1.  BTR_BRP_FD(bit[20:16]) = 0 or 1.
 *  b.CAN->CANFD_TDCT TDCO(bit[5:0]) >= 2.
 *    TDC_FILTER(bit[13:8]) = (Td(TXD-busdom) + Td(busdom-RXD))/systemclk/3
 *    (Based on the selected PHY configuration)
 */

#include "debug.h"

/* CAN Mode Definition */
#define TX_MODE           0
#define RX_MODE           1

/* Frame Format Definition */
#define Standard_Frame    0
#define Extended_Frame    1

/* Data transmit baud rate Definition */
#define Data_BPS_2M    0
#define Data_BPS_4M    1

/* CAN Communication Mode Selection */
#define CAN_MODE   TX_MODE
//#define CAN_MODE   RX_MODE

/* Frame Formate Selection */
#define Frame_Format      Standard_Frame
//#define Frame_Format   Extended_Frame

/* Data transmit baud rate Selection */
#define CANFD_Data_BPS      Data_BPS_2M
//#define CANFD_Data_BPS      Data_BPS_4M

#define USE_INTERRUPT

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

u8 txbuf[64];
u8 rxbuf[64];
CanFDRxMsg CanFDRxStructure={0};
uint8_t interrupt_rx_flag = 0;
volatile u8 canexbuf_interrupt[8];

u8 CANFD_Receive_Msg(u8 *buf);
void CAN_SoftFilterInit(CAN_FilterInitTypeDef* CAN_FilterInitStruct);
void CANFD_ReceiveViaSoftFilter(CAN_TypeDef* CANx, uint8_t FIFONumber, CanFDRxMsg* RxMessage);
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast"))); 

void USB_LP_CAN1_RX0_IRQHandler()
{
    uint8_t px,pbuf[8];
    if (CAN_GetITStatus(CAN1,CAN_IT_FMP0))
    {
        px = CANFD_Receive_Msg(pbuf);
        for (int i = 0; i < px; i++) 
        {
            canexbuf_interrupt[i] = pbuf[i];
        }
        if(px)
        {
            interrupt_rx_flag = 1;
        }
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }
}

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
    if(CAN_FilterInitStruct->CAN_FilterNumber > sizeof(CANFilterStruct) / sizeof(*CANFilterStruct)){
        return;
    }
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
 * @fn      CAN_Mode_Init
 *
 * @brief   Initializes CAN communication test mode.
 *          CAN-Bps = Fpclk1/((tpb1+1+tbs2+1+1)*brp)
 *          FDCAN-Bps = Fpclk1/((fdtbs2+1+fdtbs1+1+1)*fdbrp)
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
void CAN_Mode_Init(u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 fdtsjw, u8 fdtbs2, u8 fdtbs1, u16 fdbrp, u8 mode)
{
    GPIO_InitTypeDef        GPIO_InitSturcture = {0};
    CAN_InitTypeDef         CAN_InitSturcture = {0};
    CANFD_InitTypeDef       CANFD_InitSturcture = {0};
    CAN_FilterInitTypeDef   CAN_FilterInitSturcture = {0};

    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOA , ENABLE);
    RCC_PB1PeriphClockCmd( RCC_PB1Periph_CAN1, ENABLE);

    GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitSturcture);

    GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitSturcture);

    CAN_InitSturcture.CAN_Mode = mode;
    CAN_InitSturcture.CAN_SJW = tsjw;
    CAN_InitSturcture.CAN_BS1 = tbs1;
    CAN_InitSturcture.CAN_BS2 = tbs2;
    CAN_InitSturcture.CAN_Prescaler = brp;
    CAN_Init(CAN1, &CAN_InitSturcture);

#if (CANFD_Data_BPS == Data_BPS_4M)
    CANFD_InitSturcture.CANFD_TDCE = ENABLE;
    CANFD_InitSturcture.CANFD_TDC0 = 0x0D;
    CANFD_InitSturcture.CANFD_TDC_FILTER = 0x06;
#endif

    CANFD_InitSturcture.CANFD_TTCM = DISABLE;
    CANFD_InitSturcture.CANFD_ABOM = DISABLE;
    CANFD_InitSturcture.CANFD_AWUM = DISABLE;
    CANFD_InitSturcture.CANFD_NART = ENABLE;
    CANFD_InitSturcture.CANFD_RFLM = DISABLE;
    CANFD_InitSturcture.CANFD_TXFP = DISABLE;
    CANFD_InitSturcture.CANFD_Mode = mode;
    CANFD_InitSturcture.CANFD_SJW = fdtsjw;
    CANFD_InitSturcture.CANFD_BS1 = fdtbs1;
    CANFD_InitSturcture.CANFD_BS2 = fdtbs2;
    CANFD_InitSturcture.CANFD_Prescaler = fdbrp;
    CANFD_InitSturcture.CANFD_BRS_TXM0 = ENABLE;
    CANFD_InitSturcture.CANFD_BRS_TXM1 = ENABLE;
    CANFD_InitSturcture.CANFD_BRS_TXM2 = ENABLE;
    CANFD_Init(CAN1, &CANFD_InitSturcture);

    CAN_FilterInitSturcture.CAN_FilterNumber = 0;
#if(Frame_Format == Standard_Frame)
    /* identifier/mask mode, One 32-bit filter, StdId: 0x317 */
    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x62E0;
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0;
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0xFFFF;
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0;

#elif(Frame_Format == Extended_Frame)
    /* identifier/mask mode, One 32-bit filter, ExtId: 0x12124567 */
    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x9092;
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0x2B3C;
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0xFFFF;
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0xFFFE;

#endif

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;
    CAN_SoftFilterInit(&CAN_FilterInitSturcture);

    CANFD_ReceiveFIFO_DMAAdr(CAN1, CAN_FIFO0, (u32)&CanFDRxStructure.Data[0]);
}

/*********************************************************************
 * @fn      CANFD_Send_Msg
 *
 * @brief   CANFD Transmit function.
 *
 * @param   msg - Transmit data buffer.
 *          len - Data length.
 *
 * @return  0 - Send successful.
 *          1 - Send failed.
 */
u8 CANFD_Send_Msg( u8 *msg, u8 len )
{
    u8  mbox;
    u16 i = 0;

    CanFDTxMsg  CanFDTxStructure={0};

    #if(Frame_Format == Standard_Frame)
    CanFDTxStructure.StdId = 0x317;
    CanFDTxStructure.IDE = CAN_Id_Standard;

    #elif(Frame_Format == Extended_Frame)
    CanFDTxStructure.ExtId = 0x12345678;
    CanFDTxStructure.IDE = CAN_Id_Extended;

    #endif

    CanFDTxStructure.RTR = CAN_RTR_Data;
    CanFDTxStructure.DLC = len;

    for(i = 0; i < 64; i++) {
        CanFDTxStructure.Data[i] = msg[i];
    }

    mbox = CANFD_Transmit( CAN1, &CanFDTxStructure);

    while((CAN_TransmitStatus(CAN1, mbox) != CAN_TxStatus_Ok) && (i < 0xFFF))
    {
        i++;
    }

    if(i == 0xFFF)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*********************************************************************
 * @fn      CANFD_ReceiveViaSoftFilter
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
void CANFD_ReceiveViaSoftFilter(CAN_TypeDef* CANx, uint8_t FIFONumber, CanFDRxMsg* RxMessage)
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
                        CANFD_Receive(CANx, CAN_FIFO0, RxMessage);
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
                        CANFD_Receive(CANx, CAN_FIFO0, RxMessage);
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
 * @fn      CAN_Receive_Msg
 *
 * @brief   CAN Receive function.
 *
 * @param   buf - Receive data buffer.
 *          len - Data length.
 *
 * @return  CanRxStructure.DLC - Receive data length.
 */
u8 CANFD_Receive_Msg(u8 *buf)
{
    u8 i;

    if(CAN_MessagePending(CAN1, CAN_FIFO0) == 0)
    {
        return 0;
    }

    CANFD_ReceiveViaSoftFilter(CAN1, CAN_FIFO0, &CanFDRxStructure);

    for( i = 0;i < CanFDRxStructure.DLC; i++ )
    {
        buf [i] = CanFDRxStructure.Data[i];
    }

    return CanFDRxStructure.DLC;
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
    u8 i = 0;
    u8 cnt = 0;
    u8 px = 0;
    u8 pxbuf[64];
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);

#if(CAN_MODE == TX_MODE)
    printf("Tx Mode\r\n");

#elif(CAN_MODE == RX_MODE)
    printf("Rx Mode\r\n");

#endif

#if (CANFD_Data_BPS == Data_BPS_2M)
    /* Arbitration-Bps = 1Mbps��data-Bps = 2Mbps */
    CAN_Mode_Init(CAN_SJW_4tq, CAN_BS2_8tq, CAN_BS1_15tq, 2, CANFD_SJW_4tq, CANFD_BS2_5tq, CANFD_BS1_6tq, 2, CAN_Mode_Normal);

#elif (CANFD_Data_BPS == Data_BPS_4M)
    /* Arbitration-Bps = 1Mbps��data-Bps = 4Mbps */
    CAN_Mode_Init(CAN_SJW_4tq, CAN_BS2_8tq, CAN_BS1_15tq, 2,CANFD_SJW_4tq, CANFD_BS2_5tq, CANFD_BS1_6tq, 1, CAN_Mode_Normal);

#endif

    while(1)
    {
#if(CAN_MODE == TX_MODE)

        for(i = 0; i < 64; i++) {
            pxbuf[i] = cnt+i;
        }

        px = CANFD_Send_Msg( pxbuf, CANFD_DLC_BYTES_48 );

        if(px)
        {
            printf("Send Failed\r\n");
        }
        else
        {
            printf("Send Success\r\n");
        }

#elif(CAN_MODE == RX_MODE)
        px = CANFD_Receive_Msg(pxbuf);

        if(px)
        {
            printf("Receive Data:\r\n");
            for(i = 0; i < px; i++) {
                printf("%02x\t", pxbuf[i]);
            }
            printf("\n");
        }

#endif
        Delay_Ms(1000);
        cnt++;
        if(cnt == 0xFF)
        {
            cnt = 0;
        }
    }
}
