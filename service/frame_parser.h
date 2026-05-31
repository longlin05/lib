//
// Created by 祖龙 on 2026/5/28.
//

#ifndef LIB_FRAME_PARSER_H
#define LIB_FRAME_PARSER_H

#include <stdint.h>
#include "../common/frame_def.h"

/**
 *解析状态机
 */

// ===================== 切帧状态枚举 =====================

typedef enum {
    FRAME_PARSE_STATE_IDLE,     // 空闲状态，等待帧头
    FRAME_PARSE_STATE_HEADER,   // 正在匹配帧头
    FRAME_PARSE_STATE_LENGTH,   // 正在接收长度字段
    FRAME_PARSE_STATE_DATA,     // 正在接收数据体
    FRAME_PARSE_STATE_TAIL,     // 正在匹配帧尾
    FRAME_PARSE_STATE_CHECKSUM, // 正在校验
    FRAME_PARSE_STATE_COMPLETE, // 帧解析完成
} FrameParseState_e;

// ===================== 切帧结构体 =====================

typedef struct {
    FrameParseState_e  state;           // 当前解析状态
    FrameFormat_t      format;          // 帧格式配置
    uint8_t*           recv_buf_a;     // 接收缓冲区A指针
    uint8_t*           recv_buf_b;     // 接收缓冲区B指针
    uint8_t*           write_buf;       // 当前写入缓冲区指针
    uint8_t*           output_buf;      // 当前输出缓冲区指针
    uint32_t           buf_idx;         // 当前写入位置
    uint32_t           match_idx;       // 当前匹配位置（帧头/帧尾）
    uint32_t           data_len;        // 期望数据长度
    uint8_t            checksum;        // 临时校验值
    uint32_t           frame_length;    // 已存储帧的长度
    uint8_t            has_frame;       // 是否有完整帧（0=无，1=有）
} FrameParser_t;

// ===================== 接口函数声明 =====================

/**
 * @brief 初始化缓冲区（存储切完的帧）
 * @param parser: 帧解析器指针
 * @param format: 帧格式配置指针
 * @return 无
 */
void FrameParser_Init(FrameParser_t* parser, const FrameFormat_t* format);

/**
 * @brief 复位解析器（清空状态，重新开始）
 * @param parser: 帧解析器指针
 * @return 无
 */
void FrameParser_Reset(FrameParser_t* parser);

/**
 * @brief 喂入一个字节数据
 * @param parser: 帧解析器指针
 * @param byte: 待解析的字节数据
 * @return 返回判断结果(1=帧解析完成,0=继续解析)
 */
uint8_t FrameParser_Feed(FrameParser_t* parser, uint8_t byte);

/**
 * @brief 更新帧格式配置
 * @param parser: 帧解析器指针
 * @param format: 新的帧格式配置指针
 * @return 无
 */
void FrameParser_UpdateFormat(FrameParser_t* parser, const FrameFormat_t* format);

/**
 * @brief 查询是否有完整帧
 * @param parser: 帧解析器指针
 * @return 返回1表示有完整帧，0表示无
 */
uint8_t FrameParser_HasFrame(const FrameParser_t* parser);

/**
 * @brief 获取完整帧的缓冲区指针和长度
 * @param parser: 帧解析器指针
 * @param len: 输出参数，用于返回帧长度
 * @return 返回输出缓冲区指针（只读），无帧时返回NULL
 */
const uint8_t* FrameParser_GetFrameBuffer(const FrameParser_t* parser, uint32_t* len);

#endif //LIB_FRAME_PARSER_H