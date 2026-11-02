/* 2014.09.09
*****************************************
**   Copyright  (C)  W.ch  1999-2019   **
**   Web:      http://wch.cn           **
*****************************************
**  USB-flash File Interface for CHRV3 **
**  KEIL423, gcc 8.20          **
*****************************************
*/
/* CHRV3 U disk host file system interface, supports: FAT12/FAT16/FAT32 */

//#define DISK_BASE_BUF_LEN		512	/* The default disk data buffer size is512 bytes((selectable as2048 or even4096 to supportU disks with large sectors),if0, defining the buffer in this file is prohibited; the application specifies it inpDISK_BASE_BUF */
/* If the disk data buffer needs to be reused to saveRAM,, thenDISK_BASE_BUF_LEN can be defined as0 to disable defining the buffer in this file,, and the application shall, before callingCHRV3LibInit, put the start address of the shared buffer into thepDISK_BASE_BUF variable */

//#define NO_DEFAULT_ACCESS_SECTOR	    1		/* Disable the default disk sector read/write routines,replace them with self-written routines below */
//#define NO_DEFAULT_DISK_CONNECT		1		/* Disable the default disk connection check routine,replace it with a self-written routine below */
//#define NO_DEFAULT_FILE_ENUMER		1		/* Disable the default file name enumeration callback routine,replace it with a self-written routine below */
//#include "CHRV3SFR.H"

#include "debug.h"
#include "ch32l103.h"
#include "usb_host_config.h"
#include "CHRV3UFI.h"
#include "ch32l103_usbfs_host.h"

uint8_t USBHostTransact( uint8_t endp_pid, uint8_t tog, uint32_t timeout )
{
    uint8_t  r, trans_rerty;
    uint16_t i;
    USBFSH->HOST_TX_CTRL = USBFSH->HOST_RX_CTRL = 0;
    if( tog & 0x80 )
    {
        USBFSH->HOST_RX_CTRL = 1<<2;
    }
    if( tog & 0x40 )
    {
        USBFSH->HOST_TX_CTRL = 1<<2;
    }
    trans_rerty = 0;
    do
    {
        USBFSH->HOST_EP_PID = endp_pid;       // Specify token PID and endpoint number
        USBFSH->INT_FG = USBFS_UIF_TRANSFER;  // Allow transmission
        for( i = DEF_WAIT_USB_TOUT_200US; ( i != 0 ) && ( ( USBFSH->INT_FG & USBFS_UIF_TRANSFER ) == 0 ); i-- )
        {
            Delay_Us( 1 );
        }
        USBFSH->HOST_EP_PID = 0x00;  // Stop USB transfer
        if( ( USBFSH->INT_FG & USBFS_UIF_TRANSFER ) == 0 )
        {
            return ERR_USB_UNKNOWN;
        }
        else
        {
            /* Complete transfer */
            if( USBFSH->INT_ST & USBFS_UIS_TOG_OK )
            {
                return ERR_SUCCESS;
            }
            r = USBFSH->INT_ST & USBFS_UIS_H_RES_MASK;  // USB device answer status
            if( r == USB_PID_STALL )
            {
                return ( r | ERR_USB_TRANSFER );
            }
            if( r == USB_PID_NAK )
            {
                if( timeout == 0 )
                {
                    return ( r | ERR_USB_TRANSFER );
                }
                if( timeout < 0xFFFF )
                {
                    timeout--;
                }
                --trans_rerty;
            }
            else switch ( endp_pid >> 4 )
            {
                case USB_PID_SETUP:
                case USB_PID_OUT:
                    if( r )
                    {
                        return ( r | ERR_USB_TRANSFER );
                    }
                    break;
                case USB_PID_IN:
                    if( ( r == USB_PID_DATA0 ) && ( r == USB_PID_DATA1 ) )
                    {
                        ;
                    }
                    else if( r )
                    {
                        return ( r | ERR_USB_TRANSFER );
                    }
                    break;
                default:
                    return ERR_USB_UNKNOWN;
            }
        }
        Delay_Us( 15 );
        if( USBFSH->INT_FG & USBFS_UIF_DETECT )
        {
            Delay_Us( 200 );
            if( USBFSH_CheckRootHubPortEnable( ) == 0 )
            {
                return ERR_USB_DISCON;  // USB device disconnect event
            }
        }
    }while( ++trans_rerty < 10 );

    return ERR_USB_TRANSFER; // Reply timeout
}

