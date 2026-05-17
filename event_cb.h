//
// Created by 祖龙 on 2026/5/15.
//

#ifndef LIB_EVENT_CB_H
#define LIB_EVENT_CB_H

#include <stdint.h>

#include "reg.h"

//事件枚举
typedef enum {
    //内存池区间（0-99）
    POOL_INIT_SUCCESS = 0,  //内存池初始化完成
    POOL_INIT_FAILURE,      //内存池初始化失败
    POOL_EXPAND_SUCCESS,    //内存扩容成功
    POOL_EXPAND_FAILURE,    //内存扩容失败
    POOL_ALLOC_SUCCESS,     //内存申请成功
    POOL_ALLOC_FAILURE,     //内存申请失败
    POOL_DESTROY_SUCCESS,   //内存正常释放
    POOL_DESTROY_FAILURE,   //内存非法释放（重复释放、空指针）

    //寄存器区间（100-199）
    REG_EVENT_WRITE = 100,  // 写入值
    REG_EVENT_CHANGE,       // 值真正发生变化
    REG_EVENT_READ,         // 寄存器被读取
    REG_EVENT_ERROR,         // 非法访问/越界/写只读

    //环形FIFO缓存区间（200-299）
    FIFO_OK          = 200,   // 操作成功
    FIFO_ERR_EMPTY,           // 缓冲区空
    FIFO_ERR_FULL,            // 缓冲区满
    FIFO_ERR_PARAM,           // 入参非法
    FIFO_ERR_MEM,             // 内存分配失败
    FIFO_ERR_LEN              // 读写长度非法
}EventCb_Type;

//状态码枚举
typedef enum {
    OPERATE_SUCCESS = 0,    //操作成功
    OPERATE_FAILURE,        //操作失败
    PARA_INVALID,           //参数非法
    MEM_OVERFLOW,           //内存溢出
    EMPTY,                  //为空 / 无资源
    OUT_OF_RANGE,           //越界访问
    UNINITIALIZED,          //未初始化
    UNKNOWN_ERROR,          //未知错误
}EventCb_Status;

typedef enum {
    MODULE_NONE = 0,
    MODULE_MEM_POOL,
    MODULE_REGISTER,
    MODULE_FIFO,
}Module_Type;

//通用类型结构体
typedef struct {
    EventCb_Type event;     //事件类型
    EventCb_Status status;  //状态码
    void* priv_data;        //传递附加数据
    uint32_t data_len;      //记录priv_data长度
    Module_Type module_id;     //模块编号，判断事件来自什么工具
}EventCb_Info_t;

typedef void (*EventCb_Handler)(EventCb_Info_t *info);
typedef void (*EventCb_Handler_Reg)(RegConfig_t *param);

void EventCb_Register(EventCb_Handler handler);
void EventCb_TriggerSimple(EventCb_Type event,
                           EventCb_Status status,
                           void *priv_data,
                           uint32_t data_len,
                           uint32_t module_id);

#endif //LIB_EVENT_CB_H
