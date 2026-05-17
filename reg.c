#include <stdint.h>

#include "reg.h"
#include "event_cb.h"

//配置表
static const RegConfig_t reg_config_table[] =
{
    {REG_CTRL,   REG_WIDTH_8BIT,   0x00},
    {REG_STATUS, REG_WIDTH_16BIT,  0x0000},
    {REG_DATA,   REG_WIDTH_32BIT,  0x00000000},
};

//寄存器数值存储
static uint32_t reg_value[REG_MAX] = {0};
static uint8_t  reg_inited = 0;

//根据寄存器ID查位宽
static RegWidth_t Reg_GetWidth(RegID_t id)
{
    for (int i = 0; i < sizeof(reg_config_table)/sizeof(RegConfig_t); i++)
    {
        if (reg_config_table[i].reg_id == id)
        {
            return reg_config_table[i].width;
        }
    }
    return REG_WIDTH_32BIT;
}

//根据位宽自动裁剪数值
static uint32_t Reg_ClipValue(uint32_t val, RegWidth_t width)
{
    switch (width) {
        case REG_WIDTH_8BIT:  return val & 0xFF;
        case REG_WIDTH_16BIT: return val & 0xFFFF;
        case REG_WIDTH_32BIT:
        default: return val;
    }
}

//检查位位置是否有效
static uint8_t Reg_BitIsValid(RegID_t reg_id, uint8_t bit_pos)
{
    RegWidth_t width = Reg_GetWidth(reg_id);
    switch(width)
    {
        case REG_WIDTH_8BIT:
            return (bit_pos < 8) ? 1 : 0;
        case REG_WIDTH_16BIT:
            return (bit_pos < 16) ? 1 : 0;
        case REG_WIDTH_32BIT:
            return (bit_pos < 32) ? 1 : 0;
        default:
            return 0;
    }
}

//写寄存器
uint8_t Reg_SetValue(RegID_t reg_id, uint32_t write_val)
{
    //安全校验
    //模块未初始化，拒绝写入
    if (reg_inited == 0)
    {
        return REG_ERR_NOT_INIT;
    }

    //寄存器ID非法，拒绝写入
    if (reg_id >= REG_MAX || reg_id == REG_NONE)
    {
        return REG_ERR_INVALID_ID;
    }

    //获取寄存器当前值
    uint32_t old_val = Reg_Read(reg_id);

    //获取寄存器位宽 + 自动裁剪数值
    RegWidth_t width = Reg_GetWidth(reg_id);
    uint32_t new_val = Reg_ClipValue(write_val, width);

    //数值没变化，直接返回
    if (old_val == new_val)
    {
        return REG_NO_CHANGE;
    }

    //真正写入寄存器数组
    reg_value[reg_id] = new_val;

    //打包寄存器专属数据
    RegConfig_t reg_data = {
        .reg_id   = reg_id,
        .width    = width,
        .def_val  = new_val,
    };

    //触发寄存器值变化事件回调
    EventCb_TriggerSimple(
        REG_EVENT_CHANGE,       // 事件类型
        OPERATE_SUCCESS,        // 状态码
        &reg_data,              // 专属数据指针
        sizeof(RegConfig_t),// 数据长度
        MODULE_REGISTER         // 模块ID
    );

    //写入成功
    return REG_SUCCESS;
}

//置1
uint8_t reg_set1 (RegID_t reg_id, uint8_t bit_pos) {
    // 1. 校验位位置是否超出寄存器最大位数
    if(!Reg_BitIsValid(reg_id, bit_pos))
    {
        return REG_ERR_PARAM; // 自定义参数错误码
    }
    // 2. 读取当前寄存器原值
    uint32_t old_data = Reg_Read(reg_id);
    // 3. 位运算置1
    uint32_t new_data = old_data | (1U << bit_pos);
    // 4. 调用统一写接口完成写入，自动裁剪+判重
    return Reg_SetValue(reg_id, new_data);
}

//指定位清0
uint8_t reg_set0 (RegID_t reg_id, uint8_t bit_pos) {
    if(!Reg_BitIsValid(reg_id, bit_pos))
    {
        return REG_ERR_PARAM;
    }
    uint32_t old_data = Reg_Read(reg_id);
    uint32_t new_data = old_data & ~(1U << bit_pos);
    return Reg_SetValue(reg_id, new_data);
}

//整体翻转
uint8_t Reg_Toggle(RegID_t reg_id, uint8_t bit_pos)
{
    // 先校验位位置是否合法
    if (!Reg_BitIsValid(reg_id, bit_pos))
    {
        return REG_ERR_PARAM;
    }
    // 读取当前寄存器完整值
    uint32_t old_val = Reg_Read(reg_id);
    // 异或实现指定位翻转：0变1，1变0
    uint32_t new_val = old_val ^ (1U << bit_pos);
    // 调用底层唯一写接口完成写入
    return Reg_SetValue(reg_id, new_val);
}

//指定位翻转
uint8_t Reg_ToggleBit(RegID_t reg_id, uint8_t bit_pos)
{
    // 1. 检查位是否在寄存器有效范围内（8/16/32位）
    if (!Reg_BitIsValid(reg_id, bit_pos))
    {
        return REG_ERR_INVALID_ID; // 参数错误
    }

    // 2. 读取当前值
    uint32_t old_val = Reg_Read(reg_id);

    // 3. 指定位翻转（核心：异或 ^）
    uint32_t new_val = old_val ^ (1U << bit_pos);

    // 4. 调用统一写函数（自动裁剪、自动判重）
    return Reg_SetValue(reg_id, new_val);
}

//初始化
void Reg_Init(void)
{
    for (uint32_t i = 0; i < sizeof(reg_config_table)/sizeof(RegConfig_t); i++)
    {
        RegID_t id = reg_config_table[i].reg_id;
        reg_value[id] = reg_config_table[i].def_val;
    }
    reg_inited = 1;
}

//读取寄存器
uint32_t Reg_Read(RegID_t reg_id)
{
    // 未初始化，返回0
    if (!reg_inited)
        return 0;

    // ID非法，返回0
    if (reg_id >= REG_MAX || reg_id == REG_NONE)
        return 0;

    // 获取位宽并裁剪，返回正确值
    RegWidth_t width = Reg_GetWidth(reg_id);
    return Reg_ClipValue(reg_value[reg_id], width);
}

//读取指定位
uint8_t Reg_ReadBit(RegID_t reg_id, uint8_t bit_pos)
{
    // 先判断位是否合法
    if (!Reg_BitIsValid(reg_id, bit_pos))
        return 0;

    // 读整个寄存器 → 再取出指定位
    uint32_t val = Reg_Read(reg_id);
    return (val >> bit_pos) & 0x01;
}