uint8_t HostCtrlTransfer( uint8_t *DataBuf, uint8_t *RetLen )
{
    uint8_t  ret;
    uint16_t retlen;
    retlen = (uint16_t)(*RetLen);
    ret = USBFSH_CtrlTransfer( RootHubDev[ DEF_USB_PORT_FS ].bEp0MaxPks, DataBuf, &retlen );
    return ret;
}

void CopySetupReqPkg( const char * pReqPkt )
{
    uint8_t i;
    for(i = 0; i != sizeof(USB_SETUP_REQ); i++)
    {
#if DEF_USB_PORT_FS_EN
        ((char *)pUSBFS_SetupRequest)[i] = *pReqPkt;
#elif DEF_USB_PORT_HS_EN
        ((char *)pUSBHS_SetupRequest)[i] = *pReqPkt;
#endif
        pReqPkt++;
    }
}

uint8_t CtrlGetDeviceDescrTB( void )
{
    uint8_t ret;
    ret = USBFSH_GetDeviceDescr( &RootHubDev[ DEF_USB_PORT_FS ].bEp0MaxPks, TxBuffer );
    return ret;
}

uint8_t CtrlGetConfigDescrTB( void )
{
    uint16_t len;
    uint8_t  ret;
    ret = USBFSH_GetConfigDescr( RootHubDev[ DEF_USB_PORT_FS ].bEp0MaxPks, TxBuffer, 256, &len );
    return ret;
}

uint8_t CtrlSetUsbConfig( uint8_t cfg )
{
    uint8_t ret;
    ret = USBFSH_SetUsbConfig( RootHubDev[ DEF_USB_PORT_FS ].bEp0MaxPks, cfg );
    return ret;
}

uint8_t CtrlSetUsbAddress( uint8_t addr )
{
    uint8_t ret;
    ret = USBFSH_SetUsbAddress( RootHubDev[ DEF_USB_PORT_FS ].bEp0MaxPks, addr );
    return ret;
}

uint8_t CtrlClearEndpStall( uint8_t endp )
{
    uint8_t ret;
    ret = USBFSH_ClearEndpStall( RootHubDev[ DEF_USB_PORT_FS ].bEp0MaxPks, endp );
    return ret;
}

#ifndef FOR_ROOT_UDISK_ONLY
uint8_t CtrlGetHubDescr( void )
{

}

uint8_t HubGetPortStatus( uint8_t HubPortIndex )
{

}

uint8_t HubSetPortFeature( uint8_t HubPortIndex, uint8_t FeatureSelt )
{

}

uint8_t HubClearPortFeature( uint8_t HubPortIndex, uint8_t FeatureSelt )
{

}
#endif

CMD_PARAM_I	mCmdParam;						/* Command parameters */
#if		DISK_BASE_BUF_LEN > 0
//uint8_t	DISK_BASE_BUF[ DISK_BASE_BUF_LEN ] __attribute__((at(BA_RAM+SZ_RAM/2)));	/* ExternalRAM disk data buffer,, with a length of one sector */
uint8_t	DISK_BASE_BUF[ DISK_BASE_BUF_LEN ] __attribute__((aligned (4)));	        /* ExternalRAM disk data buffer,, with a length of one sector */
//UINT8	DISK_FAT_BUF[ DISK_BASE_BUF_LEN ] __attribute__((aligned (4)));	            /* ExternalRAM diskFAT data buffer,, with a length of one sector */
#endif

/* The following routines can be modified as needed */

#ifndef	NO_DEFAULT_ACCESS_SECTOR		/* DefineNO_DEFAULT_ACCESS_SECTOR in the application to disable the default disk sector read/write routines,, then replace them with self-written routines */
//if ( use_external_interface ) {  // Replace theU disk sector low-level read/write routines
//    CHRV3vSectorSize=512;  // Set the actual sector size,it must be a multiple of512,this value is the sector size of the disk
//    CHRV3vSectorSizeB=9;   // Set the shift count of the actual sector size,512 corresponds to9,1024 corresponds to10,2048 corresponds to11
//    CHRV3DiskStatus=DISK_MOUNTED;  // Force the block device connection to succeed(only the file system remains to be analyzed)
//}

