//
// Created by 祖龙 on 2026/5/31.
//

#ifndef LIB_PROTOCAL_H
#define LIB_PROTOCAL_H

#include <stdint.h>
#include "../common/frame_def.h"

// ===================== 命令码定义 =====================

#define CMD_HEARTBEAT            0x01  // 心跳（示例命令，后续根据硬件添加）

// ===================== 接口函数声明 =====================

/**
 * @brief 协议处理（解析帧并执行业务）
 * @param frame_buf 帧数据缓冲区
 * @param frame_len 帧长度
 * @param protocal_type 协议类型（PROTOCAL_TYPE_CUSTOM表示自动检测）
 */
void Protocal_Process(const uint8_t* frame_buf, uint32_t frame_len, 
                      ProtocalType_e protocal_type);

#endif //LIB_PROTOCAL_H
