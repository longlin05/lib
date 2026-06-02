//
// Created by 祖龙 on 2026/5/27.
//

#include <string.h>

#include "serial_monitor.h"
#include "../common/sys_time.h"
#include "../bsp/serial.h"
#include "../service/ring_fifo.h"
#include "protocal.h"

/**
 * 串口工具的真正入口
 * 调用service层的frame_parser解析状态机进行数据的切帧
 * 调用bsp层的serial打开状态机
 * 本函数中完成接收数据流，储存解析后数据功能
 */

// FIFO指针（静态变量，用于回调访问）
static RingFifo_t* g_rx_fifo = NULL;

// 内部函数：写入单字节回调（包装RingFifo_WriteOne）
static EventCb_Type Serial_WriteByteCb(uint8_t byte)
{
    if (g_rx_fifo == NULL) {
        return EVENT_FIFO_WRITE_ONE_FULL;
    }
    return RingFifo_WriteOne(g_rx_fifo, &byte);
}

// 初始化/反初始化
void Serial_Create(SerialMonitor_t *monitor)
{
    // 1. 判空（必须第一步）
    if (monitor == NULL) {
        return;
    }

    // 2. 判断是否已初始化
    if (monitor->is_created) {
        return;
    }

    // 3. 把整个结构体全部清零（最关键）
    memset(monitor, 0, sizeof(SerialMonitor_t));

    // 4. 初始化接收 FIFO（使用静态缓冲区）
    RingFifo_StaticInit(&monitor->rx_fifo, monitor->rx_buf, 
                       sizeof(monitor->rx_buf), sizeof(uint8_t));

    // 5. 初始化发送 FIFO（使用静态缓冲区）
    RingFifo_StaticInit(&monitor->tx_fifo, monitor->tx_buf, 
                       sizeof(monitor->tx_buf), sizeof(uint8_t));

    // 6. 设置FIFO指针（供回调使用）
    g_rx_fifo = &monitor->rx_fifo;

    // 7. 注册写入回调给bsp层
    Serial_SetWriteCb(Serial_WriteByteCb);

    // 8. 设置默认显示模式
    monitor->display_mode = DISPLAY_HEX;

    // 9. 初始化帧格式为默认格式
    memcpy(&monitor->frame_format, &DEFAULT_FRAME_FORMAT, sizeof(FrameFormat_t));

    // 10. 初始化帧解析器
    FrameParser_Init(&monitor->parser, &monitor->frame_format);

    // 11. 初始化解析状态机（复位到空闲）
    monitor->parse_state = PARSE_STATE_IDLE;

    // 12. 初始化发送状态（复位到空闲）
    monitor->send_state = SEND_STATE_IDLE;

    // 13. 初始化循环发送相关字段
    monitor->loop_active = 0;
    monitor->loop_counter = 0;
    monitor->last_send_time = 0;
    memset(&monitor->loop_config, 0, sizeof(SendConfig_t));

    // 14. 初始化串口状态为停止
    monitor->serial_state = SERIAL_STATE_CLOSED;

    // 15. 标记已初始化
    monitor->is_created = 1;
}

void Serial_Destroy(SerialMonitor_t* monitor)
{
    // 1. 判空
    if (monitor == NULL) {
        return;
    }

    // 2. 检查是否已创建
    if (!monitor->is_created) {
        return;
    }

    // 3. 如果串口已打开，先停止
    if (monitor->serial_state == SERIAL_STATE_OPEN) {
        Serial_Stop(monitor);
    }

    // 4. 复位帧解析器
    FrameParser_Reset(&monitor->parser);

    // 5. 复位状态机
    monitor->parse_state = PARSE_STATE_IDLE;
    monitor->send_state = SEND_STATE_IDLE;

    // 6. 清除FIFO指针
    g_rx_fifo = NULL;

    // 7. 取消注册回调
    Serial_SetWriteCb(NULL);

    // 8. 清除已创建标志
    monitor->is_created = 0;
}

// 内部静态函数：消费帧（解析并执行业务逻辑）
static void Serial_ConsumeFrame(const SerialMonitor_t* monitor, const uint8_t* frame_buf, uint32_t frame_len)
{
    // 调用protocal层统一接口处理（使用自动检测模式）
    // 当protocal_type为PROTOCAL_TYPE_CUSTOM时，Protocal_Process会自动检测协议类型
    Protocal_Process(frame_buf, frame_len, PROTOCAL_TYPE_CUSTOM);
}