uint8_t	CHRV3ReadSector( uint8_t SectCount, uint8_t *DataBuf )  /* Read multiple sectors of data from the disk into the buffer */
{
    uint8_t	retry;
//	if ( use_external_interface ) return( extReadSector( CHRV3vLbaCurrent, SectCount, DataBuf ) );  /* External interface */
	for( retry = 0; retry < 3; retry ++ ) {  /* Retry on error */
		pCBW -> mCBW_DataLen = (uint32_t)SectCount << CHRV3vSectorSizeB;  /* Data transfer length */
		pCBW -> mCBW_Flag = 0x80;
		pCBW -> mCBW_LUN = CHRV3vCurrentLun;
		pCBW -> mCBW_CB_Len = 10;
		pCBW -> mCBW_CB_Buf[ 0 ] = SPC_CMD_READ10;
		pCBW -> mCBW_CB_Buf[ 1 ] = 0x00;
		pCBW -> mCBW_CB_Buf[ 2 ] = (uint8_t)( CHRV3vLbaCurrent >> 24 );
		pCBW -> mCBW_CB_Buf[ 3 ] = (uint8_t)( CHRV3vLbaCurrent >> 16 );
		pCBW -> mCBW_CB_Buf[ 4 ] = (uint8_t)( CHRV3vLbaCurrent >> 8 );
		pCBW -> mCBW_CB_Buf[ 5 ] = (uint8_t)( CHRV3vLbaCurrent );
		pCBW -> mCBW_CB_Buf[ 6 ] = 0x00;
		pCBW -> mCBW_CB_Buf[ 7 ] = 0x00;
		pCBW -> mCBW_CB_Buf[ 8 ] = SectCount;
		pCBW -> mCBW_CB_Buf[ 9 ] = 0x00;
		CHRV3BulkOnlyCmd( DataBuf );  /* Execute commands based onBulkOnly protocol */
		if ( CHRV3IntStatus == ERR_SUCCESS ) {
			return( ERR_SUCCESS );
		}
		CHRV3IntStatus = CHRV3AnalyzeError( retry );
		if ( CHRV3IntStatus != ERR_SUCCESS ) {
			return( CHRV3IntStatus );
		}
	}
	return( CHRV3IntStatus = ERR_USB_DISK_ERR );  /* Disk operation error */
}

#ifdef	EN_DISK_WRITE
uint8_t	CHRV3WriteSector( uint8_t SectCount, uint8_t *DataBuf )  /* Write the multi-sector data block in the buffer to the disk */
{
    uint8_t	retry;
//	if ( use_external_interface ) return( extWriteSector( CHRV3vLbaCurrent, SectCount, DataBuf ) );  /* External interface */
	for( retry = 0; retry < 3; retry ++ ) {  /* Retry on error */
		pCBW -> mCBW_DataLen = (uint32_t)SectCount << CHRV3vSectorSizeB;  /* Data transfer length */
		pCBW -> mCBW_Flag = 0x00;
		pCBW -> mCBW_LUN = CHRV3vCurrentLun;
		pCBW -> mCBW_CB_Len = 10;
		pCBW -> mCBW_CB_Buf[ 0 ] = SPC_CMD_WRITE10;
		pCBW -> mCBW_CB_Buf[ 1 ] = 0x00;
		pCBW -> mCBW_CB_Buf[ 2 ] = (uint8_t)( CHRV3vLbaCurrent >> 24 );
		pCBW -> mCBW_CB_Buf[ 3 ] = (uint8_t)( CHRV3vLbaCurrent >> 16 );
		pCBW -> mCBW_CB_Buf[ 4 ] = (uint8_t)( CHRV3vLbaCurrent >> 8 );
		pCBW -> mCBW_CB_Buf[ 5 ] = (uint8_t)( CHRV3vLbaCurrent );
		pCBW -> mCBW_CB_Buf[ 6 ] = 0x00;
		pCBW -> mCBW_CB_Buf[ 7 ] = 0x00;
		pCBW -> mCBW_CB_Buf[ 8 ] = SectCount;
		pCBW -> mCBW_CB_Buf[ 9 ] = 0x00;
		CHRV3BulkOnlyCmd( DataBuf );  /* Execute commands based onBulkOnly protocol */
		if ( CHRV3IntStatus == ERR_SUCCESS ) {
			Delay_Us( 200 );  /* Delay after write operation */
			return( ERR_SUCCESS );
		}
		CHRV3IntStatus = CHRV3AnalyzeError( retry );
		if ( CHRV3IntStatus != ERR_SUCCESS ) {
			return( CHRV3IntStatus );
		}
	}
	return( CHRV3IntStatus = ERR_USB_DISK_ERR );  /* Disk operation error */
}
#endif
#endif  // NO_DEFAULT_ACCESS_SECTOR

#ifndef	NO_DEFAULT_DISK_CONNECT			/* DefineNO_DEFAULT_DISK_CONNECT in the application to disable the default disk connection check routine,, then replace it with a self-written routine */

