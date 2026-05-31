#include "finite.h"

//执行函数
//当前状态
static StateType current_state = STATE_IDLE;

// 工具函数（静态，内部用）
static void start_tool();
static void pause_tool();
static void stop_tool();
static void reset_system();
static void resume_tool();

void state_machine_run(int event)
{
    // 先根据【当前状态】处理
    switch (current_state)
    {
        case STATE_IDLE:
            // 空闲态：只处理“启动”事件
            if (event == EVENT_START)
            {
                // 执行动作
                start_tool();

                // **状态切换**（标准状态机核心动作）
                current_state = STATE_RUN;
            }
            break;

        case STATE_RUN:
            if (event == EVENT_PAUSE)
            {
                pause_tool();
                current_state = STATE_PAUSE;  // 切到暂停
            }
            else if (event == EVENT_STOP)
            {
                stop_tool();
                current_state = STATE_IDLE;   // 切回空闲
            }
            else if (event == EVENT_ERROR)
            {
                current_state = STATE_FAULT;  // 切到故障
            }
            break;

        case STATE_PAUSE:
            if (event == EVENT_RESUME)
            {
                resume_tool();
                current_state = STATE_RUN;    // 恢复运行
            }
            else if (event == EVENT_STOP)
            {
                stop_tool();
                current_state = STATE_IDLE;
            }
            break;

        case STATE_FAULT:
            if (event == EVENT_RESET)
            {
                reset_system();
                current_state = STATE_IDLE;   // 复位
            }
            break;
    }
}

//开始工具函数
static void start_tool(){}

//暂停工具函数
static void pause_tool(){}

//停止工具函数
static void stop_tool(){}

//重置状态工具函数
static void reset_system(){}

//恢复运行工具
static void resume_tool(){}