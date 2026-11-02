/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_usbotg_device.c
* Author             : WCH
* Version            : V1.0.1
* Date               : 2026/10/08
* Description        : This file provides all the USBOTG firmware functions.
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* SPDX-License-Identifier: Apache-2.0
*******************************************************************************/
#include "Motor_UsbData.h"

/* Global define */
/* OTH */
#define pMySetupReqPakHD        ((PUSB_SETUP_REQ)EP0_DatabufHD)
#define RepDescSize             62
#define DevEP0SIZE              8
#define PID_OUT                 0
#define PID_SOF                 1
#define PID_IN                  2
#define PID_SETUP               3


/******************************************************************************/
/* Global variables */
/* Endpoint Buffer */
__attribute__ ((aligned(4))) UINT8 EP0_DatabufHD[64]; //ep0(64)

__attribute__ ((aligned(4))) UINT8 EP2_DatabufHD[64+64];  //ep2_out(64)+ep2_in(64)


PUINT8  pEP0_RAM_Addr;                       //ep0(64)
PUINT8  pEP2_RAM_Addr;                       //ep2_out(64)+ep2_in(64)

const UINT8 *pDescr;
volatile UINT8  USBHD_Dev_SetupReqCode = 0xFF;                                  /* USB2.0 high-speed device Setup packet command code */
volatile UINT16 USBHD_Dev_SetupReqLen = 0x00;                                   /* USB2.0 high-speed device Setup packet length */
volatile UINT8  USBHD_Dev_SetupReqValueH = 0x00;                                /* USB2.0 high-speed device Setup packet Value high byte */
volatile UINT8  USBHD_Dev_Config = 0x00;                                        /* USB2.0 high-speed device configuration value */
volatile UINT8  USBHD_Dev_Address = 0x00;                                       /* USB2.0 high-speed device address value */
volatile UINT8  USBHD_Dev_SleepStatus = 0x00;                                   /* USB2.0 high-speed device sleep status */
volatile UINT8  USBHD_Dev_EnumStatus = 0x00;                                    /* USB2.0 high-speed device enumeration status */
volatile UINT8  USBHD_Dev_Endp0_Tog = 0x01;                                     /* USB2.0 high-speed device endpoint 0 toggle flag */
volatile UINT8  USBHD_Dev_Speed = 0x00;                                         /* USB2.0 high-speed device speed */

volatile UINT16 USBHD_Endp1_Up_Flag = 0x00;                                     /* USB2.0 high-speed device endpoint 1 data upload status: 0: idle; 1: uploading; */
volatile UINT8  USBHD_Endp1_Down_Flag = 0x00;                                   /* USB2.0 high-speed device endpoint 1 download success flag */
volatile UINT8  USBHD_Endp1_Down_Len = 0x00;                                    /* USB2.0 high-speed device endpoint 1 download length */
volatile BOOL   USBHD_Endp1_T_Tog = 0;                                          /* USB2.0 high-speed device endpoint 1 transmit tog bit toggle */
volatile BOOL   USBHD_Endp1_R_Tog = 0;

volatile UINT16 USBHD_Endp2_Up_Flag = 0x00;                                     /* USB2.0 high-speed device endpoint 2 data upload status: 0: idle; 1: uploading; */
volatile UINT16 USBHD_Endp2_Up_LoadPtr = 0x00;                                  /* USB2.0 high-speed device endpoint 2 data upload load offset */
volatile UINT8  USBHD_Endp2_Down_Flag = 0x00;                                   /* USB2.0 high-speed device endpoint 2 download success flag */

volatile UINT32V Endp2_send_seq = 0x00;
volatile UINT8   DevConfig;
volatile UINT8   SetupReqCode;
volatile UINT16  SetupReqLen;

volatile UINT8   em_status = 0;
/******************************************************************************/
/* Device Descriptor */
const UINT8  MyDevDescrHD[] =
{
        0x12,0x01,0x10,0x01,0xff,0x00,0x02,DevEP0SIZE,                   //Device descriptor
        0x86,0x1a,0x23,0x55,0x04,0x03,0x00,0x00,
        0x00,0x01
};


/* Configration Descriptor */
const UINT8  MyCfgDescrHD[] =
{
        0x09,0x02,0x27,0x00,0x01,0x01,0x00,0x80,0xf0,              //Configuration descriptor, interface descriptor, endpoint descriptor
        0x09,0x04,0x00,0x00,0x03,0xff,0x01,0x02,0x00,
        0x07,0x05,0x82,0x02,0x20,0x00,0x00,                        //Bulk upload endpoint
        0x07,0x05,0x02,0x02,0x20,0x00,0x00,                        //Bulk download endpoint
        0x07,0x05,0x81,0x03,0x08,0x00,0x01
};



