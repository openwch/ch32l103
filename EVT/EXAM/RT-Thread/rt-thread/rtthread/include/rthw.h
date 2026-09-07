/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-18     Bernard      the first version
 * 2006-04-25     Bernard      add rt_hw_context_switch_interrupt declaration
 * 2006-09-24     Bernard      add rt_hw_context_switch_to declaration
 * 2012-12-29     Bernard      add rt_hw_exception_install declaration
 * 2017-10-17     Hichard      add some macros
 * 2018-11-17     Jesven       add rt_hw_spinlock_t
 *                             add smp support
 * 2019-05-18     Bernard      add empty definition for not enable cache case
 */

#ifndef __RT_HW_H__
#define __RT_HW_H__

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Some macros define
 */
#ifndef HWREG64
#define HWREG64(x)          (*((volatile rt_uint64_t *)(x)))
#endif
#ifndef HWREG32
#define HWREG32(x)          (*((volatile rt_uint32_t *)(x)))
#endif
#ifndef HWREG16
#define HWREG16(x)          (*((volatile rt_uint16_t *)(x)))
#endif
#ifndef HWREG8
#define HWREG8(x)           (*((volatile rt_uint8_t *)(x)))
#endif

#ifndef RT_CPU_CACHE_LINE_SZ
#define RT_CPU_CACHE_LINE_SZ    32
#endif

enum RT_HW_CACHE_OPS
{
    RT_HW_CACHE_FLUSH      = 0x01,
    RT_HW_CACHE_INVALIDATE = 0x02,
};

/*
 * CPU interfaces
 */


/* define cache ops as empty */
#define rt_hw_cpu_icache_enable(...)
#define rt_hw_cpu_icache_disable(...)
#define rt_hw_cpu_icache_ops(...)
#define rt_hw_cpu_dcache_enable(...)
#define rt_hw_cpu_dcache_disable(...)
#define rt_hw_cpu_dcache_ops(...)

#define rt_hw_cpu_icache_status(...) 0
#define rt_hw_cpu_dcache_status(...) 0



void rt_hw_cpu_reset(void);
void rt_hw_cpu_shutdown(void);

const char *rt_hw_cpu_arch(void);

rt_uint8_t *rt_hw_stack_init(void       *entry,
                             void       *parameter,
                             rt_uint8_t *stack_addr,
                             void       *exit);

/*
 * Interrupt handler definition
 */
typedef void (*rt_isr_handler_t)(int vector, void *param);

struct rt_irq_desc
{
    rt_isr_handler_t handler;
    void            *param;

#ifdef RT_USING_INTERRUPT_INFO
    char             name[RT_NAME_MAX];
    rt_uint32_t      counter;
#endif
};

/*
 * Interrupt interfaces
 */
void rt_hw_interrupt_init(void);
void rt_hw_interrupt_mask(int vector);
void rt_hw_interrupt_umask(int vector);
rt_isr_handler_t rt_hw_interrupt_install(int              vector,
                                         rt_isr_handler_t handler,
                                         void            *param,
                                         const char      *name);


rt_base_t rt_hw_interrupt_disable(void);
void rt_hw_interrupt_enable(rt_base_t level);


/*
 * Context interfaces
 */

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to);
void rt_hw_context_switch_to(rt_ubase_t to);
#if 0
void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to, rt_thread_t from_thread, rt_thread_t to_thread);

#else
void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to);
#endif


void rt_hw_console_output(const char *str);

void rt_hw_backtrace(rt_uint32_t *fp, rt_ubase_t thread_entry);
void rt_hw_show_memory(rt_uint32_t addr, rt_size_t size);

/*
 * Exception interfaces
 */
void rt_hw_exception_install(rt_err_t (*exception_handle)(void *context));

/*
 * delay interfaces
 */
void rt_hw_us_delay(rt_uint32_t us);



#define RT_DEFINE_SPINLOCK(x)    rt_ubase_t x
#define RT_DECLARE_SPINLOCK(x)

#define rt_hw_spin_lock(lock)     *(lock) = rt_hw_interrupt_disable()
#define rt_hw_spin_unlock(lock)   rt_hw_interrupt_enable(*(lock))

typedef int rt_spinlock_t;




#define rt_hw_isb()
#define rt_hw_dmb()
#define rt_hw_dsb()


#ifdef __cplusplus
}
#endif

#endif
