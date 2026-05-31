#include "event_cb.h"

static EventCb_Handler s_event_cb = NULL;

//注册器函数，面向上层
void EventCb_Register(EventCb_Handler handler) {
    s_event_cb = handler;
}

//触发器函数
static void EventCb_Trigger(EventCb_Info_t *info) {
    if (s_event_cb != NULL) {
        s_event_cb(info);
    }
}

//触发器通用赋值模板,面向其他工具
void EventCb_TriggerSimple (EventCb_Type event,
                           EventCb_Status status,
                           void *priv_data,
                           uint32_t data_len,
                           uint32_t module_id) {
    EventCb_Info_t info;

    info.module_id = module_id;
    info.event = event;
    info.status = status;
    info.priv_data = priv_data;
    info.data_len  = data_len;

    EventCb_Trigger(&info);
}