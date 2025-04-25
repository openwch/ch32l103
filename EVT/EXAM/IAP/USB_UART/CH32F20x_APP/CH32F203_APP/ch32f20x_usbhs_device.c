/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32f20x_usbhs_device.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/08/08
* Description        : This file provides all the USBHS firmware functions.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32f20x_usbhs_device.h"
#include "iap.h"
/******************************************************************************/


#define Version_Num   0x0100

__attribute__ ((aligned(4))) UINT8 EP0_Databuf[ USBHS_UEP0_SIZE ];
__attribute__ ((aligned(4))) UINT8 EP1_Rx_Databuf[ USBHS_MAX_PACK_SIZE ]; 
__attribute__ ((aligned(4))) UINT8 EP1_Tx_Databuf[ USBHS_MAX_PACK_SIZE ];
__attribute__ ((aligned(4))) UINT8 EP2_Rx_Databuf[ USBHS_MAX_PACK_SIZE ]; 
__attribute__ ((aligned(4))) UINT8 EP2_Tx_Databuf[ USBHS_MAX_PACK_SIZE ];

#define pMySetupReqPak        ((PUSB_SETUP_REQ)EP0_Databuf)
//const UINT8 *pDescr;
volatile UINT8  USBHS_Dev_SetupReqCode = 0xFF;                                 
volatile UINT16 USBHS_Dev_SetupReqLen = 0x00;                                  
volatile UINT8  USBHS_Dev_SetupReqValueH = 0x00;                               
volatile UINT8  USBHS_Dev_Config = 0x00;                                        
volatile UINT8  USBHS_Dev_Address = 0x00;                                      
volatile UINT8  USBHS_Dev_SleepStatus = 0x00;                                   
volatile UINT8  USBHS_Dev_EnumStatus = 0x00;                                   
volatile UINT8  USBHS_Dev_Endp0_Tog = 0x01;                                     
volatile UINT8  USBHS_Dev_Speed = 0x01;                                         
volatile UINT8  USBHS_Int_Flag  = 0x00;                                         

volatile UINT16 USBHS_Endp1_Up_Flag = 0x00;                                     
volatile UINT8  USBHS_Endp1_Down_Flag = 0x00;                                   
volatile UINT8  USBHS_Endp1_Down_Len = 0x00;                                    
volatile UINT8  USBHS_Endp1_T_Tog = 0x00;
volatile UINT8  USBHS_Endp1_R_Tog = 0x00;

volatile UINT16 USBHS_Endp2_Up_Flag = 0x00;                                     
volatile UINT16 USBHS_Endp2_Up_LoadPtr = 0x00;                                  
volatile UINT8  USBHS_Endp2_Down_Flag = 0x00;                                   

/******************************************************************************/
UINT8 MyDevDescr[18] = { 0x12, 0x01, 0x10, 0x01, 0xFF, 0x80, 0x55, 0x40, 0x48,
        0x43, 0xe0, 0x55,  //USB MODULE
        (u8) Version_Num, (u8) (Version_Num >> 8), 
        0x00, 0x00, 0x00, 0x01 };

/* USB configuration descriptor (full speed) */
const UINT8 MyCfgDescr_FS[] = { 0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0x80,
        0x32, 0x09, 0x04, 0x00, 0x00, 0x02, 0xFF, 0x80, 0x55, 0x00, 0x07, 0x05,
        0x82, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00 };

/* USB Configuration Descriptor (High Speed) */
const UINT8 MyCfgDescr_HS[] = { 0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0x80,
        0x32, 0x09, 0x04, 0x00, 0x00, 0x02, 0xFF, 0x80, 0x55, 0x00, 0x07, 0x05,
        0x82, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00 };


/*********************************************************************
 * @fn      USBHS_RCC_Init
 *
 * @brief   Initializes the clock for USB2.0 High speed device.
 *
 * @return  none
 */
