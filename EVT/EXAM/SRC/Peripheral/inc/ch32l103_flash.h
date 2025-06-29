/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32l103_flash.h
 * Author             : WCH
 * Version            : V1.0.2
 * Date               : 2025/04/16
 * Description        : This file contains all the functions prototypes for the FLASH
 *                      firmware library.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __CH32L103_FLASH_H
#define __CH32L103_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32l103.h"

/* FLASH Status */
typedef enum
{
    FLASH_BUSY = 1,
    FLASH_ERROR_PG,
    FLASH_ERROR_WRP,
    FLASH_COMPLETE,
    FLASH_TIMEOUT,
    FLASH_RDP,
    FLASH_OP_RANGE_ERROR = 0xFD,
    FLASH_ALIGN_ERROR = 0xFE,
    FLASH_ADR_RANGE_ERROR = 0xFF,
} FLASH_Status;

/* Flash_Latency */
#define FLASH_Latency_0                ((uint32_t)0x00000000) /* FLASH Zero Latency cycle */
#define FLASH_Latency_1                ((uint32_t)0x00000001) /* FLASH One Latency cycle */
#define FLASH_Latency_2                ((uint32_t)0x00000002) /* FLASH Two Latency cycle */

/* Write Protect 1Sectors = 2KB */
#define FLASH_WRProt_Sectors0to1       ((uint32_t)0x00000001) /* Write protection of sector 0 - 1 */
#define FLASH_WRProt_Sectors2to3       ((uint32_t)0x00000002) /* Write protection of sector 2 - 3 */
#define FLASH_WRProt_Sectors4to5       ((uint32_t)0x00000004) /* Write protection of sector 4 - 5 */
#define FLASH_WRProt_Sectors6to7       ((uint32_t)0x00000008) /* Write protection of sector 6 - 7 */
#define FLASH_WRProt_Sectors8to9       ((uint32_t)0x00000010) /* Write protection of sector 8 - 9 */
#define FLASH_WRProt_Sectors10to11     ((uint32_t)0x00000020) /* Write protection of sector 10 - 11 */
#define FLASH_WRProt_Sectors12to13     ((uint32_t)0x00000040) /* Write protection of sector 12 - 13 */
#define FLASH_WRProt_Sectors14to15     ((uint32_t)0x00000080) /* Write protection of sector 14 - 15 */
#define FLASH_WRProt_Sectors16to17     ((uint32_t)0x00000100) /* Write protection of sector 16 - 17 */
#define FLASH_WRProt_Sectors18to19     ((uint32_t)0x00000200) /* Write protection of sector 18 - 19 */
#define FLASH_WRProt_Sectors20to21     ((uint32_t)0x00000400) /* Write protection of sector 20 - 21 */
#define FLASH_WRProt_Sectors22to23     ((uint32_t)0x00000800) /* Write protection of sector 22 - 23 */
#define FLASH_WRProt_Sectors24to25     ((uint32_t)0x00001000) /* Write protection of sector 24 - 25 */
#define FLASH_WRProt_Sectors26to27     ((uint32_t)0x00002000) /* Write protection of sector 26 - 27 */
#define FLASH_WRProt_Sectors28to29     ((uint32_t)0x00004000) /* Write protection of sector 28 - 29 */
#define FLASH_WRProt_Sectors30to31     ((uint32_t)0x00008000) /* Write protection of sector 30 - 31 */

#define FLASH_WRProt_Sectors0          FLASH_WRProt_Sectors0to1 /* Write protection of sector 0 - 1 */
#define FLASH_WRProt_Sectors1          FLASH_WRProt_Sectors2to3 /* Write protection of sector 2 - 3 */
#define FLASH_WRProt_Sectors2          FLASH_WRProt_Sectors4to5 /* Write protection of sector 4 - 5 */
#define FLASH_WRProt_Sectors3          FLASH_WRProt_Sectors6to7 /* Write protection of sector 6 - 7 */
#define FLASH_WRProt_Sectors4          FLASH_WRProt_Sectors8to9 /* Write protection of sector 8 - 9 */
#define FLASH_WRProt_Sectors5          FLASH_WRProt_Sectors10to11 /* Write protection of sector 10 - 11 */
#define FLASH_WRProt_Sectors6          FLASH_WRProt_Sectors12to13 /* Write protection of sector 12 - 13 */
#define FLASH_WRProt_Sectors7          FLASH_WRProt_Sectors14to15 /* Write protection of sector 14 - 15 */
#define FLASH_WRProt_Sectors8          FLASH_WRProt_Sectors16to17 /* Write protection of sector 16 - 17 */
#define FLASH_WRProt_Sectors9          FLASH_WRProt_Sectors18to19 /* Write protection of sector 18 - 19 */
#define FLASH_WRProt_Sectors10         FLASH_WRProt_Sectors20to21 /* Write protection of sector 20 - 21 */
#define FLASH_WRProt_Sectors11         FLASH_WRProt_Sectors22to23 /* Write protection of sector 22 - 23 */
#define FLASH_WRProt_Sectors12         FLASH_WRProt_Sectors24to25 /* Write protection of sector 24 - 25 */
#define FLASH_WRProt_Sectors13         FLASH_WRProt_Sectors26to27 /* Write protection of sector 26 - 27 */
#define FLASH_WRProt_Sectors14         FLASH_WRProt_Sectors28to29 /* Write protection of sector 28 - 29*/
#define FLASH_WRProt_Sectors15         FLASH_WRProt_Sectors30to31 /* Write protection of sector 30 - 31*/

