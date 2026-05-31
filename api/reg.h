//
// Created by 祖龙 on 2026/5/13.
//
#ifndef REG_H
#define REG_H

#include <stdint.h>
#include "../app/event_cb.h"

// ===================== 宏定义 =====================
#define BIT_MASK(pos)        ((pos) < 32U ? (1UL << (pos)) : 0UL)                           //生成位掩码
#define BIT_SET(val, pos)    ((val) | BIT_MASK(pos))                    //置1
#define BIT_CLEAR(val, pos)  ((val) & (~BIT_MASK(pos)))                 //清零
#define BIT_TOGGLE(val, pos) ((val) ^ BIT_MASK(pos))                    //翻转
#define BIT_READ(val, pos)   (((val) & BIT_MASK(pos)) != 0UL ? 1U : 0U) //读取位

#define REG_SUCCESS        0
#define REG_ERR_INIT       1
#define REG_ERR_ID         2
#define REG_NO_CHANGE      3
#define REG_ERR_PARAM      4
#define REG_ERR_NOT_INIT   5
#define REG_ERR_INVALID_ID 6

//================位操作实现寄存器读写=================
//位宽枚举
typedef enum
{
    REG_WIDTH_8BIT,
    REG_WIDTH_16BIT,
    REG_WIDTH_32BIT
} RegWidth_t;

//寄存器编号枚举
typedef enum
{
    REG_NONE = 0,
    REG_CTRL,        // 8位
    REG_STATUS,      // 16位
    REG_DATA,        // 32位

    REG_MAX
} RegID_t;

//寄存器配置结构体
typedef struct
{
    RegID_t      reg_id;    //寄存器id
    RegWidth_t   width;     //位宽
    uint32_t     def_val;   //默认值
} RegConfig_t;

//寄存器事件回调类型
typedef void (*RegCb_t)(EventCb_Type event, EventCb_Status status, 
                        const RegConfig_t* data, uint32_t len);

// 功能函数
void Reg_Init(void);
void Reg_SetCb(RegCb_t cb);
uint32_t Reg_Read(RegID_t reg_id);
uint8_t Reg_SetValue(RegID_t reg_id, uint32_t write_val);
uint8_t reg_set1(RegID_t reg_id, uint8_t bit_pos);
uint8_t reg_set0(RegID_t reg_id, uint8_t bit_pos);
uint8_t Reg_ReadBit(RegID_t reg_id, uint8_t bit_pos);
uint8_t Reg_Toggle(RegID_t reg_id, uint8_t bit_pos);
uint8_t Reg_ToggleBit(RegID_t reg_id, uint8_t bit_pos);

#endif //REG_H
