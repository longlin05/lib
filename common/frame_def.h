//
// Created by 祖龙 on 2026/5/28.
//

#ifndef LIB_FRAME_DEF_H
#define LIB_FRAME_DEF_H

#include "sys_def.h"

// ===================== 帧类型枚举 =====================

// 帧类型枚举
typedef enum {
    FRAME_TYPE_FIXED_HEAD_TAIL,  // 固定帧头+帧尾
    FRAME_TYPE_FIXED_LENGTH,     // 定长帧
} FrameType_e;

// 帧状态枚举
typedef enum {
    FRAME_STATUS_NORMAL,     // 正常帧
    FRAME_STATUS_INCOMPLETE, // 不完整帧
    FRAME_STATUS_ERROR,      // 错误帧（校验失败等）
} FrameStatus_e;

// 校验类型枚举
typedef enum {
    CHECKSUM_NONE,   // 无校验
    CHECKSUM_SUM,    // 累加和
    CHECKSUM_CRC16,  // CRC16（MODBUS常用）
} ChecksumType_e;

// 协议类型枚举（后续添加新协议只需在此添加）
typedef enum {
    PROTOCAL_TYPE_CUSTOM,    // 自定义协议
    PROTOCAL_TYPE_MODBUS,    // MODBUS协议
    PROTOCAL_TYPE_CAN,       // CAN协议
    PROTOCAL_TYPE_HTTP,      // HTTP协议
    PROTOCAL_TYPE_MQTT,      // MQTT协议
} ProtocalType_e;

// ===================== 帧结构体 =====================

// 帧结构体
typedef struct {
    uint8_t*       data;        // 帧数据
    uint32_t       length;      // 帧长度
    uint64_t       timestamp;   // 时间戳（毫秒）
    FrameStatus_e  status;      // 帧状态
} Frame_t;

// 帧格式配置结构体
typedef struct {
    FrameType_e     type;         // 帧类型
    ProtocalType_e  protocal;     // 协议类型
    uint8_t*        frame_head;   // 帧头（NULL表示不使用）
    uint32_t        head_len;     // 帧头长度
    uint8_t*        frame_tail;   // 帧尾（NULL表示不使用）
    uint32_t        tail_len;     // 帧尾长度
    uint32_t        fixed_len;    // 定长帧长度（type=FIXED_LENGTH时使用）
    ChecksumType_e  checksum;     // 校验类型
    uint32_t        len_pos;      // 长度域位置（从0开始计数）
    uint32_t        max_len;      // 最大帧长度
} FrameFormat_t;

// ===================== 默认帧格式 =====================

// 默认帧头字节
#define DEFAULT_FRAME_HEAD_BYTE  0xAA
// 默认帧尾字节
#define DEFAULT_FRAME_TAIL_BYTE  0xEE

// 默认帧头帧尾的静态存储
static uint8_t s_default_frame_head = DEFAULT_FRAME_HEAD_BYTE;
static uint8_t s_default_frame_tail = DEFAULT_FRAME_TAIL_BYTE;

// 默认帧格式（最常用、最安全）
static const FrameFormat_t DEFAULT_FRAME_FORMAT = {
    .type           = FRAME_TYPE_FIXED_HEAD_TAIL,
    .protocal       = PROTOCAL_TYPE_CUSTOM,
    .frame_head     = &s_default_frame_head,
    .head_len       = 1,
    .frame_tail     = &s_default_frame_tail,
    .tail_len       = 1,
    .checksum       = CHECKSUM_NONE,
    .len_pos        = 1,             // 长度域在第2个字节（索引1）
    .max_len        = 256,           // 安全最大长度
};

#endif //LIB_FRAME_DEF_H