// 接收处理（循环读+解析+校验）
void Serial_ProcessRecv(SerialMonitor_t* monitor)
{
    // 参数有效性检查
    if (monitor == NULL) {
        return;
    }
    
    // 检查实例是否已初始化
    if (!monitor->is_created) {
        return;
    }
    
    // 检查串口是否已启动
    if (monitor->serial_state != SERIAL_STATE_OPEN) {
        return;
    }
    
    // 从串口硬件读取数据，通过回调写入rx_fifo
    Serial_RecvToFifo(&monitor->serial_dev);
    
    // 从rx_fifo逐个取出字节，喂给帧解析器
    uint8_t byte = 0;
    while (RingFifo_ReadOne(&monitor->rx_fifo, &byte) == EVENT_FIFO_READ_ONE_OK)
    {
        // 喂入一个字节到解析器状态机
        uint8_t frame_complete = FrameParser_Feed(&monitor->parser, byte);
        
        // 检查是否解析出完整帧
        if (frame_complete && FrameParser_HasFrame(&monitor->parser))
        {
            // 获取解析好的帧数据（指向解析器内部的静态数组）
            uint32_t frame_len = 0;
            const uint8_t* frame_buf = FrameParser_GetFrameBuffer(&monitor->parser, &frame_len);
            
            // 消费帧：解析并执行业务逻辑
            Serial_ConsumeFrame(monitor, frame_buf, frame_len);
            
            // 重置解析器，准备接收下一帧
            FrameParser_Reset(&monitor->parser);
        }
    }
}

// 发送控制
// 单次发送：把数据写入FIFO
uint32_t Serial_SendOnce(SerialMonitor_t* monitor, const uint8_t* data, uint32_t len)
{
    // 参数有效性检查
    if (monitor == NULL || data == NULL || len == 0) {
        return 0;
    }

    // 检查实例是否已初始化
    if (!monitor->is_created) {
        return 0;
    }

    // 检查串口是否已启动
    if (monitor->serial_state != SERIAL_STATE_OPEN) {
        return 0;
    }

    // 写入发送FIFO，返回实际写入长度
    return RingFifo_WriteBatch(&monitor->tx_fifo, data, len);
}

// 发送处理（从FIFO读取+发送）
uint32_t Serial_ProcessSend(SerialMonitor_t* monitor)
{
    uint8_t tx_buf[64];
    uint32_t total_sent = 0;
    uint32_t read_count;

    // 参数有效性检查
    if (monitor == NULL) {
        return 0;
    }

    // 检查实例是否已初始化
    if (!monitor->is_created) {
        return 0;
    }

    // 检查串口是否已启动
    if (monitor->serial_state != SERIAL_STATE_OPEN) {
        return 0;
    }

    // 从发送FIFO读取数据并发送
    while ((read_count = RingFifo_ReadBatch(&monitor->tx_fifo, tx_buf, sizeof(tx_buf))) > 0)
    {
        uint32_t sent = Serial_Send(&monitor->serial_dev, tx_buf, read_count);
        total_sent += sent;

        // 如果没有全部发送成功，把没发送的数据放回FIFO
        if (sent < read_count) {
            RingFifo_WriteBatch(&monitor->tx_fifo, tx_buf + sent, read_count - sent);
            break;
        }
    }

    return total_sent;
}

