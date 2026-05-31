//
// Created by 祖龙 on 2026/5/31.
//

#include "sys_time.h"

#ifdef _WIN32
#include <windows.h>
#elif __linux__ || __APPLE__
#include <unistd.h>
#include <time.h>
#endif

// 获取当前时间（毫秒）
uint64_t Sys_GetTickMs(void)
{
#ifdef _WIN32
    return GetTickCount64();
#elif __linux__ || __APPLE__
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#else
    return 0;
#endif
}

// 延时指定毫秒
void Sys_DelayMs(uint32_t ms)
{
#ifdef _WIN32
    Sleep(ms);
#elif __linux__ || __APPLE__
    usleep(ms * 1000);
#endif
}
