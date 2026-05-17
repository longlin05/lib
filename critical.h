//
// Created by 祖龙 on 2026/5/17.
//

#ifndef LIB_CRITICAL_H
#define LIB_CRITICAL_H

#include <stdint.h>

void Critical_Enter(void);
void Critical_Exit(void);
uint32_t Critical_Lock(void);
void Critical_Unlock(uint32_t primask);

#endif //LIB_CRITICAL_H
