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

//#include "CHRV3BAS.H"

#ifndef __CHRV3UFI_H__
#define __CHRV3UFI_H__



#define CHRV3_LIB_VER		0x10

//#define DISK_BASE_BUF_LEN		512	/* The default disk data buffer size is512 bytes((selectable as2048 or even4096 to supportU disks with large sectors),if0, defining the buffer in this file is prohibited; the application specifies it inpDISK_BASE_BUF */
/* If the disk data buffer needs to be reused to saveRAM,, thenDISK_BASE_BUF_LEN can be defined as0 to disable defining the buffer in this file,, and the application shall, before callingCHRV3LibInit, put the start address of the shared buffer into thepDISK_BASE_BUF variable */

//#define NO_DEFAULT_ACCESS_SECTOR	    1		/* Disable the default disk sector read/write routines,replace them with self-written routines below */
//#define NO_DEFAULT_DISK_CONNECT		1		/* Disable the default disk connection check routine,replace it with a self-written routine below */
//#define NO_DEFAULT_FILE_ENUMER		1		/* Disable the default file name enumeration callback routine,replace it with a self-written routine below */
#define FOR_ROOT_UDISK_ONLY           1
#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************************************************************* */

/* FILE: CHRV3UF.H */

/* Error codes */
#ifndef ERR_SUCCESS
#define ERR_SUCCESS				0x00	/* Operation successful */
#endif
#ifndef ERR_DISK_DISCON
#define ERR_CHRV3_ERROR			0x81	/* CHRV3Hardware error,a reset may be neededCHRV3 */
//#define ERR_DISK_DISCON			0x82	/* Disk not connected,the disk may have been disconnected */
#define ERR_STATUS_ERR			0x83	/* Disk status error,the disk may be connecting or disconnecting */
#define ERR_HUB_PORT_FREE		0x84	/* USB-HUBAlready connected butHUB port has no disk connected,the disk may have been disconnected */
#define ERR_MBR_ERROR			0x91	/* Invalid master boot record of the disk,the disk may not be partitioned or formatted yet */
#define ERR_TYPE_ERROR			0x92	/* Unsupported disk partition type,onlyFAT12/FAT16/BigDOS/FAT32, supported; repartition with a disk management tool */
#define ERR_BPB_ERROR			0xA1	/* Disk not formatted,or parameter error,pleaseWINDOWS reformat with default parameters */
#define ERR_TOO_LARGE			0xA2	/* Disk abnormally formatted and capacity larger than4GB,or capacity larger than250GB,pleaseWINDOWS reformat with default parameters */
#define ERR_FAT_ERROR			0xA3	/* Unsupported disk file system,onlyFAT12/FAT16/FAT32, supported; pleaseWINDOWS reformat with default parameters */
#define ERR_DISK_FULL			0xB1	/* Disk files too full,remaining space too little or none,disk cleanup needed */
#define ERR_FDT_OVER			0xB2	/* Too many files in the directory,no free directory entries,FAT12/FAT16number of files under the root directory should be less than500,disk cleanup needed */
#define ERR_MISS_DIR			0xB3	/* A subdirectory of the specified path was not found,the directory name may be wrong */
#define ERR_FILE_CLOSE			0xB4	/* The file is already closed,if needed,reopen the file */
#define ERR_OPEN_DIR			0x41	/* A directory of the specified path was opened */
#define ERR_MISS_FILE			0x42	/* The file of the specified path was not found,the file name may be wrong */
#define ERR_FOUND_NAME			0x43	/* Found a file name matching the wildcard,the file name and its full path are in the command buffer,if needed,open the file */
#endif
/* Codes2XH-3XH used forUSB communication failure codes in host mode, emulated byCHRV3 subroutine, imitatingCH375 return values */
/* Codes1XH used forUSB operation status codes in host mode, emulated byCHRV3 subroutine, imitatingCH375 return values */
#ifndef ERR_USB_CONNECT
#define	ERR_USB_CONNECT_LS		0x13	/* Detected low-speedUSB device connection event */
#define	ERR_USB_CONNECT			0x15	/* DetectedUSB device connection event,disk connected */
#define	ERR_USB_DISCON			0x16	/* DetectedUSB device disconnection event,disk disconnected */
#define	ERR_USB_BUF_OVER		0x17	/* USBTransferred data erroneous or too much, buffer overflow */
#define	ERR_USB_DISK_ERR		0x1F	/* USBMemory operation failed,during initialization it may beUSB memory unsupported,during read/write the disk may be damaged or disconnected */
#define	ERR_USB_TRANSFER		0x20	/* NAK/STALLMore error codes are in0x20~0x2F */
#endif

