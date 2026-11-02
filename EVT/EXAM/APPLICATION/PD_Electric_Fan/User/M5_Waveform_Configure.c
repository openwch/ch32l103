/********************************** (C) COPYRIGHT *******************************
 * File Name          : M5_Waveform_Configure.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2026/10/08
 * Description        : Waveform display channel configuration

*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/* Includes -----------------------------------------------------------------*/
#include "M0_Control_Library.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private functions --------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
/******************************************************************************
* Function Name  : Waveform_Display
* Description    : Waveform display variable setting
* Input          : None
* Return         : None
******************************************************************************/
void Waveform_Display (void)
{
    //sendUsbData.sendDataBuf[sendUsbData.head+0] = (uint16_t)(SpeedRamp_M.Aftrampcom>>10);
//    sendUsbData.sendDataBuf[sendUsbData.head+0] = (uint16_t)(SpeedRamp_M.Aftramp>>10);
    sendUsbData.sendDataBuf[sendUsbData.head+0] = (uint16_t)(MLBG_M.Cal_Struc.Angspeed_Inst>>10);
//    sendUsbData.sendDataBuf[sendUsbData.head+4] = (uint16_t)(MLBG_M.Cal_Struc.Angspeed_Aver1>>10);
//    sendUsbData.sendDataBuf[sendUsbData.head+2] = (uint16_t)(MLBG_M.Cal_Struc.Angspeed_Aver2>>10);
    sendUsbData.sendDataBuf[sendUsbData.head+1] = (uint16_t)(MLBG_M.Cal_Struc.CalphaNew>>10);
    sendUsbData.sendDataBuf[sendUsbData.head+2] = (uint16_t)(MLBG_M.Cal_Struc.EalphaNew>>10);
    sendUsbData.sendDataBuf[sendUsbData.head+3] = (uint16_t)(ADC_M.DCVoltAver>>10);
    sendUsbData.sendDataBuf[sendUsbData.head+4] = (uint16_t)(RunningStatus_M*1000);
    sendUsbData.head += MaxUsbDataLen;
}
