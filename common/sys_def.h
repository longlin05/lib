//
// Created by 祖龙 on 2026/5/27.
//

#ifndef LIB_SYS_DEF_H
#define LIB_SYS_DEF_H

//布尔类型，避免使用stdbool库
typedef enum
{
    FALSE = 0U,
    TRUE  = 1U
} bool_t;

// 基本整数类型
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned long long uint64_t;

#endif //LIB_SYS_DEF_H