/* Disk and file status */
#define DISK_UNKNOWN			0x00	/* Not initialized yet,unknown status */
#define DISK_DISCONNECT			0x01	/* Disk not connected or already disconnected */
#define DISK_CONNECT			0x02	/* Disk connected,but not initialized yet or the disk is unrecognizable */
#define DISK_USB_ADDR			0x04	/* Disk allocatedUSB device address,but not configured yetUSB initialize the disk */
#define DISK_MOUNTED			0x05	/* Disk initialized successfully,but the file system is not analyzed yet or unsupported */
#define DISK_READY				0x10	/* File system of the disk analyzed and supported */
#define DISK_OPEN_ROOT			0x12	/* Root directory opened,sector mode,directory contents can only be read/written in sectors,must be closed after use,noteFAT12/FAT16the root directory has a fixed length */
#define DISK_OPEN_DIR			0x13	/* Subdirectory opened,sector mode,directory contents can only be read/written in sectors */
#define DISK_OPEN_FILE			0x14	/* File opened,sector mode,data can be read/written in sectors */
#define DISK_OPEN_FILE_B		0x15	/* File opened,byte mode,data can be read/written in bytes */

/* FATType flags */
#ifndef DISK_FAT16
#define DISK_FS_UNKNOWN			0		/* Unknown file system */
#define DISK_FAT12				1		/* FAT12 file system */
#define DISK_FAT16				2		/* FAT16 file system */
#define DISK_FAT32				3		/* FAT32 file system */
#endif

/* FATFile directory information in the data area */
typedef struct _FAT_DIR_INFO {
	uint8_t	DIR_Name[11];				/* 00H,File name,total11 bytes,pad with spaces if insufficient */
	uint8_t	DIR_Attr;					/* 0BH,File attributes,see the description below */
	uint8_t	DIR_NTRes;					/* 0CH */
	uint8_t	DIR_CrtTimeTenth;			/* 0DH,File creation time,counted in0.1 seconds */
	uint16_t	DIR_CrtTime;				/* 0EH,File creation time */
	uint16_t	DIR_CrtDate;				/* 10H,File creation date */
	uint16_t	DIR_LstAccDate;				/* 12H,Date of the last access */
	uint16_t	DIR_FstClusHI;				/* 14H */
	uint16_t	DIR_WrtTime;				/* 16H,File modification time,see macroMAKE_FILE_TIME */
	uint16_t	DIR_WrtDate;				/* 18H,File modification date,see macroMAKE_FILE_DATA */
	uint16_t	DIR_FstClusLO;				/* 1AH */
	uint32_t	DIR_FileSize;				/* 1CH,File length */
} FAT_DIR_INFO;							/* 20H */

typedef FAT_DIR_INFO *PX_FAT_DIR_INFO;

/* File attributes */
#define ATTR_READ_ONLY			0x01	/* File is read-only */
#define ATTR_HIDDEN				0x02	/* File is hidden */
#define ATTR_SYSTEM				0x04	/* File is a system file */
#define ATTR_VOLUME_ID			0x08	/* Volume label */
#define ATTR_DIRECTORY			0x10	/* Subdirectory */
#define ATTR_ARCHIVE			0x20	/* File is archive */
#define ATTR_LONG_NAME			( ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID )
/* File attributes uint8_t */
/* bit0 bit1 bit2 bit3 bit4 bit5 bit6 bit7 */
/*  read-only   hidden   system   volume   directory   archive   undefined   */
/*  read   hide   system   label   directory   archive            */
/* File time uint16_t */
/* Time = (Hour<<11) + (Minute<<5) + (Second>>1) */
#define MAKE_FILE_TIME( h, m, s )	( (h<<11) + (m<<5) + (s>>1) )	/* Generate file time data for the specified hour/minute/second */
/* File date uint16_t */
/* Date = ((Year-1980)<<9) + (Month<<5) + Day */
#define MAKE_FILE_DATE( y, m, d )	( ((y-1980)<<9) + (m<<5) + d )	/* Generate file date data for the specified year/month/day */