/* Language Descriptor */
const UINT8  MyLangDescrHD[] =
{
        0x04, 0x03, 0x09, 0x04
};

/* Manufactor Descriptor */
const UINT8  MyManuInfoHD[] =
{
        0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0
};

/* Product Information */
const UINT8  MyProdInfoHD[] =
{
        0x0C, 0x03, 'C', 0, 'H', 0, '1', 0, '0', 0, 'x', 0
};
const UINT8 DataBuf[26]={0x30,0x00,0xc3,0x00,0xff,0xec,0x9f,0xec,0xff,0xec,0xdf,0xec,
                    0xdf,0xec,0xdf,0xec,0x9f,0xec,0x9f,0xec,0x9f,0xec,0x9f,0xec,
                    0xff,0xec};


/* USB full-speed mode, other speed configuration descriptor */
UINT8 TAB_USB_FS_OSC_DESC[ sizeof( MyCfgDescrHD ) ] =
{
    0x09, 0x07,                                                                 /* Other parts are copied by the program */
};

t_sendUsbData sendUsbData = {0,0,0,0,0,0,0};


const UINT8 usbFrameByteNum[12] = {0, 32, 32, 28, 28, 24, 28, 32, 20, 22, 24, 26};
const UINT8 needSendLen[12] = {0, USB_NEEDSEND_LEN1, USB_NEEDSEND_LEN2, USB_NEEDSEND_LEN3, USB_NEEDSEND_LEN4, USB_NEEDSEND_LEN5,
        USB_NEEDSEND_LEN6, USB_NEEDSEND_LEN7, USB_NEEDSEND_LEN8, USB_NEEDSEND_LEN9, USB_NEEDSEND_LEN10, USB_NEEDSEND_LEN11};
//void USBHD_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      USBOTG_FS_DeviceInit
 *
 * @brief   Initializes USB device.
 *
 * @return  none
 */
/*
void MOTOR_USBDeviceInit( void )
{
    USBOTG_FS->BASE_CTRL = 0x00;
    USBOTG_FS->UEP2_3_MOD = USBHD_UEP2_RX_EN|USBHD_UEP2_TX_EN;

    USBOTG_FS->UEP0_DMA = (UINT16)(UINT32)pEP0_RAM_Addr;
    USBOTG_FS->UEP2_DMA = (UINT16)(UINT32)pEP2_RAM_Addr;
    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_RES_ACK;
    USBOTG_FS->UEP2_RX_CTRL = USBHD_UEP_R_RES_ACK;


    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_RES_NAK;
    USBOTG_FS->UEP1_TX_LEN = 8;
    USBOTG_FS->UEP2_TX_LEN = 8;
    USBOTG_FS->UEP3_TX_LEN = 8;
    USBOTG_FS->UEP4_TX_LEN = 8;
    USBOTG_FS->UEP5_TX_LEN = 8;
    USBOTG_FS->UEP6_TX_LEN = 8;
    USBOTG_FS->UEP7_TX_LEN = 8;

    USBOTG_FS->UEP1_TX_CTRL = USBHD_UEP_T_RES_NAK;
    USBOTG_FS->UEP2_TX_CTRL = USBHD_UEP_T_RES_NAK|USBHD_UEP_AUTO_TOG;
    USBOTG_FS->UEP3_TX_CTRL = USBHD_UEP_T_RES_NAK|USBHD_UEP_AUTO_TOG;
    USBOTG_FS->UEP4_TX_CTRL = USBHD_UEP_T_RES_NAK|USBHD_UEP_AUTO_TOG;
    USBOTG_FS->UEP5_TX_CTRL = USBHD_UEP_T_RES_NAK|USBHD_UEP_AUTO_TOG;
    USBOTG_FS->UEP6_TX_CTRL = USBHD_UEP_T_RES_NAK|USBHD_UEP_AUTO_TOG;
    USBOTG_FS->UEP7_TX_CTRL = USBHD_UEP_T_RES_NAK|USBHD_UEP_AUTO_TOG;


    USBOTG_FS->INT_FG   = 0xFF;
    USBOTG_FS->INT_EN   = USBHD_UIE_SUSPEND | USBHD_UIE_BUS_RST | USBHD_UIE_TRANSFER;
    USBOTG_FS->DEV_ADDR = 0x00;

    USBOTG_FS->BASE_CTRL = USBHD_UC_DEV_PU_EN | USBHD_UC_INT_BUSY | USBHD_UC_DMA_EN;
    USBOTG_FS->UDEV_CTRL = USBHD_UD_PD_DIS|USBHD_UD_PORT_EN;
    em_status = 0;

}

/*********************************************************************
 * @fn      MOTOR_USBDeviceInit
 *
 * @brief   Initializes the usbotg clock configuration.
 *
 * @return  none
 */
