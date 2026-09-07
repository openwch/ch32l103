/********************************** (C) COPYRIGHT  *******************************
 * File Name          : wui_port.h
 * Author             : WCH
 * Version            : V1.2.0
 * Date               : 2026/08/15
 * Description        : Port layer functions
 *********************************************************************************
 * Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __WUI_PORT_H
#define __WUI_PORT_H

#include <stdint.h>
#include "wui.h"

void HW_Key_event(uint32_t _key_code);

const wui_hal_t* wui_get_hal(void);

#endif // __WUI_PORT_H
