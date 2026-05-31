//
// Created by 祖龙 on 2026/5/28.
//

#ifndef LIB_SERIAL_H
#define LIB_SERIAL_H

#include "../common/error_code.h"
#include "../common/error_def.h"
#include "../common/serial_def.h"

// ===================== 回调类型定义 =====================

/**
 * @brief 写入单字节回调类型
 * @param byte: 待写入的字节
 * @return 返回事件码(成功/失败)
 */
typedef EventCb_Type (*Serial_WriteByteCb_t)(uint8_t byte);

// ===================== 接口函数声明 =====================

/**
 * @brief 注册写入回调
 * @param cb: 回调函数指针
 */
void Serial_SetWriteCb(Serial_WriteByteCb_t cb);

/**
 * @brief 打开串口（初始化）
 * @param dev: 串口设备指针
 * @param config: 串口配置指针
 * @return 返回状态码(OPERATE_SUCCESS/OPERATE_FAILURE/PARA_INVALID)
 */
EventCb_Status Serial_Open(SerialDevice_t* dev, const SerialConfig_t* config);

/**
 * @brief 关闭串口
 * @param dev: 串口设备指针
 * @return 返回状态码(OPERATE_SUCCESS/OPERATE_FAILURE/PARA_INVALID)
 */
EventCb_Status Serial_Close(SerialDevice_t* dev);

/**
 * @brief 接收数据并通过回调写入
 * @param dev: 串口设备指针
 * @return 返回写入的字节数
 */
uint32_t Serial_RecvToFifo(const SerialDevice_t* dev);

/**
 * @brief 发送数据
 * @param dev: 串口设备指针
 * @param data: 要发送的数据指针
 * @param len: 要发送的数据长度
 * @return 返回实际发送的字节数
 */
uint32_t Serial_Send(const SerialDevice_t* dev, const uint8_t* data, uint32_t len);


#endif //LIB_SERIAL_H