void USBHS_RCC_Init( void )
{
#if defined (CH32F20x_D8C) 	
    RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
    RCC_USBHSConfig(RCC_USBPLL_Div2);
    RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
    RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
    RCC_USBHSPHYPLLALIVEcmd(ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
#endif		
}

/*********************************************************************
 * @fn      USBHS_Device_Endp_Init
 *
 * @brief   USBHS Device Endp Init
 *
 * @return  none
 */
void USBHS_Device_Endp_Init ( void )
{

      USBHSD->ENDP_CONFIG = USBHS_EP0_T_EN | USBHS_EP0_R_EN 
            | USBHS_EP2_T_EN | USBHS_EP2_R_EN ;


    USBHSD->ENDP_TYPE = 0x00;
    USBHSD->BUF_MODE = 0x00;
    USBHSD->UEP0_MAX_LEN = 64;
    USBHSD->UEP2_MAX_LEN = 64;

    USBHSD->UEP0_DMA    = (UINT32)(UINT8 *)EP0_Databuf;
    USBHSD->UEP2_TX_DMA = (UINT32)(UINT8 *)EP2_Tx_Databuf;
    USBHSD->UEP2_RX_DMA = (UINT32)(UINT8 *)EP2_Rx_Databuf;


    USBHSD->UEP0_TX_LEN  = 0;
    USBHSD->UEP0_TX_CTRL = USBHS_EP_T_RES_NAK;
    USBHSD->UEP0_RX_CTRL = USBHS_EP_R_RES_ACK;

    USBHSD->UEP2_TX_LEN  = 0;
    USBHSD->UEP2_TX_CTRL = USBHS_EP_T_RES_NAK;
    USBHSD->UEP2_RX_CTRL =  USBHS_EP_R_RES_ACK;


}

/*********************************************************************
 * @fn      USBHS_Device_Init
 *
 * @brief   USBHS Device Init
 *
 * @return  none
 */
void USBHS_Device_Init ( FunctionalState sta )
{
    if( sta )
    {

        USBHSD->HOST_CTRL = 0x00;
        USBHSD->HOST_CTRL = USBHS_SUSPENDM;

        USBHSD->CONTROL   = 0x00;
        USBHSD->CONTROL   = USBHS_DMA_EN | USBHS_INT_BUSY_EN | USBHS_FULL_SPEED;


        USBHSD->INT_EN    = 0;
        USBHSD->INT_EN    = USBHS_SETUP_ACT_EN | USBHS_TRANSFER_EN | USBHS_DETECT_EN | USBHS_SUSPEND_EN;
			
        USBHSD->ENDP_CONFIG = 0xffffffff;


        USBHS_Device_Endp_Init( );
        Delay_Us(10);


        USBHSD->CONTROL |= USBHS_DEV_PU_EN;
    }
    else
    {
        USBHSD->CONTROL &= ~USBHS_DEV_PU_EN;
        USBHSD->CONTROL |= USBHS_ALL_CLR | USBHS_FORCE_RST;
    }
}

/*********************************************************************
 * @fn      USBHS_Device_SetAddress
 *
 * @brief   USBHS Device SetAddress
 *
 * @return  none
 */
void USBHS_Device_SetAddress( UINT32 address )
{
    USBHSD->DEV_AD = 0;
    USBHSD->DEV_AD = address & 0xff;
}

/*********************************************************************
 * @fn      USBHS_IRQHandler
 *
 * @brief   This function handles USBHS exception.
 *
 * @return  none
 */
void USBHS_IRQHandler( void )
{
    UINT32 end_num;
    UINT32 rx_token;
    UINT16 len = 0x00;
    UINT8 errflag = 0x00;
    UINT8 chtype;

    USBHS_Int_Flag = USBHSD->INT_FG;

    if (USBHS_Int_Flag & USBHS_TRANSFER_FLAG) {
        /* Endpoint transport handling */
        end_num = (USBHSD->INT_ST) & MASK_UIS_ENDP;
        rx_token = (((USBHSD->INT_ST) & MASK_UIS_TOKEN) >> 4) & 0x03;
        /* 00: OUT, 01:SOF, 10:IN, 11:SETUP */
        if (end_num == 0) {
            /* process of endpoint 0 */
            if (rx_token == PID_IN ) {
                /* Upload successfully interrupted for endpoint 0 */
                switch (USBHS_Dev_SetupReqCode) {
                case USB_GET_DESCRIPTOR:
                case 0x20: {
                    len = USBHS_Dev_SetupReqLen >= USBHS_UEP0_SIZE ?
                            USBHS_UEP0_SIZE : USBHS_Dev_SetupReqLen;
                    memcpy(EP0_Databuf, pDescr, len);
                    USBHS_Dev_SetupReqLen -= len;
                    pDescr += len;
                    USBHSD->UEP0_TX_LEN = len;
                    USBHSD->UEP0_TX_CTRL ^= USBHS_EP_T_TOG_1;
                    break;
                }

                case USB_SET_ADDRESS:
                    USBHS_Device_SetAddress(USBHS_Dev_Address);
                    USBHSD->UEP0_TX_LEN = 0;
                    USBHSD->UEP0_TX_CTRL = USBHS_EP_T_RES_NAK;
                    USBHSD->UEP0_RX_CTRL = USBHS_EP_R_RES_ACK;
                    break;

                default:
    /* The status phase is completed and interrupted or the 0-length data packet is forced to be uploaded to end the control transmission. */
                    USBHSD->UEP0_TX_LEN = 0;
                    USBHSD->UEP0_TX_CTRL = USBHS_EP_T_RES_NAK;
                    USBHSD->UEP0_RX_CTRL = USBHS_EP_R_RES_ACK;
                    pDescr = NULL;
                    break;
                }
            } else if (rx_token == PID_OUT ) {
                USBHSD->UEP0_TX_LEN = 0;
                USBHSD->UEP0_TX_CTRL |= USBHS_EP_T_RES_ACK;
                USBHSD->UEP0_RX_CTRL |= USBHS_EP_R_RES_ACK;
            }
        } else if (end_num == 2) {
            if (rx_token == PID_IN ) {

                len = USBHSD->UEP2_TX_LEN;

                USBHSD->UEP2_TX_CTRL ^= USBHS_EP_T_TOG_1;

                USBHSD->UEP2_TX_CTRL = (USBHSD->UEP2_TX_CTRL
                        & (~USBHS_EP_T_RES_MASK)) | USBHS_EP_T_RES_NAK;

            } else if (rx_token == PID_OUT ) {

                if (USBHSD->INT_ST & USBHS_DEV_UIS_TOG_OK) {
                    USBHSD->UEP2_RX_CTRL ^= USBHS_EP_R_TOG_1;
                    EP2_Rx_Cnt = USBHSD->RX_LEN;
                    EP2_OUT_Flag = 1;

                    DevEPhs_OUT_Deal(EP2_Rx_Cnt);
                }

            }
        }
        USBHSD->INT_FG = USBHS_TRANSFER_FLAG;
    } else if (USBHS_Int_Flag & USBHS_SETUP_FLAG) {
        /* SETUP package processing */
        USBHS_Dev_SetupReqLen = pMySetupReqPak->wLength;
        USBHS_Dev_SetupReqCode = pMySetupReqPak->bRequest;
        chtype = pMySetupReqPak->bRequestType;
        len = 0x00;
        errflag = 0x00;

        /* Determine whether the current request is a standard request or another request */
        if (( pMySetupReqPak->bRequestType & USB_REQ_TYP_MASK)
                != USB_REQ_TYP_STANDARD) {
            /* Other requests, such as class requests, vendor requests, etc. */
            if ( pMySetupReqPak->bRequestType & 0x40) {
                /* Vendor request */
                switch (USBHS_Dev_SetupReqCode) {
                default:
                    errflag = 0xFF; /* operation failed */
                    break;
                }
            } else if ( pMySetupReqPak->bRequestType & 0x20) {
                /* class request */
            }
            /* Determine whether it can be processed normally */
            if (errflag != 0xFF) {
                if (USBHS_Dev_SetupReqLen > len) {
                    USBHS_Dev_SetupReqLen = len;
                }
                len = (USBHS_Dev_SetupReqLen >= USBHS_UEP0_SIZE) ?
                        USBHS_UEP0_SIZE : USBHS_Dev_SetupReqLen;
                memcpy(EP0_Databuf, pDescr, len);
                pDescr += len;
            }
        } else {
            /* Handles standard USB request packets */
            switch (USBHS_Dev_SetupReqCode) {
            case USB_GET_DESCRIPTOR: {
                switch ((( pMySetupReqPak->wValue) >> 8)) {
                case USB_DESCR_TYP_DEVICE:
                    /* Get device descriptor */
                    pDescr = MyDevDescr;
                    len = MyDevDescr[0];
                    break;

                case USB_DESCR_TYP_CONFIG:
                    /* Get configuration descriptor */
                    pDescr = MyCfgDescr_HS;
                    len = MyCfgDescr_HS[2];
                    break;

                case USB_DESCR_TYP_STRING:
                    /* Get string descriptor */
                    switch (( pMySetupReqPak->wValue) & 0xff) {
                    case 0:
                        /* Language string descriptor */

                        break;

                    case 1:
                        /* USB vendor string descriptor */

                        break;

                    case 2:
                        /* USB product string descriptor */

                        break;

                    case 3:
                        /* USB serial number string descriptor */

                        break;

                    default:
                        errflag = 0xFF;
                        break;
                    }
                    break;

                case 6:
                    /* device qualified descriptor */

                    break;

                case 7:
                    /* Other speed configuration descriptors */
                    if (USBHS_Dev_Speed == 0x01)
                        errflag = 0xFF;
                    break;

                case 0x0f:
                    /* BOS descriptor */
                    /* USB2.0 devices do not support BOS descriptors */
                    errflag = 0xFF;
                    break;
                default:
                    errflag = 0xFF;
                    break;
                }

                /* Determine whether it can be processed normally */
                if (errflag != 0xFF) {
                    if (USBHS_Dev_SetupReqLen > len) {
                        USBHS_Dev_SetupReqLen = len;
                    }
                    len = (USBHS_Dev_SetupReqLen >= USBHS_UEP0_SIZE) ?
                            USBHS_UEP0_SIZE : USBHS_Dev_SetupReqLen;
                    memcpy(EP0_Databuf, pDescr, len);
                    pDescr += len;
                }
            }
                break;

            case USB_SET_ADDRESS:
                /* Set address */
                USBHS_Dev_Address = ( pMySetupReqPak->wValue) & 0xff;
                break;

            case USB_GET_CONFIGURATION:
                /* Get configuration value */
                EP0_Databuf[0] = USBHS_Dev_Config;
                if (USBHS_Dev_SetupReqLen > 1) {
                    USBHS_Dev_SetupReqLen = 1;
                }
                break;

            case USB_SET_CONFIGURATION:
                /* Set configuration values */
                USBHS_Dev_Config = ( pMySetupReqPak->wValue) & 0xff;
                USBHS_Dev_EnumStatus = 0x01;
                break;

            case USB_CLEAR_FEATURE: {
                /* Clear properties */
                if (( pMySetupReqPak->bRequestType & USB_REQ_RECIP_MASK)
                        == USB_REQ_RECIP_ENDP) {
                    /* clear endpoint */
                    switch (( pMySetupReqPak->wIndex) & 0xff)/* wIndexL */
                    {
                    case 0x82:
                        /* SET Endp2 Tx to USBHS_EP_T_RES_NAK;USBHS_EP_T_TOG_0;len = 0 */
                        USBHSD->UEP2_TX_LEN = 0;
                        USBHSD->UEP2_TX_CTRL = USBHS_EP_T_RES_NAK
                                | USBHS_EP_T_TOG_0;
                        break;

                    case 0x02:
                        /* SET Endp2 Rx to USBHS_EP_R_RES_ACK;USBHS_EP_R_TOG_0 */
                        USBHSD->UEP2_TX_CTRL = USBHS_EP_R_RES_ACK
                                | USBHS_EP_R_TOG_0;
                        break;

                    case 0x81:
                        /* SET Endp1 Tx to USBHS_EP_T_RES_NAK;USBHS_EP_T_TOG_0;len = 0 */
                        USBHSD->UEP1_TX_LEN = 0;
                        USBHSD->UEP1_TX_CTRL = USBHS_EP_T_RES_NAK
                                | USBHS_EP_T_TOG_0;
                        break;

                    case 0x01:
                        /* SET Endp1 Rx to USBHS_EP_R_RES_NAK;USBHS_EP_R_TOG_0 */
                        USBHSD->UEP1_RX_CTRL = USBHS_EP_R_RES_ACK
                                | USBHS_EP_R_TOG_0;
                        break;

                    default:
                        errflag = 0xFF;
                        break;
                    }
                }

                else {
                    errflag = 0xFF;

                }
                break;
                case USB_SET_FEATURE:
                /* Set properties */
                if (( pMySetupReqPak->bRequestType & 0x1F) == 0x00) {
                    /* Set up device */
                    if ( pMySetupReqPak->wValue == 0x01) {
                        errflag = 0xFF;

                    } else {
                        errflag = 0xFF;
                    }
                } else if (( pMySetupReqPak->bRequestType & 0x1F) == 0x02) {
                    /* Set endpoint */
                    if ( pMySetupReqPak->wValue == 0x00) {
                        /* Set the specified endpoint STALL */
                        switch (( pMySetupReqPak->wIndex) & 0xff) {
                        case 0x82:
                            /* Set endpoint 2 IN STALL */
                            USBHSD->UEP2_TX_CTRL = ( USBHSD->UEP2_TX_CTRL
                                    & ~USBHS_EP_T_RES_MASK)
                                    | USBHS_EP_T_RES_STALL;
                            break;

                        case 0x02:
                            /* Set endpoint 2 OUT Stall */
                            USBHSD->UEP2_RX_CTRL = ( USBHSD->UEP2_RX_CTRL
                                    & ~USBHS_EP_R_RES_MASK)
                                    | USBHS_EP_R_RES_STALL;
                            break;

                        case 0x81:
                            /* Set endpoint 1 IN STALL */
                            USBHSD->UEP1_TX_CTRL = ( USBHSD->UEP1_TX_CTRL
                                    & ~USBHS_EP_T_RES_MASK)
                                    | USBHS_EP_T_RES_STALL;
                            break;

                        case 0x01:
                            /* Set endpoint 1 OUT STALL */
                            USBHSD->UEP1_RX_CTRL = ( USBHSD->UEP1_RX_CTRL
                                    & ~USBHS_EP_R_RES_MASK)
                                    | USBHS_EP_R_RES_STALL;
                            break;

                        default:
                            errflag = 0xFF;
                            break;
                        }
                    } else {
                        errflag = 0xFF;
                    }
                } else {
                    errflag = 0xFF;
                }
                break;
            }

            case USB_GET_INTERFACE:
                EP0_Databuf[0] = 0x00;
                if (USBHS_Dev_SetupReqLen > 1) {
                    USBHS_Dev_SetupReqLen = 1;
                }
                break;

            case USB_SET_INTERFACE:
                EP0_Databuf[0] = 0x00;
                if (USBHS_Dev_SetupReqLen > 1) {
                    USBHS_Dev_SetupReqLen = 1;
                }
                break;

            case USB_GET_STATUS:
                /* Respond based on the actual status of the current endpoint */
                EP0_Databuf[0] = 0x00;
                EP0_Databuf[1] = 0x00;
                if ( pMySetupReqPak->wIndex == 0x81) {
                    if (( USBHSD->UEP1_TX_CTRL & USBHS_EP_T_RES_MASK)
                            == USBHS_EP_T_RES_STALL) {
                        EP0_Databuf[0] = 0x01;
                    }
                } else if ( pMySetupReqPak->wIndex == 0x01) {
                    if (( USBHSD->UEP1_RX_CTRL & USBHS_EP_R_RES_MASK)
                            == USBHS_EP_R_RES_STALL) {
                        EP0_Databuf[0] = 0x01;
                    }
                } else if ( pMySetupReqPak->wIndex == 0x82) {
                    if (( USBHSD->UEP2_TX_CTRL & USBHS_EP_T_RES_MASK)
                            == USBHS_EP_T_RES_STALL) {
                        EP0_Databuf[0] = 0x01;
                    }
                } else if ( pMySetupReqPak->wIndex == 0x02) {
                    if (( USBHSD->UEP2_RX_CTRL & USBHS_EP_R_RES_MASK)
                            == USBHS_EP_R_RES_STALL) {
                        EP0_Databuf[0] = 0x01;
                    }
                }
                if (USBHS_Dev_SetupReqLen > 2) {
                    USBHS_Dev_SetupReqLen = 2;
                }
                break;

            default:
                errflag = 0xff;
                break;
            }
        }

        /* Endpoint 0 processing */
        if (errflag == 0xFF) {
            /* IN - STALL / OUT - DATA - STALL */
            USBHS_Dev_SetupReqCode = 0xFF;
            USBHSD->UEP0_TX_LEN = 0;
            USBHSD->UEP0_TX_CTRL = USBHS_EP_T_RES_STALL;
            USBHSD->UEP0_RX_CTRL = USBHS_EP_R_RES_STALL;
        } else {
            /* DATA stage (IN -DATA1-ACK) */
            if (chtype & 0x80) {
                len = (USBHS_Dev_SetupReqLen > USBHS_UEP0_SIZE) ?
                        USBHS_UEP0_SIZE : USBHS_Dev_SetupReqLen;
                USBHS_Dev_SetupReqLen -= len;
            } else {
                len = 0;
            }
            USBHSD->UEP0_TX_LEN = len;
            USBHSD->UEP0_TX_CTRL = USBHS_EP_T_RES_ACK | USBHS_EP_T_TOG_1;
        }
        USBHSD->INT_FG = USBHS_SETUP_FLAG;
    } else if (USBHS_Int_Flag & USBHS_DETECT_FLAG) {
        USBHS_Dev_Address = 0x00;
        USBHS_Device_Endp_Init(); /* USBHS device endpoint initialization */
        USBHS_Device_SetAddress(USBHS_Dev_Address); /* USBHS device set device address */
        USBHSD->INT_FG = USBHS_DETECT_FLAG;
    } else if (USBHS_Int_Flag & USBHS_SUSPEND_FLAG) {
        USBHS_Dev_SleepStatus &= ~0x02;
        USBHS_Dev_EnumStatus = 0x01;
        USBHSD->INT_FG = USBHS_SUSPEND_FLAG;
    }
}



/*********************************************************************
 * @fn      void DevEPhs_IN_Deal
 *
 * @brief   Device endpoint2 IN.
 *
 * @param   l - IN length(<64B)
 *
 * @return  none
 */
void DevEPhs_IN_Deal(UINT8 l) {
    USBHSD->UEP2_TX_LEN = l;
    USBHSD->UEP2_TX_CTRL = (USBHSD->UEP2_TX_CTRL & (~USBHS_EP_T_RES_MASK))
            | USBHS_EP_T_RES_ACK;
}

/*********************************************************************
 * @fn      DevEPhs_OUT_Deal
 *
 * @brief   Deal device Endpoint 2 OUT.
 *
 * @param   l - Data length.
 *
 * @return  none
 */
void DevEPhs_OUT_Deal(UINT8 l) {
    UINT8 s, EP2_Tx_Cnt;
    memcpy(EP2_Rx_Buffer, EP2_Rx_Databuf, l);

    s = RecData_Deal();
    if (s != ERR_End) {
        EP2_Tx_Databuf[0] = 0x00;
        if (s == ERR_ERROR)
				{
            EP2_Tx_Databuf[1] = 0x01;
				}
        else
				{
            EP2_Tx_Databuf[1] = 0x00;
				}
        EP2_Tx_Cnt = 2;
        DevEPhs_IN_Deal(EP2_Tx_Cnt);

    }
}



