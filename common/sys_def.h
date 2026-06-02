#ifndef LIB_SYS_DEF_H
#define LIB_SYS_DEF_H

// 布尔类型定义（避免使用stdbool库）
typedef enum {
    FALSE = 0U,
    TRUE  = 1U
} bool_t;

// 优先使用标准库stddef.h（定义size_t、NULL等）
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    #include <stddef.h>
#elif defined(__GNUC__) || defined(__CC_ARM) || defined(__ICCARM__)
    #include <stddef.h>
#else
    // 非标准环境，自定义size_t和NULL
    #ifndef NULL
        #define NULL ((void *)0)
    #endif
    typedef unsigned int size_t;
#endif

// 整数类型定义：优先使用标准库stdint.h，否则自定义
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    #include <stdint.h>
#elif defined(__GNUC__) || defined(__CC_ARM) || defined(__ICCARM__)
    #include <stdint.h>
#else
    typedef unsigned char   uint8_t;
    typedef unsigned short  uint16_t;
    typedef unsigned int    uint32_t;
    typedef unsigned long long uint64_t;
    
    typedef signed char     int8_t;
    typedef signed short    int16_t;
    typedef signed int      int32_t;
    typedef signed long long int64_t;
#endif

#endif //LIB_SYS_DEF_H