/*
void MOTOR_USB_RCC_Init( void )
{
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div3);
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_OTG_FS, ENABLE );
}

/*********************************************************************
 * @fn      Motor_Usb_Init
 *
 * @brief   Initializes the USBOTG full speed device.
 *
 * @return  none
 */
/*
void Motor_Usb_Init( void )
{

    //Endpoint buffer initialization //
    pEP0_RAM_Addr = EP0_DatabufHD;
    pEP2_RAM_Addr = EP2_DatabufHD;

    // Enable USB clock //
    MOTOR_USB_RCC_Init( );
    Delay_Us(100);
    // USB device initialization //
    MOTOR_USBDeviceInit( );

    EXTEN->EXTEN_CTR &= ~EXTEN_USBD_PU_EN;
    Delay_Ms(50);
    EXTEN->EXTEN_CTR |= EXTEN_USBD_PU_EN;
    // Enable USB interrupt//
    //USB interrupt configuration
}

/*********************************************************************
 * @fn      OTG_FS_IRQHandler
 *
 * @brief   This function handles OTG_FS exception.
 *
 * @return  none
 */
/*
void USBHD_IRQHandler( void )
{
    UINT8  len, chtype;
    UINT8  intflag, errflag = 0;
    static UINT8   num = 0;
    intflag = USBOTG_FS->INT_FG;

    if( intflag & USBHD_UIF_TRANSFER )
    {
        switch ( USBOTG_FS->INT_ST & USBHD_UIS_TOKEN_MASK )
        {
            // SETUP packet handling //
            case USBHD_UIS_TOKEN_SETUP:

                USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_TOG|USBHD_UEP_T_RES_NAK;
                USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_TOG|USBHD_UEP_R_RES_ACK;
                SetupReqLen  = pSetupReqPakHD->wLength;
                SetupReqCode = pSetupReqPakHD->bRequest;
                chtype = pSetupReqPakHD->bRequestType;


                len = 0;
                errflag = 0;
                // Check whether the current request is a standard request or another request
                if ( ( pSetupReqPakHD->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
                {
                    if ( ( chtype & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
                    {
                        if(chtype == 0xc0)
                        {
                            EP0_DatabufHD[0] = DataBuf[num];
                            EP0_DatabufHD[1] = DataBuf[num+1];
                            len = 2;
                            if(num<24)
                            {
                                num += 2;
                            }
                            else
                            {
                                num = 24;
                            }
                        }
                        else if(chtype == 0x40)
                        {
                            len = 9;
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                    }
                }
                else
                {
                    // Handle standard USB request packet //
                    switch( SetupReqCode )
                    {
                        case USB_GET_DESCRIPTOR:
                        {
                            switch( ((pSetupReqPakHD->wValue)>>8) )
                            {
                                case USB_DESCR_TYP_DEVICE:
                                   // Get device descriptor //
                                    pDescr = MyDevDescrHD;
                                    len = MyDevDescrHD[0];
                                    break;

                                case USB_DESCR_TYP_CONFIG:
                                    // Get configuration descriptor//
                                    pDescr = MyCfgDescrHD;
                                    len = MyCfgDescrHD[2];
                                    break;

                                case USB_DESCR_TYP_STRING:
                                    // Get string descriptor //
                                    switch( (pSetupReqPakHD->wValue)&0xff )
                                    {
                                        case 0:
                                            // Language string descriptor //
                                        pDescr = MyLangDescrHD;
                                        len = MyLangDescrHD[0];
                                            break;

                                        case 1:
                                            //USB manufacturer string descriptor//
                                            pDescr = MyManuInfoHD;
                                            len = MyManuInfoHD[0];
                                            break;

                                        case 2:
                                            // USB产品字符串描述符 //
                                            pDescr = MyProdInfoHD;
                                            len = MyProdInfoHD[0];
                                            break;

                                        default:
                                            errflag = 0xFF;
                                            break;
                                    }
                                    break;
                                    case USB_DESCR_TYP_SPEED:
                                        // Other speed configuration descriptor//
                                        //Other speed configuration descriptor //
                                        if( USBHD_Dev_Speed == 0x00 )
                                        {
                                          // Full-speed mode//
                                          memcpy( &TAB_USB_FS_OSC_DESC[ 2 ], &MyCfgDescrHD[ 2 ], sizeof( MyCfgDescrHD ) - 2 );
                                          pDescr = ( PUINT8 )&TAB_USB_FS_OSC_DESC[ 0 ];
                                          len = sizeof( TAB_USB_FS_OSC_DESC );
                                        }
                                        else
                                        {
                                          errflag = 0xFF;
                                        }
                                        break;
                                default :
                                    errflag = 0xff;
                                    break;
                            }
                            if( SetupReqLen>len )   SetupReqLen = len;
                            len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                            memcpy( pEP0_DataBuf, pDescr, len );
                            pDescr += len;
                        }
                            break;

                        case USB_SET_ADDRESS:
                            //Set address //
                            SetupReqLen = (pSetupReqPakHD->wValue)&0xff;
                            break;

                        case USB_GET_CONFIGURATION:
                            //Get configuration value //
                            pEP0_DataBuf[0] = DevConfig;
                            if ( SetupReqLen > 1 ) SetupReqLen = 1;
                            break;

                        case USB_SET_CONFIGURATION:
                            // Set configuration value//
                            em_status = 1;
                            DevConfig = (pSetupReqPakHD->wValue)&0xff;
                            break;

                        case USB_CLEAR_FEATURE:
                            //Clear feature//
                            if ( ( pSetupReqPakHD->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
                            {
                                // Clear endpoint//
                                switch( (pSetupReqPakHD->wIndex)&0xff )
                                {
                                case 0x82:
                                    USBOTG_FS->UEP2_TX_CTRL = (USBOTG_FS->UEP2_TX_CTRL & ~( USBHD_UEP_T_TOG|USBHD_UEP_T_RES_MASK )) | USBHD_UEP_T_RES_NAK;
                                    break;

                                case 0x02:
                                    USBOTG_FS->UEP2_RX_CTRL = (USBOTG_FS->UEP2_RX_CTRL & ~( USBHD_UEP_R_TOG|USBHD_UEP_R_RES_MASK )) | USBHD_UEP_R_RES_ACK;
                                    break;

                                case 0x81:
                                    USBOTG_FS->UEP1_TX_CTRL = (USBOTG_FS->UEP1_TX_CTRL & ~( USBHD_UEP_T_TOG|USBHD_UEP_T_RES_MASK )) | USBHD_UEP_T_RES_NAK;
                                    break;

                                case 0x01:
                                    USBOTG_FS->UEP1_RX_CTRL = (USBOTG_FS->UEP1_RX_CTRL & ~( USBHD_UEP_R_TOG|USBHD_UEP_R_RES_MASK )) | USBHD_UEP_R_RES_ACK;
                                    break;

                                default:
                                    errflag = 0xFF;
                                    break;

                                }
                            }
                            else    errflag = 0xFF;
                            break;
                        case USB_GET_INTERFACE:
                            // 获取接口 //
                            pEP0_DataBuf[0] = 0x00;
                            if ( SetupReqLen > 1 ) SetupReqLen = 1;
                            break;


                        case USB_GET_STATUS:
                            // Respond according to the actual state of the current endpoint //
                            EP0_DatabufHD[ 0 ] = 0x00;
                            EP0_DatabufHD[ 1 ] = 0x00;
                            if( USBHD_Dev_SetupReqLen > 2 )
                            {
                                USBHD_Dev_SetupReqLen = 2;
                            }
                            break;

                        default:
                            errflag = 0xff;
                            break;
                    }
                }
                if( errflag == 0xff)
                {
                    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_TOG|USBHD_UEP_T_RES_STALL;
                    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_TOG|USBHD_UEP_R_RES_STALL;
                }
                else
                {
                    if( chtype & 0x80 )
                    {
                        len = (SetupReqLen>DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                        SetupReqLen -= len;
                    }
                    else  len = 0;

                    USBOTG_FS->UEP0_TX_LEN  = len;
                    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_TOG|USBHD_UEP_T_RES_ACK;
                    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_TOG|USBHD_UEP_R_RES_ACK;
                }
                break;

            case USBHD_UIS_TOKEN_IN:
                switch ( USBOTG_FS->INT_ST & ( USBHD_UIS_TOKEN_MASK | USBHD_UIS_ENDP_MASK ) )
                {
                    case USBHD_UIS_TOKEN_IN:
                        switch( SetupReqCode )
                        {
                            case USB_GET_DESCRIPTOR:
                                    len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen;
                                    memcpy( pEP0_DataBuf, pDescr, len );
                                    SetupReqLen -= len;
                                    pDescr += len;
                                    USBOTG_FS->UEP0_TX_LEN   = len;
                                    USBOTG_FS->UEP0_TX_CTRL ^= USBHD_UEP_T_TOG;
                                    break;

                            case USB_SET_ADDRESS:
                                    USBOTG_FS->DEV_ADDR = (USBOTG_FS->DEV_ADDR&USBHD_UDA_GP_BIT) | SetupReqLen;
                                    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_RES_NAK;
                                    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_RES_ACK;
                                    break;

                            default:
                                    USBOTG_FS->UEP0_TX_LEN = 0;
                                    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_RES_NAK;
                                    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_RES_ACK;
                                    break;

                        }
                        break;


                case USBHD_UIS_TOKEN_IN | 2:
                    USBOTG_FS->UEP2_TX_CTRL = (USBOTG_FS->UEP2_TX_CTRL & ~USBHD_UEP_T_RES_MASK) | USBHD_UEP_T_RES_NAK;
                    break;


                default :
                    break;

                }
                break;

            case USBHD_UIS_TOKEN_OUT:
                switch ( USBOTG_FS->INT_ST & ( USBHD_UIS_TOKEN_MASK | USBHD_UIS_ENDP_MASK ) )
                {
                    case USBHD_UIS_TOKEN_OUT:
                            len = USBOTG_FS->RX_LEN;
                            break;



                    case USBHD_UIS_TOKEN_OUT | 2:
                        if ( USBOTG_FS->INT_ST & USBHD_UIS_TOG_OK )
                        {
                           // USBOTG_FS->UEP2_RX_CTRL ^= USBHD_UEP_R_TOG;
                            len = USBOTG_FS->RX_LEN;
                            DevEP2_OUT_Deal( len );
                        }
                        break;

//
                }

                break;

            case USBHD_UIS_TOKEN_SOF:

                break;

            default :
                break;

        }

        USBOTG_FS->INT_FG = USBHD_UIF_TRANSFER;
    }
    else if( intflag & USBHD_UIF_BUS_RST )
    {
        if (em_status)
        {
           // do nothing
        }
        else
        {
            USBOTG_FS->DEV_ADDR = 0;

            USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_RES_ACK;
            USBOTG_FS->UEP1_RX_CTRL = USBHD_UEP_R_RES_ACK;
            USBOTG_FS->UEP2_RX_CTRL = USBHD_UEP_R_RES_ACK;
            USBOTG_FS->UEP3_RX_CTRL = USBHD_UEP_R_RES_ACK;
            USBOTG_FS->UEP4_RX_CTRL = USBHD_UEP_R_RES_ACK;
            USBOTG_FS->UEP5_RX_CTRL = USBHD_UEP_R_RES_ACK;
            USBOTG_FS->UEP6_RX_CTRL = USBHD_UEP_R_RES_ACK;
            USBOTG_FS->UEP7_RX_CTRL = USBHD_UEP_R_RES_ACK;

            USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_RES_NAK;
            USBOTG_FS->UEP1_TX_CTRL = USBHD_UEP_T_RES_NAK;
            USBOTG_FS->UEP2_TX_CTRL = USBHD_UEP_T_RES_NAK|USBHD_UEP_AUTO_TOG;
            USBOTG_FS->UEP3_TX_CTRL = USBHD_UEP_T_RES_NAK;
            USBOTG_FS->UEP4_TX_CTRL = USBHD_UEP_T_RES_NAK;
            USBOTG_FS->UEP5_TX_CTRL = USBHD_UEP_T_RES_NAK;
            USBOTG_FS->UEP6_TX_CTRL = USBHD_UEP_T_RES_NAK;
            USBOTG_FS->UEP7_TX_CTRL = USBHD_UEP_T_RES_NAK;
        }

        USBOTG_FS->INT_FG |= USBHD_UIF_BUS_RST;
    }
    else if( intflag & USBHD_UIF_SUSPEND )
    {

        if ( USBOTG_FS->MIS_ST & USBHD_UMS_SUSPEND ) {;}
        else{;}
        USBOTG_FS->INT_FG = USBHD_UIF_SUSPEND;
    }
    else
    {
        USBOTG_FS->INT_FG = intflag;
    }
}

/*********************************************************************
 * @fn      DevEP1_IN_Deal
 *
 * @brief   Device endpoint1 IN.
 *
 * @param   l - IN length(<64B)
 *
 * @return  none
 */
