//
// Created by 祖龙 on 2026/5/17.
//

#include "critical.h"

void Critical_Enter(void)
{
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    __disable_irq();
#elif defined(__GNUC__)
    __asm volatile ("cpsid i" ::: "memory");
#elif defined(__ICCARM__)
    __disable_interrupt();
#endif
}

void Critical_Exit(void)
{
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    __enable_irq();
#elif defined(__GNUC__)
    __asm volatile ("cpsie i" ::: "memory");
#elif defined(__ICCARM__)
    __enable_interrupt();
#endif
}

uint32_t Critical_Lock(void)
{
    uint32_t primask = 0U;

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    primask = __get_PRIMASK();
    __disable_irq();
#elif defined(__GNUC__)
    __asm volatile ("mrs %0, primask" : "=r"(primask) :: "memory");
    __asm volatile ("cpsid i" ::: "memory");
#elif defined(__ICCARM__)
    primask = __get_PRIMASK();
    __disable_interrupt();
#endif

    return primask;
}

void Critical_Unlock(uint32_t primask)
{
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    __set_PRIMASK(primask);
#elif defined(__GNUC__)
    __asm volatile ("msr primask, %0" :: "r"(primask) : "memory");
#elif defined(__ICCARM__)
    __set_PRIMASK(primask);
#endif
}