//
// Created by 祖龙 on 2026/5/28.
//

#ifndef LIB_SERIAL_DEF_H
#define LIB_SERIAL_DEF_H

#include <stdint.h>

// ===================== 串口基础配置 =====================

// 波特率枚举
typedef enum {
    BAUD_9600   = 9600,
    BAUD_19200  = 19200,
    BAUD_38400  = 38400,
    BAUD_57600  = 57600,
    BAUD_115200 = 115200,
} BaudRate_e;

// 数据位枚举
typedef enum {
    DATABIT_5 = 5,
    DATABIT_6 = 6,
    DATABIT_7 = 7,
    DATABIT_8 = 8,   // 最常用
} DataBit_e;

// 校验位枚举
typedef enum {
    PARITY_NONE = 0, // 无校验（最常用）
    PARITY_ODD  = 1, // 奇校验
    PARITY_EVEN = 2, // 偶校验
} Parity_e;

// 停止位枚举
typedef enum {
    STOPBIT_1 = 0, // 1位停止位（最常用）
    STOPBIT_2 = 1, // 2位停止位
} StopBit_e;

// 串口配置结构体（核心）
typedef struct {
    char            port[16];    // 串口号 "COM3"
    BaudRate_e      baudrate;    // 波特率
    DataBit_e       databit;     // 数据位
    Parity_e        parity;      // 校验位
    StopBit_e       stopbit;     // 停止位
} SerialConfig_t;

// 串口设备结构体（句柄 + 配置）
typedef struct {
    void*           handle;      // Windows 串口句柄
    SerialConfig_t  config;      // 保存当前配置
    uint8_t         is_open;     // 状态：是否打开
} SerialDevice_t;

// 串口状态枚举
typedef enum {
    SERIAL_STATE_CLOSED,    // 串口未打开
    SERIAL_STATE_OPEN,      // 串口已打开
    SERIAL_STATE_ERROR,     // 串口异常
} SerialState_e;

#endif //LIB_SERIAL_DEF_H
