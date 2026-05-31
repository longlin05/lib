//
// Created by 祖龙 on 2026/5/31.
//

#include "protocal.h"
#include <string.h>
#include "../common/error_def.h"

/**
 *解析帧工具
 *具体分发的业务根据硬件要求编写
 */

// ===================== 内部静态函数声明 =====================

static EventCb_Type Protocal_ParseFrame_Internal(const uint8_t* frame_buf, uint32_t frame_len, 
                                                  ProtocalType_e protocal_type, 
                                                  uint8_t* cmd_code, uint8_t* data, uint32_t* data_len);

static EventCb_Type Protocal_Dispatch_Internal(uint8_t cmd_code, const uint8_t* data, uint32_t data_len);

static void Protocal_Business_Heartbeat(const uint8_t* data, uint32_t data_len);

// ===================== 协议特征定义 =====================

#define MODBUS_MIN_LEN    4       // MODBUS最小帧长度
#define MODBUS_MAX_LEN    253     // MODBUS最大帧长度
#define MODBUS_FUNC_MIN   0x01    // MODBUS功能码最小值
#define MODBUS_FUNC_MAX   0x7E    // MODBUS功能码最大值

#define CAN_ID_STD_MASK   0x7FF   // CAN标准ID掩码

#define MQTT_MIN_LEN      2       // MQTT最小帧长度
#define MQTT_MAX_LEN      268     // MQTT最大帧长度（固定头部+可变头部+载荷）
#define MQTT_CTRL_MIN     0x01    // MQTT控制报文类型最小值
#define MQTT_CTRL_MAX     0x0E    // MQTT控制报文类型最大值

// ===================== 对外接口函数 =====================

/**
 * @brief 协议类型自动检测
 * @param frame_buf 帧数据缓冲区
 * @param frame_len 帧长度
 * @return 检测到的协议类型
 */
static ProtocalType_e Protocal_DetectType(const uint8_t* frame_buf, uint32_t frame_len)
{
    // 参数有效性检查（使用 volatile 避免编译器优化）
    volatile const uint8_t* volatile_frame_buf = (volatile const uint8_t*)frame_buf;
    volatile uint32_t volatile_frame_len = frame_len;
    
    if (volatile_frame_buf == NULL || volatile_frame_len == 0) {
        return PROTOCAL_TYPE_CUSTOM;
    }
    
    // MODBUS协议检测（RTU模式）
    // 特征：地址(1) + 功能码(1) + 数据(N) + CRC(2)
    if (frame_len >= MODBUS_MIN_LEN && frame_len <= MODBUS_MAX_LEN) {
        uint8_t func_code = frame_buf[1];
        if (func_code >= MODBUS_FUNC_MIN && func_code <= MODBUS_FUNC_MAX) {
            // 检查是否为有效的MODBUS功能码
            // 这里可以添加更多MODBUS特征检测（如CRC校验）
            return PROTOCAL_TYPE_MODBUS;
        }
    }
    
    // CAN协议检测（标准帧格式）
    // 特征：ID(2-4字节) + DLC(1) + 数据(0-8)
    if (frame_len >= 3 && frame_len <= 13) {
        // 检查是否符合CAN帧长度特征
        uint8_t dlc = frame_buf[2];  // 假设第3字节是DLC
        if (dlc <= 8 && (frame_len == dlc + 3)) {
            return PROTOCAL_TYPE_CAN;
        }
    }
    
    // HTTP协议检测
    // 特征：以 "GET " 或 "POST " 开头
    if (frame_len >= 4) {
        if ((frame_buf[0] == 'G' && frame_buf[1] == 'E' && frame_buf[2] == 'T' && frame_buf[3] == ' ') ||
            (frame_buf[0] == 'P' && frame_buf[1] == 'O' && frame_buf[2] == 'S' && frame_buf[3] == 'T')) {
            return PROTOCAL_TYPE_HTTP;
        }
    }
    
    // MQTT协议检测
    // 特征：固定头部(2+) + 可变头部 + 载荷
    // 第1个字节高4位为控制报文类型(0x01-0x0E)，低4位为标志位
    if (frame_len >= MQTT_MIN_LEN && frame_len <= MQTT_MAX_LEN) {
        uint8_t ctrl_type = (frame_buf[0] >> 4) & 0x0F;
        if (ctrl_type >= MQTT_CTRL_MIN && ctrl_type <= MQTT_CTRL_MAX) {
            return PROTOCAL_TYPE_MQTT;
        }
    }
    
    // 默认返回自定义协议
    return PROTOCAL_TYPE_CUSTOM;
}

/**
 * @brief 协议处理（解析帧并执行业务）
 * @param frame_buf 帧数据缓冲区
 * @param frame_len 帧长度
 * @param protocal_type 协议类型（PROTOCAL_TYPE_CUSTOM表示自动检测）
 */
