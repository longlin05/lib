//
// Created by 祖龙 on 2026/5/27.
//

#ifndef FINITE_H
#define FINITE_H

//枚举状态
typedef enum {
    STATE_IDLE,    // 空闲
    STATE_RUN,     // 运行
    STATE_PAUSE,   // 暂停
    STATE_FAULT    // 故障
} StateType;

// 事件定义
#define EVENT_START    1
#define EVENT_PAUSE    2
#define EVENT_STOP     3
#define EVENT_ERROR    4
#define EVENT_RESET    5
#define EVENT_RESUME   6

// 函数声明（给外部调用）
void state_machine_run(int event);

#endif //FINITE_H