/*
void DevEP1_IN_Deal( UINT8 l )
{
    USBOTG_FS->UEP1_TX_LEN = l;
    USBOTG_FS->UEP1_TX_CTRL  = (USBOTG_FS->UEP1_TX_CTRL & ~USBHD_UEP_T_RES_MASK)| USBHD_UEP_T_RES_ACK;
    USBOTG_FS->UEP1_TX_CTRL ^= USBHD_UEP_T_TOG;
    USBHD_Endp2_Up_Flag = 0x01;
}

/*********************************************************************
 * @fn      DevEP2_IN_Deal
 *
 * @brief   Device endpoint2 IN.
 *
 * @param   l - IN length(<64B)
 *
 * @return  none
 */
/*
void DevEP2_IN_Deal( UINT8 l )
{
    USBOTG_FS->UEP2_TX_LEN = l;
    USBOTG_FS->UEP2_TX_CTRL = (USBOTG_FS->UEP2_TX_CTRL & ~USBHD_UEP_T_RES_MASK)| USBHD_UEP_T_RES_ACK;
}




/*********************************************************************
 * @fn      DevEP2_OUT_Deal
 *
 * @brief   Deal device Endpoint 2 OUT.
 *
 * @param   l - Data length.
 *
 * @return  none
 */
/*
void DevEP2_OUT_Deal( UINT8 l )
{
    UINT8 i,j;
    UINT16 AddSum;
//    PUINT8 inOutAddress;
//    if(R8_UEP2_CTRL&RB_UEP_R_TOG)     //---The receive currently points to DATA1, so the data already received and pending processing is DATA0---
//    {
//        inOutAddress = pEP2_OUT_DataBuf0;
//    }
//    else
//    {
//        inOutAddress = pEP2_OUT_DataBuf1;
//    }
    for(i=0; i<l; i++)
    {
        if(sendUsbData.recDataLen == 0)
        {
            if(pEP2_OUT_DataBuf[i] == 0xAA)
            {
                sendUsbData.recDataBuf[0] = pEP2_OUT_DataBuf[i];
                sendUsbData.recDataLen = 1;
            }
        }
        else if(sendUsbData.recDataLen == 1)
        {
            if(pEP2_OUT_DataBuf[i] == 0x55)
            {
                sendUsbData.recDataBuf[1] = pEP2_OUT_DataBuf[i];
                sendUsbData.recDataLen = 2;
            }
            else
            {
                sendUsbData.recDataLen = 0;
            }
        }
        else if(sendUsbData.recDataLen == 2)
        {
            if(pEP2_OUT_DataBuf[i] <= 16)     //---帧长度---
            {
                sendUsbData.recDataBuf[2] = pEP2_OUT_DataBuf[i];
                sendUsbData.recDataLen = 3;
            }
            else
            {
                sendUsbData.recDataLen = 0;
            }
        }
        else
        {
            sendUsbData.recDataBuf[sendUsbData.recDataLen] = pEP2_OUT_DataBuf[i];
            sendUsbData.recDataLen++;
            if(sendUsbData.recDataLen >= sendUsbData.recDataBuf[2])
            {
                AddSum = 0;
                for(j = 0; j < sendUsbData.recDataBuf[2]-1; j++)
                {
                    AddSum += sendUsbData.recDataBuf[j];
                }
                sendUsbData.recDataLen = 0;
                if(sendUsbData.recDataBuf[sendUsbData.recDataBuf[2]-1] == (AddSum&0x00FF))   //---校验正确，则辨识是什么处理---
                {
                    switch(sendUsbData.recDataBuf[3])
                    {
                        case 0x01:      //---Set the data fetch channel and upload data--- sendDataP---
                            AddSum = 0;
                            sendUsbData.recDataBuf[3] = 0x81;
                            for(j = 0; j < sendUsbData.recDataBuf[2]-1; j++)
                            {
                                pEP2_IN_DataBuf[j] = sendUsbData.recDataBuf[j];
                                AddSum += sendUsbData.recDataBuf[j];
                            }
                            pEP2_IN_DataBuf[sendUsbData.recDataBuf[2]-1] = (AddSum&0x00FF);
                            //----发送----
                            USBOTG_FS->UEP2_TX_LEN = sendUsbData.recDataBuf[2];
                            USBOTG_FS->UEP2_TX_CTRL = (USBOTG_FS->UEP2_TX_CTRL & ~USBHD_UEP_T_RES_MASK) | USBHD_UEP_T_RES_ACK;
                            //------A delay is needed to ensure that large-volume data transmission does not conflict with returning the 0x81 command------
                            sendUsbData.delayNum = 250;
                            sendUsbData.sendDataLen = sendUsbData.recDataBuf[2] - 5;
                            for(j = 0; j < MaxUsbDataLen; j++)
                            {
                                sendUsbData.sendDataType[j] = 0;
                            }
                            for(j = 0; j < sendUsbData.sendDataLen; j++)
                            {
                                sendUsbData.sendDataType[j] = sendUsbData.recDataBuf[j+4]-1;
                            }
                            //sendUsbData.finishFlag = 1;
                            break;
                        case 0x02:      //---关闭上传数据---
                            sendUsbData.sendDataLen = 0;
                            break;
                    }
                    break;     //---Break out of the current i loop----
                }
            }
        }
    }
}
*/
//__attribute__((section(".highcode")))
UINT8 getUsbBufLen(void)
{
    UINT8 reDataLen;
    if(sendUsbData.head >= sendUsbData.end)
    {
        reDataLen = sendUsbData.head - sendUsbData.end;
    }
    else
    {
        reDataLen = (UINT16)USB_SENDBUF_LEN - sendUsbData.end + sendUsbData.head;
    }
    return reDataLen;
}
/*
void Deal_sendMonitorData(void)
{
    UINT8 i, frameByteNum;
    if(sendUsbData.delayNum == 0)
    {
        //sendUsbData.delayNum = 100;
        if(sendUsbData.sendDataLen)
        {
            if(getUsbBufLen() >= needSendLen[sendUsbData.sendDataLen])
            {
                if(USBOTG_FS->UEP2_TX_CTRL&USBHD_UEP_T_RES_NAK)
                {
                    frameByteNum = 4;
                    while(frameByteNum < usbFrameByteNum[sendUsbData.sendDataLen])
                    {
                        for(i = 0; i < sendUsbData.sendDataLen; i++)
                        {
                              pEP2_IN_DataBuf[frameByteNum] = (UINT8)((sendUsbData.sendDataBuf[sendUsbData.end+i])&0x00FF);
                              frameByteNum++;
                              pEP2_IN_DataBuf[frameByteNum] = (UINT8)((sendUsbData.sendDataBuf[sendUsbData.end+i])>>8);
                              frameByteNum++;
                        }
                        sendUsbData.end += MaxUsbDataLen;
                    }
                    sendUsbData.heartBeat++;
                    pEP2_IN_DataBuf[0] = 0xAA;
                    pEP2_IN_DataBuf[1] = 0x55;
                    pEP2_IN_DataBuf[2] = usbFrameByteNum[sendUsbData.sendDataLen];
                    pEP2_IN_DataBuf[3] = sendUsbData.heartBeat;
                    USBOTG_FS->UEP2_TX_LEN = usbFrameByteNum[sendUsbData.sendDataLen];
                    USBOTG_FS->UEP2_TX_CTRL = (USBOTG_FS->UEP2_TX_CTRL & ~USBHD_UEP_T_RES_MASK) | USBHD_UEP_T_RES_ACK;
                }
            }
        }
    }
    else
    {
        sendUsbData.delayNum--;
    }
}
*/
__attribute__((section(".highcode")))
void SPI_Deal_sendMonitorData(void)
{
    u_int16_t data=0;
    u_int8_t i=0, frameByteNum=0;
    UINT8 reDataLen;
    if(sendUsbData.head >= sendUsbData.end)
    {
        reDataLen = sendUsbData.head - sendUsbData.end;
    }
    else
    {
        reDataLen = (UINT16)USB_SENDBUF_LEN - sendUsbData.end + sendUsbData.head;
    }
    sendUsbData.sendDataLen=6;
    if(sendUsbData.sendDataLen)
    {
        if(reDataLen >= needSendLen[sendUsbData.sendDataLen])//头减尾>=14 且缓冲区为空
        {
            GPIO_ResetBits(GPIOB,GPIO_Pin_8);                              //拉低片选 告诉从机准备接收
            __asm volatile ("nop");                                         //Wait for data to pass through the optocoupler
            while((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);     //Wait for the transmit buffer to be empty

            SPI1->DATAR = 0x55AA;                                           //包头AA55
            while((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);     //Wait for the transmit buffer to be empty

            sendUsbData.heartBeat++;
            data=sendUsbData.heartBeat<<8;
            data|=usbFrameByteNum[sendUsbData.sendDataLen];
            SPI1->DATAR = data;                                            //心跳包+发送长度
            while((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);    //Wait for the transmit buffer to be empty


            frameByteNum = 4;
            while(frameByteNum < usbFrameByteNum[sendUsbData.sendDataLen])//{0, 32, 32, 28, 28, 24, 28, 32, 20, 22, 24, 26};
            {
                for(i = 0; i < sendUsbData.sendDataLen; i++)
                {
                    SPI1->DATAR=sendUsbData.sendDataBuf[sendUsbData.end+i];
                    while((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);    //Wait for the transmit buffer to be empty
                    frameByteNum+=2;
                }
                sendUsbData.end += MaxUsbDataLen;
            }

            while( (SPI1->STATR & SPI_I2S_FLAG_BSY) != (uint16_t)RESET);    //判断是否发送完毕 发送完毕
            __asm volatile ("nop");                                         //等待数据通过光耦
            GPIO_SetBits(GPIOB,GPIO_Pin_8);                                //Pull chip select high
                                          //Accumulate the tail pointer
        }
    }
}
void SPI_FullDuplex_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    SPI_InitTypeDef SPI_InitStructure={0};

    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOA | RCC_PB2Periph_GPIOB | RCC_PB2Periph_SPI1|RCC_PB2Periph_USART1|RCC_PB2Periph_AFIO, ENABLE );
    GPIO_PinRemapConfig(GPIO_PartialRemap1_SPI1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOB, &GPIO_InitStructure );//SCK

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//NSS作为GPIO口使用，设置为推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB,GPIO_Pin_8);    //拉高片选

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//2-wire full duplex
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;     //主机
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;//16位
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;//72/4=16M
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
    SPI_Init( SPI1, &SPI_InitStructure );

    GPIO_ResetBits(GPIOB,GPIO_Pin_8); //Pull chip select low
    SPI_Cmd( SPI1, ENABLE );           //启动SPI
}
/******************************************************************************
* Function Name  : Usart_Init
* Description    : SPI initialization
* Input          : uint32_t Baud 4000000
* Output         : None
* Return         : None
******************************************************************************/
void Usart_Init(uint32_t Baud)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOB|RCC_PB2Periph_USART1|RCC_PB2Periph_AFIO, ENABLE );
//    GPIO_PinRemapConfig(GPIO_PartialRemap1_USART1,ENABLE);
    GPIO_PinRemapConfig(GPIO_PartialRemap4_USART1,ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = Baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx ;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}
/******************************************************************************
* Function Name  : Usart_Deal_sendMonitorData
* Description    : Usart发送
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void Usart_Deal_sendMonitorData(void)
{
    u_int8_t i=0, frameByteNum=0;
    sendUsbData.sendDataLen=6;
    if(sendUsbData.sendDataLen)
    {
        if(getUsbBufLen() >= needSendLen[sendUsbData.sendDataLen])//头减尾>=14 且缓冲区为空
        {
            __asm volatile ("nop");                                         //等待数据通过光耦
            while((USART1->STATR & USART_FLAG_TXE) == (uint16_t)RESET);     //Wait for data to be transferred to the shift register
            USART1->DATAR=0xAA;
            while((USART1->STATR & USART_FLAG_TXE) == (uint16_t)RESET);      //Wait for data to be transferred to the shift register
            USART1->DATAR=0x55;
            while((USART1->STATR & USART_FLAG_TXE) == (uint16_t)RESET);      //Wait for data to be transferred to the shift register

            USART1->DATAR=usbFrameByteNum[sendUsbData.sendDataLen];
            while((USART1->STATR & USART_FLAG_TXE) == (uint16_t)RESET);      //等待数据转移到移位寄存器
            sendUsbData.heartBeat++;
            USART1->DATAR=sendUsbData.heartBeat;
            while((USART1->STATR & USART_FLAG_TXE) == (uint16_t)RESET);      //等待数据转移到移位寄存器

            frameByteNum = 4;
            while(frameByteNum < usbFrameByteNum[sendUsbData.sendDataLen])//{0, 32, 32, 28, 28, 24, 28, 32, 20, 22, 24, 26};
            {
                for(i = 0; i < sendUsbData.sendDataLen; i++)
                {
                    USART1->DATAR = (u_int8_t)((sendUsbData.sendDataBuf[sendUsbData.end+i])&0x00FF);
                    frameByteNum++;
                    while((USART1->STATR & USART_FLAG_TXE) == (uint16_t)RESET);      //等待数据转移到移位寄存器
                    USART1->DATAR = (u_int8_t)((sendUsbData.sendDataBuf[sendUsbData.end+i])>>8);
                    frameByteNum++;
                    while((USART1->STATR & USART_FLAG_TXE) == (uint16_t)RESET);      //等待数据转移到移位寄存器
                }
                sendUsbData.end += MaxUsbDataLen;
            }

            while((USART1->STATR & USART_FLAG_TXE) == (uint16_t)RESET);      //等待数据转移到移位寄存器
            __asm volatile ("nop");                                         //等待数据通过光耦
        }
    }
}
