/********************************** (C) COPYRIGHT *******************************
* File Name          : PD_PHY.C
* Author             : WCH
* Version            : V1.1
* Date               : 2026/10/08
* Description        : 
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************/


/*******************************************************************************/
/* Header file inclusion */
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "PD_User.h"

//#include "../Proj/USB_Billboard/ch32x035_usbfs_device.h"




u32 SinkCap[] = {0x3ea1912c,0x0002d0c8,0x0003C0FA,0x0004B0C8,0x00064096,0x0088c1f4,0x0007812c,0x0005512c,0x0085012c,0x0003212c,};   //5\9\12\15\20\28\24\17\16\10
u8 SinkCapCnt = 3;





u8  tAMETime_start=0;
u16 tAMETime=0;




void PD_User_Snk_DevIn(void)
{

}



void PD_User_Snk_Rx_SrcCap(void)
{

}



void PD_User_Snk_Rx_PS_RDY(void)
{


}



void PD_User_Src_DevIn(void)
{

}



void PD_User_Src_VoltChange(void)
{

}



void PD_User_DevOut(void)
{

}









void PD_User_Timer(void)
{


}
