/*
Convention: USBUSB device address allocation rules(seeUSB_DEVICE_ADDR)
Address value  device position
0x02    Built-inRoot-HUB0 subordinateUSB device or external HUBHUB
0x03    Built-inRoot-HUB1 subordinateUSB device or external HUBHUB
0x1x    Built-inRoot-HUB0 subordinate externalHUB portx subordinateUSB device,x, x ranges 1~n1~n
0x2x    Built-inRoot-HUB1 subordinate externalHUB portx subordinateUSB device,x, x ranges 1~n1~n
*/
#if DEF_USB_PORT_FS_EN
#define		UHUB_DEV_ADDR	(USBFSH->DEV_ADDR)
#define		UHUB_MIS_STAT	(USBFSH->MIS_ST)
#define		UHUB_HOST_CTRL	(USBFSH->HOST_CTRL)
#define		UHUB_INT_FLAG	(USBFSH->INT_FG)
#define		bUMS_ATTACH		USBFS_UMS_DEV_ATTACH
#define		bUMS_SUSPEND	USBFS_UMS_SUSPEND
#define     DEF_ADR_OFFSET  0
#elif DEF_USB_PORT_HSS_EN
#define     UHUB_DEV_ADDR   (USBHSH->DEV_ADDR)
#define     UHUB_MIS_STAT   (USBHSH->MIS_ST)
#define     UHUB_HOST_CTRL  (USBHSH->HOST_CTRL)
#define     UHUB_INT_FLAG   (USBHSH->INT_FG)
#define     bUMS_ATTACH     USBHS_UMS_DEV_ATTACH
#define     bUMS_SUSPEND    USBHS_UMS_SUSPEND
#define     DEF_ADR_OFFSET  1
#endif

/* Check whether the disk is connected */
uint8_t	CHRV3DiskConnect( void )
{
    uint8_t	ums, devaddr;
	UHUB_DEV_ADDR = UHUB_DEV_ADDR & 0x7F;
	ums = UHUB_MIS_STAT;
	devaddr = UHUB_DEV_ADDR;
	if ( devaddr == (USB_DEVICE_ADDR + DEF_ADR_OFFSET) )
	{
	    /* Built-inRoot-HUB subordinateUSB device */
		if ( ums & bUMS_ATTACH )
		{
		    /* Built-inRoot-HUB subordinateUSB device present */
			if ( ( ums & bUMS_SUSPEND ) == 0 )
			{
			    /* Built-inRoot-HUB subordinateUSB device present and not unplugged */
				return( ERR_SUCCESS );  /* USB device already connected and not unplugged */
			}
			else
			{
			    /* Built-inRoot-HUB subordinateUSB device present and not unplugged */
				CHRV3DiskStatus = DISK_CONNECT;  /* Was disconnected before */
				return( ERR_SUCCESS );  /* ExternalHUB orUSB device connected, or reconnected after disconnection */
			}
		}
		else
		{
		    /* USB device disconnected */
mDiskDisconn:
			CHRV3DiskStatus = DISK_DISCONNECT;
			return( ERR_USB_DISCON );
		}
	}
	else
	{
		goto mDiskDisconn;
	}
}
#endif  // NO_DEFAULT_DISK_CONNECT

#ifndef	NO_DEFAULT_FILE_ENUMER			/* DefineNO_DEFAULT_FILE_ENUMER in the application to disable the default file name enumeration callback routine,, then replace it with a self-written routine */
void xFileNameEnumer( void )			/* File name enumeration callback routine */
{
/* If the specified enumeration indexCHRV3vFileSize is0xFFFFFFFF, callFileOpen afterwards; every time a file is foundFileOpen this callback routine will be called,
    callback routinexFileNameEnumer after returning,FileOpen decrementsCHRV3vFileSize and enumeration continues until no more files or directories are found. Recommended practice: 
   Before callingFileOpen, define a global variable as0, whenFileOpen calls back into this routine; fromCHRV3vFdtOffset get the structureFAT_DIR_INFO,
   AnalyzeDIR_Attr andDIR_Name in the structure to check whether it is the wanted file or directory name; record the info and increment the global counter,
   WhenFileOpen returns, check the return value: if it isERR_MISS_FILE orERR_FOUND_NAME, treat it as success; the global variable holds the number of valid files found.
   If, in this callback routine,xFileNameEnumer setsCHRV3vFileSize to1, thenFileOpen can be notified to end the search early. Below is a callback routine example */
#if	0
    uint8_t			i;
    uint16_t	    FileCount;
	PX_FAT_DIR_INFO	pFileDir;
	uint8_t			*NameBuf;
	pFileDir = (PX_FAT_DIR_INFO)( pDISK_BASE_BUF + CHRV3vFdtOffset );  /* CurrentFDT's start address */
	FileCount = (UINT16)( 0xFFFFFFFF - CHRV3vFileSize );  /* The current enumeration index of the file name,CHRV3vFileSize, initially0xFFFFFFFF,, decremented after a file name is found */
	if ( FileCount < sizeof( FILE_DATA_BUF ) / 12 ) {  /* Check whether the buffer is large enough to hold,, assuming each file name takes12 bytes */
		NameBuf = & FILE_DATA_BUF[ FileCount * 12 ];  /* Calculate the buffer address for saving the current file name */
		for ( i = 0; i < 11; i ++ ) NameBuf[ i ] = pFileDir -> DIR_Name[ i ];  /* Copy the file name,, length11 characters,, spaces not handled */
//		if ( pFileDir -> DIR_Attr & ATTR_DIRECTORY ) NameBuf[ i ] = 1;  /* Check whether it is a directory name */
		NameBuf[ i ] = 0;  /* File name terminator */
	}
#endif
}
#endif  // NO_DEFAULT_FILE_ENUMER

