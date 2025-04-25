/*
 * File      : board.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-09-22     Bernard      add board.h to this bsp
 * 2017-10-20     ZYH          emmm...setup for HAL Libraries
 */

// <<< Use Configuration Wizard in Context Menu >>>
#ifndef __BOARD_H__
#define __BOARD_H__

#include "ch32f20x.h"
#define CH32F20X_PIN_NUMBERS   100



#define CH32_FLASH_START_ADRESS       ((uint32_t)0x08000000)
#define CH32_FLASH_SIZE               (256 * 1024)
#define CH32_FLASH_END_ADDRESS        ((uint32_t)(CH32_FLASH_START_ADRESS + CH32_FLASH_SIZE))

#define CH32_SRAM1_SIZE               (64)
#define CH32_SRAM1_START              (0x20000000)
#define CH32_SRAM1_END                (CH32_SRAM1_START + CH32_SRAM1_SIZE * 1024)

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="CSTACK"
#define HEAP_BEGIN      (__segment_end("CSTACK"))
#else
extern int __bss_end;
#define HEAP_BEGIN      ((void *)&__bss_end)
#endif

#define HEAP_END                       CH32_SRAM1_END




void rt_hw_board_init(void);

#endif /* __BOARD_H__ */
