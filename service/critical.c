//
// Created by 祖龙 on 2026/5/17.
//

#include "critical.h"

#if defined(__x86_64__) || defined(__i386__)
    #include <stdint.h>
    #include <sched.h>
#endif

// x86/x86_64 平台的全局锁变量
#if defined(__x86_64__) || defined(__i386__)
static volatile int critical_lock = 0;
#endif

void Critical_Enter(void)
{
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    __disable_irq();
#elif defined(__GNUC__) && (defined(__arm__) || defined(__thumb__))
    __asm volatile ("cpsid i" ::: "memory");
#elif defined(__ICCARM__)
    __disable_interrupt();
#elif defined(__x86_64__) || defined(__i386__)
    #if defined(__GNUC__)
        // 简单的自旋锁实现
        while (__sync_lock_test_and_set(&critical_lock, 1)) {
            sched_yield();
        }
    #endif
#endif
}

void Critical_Exit(void)
{
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    __enable_irq();
#elif defined(__GNUC__) && (defined(__arm__) || defined(__thumb__))
    __asm volatile ("cpsie i" ::: "memory");
#elif defined(__ICCARM__)
    __enable_interrupt();
#elif defined(__x86_64__) || defined(__i386__)
    #if defined(__GNUC__)
        __sync_lock_release(&critical_lock);
    #endif
#endif
}

uint32_t Critical_Lock(void)
{
    uint32_t primask = 0U;

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    primask = __get_PRIMASK();
    __disable_irq();
#elif defined(__GNUC__) && (defined(__arm__) || defined(__thumb__))
    __asm volatile ("mrs %0, primask" : "=r"(primask) :: "memory");
    __asm volatile ("cpsid i" ::: "memory");
#elif defined(__ICCARM__)
    primask = __get_PRIMASK();
    __disable_interrupt();
#elif defined(__x86_64__) || defined(__i386__)
    (void)primask;
    #if defined(__GNUC__)
        while (__sync_lock_test_and_set(&critical_lock, 1)) {
            sched_yield();
        }
    #endif
#endif

    return primask;
}

void Critical_Unlock(uint32_t primask)
{
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    __set_PRIMASK(primask);
#elif defined(__GNUC__) && (defined(__arm__) || defined(__thumb__))
    __asm volatile ("msr primask, %0" :: "r"(primask) : "memory");
#elif defined(__ICCARM__)
    __set_PRIMASK(primask);
#elif defined(__x86_64__) || defined(__i386__)
    (void)primask;
    #if defined(__GNUC__)
        __sync_lock_release(&critical_lock);
    #endif
#endif
}