/* File name */
#define PATH_WILDCARD_CHAR		0x2A	/* Wildcard character of the path name '*' */
#define PATH_SEPAR_CHAR1		0x5C	/* Separator of the path name '\' */
#define PATH_SEPAR_CHAR2		0x2F	/* Separator of the path name '/' */
#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN			64		/* Maximum path length,including all slash separators, dot separators and the path terminator00H */
#endif

/* External command parameters */
typedef union _CMD_PARAM
{
    struct
    {
        uint8_t mBuffer[ MAX_PATH_LEN ];
    } Other;
    struct
    {
        uint32_t mTotalSector;          /* Return:  the total sector count of the current logical drive */
        uint32_t mFreeSector;           /* Return:  the remaining sector count of the current logical drive */
        uint32_t mSaveValue;
    } Query;                            /* CMD_DiskQuery, Query disk information */
    struct
    {
        uint8_t mPathName[ MAX_PATH_LEN ];  /* Input parameter: path: [drive letter,colon,slash,directory/file name with extension...,terminator00H], , drive letter and colon can be omitted, e.g."C:\DIR1.EXT\DIR2\FILENAME.EXT",00H */
    } Open;                             /* CMD_FileOpen, Open the file */
//  struct
//  {
//      uint8_t mPathName[ MAX_PATH_LEN ];  /* Input parameter: path: [drive letter,colon,slash,directory/file name with extension(with wildcard*)...,terminator00H], , drive letter and colon can be omitted, e.g."C:\DIR1.EXT\DIR2\FILE*",00H */
//  } Open;                             /* CMD_FileOpen, Enumerate files, CHRV3vFileSize, with the highest bit1, if set, callxFileNameEnumer,, if clear,0 return the file name of the specified index */
    struct
    {
        uint8_t mUpdateLen;             /* Input parameter: whether to allow length update: 0prohibited,1allowed */
    } Close;                            /* CMD_FileClose, Close the current file */
    struct
    {
        uint8_t mPathName[ MAX_PATH_LEN ];  /* Input parameter: path: [drive letter,colon,slash,directory/file name with extension...,terminator00H], , drive letter and colon can be omitted, e.g."C:\DIR1.EXT\DIR2\FILENAME.EXT",00H */
    } Create;                           /* CMD_FileCreate, Create and open the file,if the file already exists, delete it first and then create a new one */
    struct
    {
        uint8_t mPathName[ MAX_PATH_LEN ];  /* Input parameter: path: [drive letter,colon,slash,directory/file name with extension...,terminator00H], , drive letter and colon can be omitted, e.g."C:\DIR1.EXT\DIR2\FILENAME.EXT",00H */
    } Erase;                            /* CMD_FileErase, Delete the file and close it */
    struct
    {
        uint32_t mFileSize;             /* Input parameter: new file length,if0FFFFFFFFH, do not modify, , return the: original length */
        uint16_t mFileDate;             /* Input parameter: new file date,if0FFFFH, do not modify, , return the: original date */
        uint16_t mFileTime;             /* Input parameter: new file time,if0FFFFH, do not modify, , return the: original time */
        uint8_t  mFileAttr;             /* Input parameter: new file attribute,if0FFH, do not modify, , return the: original attribute */
    } Modify;                           /* CMD_FileQuery, Query the information of the current file; CMD_FileModify, query or modify the information of the current file */
    struct
    {
        uint32_t mSaveCurrClus;
        uint32_t mSaveLastClus;
    } Alloc;                            /* CMD_FileAlloc, Adjust the disk space allocated for the file according to the file length */
    struct
    {
        uint32_t mSectorOffset;      /* Input parameter: sector offset,0, move to the file head,0FFFFFFFFH, move to the file tail, , return:  the absolute linear sector number corresponding to the current file pointer, 0FFFFFFFFH, end of file reached */
        uint32_t mLastOffset;
    } Locate;                           /* CMD_FileLocate, Move the current file pointer */
    struct
    {
        uint8_t mSectorCount;           /* Input parameter: sectors to read, ; return: as the actually read sector count */
        uint8_t mActCnt;
        uint8_t mLbaCount;
        uint8_t mRemainCnt;
        uint8_t *mDataBuffer;           /* Input parameter: buffer start address, ; return: as the current buffer address */
        uint32_t mLbaStart;
    } Read;                             /* CMD_FileRead, Read data from the current file */
    struct
    {
        uint8_t mSectorCount;           /* Input parameter: sectors to write, ; return: as the actually written sector count */
        uint8_t mActCnt;
        uint8_t mLbaCount;
        uint8_t mAllocCnt;
        uint8_t *mDataBuffer;           /* Input parameter: buffer start address, ; return: as the current buffer address */
        uint32_t mLbaStart;
        uint32_t mSaveValue;
    } Write;                            /* CMD_FileWrite, Write data to the current file */
    struct
    {
        uint32_t mDiskSizeSec;          /* Return:  the total sector count of the whole physical disk, returned only on the first call */
    } DiskReady;                        /* CMD_DiskReady, Disk ready */
    struct
    {
        uint32_t mByteOffset;           /* Input parameter: byte-based offset, byte-based file pointer, ; return:  the absolute linear sector number of the current file pointer, 0FFFFFFFFH; end of file reached */
        uint32_t mLastOffset;
    } ByteLocate;                       /* CMD_ByteLocate, Move the current file pointer in bytes */
    struct
    {
        uint16_t mByteCount;            /* Input parameter: bytes to read, ; return: as the actually read byte count */
        uint8_t *mByteBuffer;           /* Input parameter: buffer holding the read data blocks */
        uint16_t mActCnt;
    } ByteRead;                         /* CMD_ByteRead, Read data blocks from the current file in bytes */
    struct
    {
        uint16_t mByteCount;            /* Input parameter: bytes to write, ; return: as the actually written byte count */
        uint8_t *mByteBuffer;           /* Input parameter: buffer holding the read data blocks */
        uint16_t mActCnt;
    } ByteWrite;                        /* CMD_ByteWrite, Write data blocks to the current file in bytes */
    struct
    {
        uint8_t mSaveVariable;          /* Input parameter: : if0, restore a singleU drive's variables,, if0x80, restore multipleU drive's variables,, other values back up//save variables */
        uint8_t mReserved[3];
        uint8_t *mBuffer;               /* Input parameter: backup buffer for the subroutine library variables,with length not less than80 bytes */
    } SaveVariable;                     /* CMD_SaveVariable, Back up/save/restore the subroutine library variables */
} CMD_PARAM;