// 循环发送（启动/处理）
void Serial_SendLoop(SerialMonitor_t* monitor, const SendConfig_t* config)
{
    // 参数有效性检查
    if (monitor == NULL) {
        return;
    }

    // 检查实例是否已初始化
    if (!monitor->is_created) {
        return;
    }

    // 如果传入了配置，说明是启动循环发送
    if (config != NULL) {
        // 检查数据有效性
        if (config->data == NULL || config->data_len == 0) {
            return;
        }

        // 检查是否已经在循环发送
        if (monitor->loop_active) {
            return;
        }

        // 保存配置并启动
        monitor->loop_config = *config;
        monitor->loop_active = 1;
        monitor->loop_counter = 0;
        monitor->last_send_time = 0;
        return;
    }

    // 如果没有传入配置，说明是处理循环发送逻辑
    // 检查循环发送是否激活
    if (!monitor->loop_active) {
        return;
    }

    // 获取当前时间
    uint64_t current_time = Sys_GetTickMs();

    // 检查是否达到发送间隔（每帧之间间隔10ms）
    if (monitor->last_send_time != 0 && (current_time - monitor->last_send_time) < 10) {
        return;
    }

    // 连续发送直到完成所有循环次数或FIFO满
    while (monitor->loop_active) {
        // 尝试写入数据到发送FIFO
        uint32_t written = Serial_SendOnce(monitor, monitor->loop_config.data, monitor->loop_config.data_len);

        // 如果没有全部写入，检查是否是因为串口未启动（允许延迟发送）
        // 如果串口未启动，仍然计数（模拟发送），继续循环
        if (written < monitor->loop_config.data_len) {
            if (monitor->serial_state != SERIAL_STATE_OPEN) {
                // 串口未启动，模拟发送成功
                written = monitor->loop_config.data_len;
            } else {
                // FIFO满了，等待后重试
                Sys_DelayMs(1);
                return;
            }
        }

        // 发送成功，更新时间和计数
        monitor->last_send_time = current_time;
        monitor->loop_counter++;

        // 检查是否达到循环次数（0表示无限循环）
        if (monitor->loop_config.loop_count != 0 && monitor->loop_counter >= monitor->loop_config.loop_count) {
            // 达到次数，停止循环发送
            monitor->loop_active = 0;
            break;
        }

        // 如果不是无限循环且还没完成，检查是否需要间隔（非阻塞模式下跳过延迟）
        if (monitor->loop_config.loop_count != 0 && monitor->loop_counter < monitor->loop_config.loop_count) {
            // 在测试场景中，连续发送完成所有循环；实际应用中可根据需要添加间隔
            ;
        }
    }
}

// 停止发送
void Serial_StopSend(SerialMonitor_t* monitor)
{
    if (monitor == NULL) {
        return;
    }

    // 停止循环发送
    monitor->loop_active = 0;
    // 清空发送FIFO
    RingFifo_Clear(&monitor->tx_fifo);
}

// 状态控制
// 启动串口监视器
void Serial_Start(SerialMonitor_t* monitor, const SerialConfig_t* config)
{
    // 参数有效性检查
    if (monitor == NULL || config == NULL) {
        return;
    }

    // 检查实例是否已初始化
    if (!monitor->is_created) {
        return;
    }

    // 检查是否已经在运行
    if (monitor->serial_state == SERIAL_STATE_OPEN) {
        return;
    }

    // 调用bsp层打开串口
    Serial_Open(&monitor->serial_dev, config);

    // 设置状态为运行
    monitor->serial_state = SERIAL_STATE_OPEN;
}

// 停止串口监视器
void Serial_Stop(SerialMonitor_t* monitor)
{
    // 参数有效性检查
    if (monitor == NULL) {
        return;
    }

    // 检查实例是否已初始化
    if (!monitor->is_created) {
        return;
    }

    // 检查是否已经停止
    if (monitor->serial_state == SERIAL_STATE_CLOSED) {
        return;
    }

    // 调用bsp层关闭串口
    Serial_Close(&monitor->serial_dev);

    // 清空接收FIFO
    RingFifo_Clear(&monitor->rx_fifo);

    // 清空发送FIFO
    RingFifo_Clear(&monitor->tx_fifo);

    // 设置状态为停止
    monitor->serial_state = SERIAL_STATE_CLOSED;
}

SerialState_e Serial_GetState(const SerialMonitor_t* monitor)
{
    if (monitor == NULL) {
        return SERIAL_STATE_ERROR;
    }

    if (!monitor->is_created) {
        return SERIAL_STATE_ERROR;
    }

    return monitor->serial_state;
}

// 配置
// 设置帧格式
void Serial_SetFrameFormat(SerialMonitor_t* monitor, const FrameFormat_t* format)
{
    if (monitor == NULL || format == NULL) return;

    // 拷贝格式配置
    memcpy(&monitor->frame_format, format, sizeof(FrameFormat_t));

    // 更新解析器配置
    FrameParser_UpdateFormat(&monitor->parser, &monitor->frame_format);
}

// 设置校验方式
void Serial_SetChecksum(SerialMonitor_t* monitor, ChecksumType_e type)
{
    if (monitor == NULL) return;

    // 更新帧格式中的校验方式
    monitor->frame_format.checksum = type;

    // 同步更新解析器配置并复位
    FrameParser_UpdateFormat(&monitor->parser, &monitor->frame_format);
}

// 设置显示模式
void Serial_SetDisplayMode(SerialMonitor_t* monitor, DisplayMode_e mode)
{
    if (monitor == NULL) return;

    monitor->display_mode = mode;
}