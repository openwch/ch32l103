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
 *USART Print debugging routine:
 *USART1_Tx(PA9).
 *This example demonstrates using USART1(PA9) as a print debug port output.
 *
 */

#include "debug.h"
#include "sd_card.h"
#include "FatFs/ff.h"

/*
 *@Note
  SPI SD card FatFs example:
  SPI1_SCK(PA5), SPI1_MISO(PA6), SPI1_MOSI(PA7), CS(PA2).
  Mount the FAT filesystem on the SD card, list the root directory,
  then create, write and read a test file.

 pins:
    SDCARD -- MCU
    DATA3  -- PA2
    CLK    -- PA5(SPI1_SCK)
    DATA0  -- PA6(SPI1_MISO)
    CMD    -- PA7(SPI1_MOSI)

*/

/* Global typedef */

/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    FATFS     fs;
    FIL       fnew;
    DIR       dir;
    FILINFO   fno;
    UINT      bw, br;
    FRESULT   fres;
    BYTE      work[FF_MAX_SS];
    uint8_t   write_buf[] = {"Hello FatFs!\r\nHello FatFs!"};
    uint8_t   read_buf[32];
    MKFS_PARM opt = {FM_ANY, 1, 0, 0, 0};
    uint16_t  i;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("SD card FatFs test\r\n");

    /* init SPI and GPIO */
    SD_Init();

    /* mount volume "0:" */
    fres = f_mount(&fs, "0:", 1);
    if (fres == FR_NO_FILESYSTEM)
    {
        printf("No filesystem, formatting...\r\n");
        fres = f_mkfs("0:", &opt, work, sizeof(work));
        if (fres != FR_OK)
        {
            printf("f_mkfs failed! res=%d\r\n", fres);
            while (1)
                ;
        }
        fres = f_mount(&fs, "0:", 1);
    }
    if (fres != FR_OK)
    {
        printf("f_mount failed! res=%d\r\n", fres);
        while (1)
            ;
    }
    printf("Disk mounted OK\r\n");

    /* list files and folders in the root directory */
    fres = f_opendir(&dir, "0:/");
    if (fres == FR_OK)
    {
        printf("Root directory:\r\n");
        while (1)
        {
            fres = f_readdir(&dir, &fno);
            if (fres != FR_OK || fno.fname[0] == 0)
            {
                break;
            }
            if (fno.fattrib & AM_DIR)
            {
                printf("  [DIR ] %s\r\n", fno.fname);
            }
            else
            {
                printf("  [FILE] %s  %d bytes\r\n", fno.fname, fno.fsize);
            }
        }
        f_closedir(&dir);
    }
    else
    {
        printf("f_opendir failed! res=%d\r\n", fres);
    }

    /* create a file and write data */
    fres = f_open(&fnew, "0:/test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (fres != FR_OK)
    {
        printf("f_open(write) failed! res=%d\r\n", fres);
        while (1)
            ;
    }

    fres = f_write(&fnew, write_buf, sizeof(write_buf) - 1, &bw);
    if (fres != FR_OK || bw != sizeof(write_buf) - 1)
    {
        printf("f_write failed! res=%d\r\n", fres);
        while (1)
            ;
    }
    f_close(&fnew);
    printf("Write file OK: %s\r\n", write_buf);

    /* open the file and read data back */
    fres = f_open(&fnew, "0:/test.txt", FA_OPEN_EXISTING | FA_READ);
    if (fres != FR_OK)
    {
        printf("f_open(read) failed! res=%d\r\n", fres);
        while (1)
            ;
    }

    for (i = 0; i < sizeof(read_buf); i++)
    {
        read_buf[i] = 0;
    }

    fres = f_read(&fnew, read_buf, sizeof(write_buf) - 1, &br);
    if (fres != FR_OK)
    {
        printf("f_read failed! res=%d\r\n", fres);
        while (1)
            ;
    }
    f_close(&fnew);

    printf("Read file OK (%d bytes): %s\r\n", br, read_buf);
    printf("FatFs test finished\r\n");

    while (1)
    {
    }
}
