//
// Created by 祖龙 on 2026/5/15.
//

#ifndef LIB_EVENT_CB_H
#define LIB_EVENT_CB_H

#include "../common/sys_def.h"
#include "../common/error_code.h"
#include "../common/error_def.h"

// 向前声明
typedef struct single_list_node single_list_node_t;

typedef enum {
    MODULE_NONE = 0,
    MODULE_MEM_POOL,        //动态内存池
    MODULE_REGISTER,        //事件型寄存器
    MODULE_FIFO,            //环形FIFO缓存
    MODULE_LINK_LIST,       //单链表
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

/**
 * @brief 注册器，调用者调用以注册回调工具
 * @param handler 函数名或函数指针
 */
void EventCb_Register(EventCb_Handler handler);

/**
 * @brief 触发器赋值函数
 * @param event 事件类型
 * @param status 状态码
 * @param priv_data 传递的附加数据（可用结构体打包）
 * @param data_len 附加数据长度
 * @param module_id 模块编号
 */
void EventCb_TriggerSimple(EventCb_Type event,
                           EventCb_Status status,
                           void *priv_data,
                           uint32_t data_len,
                           uint32_t module_id);

#endif //LIB_EVENT_CB_H