#define FLASH_WRProt_AllSectors        ((uint32_t)0xFFFF) /* Write protection of all Sectors */

/* Option_Bytes_IWatchdog */
#define OB_IWDG_SW                     ((uint16_t)0x0001) /* Software IWDG selected */
#define OB_IWDG_HW                     ((uint16_t)0x0000) /* Hardware IWDG selected */

/* Option_Bytes_nRST_STOP */
#define OB_STOP_NoRST                  ((uint16_t)0x0002) /* No reset generated when entering in STOP */
#define OB_STOP_RST                    ((uint16_t)0x0000) /* Reset generated when entering in STOP */

/* Option_Bytes_nRST_STDBY  */
#define OB_STDBY_NoRST                 ((uint16_t)0x0004) /* No reset generated when entering in STANDBY */
#define OB_STDBY_RST                   ((uint16_t)0x0000) /* Reset generated when entering in STANDBY */

/* Option_Bytes_OB_CAN_BusOff_MODE */
#define OB_CAN_BusOff_MODE1            ((uint16_t)0x0020) /* Bus off recovery mode1  */
#define OB_CAN_BusOff_MODE2            ((uint16_t)0x0000) /* Bus off recovery mode2 */

/* FLASH_Interrupts */
#define FLASH_IT_ERROR                 ((uint32_t)0x00000400) /* FPEC error interrupt source */
#define FLASH_IT_EOP                   ((uint32_t)0x00001000) /* End of FLASH Operation Interrupt source */
#define FLASH_IT_BANK1_ERROR           FLASH_IT_ERROR         /* FPEC BANK1 error interrupt source */
#define FLASH_IT_BANK1_EOP             FLASH_IT_EOP           /* End of FLASH BANK1 Operation Interrupt source */

/* FLASH_Flags */
#define FLASH_FLAG_BSY                 ((uint32_t)0x00000001) /* FLASH Busy flag */
#define FLASH_FLAG_WRPRTERR            ((uint32_t)0x00000010) /* FLASH Write protected error flag */
#define FLASH_FLAG_EOP                 ((uint32_t)0x00000020) /* FLASH End of Operation flag */
#define FLASH_FLAG_WAKE_UP             ((uint32_t)0x00000040) /* FLASH Wake up of Operation flag */
#define FLASH_FLAG_OPTERR              ((uint32_t)0x80000001) /* FLASH Option Byte error flag */

#define FLASH_FLAG_BANK1_BSY           FLASH_FLAG_BSY       /* FLASH BANK1 Busy flag*/
#define FLASH_FLAG_BANK1_EOP           FLASH_FLAG_EOP       /* FLASH BANK1 End of Operation flag */
#define FLASH_FLAG_BANK1_WRPRTERR      FLASH_FLAG_WRPRTERR  /* FLASH BANK1 Write protected error flag */

/*Functions used for all devices*/
void         FLASH_SetLatency(uint32_t FLASH_Latency);
void         FLASH_Unlock(void);
void         FLASH_Lock(void);
FLASH_Status FLASH_ErasePage(uint32_t Page_Address);
FLASH_Status FLASH_EraseAllPages(void);
FLASH_Status FLASH_EraseOptionBytes(void);
FLASH_Status FLASH_EnableWriteProtection(uint32_t FLASH_Pages);
FLASH_Status FLASH_EnableReadOutProtection(void);
FLASH_Status FLASH_UserOptionByteConfig(uint8_t OB_IWDG, uint8_t OB_STOP, uint8_t OB_STDBY, uint8_t OB_RST);
uint32_t     FLASH_GetUserOptionByte(void);
uint32_t     FLASH_GetWriteProtectionOptionByte(void);
FlagStatus   FLASH_GetReadOutProtectionStatus(void);
void         FLASH_ITConfig(uint32_t FLASH_IT, FunctionalState NewState);
FlagStatus   FLASH_GetFlagStatus(uint32_t FLASH_FLAG);
void         FLASH_ClearFlag(uint32_t FLASH_FLAG);
FLASH_Status FLASH_GetStatus(void);
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout);
void         FLASH_Unlock_Fast(void);
void         FLASH_Lock_Fast(void);
void         FLASH_BufReset(void);
void         FLASH_BufLoad(uint32_t Address, uint32_t Data0);
void         FLASH_ErasePage_Fast(uint32_t Page_Address);
void         FLASH_EraseBlock_32K_Fast(uint32_t Block_Address);
void         FLASH_ProgramPage_Fast(uint32_t Page_Address);
FLASH_Status FLASH_ROM_ERASE(uint32_t StartAddr, uint32_t Length);
FLASH_Status FLASH_ROM_WRITE(uint32_t StartAddr, uint32_t *pbuf, uint32_t Length);

#ifdef __cplusplus
}
#endif

#endif