void Protocal_Process(const uint8_t* frame_buf, uint32_t frame_len, 
                      ProtocalType_e protocal_type)
{
    uint8_t cmd_code = 0;
    uint8_t data[128] = {0};  // 数据区缓冲区
    uint32_t data_len = 0;
    
    // 1. 参数有效性检查
    if (frame_buf == NULL) {
        return;
    }
    
    // 2. 如果是自动检测模式，先检测协议类型
    if (protocal_type == PROTOCAL_TYPE_CUSTOM) {
        protocal_type = Protocal_DetectType(frame_buf, frame_len);
    }
    
    // 3. 调用内部解析函数
    EventCb_Type parse_result = Protocal_ParseFrame_Internal(frame_buf, frame_len, protocal_type, 
                                                             &cmd_code, data, &data_len);
    
    // 4. 如果解析成功，调用内部分发函数
    if (parse_result == PROTOCAL_PARSE_SUCCESS) {
        EventCb_Type dispatch_result = Protocal_Dispatch_Internal(cmd_code, data, data_len);
        (void)dispatch_result;  // 暂时不处理分发结果
    }
}

// ===================== 内部静态解析帧函数 =====================

static EventCb_Type Protocal_ParseFrame_Internal(const uint8_t* frame_buf, uint32_t frame_len, 
                                                  ProtocalType_e protocal_type, 
                                                  uint8_t* cmd_code, uint8_t* data, uint32_t* data_len)
{
    // 参数有效性检查（防御性编程，避免空指针访问）
    // 使用 volatile 避免编译器优化掉这个检查
    volatile uint8_t* volatile_cmd_code = (volatile uint8_t*)cmd_code;
    volatile uint8_t* volatile_data = (volatile uint8_t*)data;
    volatile uint32_t* volatile_data_len = (volatile uint32_t*)data_len;
    
    if (volatile_cmd_code == NULL || volatile_data == NULL || volatile_data_len == NULL) {
        return PROTOCAL_PARSE_INVALID_PARAM;
    }
    
    // 根据协议类型解析帧
    switch (protocal_type)
    {
        case PROTOCAL_TYPE_CUSTOM:
            // 自定义协议解析（示例，后续根据实际协议格式修改）
            // 假设协议格式：帧头(2) + 命令码(1) + 数据(N) + 校验(1) + 帧尾(1)
            if (frame_len < 5) {
                return PROTOCAL_PARSE_FRAME_TOO_SHORT;
            }
            
            // 示例：第3个字节是命令码
            *cmd_code = frame_buf[2];
            // 示例：数据区从第4个字节开始，长度=总长度-5
            *data_len = frame_len - 5;
            if (*data_len > 0) {
                memcpy(data, &frame_buf[3], *data_len);
            }
            break;
            
        case PROTOCAL_TYPE_MODBUS:
            // MODBUS协议解析（预留，后续根据硬件添加）
            break;
            
        case PROTOCAL_TYPE_CAN:
            // CAN协议解析（预留，后续根据硬件添加）
            break;
            
        case PROTOCAL_TYPE_MQTT:
            // MQTT协议解析
            // 固定头部：控制报文类型(高4位) + 标志位(低4位) + 剩余长度(1-4个字节)
            if (frame_len < 2) {
                return PROTOCAL_PARSE_FRAME_TOO_SHORT;
            }
            
            // 命令码 = 控制报文类型
            *cmd_code = (frame_buf[0] >> 4) & 0x0F;
            
            // 跳过固定头部，数据区从剩余长度之后开始
            // 这里简化处理，假设剩余长度为1个字节
            *data_len = frame_len - 2;
            if (*data_len > 0) {
                memcpy(data, &frame_buf[2], *data_len);
            }
            break;
            
        default:
            return PROTOCAL_PARSE_UNKNOWN_TYPE;
    }
    
    return PROTOCAL_PARSE_SUCCESS;
}

// ===================== 内部静态业务分发函数 =====================

static EventCb_Type Protocal_Dispatch_Internal(uint8_t cmd_code, const uint8_t* data, uint32_t data_len)
{
    // 按命令码分发给不同的业务处理函数
    switch (cmd_code)
    {
        case CMD_HEARTBEAT:
            Protocal_Business_Heartbeat(data, data_len);
            return PROTOCAL_PARSE_SUCCESS;
            
        default:
            // 未知命令码
            return PROTOCAL_PARSE_CMD_UNKNOWN;
    }
}

// ===================== 内部静态业务函数实现 =====================

/**
 * @brief 心跳业务处理（示例业务，后续根据硬件添加更多业务）
 * @param data 数据区
 * @param data_len 数据区长度
 */
static void Protocal_Business_Heartbeat(const uint8_t* data, uint32_t data_len)
{
    // TODO: 后续根据硬件添加具体业务逻辑
    (void)data;
    (void)data_len;
}