typedef CMD_PARAM CMD_PARAM_I;
//typedef CMD_PARAM *P_CMD_PARAM;

/* SCSICommand code */
#ifndef SPC_CMD_INQUIRY
#define SPC_CMD_INQUIRY			0x12
#define SPC_CMD_READ_CAPACITY	0x25
#define SPC_CMD_READ10			0x28
#define SPC_CMD_WRITE10			0x2A
#define SPC_CMD_TEST_READY		0x00
#define SPC_CMD_REQUEST_SENSE	0x03
#define SPC_CMD_MODESENSE6		0x1A
#define SPC_CMD_MODESENSE10		0x5A
#define SPC_CMD_START_STOP		0x1B
#endif

/* FILE: CHRV3UFI.C */
#define EN_DISK_WRITE			1

#ifndef DISK_BASE_BUF_LEN
#define DISK_BASE_BUF_LEN		512		/* The default disk data buffer size is512 bytes,(recommended:2048 or even4096 to supportU disks with large sectors,if0, defining the buffer in the.H file is prohibited; the application specifies it inpDISK_BASE_BUF */
#endif

/* Variables provided in the subroutine library */
extern  volatile uint8_t CHRV3IntStatus;  /* CHRV3Interrupt status of the operation */
extern  volatile uint8_t CHRV3DiskStatus; /* Disk and file status */
extern  uint8_t  CHRV3vDiskFat;         /* Logical drive'sFAT flags:1=FAT12,2=FAT16,3=FAT32 */
extern  uint8_t  CHRV3vSecPerClus;      /* Sectors per cluster of the logical drive */
extern  uint8_t  CHRV3vSectorSizeB;     /* log2(CHRV3vSectorSize) */
extern  uint32_t CHRV3vStartLba;        /* Start absolute sector number of the logical driveLBA */
extern  uint32_t CHRV3vDiskRoot;        /* ForFAT16 drives: sectors occupied by the root directory,; forFAT32 drives: start cluster number of the root directory */
extern  uint32_t CHRV3vDataStart;       /* Start of the data area of the logical driveLBA */
extern  uint32_t CHRV3vStartCluster;    /* Start cluster number of the current file or directory */
extern  uint32_t CHRV3vFileSize;        /* Length of the current file */
extern  uint32_t CHRV3vCurrentOffset;   /* Current file pointer,byte offset of the current read/write position */
extern  uint32_t CHRV3vFdtLba;          /* CurrentFDT locationLBA address */
extern  uint32_t CHRV3vLbaCurrent;      /* StartLBA address */
extern  uint16_t CHRV3vFdtOffset;       /* CurrentFDT offset address within the sector */
extern  uint16_t CHRV3vSectorSize;      /* Sector size of the disk */
extern  uint8_t  CHRV3vCurrentLun;      /* Logical unit number currently operated by the disk */
extern  uint8_t  CHRV3vSubClassIs6;     /* USBThe subclass of the storage device is6,0, otherwise it is not6 */
extern  uint8_t  *pDISK_BASE_BUF;       /* Pointing to the externalRAM disk data buffer, with length not less thanCHRV3vSectorSize, initialized by the application */
extern  uint8_t  *pDISK_FAT_BUF;        /* Pointing to the externalRAM diskFAT data buffer, with length not less thanCHRV3vSectorSize, initialized by the application */
extern  uint16_t CHRV3vPacketSize;     /* USBMaximum packet length of the storage device:64@FS,512@HS/SS,initialized by the application */
extern  uint32_t *pTX_DMA_A_REG;        /* Pointing to the transmitDMA address register,, initialized by the application */
extern  uint32_t *pRX_DMA_A_REG;        /* Pointing to the receiveDMA address register,, initialized by the application */
extern  uint16_t *pTX_LEN_REG;          /* Pointing to the transmit length register,, initialized by the application */
extern  uint16_t *pRX_LEN_REG;          /* Pointing to the receive length register,, initialized by the application */

