/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2026/10/18
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 SPI uses DMA, master/slave mode transceiver routine:
 Master:SPI1_SCK(PA5)\SPI1_MISO(PA6)\SPI1_MOSI(PA7).
 Slave:SPI1_SCK(PA5)\SPI1_MISO(PA6)\SPI1_MOSI(PA7).

 This example demonstrates that Master and Slave use DMA full-duplex transmission
 and reception at the same time, repeated every 3s.
 The slave reception is interrupt-driven (DMA1_Channel2 TC interrupt): the ISR
 prints the received data, compares it with the transmitted data (same/diff),
 and restarts the SPI DMA for the next transfer.
 Note: The two boards download the Master and Slave programs respectively, and power
 on at the same time.
     Hardware connection:PA5 -- PA5
                         PA6 -- PA6
                         PA7 -- PA7
 When using SPI slave mode to send data:
   -the CPOL bit should be set to 1
   -the data should be sent using spi mode 2 or spi mode 3.
*/

#include "debug.h"
#include "string.h"

/* SPI Mode Definition */
#define HOST_MODE  0
#define SLAVE_MODE 1

/* SPI Communication Mode Selection */
#define SPI_MODE   HOST_MODE
// #define SPI_MODE   SLAVE_MODE

/* Global define */
#define Size       18

/* Global Variable */
u8 TxData[Size] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                   0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
                   0x21, 0x22, 0x23, 0x24, 0x25, 0x26};
u8 RxData[Size];


/*********************************************************************
 * @fn      SPI_FullDuplex_Init
 *
 * @brief   Configuring the SPI for full-duplex communication.
 *
 * @return  none
 */
void SPI_FullDuplex_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef  SPI_InitStructure  = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA | RCC_PB2Periph_SPI1, ENABLE);

#if (SPI_MODE == HOST_MODE)
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA, GPIO_Pin_4);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif (SPI_MODE == SLAVE_MODE)
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#endif

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;

#if (SPI_MODE == HOST_MODE)
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

#elif (SPI_MODE == SLAVE_MODE)
    SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;

#endif

    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial     = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

/*********************************************************************
 * @fn      DMA_Tx_Init
 *
 * @brief   Initializes the DMAy Channelx configuration.
 *
 * @param   DMA_CHx - x can be 1 to 7.
 *          ppadr - Peripheral base address.
 *          memadr - Memory base address.
 *          bufsize - DMA channel buffer size.
 *
 * @return  none
 */
void DMA_Tx_Init(DMA_Channel_TypeDef* DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA_CHx);

    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr     = memadr;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize         = bufsize;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);
}

/*********************************************************************
 * @fn      DMA_Rx_Init
 *
 * @brief   Initializes the SPI1 DMA Channelx configuration.
 *
 * @param   DMA_CHx - x can be 1 to 7.
 *          ppadr - Peripheral base address.
 *          memadr - Memory base address.
 *          bufsize - DMA channel buffer size.
 *
 * @return  none
 */
void DMA_Rx_Init(DMA_Channel_TypeDef* DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA_CHx);

    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr     = memadr;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize         = bufsize;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);
}

/*********************************************************************
 * @fn      DMA_StartTransfer
 *
 * @brief   Starts a DMA transfer on the given channel without waiting.
 *
 * @param   DMAy_Channelx - DMA channel used for the transfer
 *          Data          - pointer to the data buffer
 *          DataLength    - number of halfwords to transfer
 *
 * @return  none
 */
void DMA_StartTransfer(DMA_Channel_TypeDef* DMAy_Channelx, u8* Data, u16 DataLength)
{
    DMA_Cmd(DMAy_Channelx, DISABLE);
    /* Point to the new data buffer */
    DMAy_Channelx->MADDR = (uint32_t)Data;
    /* Set the transfer halfword count */
    DMAy_Channelx->CNTR = DataLength;
    /* Start the transfer */
    DMA_Cmd(DMAy_Channelx, ENABLE);
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
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());

    SPI_FullDuplex_Init();

    DMA_Tx_Init(DMA1_Channel3, (u32)&SPI1->DATAR, (u32)TxData, Size);
    DMA_Rx_Init(DMA1_Channel2, (u32)&SPI1->DATAR, (u32)RxData, Size);

#if (SPI_MODE == HOST_MODE)
    printf("Host Mode\r\n");
    Delay_Ms(2000);
#else
    printf("Slave Mode\r\n");
    Delay_Ms(1000);

    DMA_ClearITPendingBit(DMA1_IT_TC2);
    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);
#endif

    #if (SPI_MODE == HOST_MODE)
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    Delay_Ms(1);
#endif

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
    DMA_StartTransfer(DMA1_Channel2, RxData, Size);
    DMA_StartTransfer(DMA1_Channel3, TxData, Size);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);

#if (SPI_MODE == HOST_MODE)
    while ((!DMA_GetFlagStatus(DMA1_FLAG_TC2)) || (!DMA_GetFlagStatus(DMA1_FLAG_TC3)))
        ;

    DMA_ClearFlag(DMA1_FLAG_TC2);
    DMA_ClearFlag(DMA1_FLAG_TC3);

    Delay_Ms(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
#endif

    while (1)
    {
#if (SPI_MODE == HOST_MODE)
        Delay_Ms(3000);

        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);

        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        Delay_Ms(1);

        DMA_StartTransfer(DMA1_Channel2, RxData, Size);
        DMA_StartTransfer(DMA1_Channel3, TxData, Size);

        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);

        while ((!DMA_GetFlagStatus(DMA1_FLAG_TC2)) || (!DMA_GetFlagStatus(DMA1_FLAG_TC3)))
            ;

        DMA_ClearFlag(DMA1_FLAG_TC2);
        DMA_ClearFlag(DMA1_FLAG_TC3);

        Delay_Ms(1);
        GPIO_SetBits(GPIOA, GPIO_Pin_4);

        printf("tx\n");

#else

#endif
    }
}

/*********************************************************************
 * @fn      DMA1_Channel2_IRQHandler
 *
 * @brief   This function handles DMA1 Channel 2 (SPI1 RX) global interrupt.
 */
void DMA1_Channel2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void DMA1_Channel2_IRQHandler(void)
{
    u16 i = 0;

    if (DMA_GetITStatus(DMA1_IT_TC2) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC2);

        for (i = 0; i < Size; i++)
        {
            printf(" RxData:%02x\r\n", RxData[i]);
        }

        if (memcmp(TxData, RxData, Size) == 0)
        {
            printf("same\r\n");
        }
        else
        {
            printf("diff\r\n");
        }

        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
        DMA_StartTransfer(DMA1_Channel2, RxData, Size);
        DMA_StartTransfer(DMA1_Channel3, TxData, Size);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
    }
}
