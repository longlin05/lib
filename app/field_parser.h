//
// Created by 祖龙 on 2026/5/28.
//

#ifndef LIB_FIELD_PARSER_H
#define LIB_FIELD_PARSER_H

#include <stdint.h>

// ===================== 字段解析配置 =====================

// 字节序枚举
typedef enum {
    BYTE_ORDER_BIG_ENDIAN,    // 大端序
    BYTE_ORDER_LITTLE_ENDIAN, // 小端序
} ByteOrder_e;

// 字段类型枚举
typedef enum {
    FIELD_TYPE_INT8,    // 有符号8位整数
    FIELD_TYPE_UINT8,   // 无符号8位整数
    FIELD_TYPE_INT16,   // 有符号16位整数
    FIELD_TYPE_UINT16,  // 无符号16位整数
    FIELD_TYPE_INT32,   // 有符号32位整数
    FIELD_TYPE_UINT32,  // 无符号32位整数
    FIELD_TYPE_FLOAT,   // 单精度浮点
    FIELD_TYPE_BITFIELD,// 位域
} FieldType_e;

// 字段解析配置结构体
typedef struct {
    uint32_t       offset;      // 字段偏移（字节）
    FieldType_e    type;        // 字段类型
    ByteOrder_e    byte_order;  // 字节序
    uint8_t        bit_start;   // 位域起始位（仅FIELD_TYPE_BITFIELD使用）
    uint8_t        bit_length;  // 位域长度（仅FIELD_TYPE_BITFIELD使用）
    char           name[32];    // 字段名称（用于显示）
} FieldConfig_t;

#endif //LIB_FIELD_PARSER_H