extern	CMD_PARAM_I	mCmdParam;				/* Command parameters */

extern	__attribute__ ((aligned(4)))   uint8_t 	RxBuffer[ ];  // IN, must even address
extern	__attribute__ ((aligned(4)))   uint8_t	TxBuffer[ ];  // OUT, must even address

//#define		PXUDISK_BOC_CBW	PUDISK_BOC_CBW
//#define		PXUDISK_BOC_CSW	PUDISK_BOC_CSW

#ifndef	pSetupReq
#define	pSetupReq	((PUSB_SETUP_REQ)TxBuffer)
#endif

#ifndef	pCBW
#define	pCBW		((PXUDISK_BOC_CBW)TxBuffer)
#define	pCSW		((PXUDISK_BOC_CSW)RxBuffer)
#endif
#ifndef	pBOC_buf
#define	pBOC_buf	(TxBuffer+((USB_BO_CBW_SIZE+4)&0xFE))
#endif

#if		DISK_BASE_BUF_LEN > 0
extern	uint8_t	DISK_BASE_BUF[ DISK_BASE_BUF_LEN ];	/* ExternalRAM disk data buffer,, with a length of one sector */
#endif
extern	uint8_t	CHRV3ReadSector( uint8_t SectCount, uint8_t * DataBuf );	/* Read multiple sectors of data from the disk into the buffer */
#ifdef	EN_DISK_WRITE
extern	uint8_t	CHRV3WriteSector( uint8_t SectCount, uint8_t * DataBuf );	/* Write the multi-sector data block in the buffer to the disk */
#endif

extern	uint8_t	CHRV3DiskConnect( void );	/* Check whether the disk is connected and update the disk status */
extern	void	xFileNameEnumer( void );	/* Call externally defined subroutines,file name enumeration callback subroutine */

extern	uint8_t	CHRV3LibInit( void );		/* InitializeCHRV3 library, ,return on successful operation0 */

/* Subroutines provided in the subroutine library */
/* In the subroutines below, file operation subroutinesCHRV3File*and disk query subroutinesCHRV3DiskQuerymay all use the disk data bufferpDISK_BASE_BUF,
   AndpDISK_BASE_BUF may hold disk info; it must be ensured that,  is not used for other purposespDISK_BASE_BUF,
   IfRAM memory is small, , to temporarily usepDISK_BASE_BUF for other purposes, , after temporary use, must callCHRV3DirtyBuffer to clear the disk buffer */
