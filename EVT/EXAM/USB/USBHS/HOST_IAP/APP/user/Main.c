/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/08/08
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
   The matching APP code has modified.‘Options for target’-‘Target’-
  ‘Read/Only Memory Areas’-‘on-chip’IROM1 of the region， change Start be‘0x805000’，
    size is 0xB000，The rest remains unchanged. To be used as a companion APP for IAP. The generated 
   Hex file needs to be converted into a bin file,You can use the HexToBin.exe tool under the HOST_IAP 
   folder and modify the file name to APP.bin.
*/

#include "stdio.h"
#include "string.h"
#include "debug.h"

#pragma  diag_suppress 870

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    Delay_Init( );
    Delay_Ms(100);
    USART_Printf_Init(115200);
    printf( "SystemClk:%d\r\n", SystemCoreClock );
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );

    while(1)
    {
        Delay_Ms( 200 );
        printf( "USER App...\r\n" );
    }
}



