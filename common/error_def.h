//
// Created by 祖龙 on 2026/5/27.
//

#ifndef LIB_ERROR_DEF_H
#define LIB_ERROR_DEF_H

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
    REG_EVENT_INIT = 100,   // 寄存器初始化完成
    REG_EVENT_WRITE,        // 写入值
    REG_EVENT_CHANGE,       // 值真正发生变化
    REG_EVENT_READ,         // 寄存器被读取
    REG_EVENT_ERROR,        // 非法访问/越界/写只读

    //环形FIFO缓存区间（200-299）
    EVENT_FIFO_INIT_SUCCESS = 200,    // FIFO 初始化成功
    EVENT_FIFO_INIT_FAILED,             // FIFO 初始化失败
    EVENT_FIFO_WRITE_ONE_OK,            // 单元素写入成功
    EVENT_FIFO_WRITE_ONE_FULL,          // 单元素写入失败（FIFO 满）
    EVENT_FIFO_WRITE_BATCH_OK,          // 批量写入全部成功
    EVENT_FIFO_WRITE_BATCH_PARTIAL,     // 批量写入部分成功
    EVENT_FIFO_WRITE_BATCH_FULL,        // 批量写入失败（FIFO 满）
    EVENT_FIFO_READ_ONE_OK,             // 单元素读取成功
    EVENT_FIFO_READ_ONE_EMPTY,          // 单元素读取失败（FIFO 空）
    EVENT_FIFO_READ_BATCH_OK,           // 批量读取全部成功
    EVENT_FIFO_READ_BATCH_PARTIAL,      // 批量读取部分成功
    EVENT_FIFO_READ_BATCH_EMPTY,        // 批量读取失败（FIFO 空）
    EVENT_FIFO_RESET,                   // FIFO 重置成功
    EVENT_FIFO_DEINIT,                  // FIFO 反初始化/销毁完成
    EVENT_FIFO_CLEAR,                   // FIFO 清空完成

    //单链表区间（300-399）
    SINGLE_LIST_OK = 300,           //操作成功

    /* 入参错误 */
    SINGLE_LIST_NULL_PTR,           //空指针（链表/节点为NULL）
    SINGLE_LIST_INVALID_PAR,        //无效参数（索引越界等）
    SINGLE_LIST_ERR_PARAM,          //入参错误
    /* 状态错误 */
    SINGLE_LIST_EMPTY,              //链表为空
    SINGLE_LIST_NODE_NOT_EXIST,     //节点不存在/不属于该链表
    SINGLE_LIST_NODE_EXIST,         //节点已存在（重复）

    /* 内存错误 */
    SINGLE_LIST_MEM_ALLOC_FAIL,     //内存池申请失败
    SINGLE_LIST_MEM_FREE_FAIL,      //内存池释放失败

    /* 操作失败 */
    SINGLE_LIST_OPER_FAIL,          //通用操作失败
    SINGLE_LIST_CYCLE,              //链表存在环
    SINGLE_LIST_NOT_FOUND,          //找不到节点

    //串口监视器区间（400-499）
    SERIAL_EVENT_OPEN_SUCCESS = 400,  // 串口打开成功
    SERIAL_EVENT_OPEN_FAILURE,        // 串口打开失败
    SERIAL_EVENT_CLOSE_SUCCESS,       // 串口关闭成功
    SERIAL_EVENT_CLOSE_FAILURE,       // 串口关闭失败
    SERIAL_EVENT_RECV_DATA,           // 接收到数据帧
    SERIAL_EVENT_SEND_SUCCESS,        // 发送成功
    SERIAL_EVENT_SEND_FAILURE,        // 发送失败
    SERIAL_EVENT_FRAME_ERROR,         // 帧错误（校验失败等）
    SERIAL_EVENT_FRAME_INCOMPLETE,    // 帧不完整
    SERIAL_EVENT_INIT_SUCCESS,        // 串口监视器初始化成功
    SERIAL_EVENT_INIT_FAILURE,        // 串口监视器初始化失败
    SERIAL_EVENT_DESTROY_SUCCESS,      // 串口监视器销毁成功
    
    //协议解析区间（500-599）
    PROTOCAL_PARSE_SUCCESS = 500,     // 协议解析成功
    PROTOCAL_PARSE_INVALID_PARAM,     // 无效参数
    PROTOCAL_PARSE_UNKNOWN_TYPE,      // 未知协议类型
    PROTOCAL_PARSE_FRAME_TOO_SHORT,   // 帧太短
    PROTOCAL_PARSE_CMD_UNKNOWN,       // 未知命令码
}EventCb_Type;

#endif //LIB_ERROR_DEF_H