extern	uint8_t	CHRV3GetVer( void );		/* Get the version number of the current subroutine library */
extern	void	CHRV3DirtyBuffer( void );	/* Clear the disk buffer */
extern	uint8_t	CHRV3BulkOnlyCmd( uint8_t * DataBuf );	/* Execute commands based onBulkOnly protocol */
extern	uint8_t	CHRV3DiskReady( void );		/* Check whether the disk is ready */
extern	uint8_t	CHRV3AnalyzeError( uint8_t iMode );	/* USBAnalyze operation failureCHRV3IntStatusreturn the error status */
extern	uint8_t	CHRV3FileOpen( void );		/* Open or enumerate files */
extern	uint8_t	CHRV3FileClose( void );		/* Close the current file */
#ifdef	EN_DISK_WRITE
extern	uint8_t	CHRV3FileErase( void );		/* Delete the file and close it */
extern	uint8_t	CHRV3FileCreate( void );	/* Create and open the file,if the file already exists, delete it first and then create a new one */
extern	uint8_t	CHRV3FileAlloc( void );		/* Adjust the disk space allocated for the file according to the file length */
#endif
extern	uint8_t	CHRV3FileModify( void );	/* Query or modify the information of the current file */
extern	uint8_t	CHRV3FileQuery( void );		/* Query the information of the current file */
extern	uint8_t	CHRV3FileLocate( void );	/* Move the current file pointer */
extern	uint8_t	CHRV3FileRead( void );		/* Read data from the current file into the specified buffer */
#ifdef	EN_DISK_WRITE
extern	uint8_t	CHRV3FileWrite( void );		/* Write the specified buffer data to the current file */
#endif
extern	uint8_t	CHRV3ByteLocate( void );	/* Move the current file pointer in bytes */
extern	uint8_t	CHRV3ByteRead( void );		/* Read data blocks from the current position in bytes */
#ifdef	EN_DISK_WRITE
extern	uint8_t	CHRV3ByteWrite( void );		/* Write data blocks to the current position in bytes */
#endif
extern	uint8_t	CHRV3DiskQuery( void );		/* Query disk information */
extern	void	CHRV3SaveVariable( void );	/* Backup/save/restore the subroutine library variables,for switching the library among multiple chips orU drives */

extern	void	mDelayuS( uint16_t n );		// Delay in units ofuS
extern	void	mDelaymS( uint16_t n );		// Delay in units ofmS
extern	uint8_t	USBHostTransact( uint8_t endp_pid, uint8_t tog, uint32_t timeout );	// CHRV3 transfer transaction,input destination endpoint address/PID token,, sync flag,NAK, NAK retry timeout,, return0 on success,, timeout/retry on error
extern	uint8_t	HostCtrlTransfer( uint8_t * DataBuf, uint8_t * RetLen );	// Perform control transfer,8 the 8-byte request code is inpSetupReq,DataBuf is the optional transceive buffer,, the actual transferred length is returned inReqLen pointed-to variable
extern	void	CopySetupReqPkg( const char * pReqPkt );  // Copy the setup packet of the control transfer
extern	uint8_t	CtrlGetDeviceDescrTB( void );       // Get the device descriptor,, returned inTxBuffer
extern	uint8_t	CtrlGetConfigDescrTB( void );       // Get the configuration descriptor,, returned inTxBuffer
extern	uint8_t	CtrlSetUsbAddress( uint8_t addr );  // SetUSB device address
extern	uint8_t	CtrlSetUsbConfig( uint8_t cfg );    // SetUSB device configuration
extern	uint8_t	CtrlClearEndpStall( uint8_t endp ); // Clear endpointSTALL
#ifndef	FOR_ROOT_UDISK_ONLY
//extern	uint8_t	CtrlGetHubDescr( void );  // GetHUB descriptor,, returned inTxBuffer
extern	uint8_t	HubGetPortStatus( uint8_t HubPortIndex );  // QueryHUB port status,, returned inTxBuffer
//extern	uint8_t	HubSetPortFeature( uint8_t HubPortIndex, uint8_t FeatureSelt );  // SetHUB port feature
extern	uint8_t	HubClearPortFeature( uint8_t HubPortIndex, uint8_t FeatureSelt );  // ClearHUB port feature
#endif

#ifdef __cplusplus
}
#endif

#endif
