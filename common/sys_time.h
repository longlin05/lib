//
// Created by 祖龙 on 2026/5/31.
//

#ifndef LIB_SYS_TIME_H
#define LIB_SYS_TIME_H

#include "sys_def.h"

// 获取当前时间（毫秒）
uint64_t Sys_GetTickMs(void);

// 延时指定毫秒
void Sys_DelayMs(uint32_t ms);

#endif //LIB_SYS_TIME_H
