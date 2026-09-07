/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */
 
#include "board.h"

#include <stdint.h>
#include "drv_usart.h"

#include <rthw.h>
#include <rtthread.h>


// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;

static uint32_t _SysTick_Config(rt_uint32_t ticks)
{
    NVIC_SetPriority(SysTick_IRQn,0xf0);
    NVIC_SetPriority(Software_IRQn,0xf0);
    NVIC_EnableIRQ(SysTick_IRQn);
    NVIC_EnableIRQ(Software_IRQn);
    SysTick->CTLR=0;
    SysTick->SR&=~(1<<0);
    SysTick->CNT=0;
    SysTick->CMP=ticks-1;
    SysTick->CTLR=0xF;
    return 0;
}

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)

#if !defined(USING_MAX_HEAP_SIZE)
#define RT_HEAP_SIZE (8*1024)
static uint32_t rt_heap[RT_HEAP_SIZE];     // heap default size: 4K(1024 * 4)
rt_weak void *rt_heap_begin_get(void)
{
    return rt_heap;
}

rt_weak void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}

#else

rt_weak void *rt_heap_begin_get(void)
{
    return HEAP_BEGIN;
}
rt_weak void *rt_heap_end_get(void)
{
    return HEAP_END ;
}


#endif //USING_MAX_HEAP_SIZE

#endif

/**
 * This function will initial your board.
 */
void rt_hw_board_init()
{
    /* System Tick Configuration */
    _SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
    /* USART driver initialization is open by default */
#ifdef RT_USING_SERIAL
    rt_hw_usart_init();
#endif
#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

}

void SysTick_Handler(void) __attribute__((interrupt()));
void SysTick_Handler(void)
{
    //rt_kprintf("1!\n");
    GET_INT_SP();
    /* enter interrupt */
    rt_interrupt_enter();
    SysTick->SR&=~(1<<0);
     //   rt_kprintf("2!\n");
    rt_tick_increase();
    /* leave interrupt */
    rt_interrupt_leave();
    FREE_INT_SP();

}
