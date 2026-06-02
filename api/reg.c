#include <stddef.h>

#include "reg.h"
#include "../common/error_def.h"

static const RegConfig_t reg_config_table[] = {
    {REG_CTRL,   REG_WIDTH_8BIT,   0x00},
    {REG_STATUS, REG_WIDTH_16BIT,  0x0000},
    {REG_DATA,   REG_WIDTH_32BIT,  0x00000000},
};

static uint32_t reg_value[REG_MAX] = {0};
static uint8_t reg_inited = 0;
static RegCb_t s_reg_cb = NULL;

static void Reg_TriggerCb(EventCb_Type event, EventCb_Status status, 
                          const RegConfig_t* data) {
    if (s_reg_cb != NULL) {
        s_reg_cb(event, status, data, sizeof(RegConfig_t));
    }
}

void Reg_SetCb(RegCb_t cb) {
    s_reg_cb = cb;
}

static RegWidth_t Reg_GetWidth(RegID_t id) {
    for (uint32_t i = 0; i < sizeof(reg_config_table)/sizeof(RegConfig_t); i++) {
        if (reg_config_table[i].reg_id == id) {
            return reg_config_table[i].width;
        }
    }
    return REG_WIDTH_32BIT;
}

static uint32_t Reg_ClipValue(uint32_t val, RegWidth_t width) {
    switch (width) {
        case REG_WIDTH_8BIT:  return val & 0xFF;
        case REG_WIDTH_16BIT: return val & 0xFFFF;
        case REG_WIDTH_32BIT:
        default: return val;
    }
}

static uint8_t Reg_BitIsValid(RegID_t reg_id, uint8_t bit_pos) {
    RegWidth_t width = Reg_GetWidth(reg_id);
    switch(width) {
        case REG_WIDTH_8BIT:  return (bit_pos < 8) ? 1 : 0;
        case REG_WIDTH_16BIT: return (bit_pos < 16) ? 1 : 0;
        case REG_WIDTH_32BIT: return (bit_pos < 32) ? 1 : 0;
        default: return 0;
    }
}

uint8_t Reg_SetValue(RegID_t reg_id, uint32_t write_val) {
    if (reg_inited == 0) {
        RegConfig_t err_data = {reg_id, REG_WIDTH_32BIT, write_val};
        Reg_TriggerCb(REG_EVENT_ERROR, OPERATE_FAILURE, &err_data);
        return REG_ERR_NOT_INIT;
    }

    if (reg_id >= REG_MAX || reg_id == REG_NONE) {
        RegConfig_t err_data = {reg_id, REG_WIDTH_32BIT, write_val};
        Reg_TriggerCb(REG_EVENT_ERROR, OPERATE_FAILURE, &err_data);
        return REG_ERR_INVALID_ID;
    }

    uint32_t old_val = Reg_Read(reg_id);
    RegWidth_t width = Reg_GetWidth(reg_id);
    uint32_t new_val = Reg_ClipValue(write_val, width);

    if (old_val == new_val) {
        return REG_NO_CHANGE;
    }

    reg_value[reg_id] = new_val;

    RegConfig_t reg_data = {
        .reg_id   = reg_id,
        .width    = width,
        .def_val  = new_val,
    };

    Reg_TriggerCb(REG_EVENT_CHANGE, OPERATE_SUCCESS, &reg_data);

    return REG_SUCCESS;
}

uint8_t reg_set1(RegID_t reg_id, uint8_t bit_pos) {
    if(!Reg_BitIsValid(reg_id, bit_pos)) {
        return REG_ERR_PARAM;
    }
    uint32_t old_data = Reg_Read(reg_id);
    uint32_t new_data = old_data | (1U << bit_pos);
    return Reg_SetValue(reg_id, new_data);
}

uint8_t reg_set0(RegID_t reg_id, uint8_t bit_pos) {
    if(!Reg_BitIsValid(reg_id, bit_pos)) {
        return REG_ERR_PARAM;
    }
    uint32_t old_data = Reg_Read(reg_id);
    uint32_t new_data = old_data & ~(1U << bit_pos);
    return Reg_SetValue(reg_id, new_data);
}

uint8_t Reg_Toggle(RegID_t reg_id, uint8_t bit_pos) {
    if (!Reg_BitIsValid(reg_id, bit_pos)) {
        return REG_ERR_PARAM;
    }
    uint32_t old_val = Reg_Read(reg_id);
    uint32_t new_val = old_val ^ (1U << bit_pos);
    return Reg_SetValue(reg_id, new_val);
}

uint8_t Reg_ToggleBit(RegID_t reg_id, uint8_t bit_pos) {
    if (!Reg_BitIsValid(reg_id, bit_pos)) {
        return REG_ERR_INVALID_ID;
    }
    uint32_t old_val = Reg_Read(reg_id);
    uint32_t new_val = old_val ^ (1U << bit_pos);
    return Reg_SetValue(reg_id, new_val);
}

void Reg_Init(void) {
    for (uint32_t i = 0; i < sizeof(reg_config_table)/sizeof(RegConfig_t); i++) {
        RegID_t id = reg_config_table[i].reg_id;
        reg_value[id] = reg_config_table[i].def_val;
    }
    reg_inited = 1;
    
    RegConfig_t init_data = {REG_NONE, REG_WIDTH_32BIT, 0};
    Reg_TriggerCb(REG_EVENT_INIT, OPERATE_SUCCESS, &init_data);
}

uint32_t Reg_Read(RegID_t reg_id) {
    if (!reg_inited) return 0;
    if (reg_id >= REG_MAX || reg_id == REG_NONE) return 0;
    
    RegWidth_t width = Reg_GetWidth(reg_id);
    return Reg_ClipValue(reg_value[reg_id], width);
}

uint8_t Reg_ReadBit(RegID_t reg_id, uint8_t bit_pos) {
    if (!Reg_BitIsValid(reg_id, bit_pos)) return 0;
    uint32_t val = Reg_Read(reg_id);
    return (val >> bit_pos) & 0x01;
}