uint8_t	CHRV3LibInit( void )  /* InitializeCHRV3 the library,; return on successful operation0 */
{
    uint8_t s;
    s = CHRV3GetVer( );
	if( s < CHRV3_LIB_VER )
	{
        return( 0xFF );  /* Get the version number of the current subroutine library,return an error if the version is too low */
	}
	printf( "lib vision:%02x\r\n",s );
#if		DISK_BASE_BUF_LEN > 0
	pDISK_BASE_BUF = & DISK_BASE_BUF[0]; /* Pointing to the externalRAM disk data buffer */
	pDISK_FAT_BUF = & DISK_BASE_BUF[0];  /* Pointing to the externalRAM diskFAT data buffer,, can share withpDISK_BASE_BUF to saveRAM */
//	pDISK_FAT_BUF = & DISK_FAT_BUF[0];   /* Pointing to the externalRAM diskFAT data buffer,, independent ofpDISK_BASE_BUF for higher speed */
/* To improve file access speed,, callCHRV3LibInit in the main program, then re-,pointpDISK_FAT_BUF to another independently allocated buffer of the same size aspDISK_BASE_BUF */
#endif
	CHRV3DiskStatus = DISK_UNKNOWN;  /* Unknown status */
	CHRV3vSectorSizeB = 9;  /* The sectors of the default physical disk are512B */
	CHRV3vSectorSize = 512; /* The sectors of the default physical disk are512B,this value is the sector size of the disk */
	CHRV3vStartLba = 0;     /* Automatic analysis by defaultFDD andHDD */
#if DEF_USB_PORT_FS_EN
	CHRV3vPacketSize = 64;  /* USBMaximum packet length of the storage device:64@FS,512@HS/SS,, initialized by the application,, after enumerating theU drive: if high-speed or super-speed, update it to512 */
    pTX_DMA_A_REG = (uint32_t *)&(USBFSH->HOST_TX_DMA);  /* Pointing to the transmitDMA address register,, initialized by the application */
    pRX_DMA_A_REG = (uint32_t *)&(USBFSH->HOST_RX_DMA);  /* Pointing to the receiveDMA address register,, initialized by the application */
    pTX_LEN_REG = (uint16_t *)&(USBFSH->HOST_TX_LEN);    /* Pointing to the transmit length register,, initialized by the application */
    pRX_LEN_REG = (uint16_t *)&(USBFSH->RX_LEN);         /* Pointing to the receive length register,, initialized by the application */
#elif DEF_USB_PORT_HS_EN
    CHRV3vPacketSize = 512;  /* USBMaximum packet length of the storage device:64@FS,512@HS/SS,, initialized by the application,, after enumerating theU drive: if high-speed or super-speed, update it to512 */
    pTX_DMA_A_REG = (uint32_t *)&(USBHSH->HOST_TX_DMA);  /* Pointing to the transmitDMA address register,, initialized by the application */
    pRX_DMA_A_REG = (uint32_t *)&(USBHSH->HOST_RX_DMA);  /* Pointing to the receiveDMA address register,, initialized by the application */
    pTX_LEN_REG = (uint16_t *)&(USBHSH->HOST_TX_LEN);    /* Pointing to the transmit length register,, initialized by the application */
    pRX_LEN_REG = (uint16_t *)&(USBHSH->RX_LEN);         /* Pointing to the receive length register,, initialized by the application */
#endif

	return( ERR_SUCCESS );
}

void mDelaymS( uint16_t n )
{
	Delay_Ms(n);
}

