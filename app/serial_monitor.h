//
// Created by 祖龙 on 2026/5/27.
//

#ifndef LIB_SERIAL_MONITOR_H
#define LIB_SERIAL_MONITOR_H

#include "../common/sys_def.h"

#include "../common/serial_def.h"
#include "../common/frame_def.h"
#include "../service/ring_fifo.h"
#include "../service/frame_parser.h"


// ===================== 数据显示配置 =====================

// 显示模式枚举
typedef enum {
    DISPLAY_HEX,    // 十六进制显示
    DISPLAY_ASCII,  // ASCII显示
} DisplayMode_e;

// ===================== 发送状态 =====================

typedef enum {
    SEND_STATE_IDLE,    // 空闲
    SEND_STATE_SENDING, // 发送中
} SendState_e;

// ===================== 解析状态 =====================

typedef enum {
    PARSE_STATE_IDLE,   // 空闲
    PARSE_STATE_BUSY,   // 解析中
} ParseState_e;

// ===================== 解析完成的帧 =====================

typedef struct {
    uint8_t*  data;        // 帧数据
    uint32_t  length;      // 帧长度
    uint64_t  timestamp;   // 时间戳
    FrameStatus_e status;  // 帧状态
} ParsedFrame_t;

// ===================== 发送功能配置 =====================

// 发送模式枚举
typedef enum {
    SEND_MODE_ONCE,      // 单次发送
    SEND_MODE_LOOP,      // 循环定时发送
} SendMode_e;

// 发送配置结构体
typedef struct {
    SendMode_e    mode;         // 发送模式
    uint32_t      interval_ms;  // 循环发送间隔（毫秒）
    uint32_t      loop_count;   // 循环次数（0表示无限循环）
    const uint8_t* data;        // 要发送的数据指针
    uint32_t      data_len;     // 数据长度
} SendConfig_t;

// ===================== 设备句柄 =====================

typedef struct {
    // BSP 串口设备
    SerialDevice_t serial_dev;

    // 接收 FIFO
    RingFifo_t rx_fifo;
    uint8_t rx_buf[256];  // 接收FIFO用缓冲区

    // 发送 FIFO
    RingFifo_t tx_fifo;
    uint8_t tx_buf[256];  // 发送FIFO用缓冲区

    // 帧解析器
    FrameParser_t parser;

    // 解析状态机
    ParseState_e parse_state;

    // 发送状态
    SendState_e send_state;

    // 显示模式
    DisplayMode_e display_mode;

    // 帧格式配置
    FrameFormat_t frame_format;

    // 解析完成的帧
    ParsedFrame_t parsed_frame;

    // 循环发送相关
    SendConfig_t loop_config;    // 循环发送配置
    uint8_t      loop_active;    // 循环发送是否激活
    uint32_t     loop_counter;   // 当前循环计数
    uint64_t     last_send_time; // 上次发送时间（毫秒）

    // 实例状态
    uint8_t is_created;  // 是否已初始化

    // 串口状态
    SerialState_e serial_state;  // 当前串口状态

} SerialMonitor_t;

// ===================== 接口函数声明 =====================

// 初始化/反初始化
/**
 * @brief 初始化（创建）串口监视器
 * @param monitor 实例结构体
 */
void Serial_Create(SerialMonitor_t *monitor);

/**
 * @brief 反初始化（销毁）串口监视器
 * @param monitor 实例结构体
 */
void Serial_Destroy(SerialMonitor_t* monitor);

// 串口操作
/**
 * @brief 接收处理（循环读取+校验）
 * @param monitor 实例结构体
 * @note 该函数需在主循环中周期性调用，内部会：
 *       1. 从串口读取数据到接收FIFO
 *       2. 从FIFO取出字节喂给帧解析器
 *       3. 解析出完整帧后触发接收数据回调
 */
void Serial_ProcessRecv(SerialMonitor_t* monitor);

/**
 * @brief 发送处理（从FIFO读取+发送）
 * @param monitor 实例结构体
 * @note 该函数需在主循环中周期性调用，内部会：
 *       1. 从发送FIFO读取数据
 *       2. 通过串口发送出去
 *       3. 返回实际发送的长度
 */
uint32_t Serial_ProcessSend(SerialMonitor_t* monitor);

// 发送控制
/**
 * @brief 单次发送
 * @param monitor 实例结构体
 * @param data 要发送的数据
 * @param len 数据长度
 * @return 实际写入FIFO的长度
 */
uint32_t Serial_SendOnce(SerialMonitor_t* monitor, const uint8_t* data, uint32_t len);

/**
 * @brief 循环发送（启动/处理）
 * @param monitor 实例结构体
 * @param config 发送配置（传入NULL表示仅处理循环发送）
 */
void Serial_SendLoop(SerialMonitor_t* monitor, const SendConfig_t* config);

/**
 * @brief 停止发送
 * @param monitor 实例结构体
 */
void Serial_StopSend(SerialMonitor_t* monitor);

// 状态控制
/**
 * @brief 启动串口监视器
 * @param monitor 实例结构体
 * @param config 串口配置
 */
void Serial_Start(SerialMonitor_t* monitor, const SerialConfig_t* config);

/**
 * @brief 停止串口监视器
 * @param monitor 实例结构体
 */
void Serial_Stop(SerialMonitor_t* monitor);

// 状态控制
SerialState_e Serial_GetState(const SerialMonitor_t* monitor);

// 配置
/**
 * @brief 设置帧格式
 * @param monitor 工具实例
 * @param format 调用者传入的帧格式
 */
void Serial_SetFrameFormat(SerialMonitor_t* monitor, const FrameFormat_t* format);

/**
 * @brief 设置校验方式
 * @param monitor 工具实例
 * @param type 校验类型（CHECKSUM_NONE/CHECKSUM_SUM/CHECKSUM_CRC16）
 */
void Serial_SetChecksum(SerialMonitor_t* monitor, ChecksumType_e type);

/**
 * @brief 设置显示模式
 * @param monitor 工具实例
 * @param mode 显示模式（DISPLAY_HEX/DISPLAY_ASCII）
 */
void Serial_SetDisplayMode(SerialMonitor_t* monitor, DisplayMode_e mode);

#endif //LIB_SERIAL_MONITOR_H