//
// Created by 祖龙 on 2026/5/17.
//

#ifndef RING_FIFO_H
#define RING_FIFO_H

#include "../common/sys_def.h"
#include "mem_pool.h"
#include "../common/error_def.h"

// ===================== 功能裁剪配置宏 =====================
#define FIFO_USE_DYNAMIC_MEM      1   // 1启用动态内存 0仅静态
#define FIFO_USE_LOCK_SAFE        1   // 1开启多线程/中断安全
#define FIFO_USE_BIT_OPT          1   // 1启用2次方位运算优化
#define FIFO_USE_DATA_COUNT       1   // 1用计数判空满 0用空余一格

// 回调类型定义
typedef void (*RingFifoCb_t)(uint32_t event, uint32_t status, 
                             void* data, uint32_t len);

// 默认FIFO元素大小(字节)
#define FIFO_BUF_CAPACITY     64       // 容量
#define FIFO_ELEMENT_SIZE     4        // 每个元素大小（字节）
#define FIFO_BUF_TOTAL_SIZE   (FIFO_BUF_CAPACITY * FIFO_ELEMENT_SIZE)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// 环形FIFO句柄结构体
typedef struct
{
    uint8_t     *buf;           // 数据缓存起始地址
    uint32_t    capacity;       // 最大容纳元素个数
    uint32_t    elem_size;      // 单个元素字节大小
    uint32_t    wr_idx;         // 写索引
    uint32_t    rd_idx;         // 读索引
#if FIFO_USE_DATA_COUNT
    uint32_t    data_cnt;       // 当前有效元素总数
#endif
#if FIFO_USE_LOCK_SAFE
    bool_t      lock_flag;      // 简易互斥锁标记
#endif
    bool_t      is_dynamic;     // 是否动态申请内存
}RingFifo_t;

/**
 * @brief 静态FIFO初始化(外部提供数组)
 * @param fifo: FIFO句柄
 * @param buf: FIFO缓冲区指针
 * @param cap: 容量(元素个数)
 * @param elem_size: 单个元素大小(字节)
 * @return 返回事件码
 */
EventCb_Type RingFifo_StaticInit(RingFifo_t *fifo, const uint8_t *buf,
                                 uint32_t cap, uint32_t elem_size);

/**
 * @brief 动态FIFO初始化(内部自动申请内存)
 * @param fifo: FIFO句柄
 * @param pool: 内存池指针
 * @param cap: 容量(元素个数)
 * @param elem_size: 单个元素大小(字节)
 * @return 返回事件码
 */
EventCb_Type RingFifo_DynamicInit(RingFifo_t *fifo, mem_pool_t *pool,
                                  uint32_t cap, uint32_t elem_size);

/**
 * @brief 重置FIFO(不清内存,只复位指针计数)
 * @param fifo: FIFO句柄
 * @param clear_buf: 是否清除缓冲区内容
 */
void RingFifo_Reset(RingFifo_t *fifo, bool_t clear_buf);

/**
 * @brief 清空FIFO(不清内存，只清空结构体)
 * @param fifo
 */
void RingFifo_Clear(RingFifo_t *fifo);

/**
 * @brief 销毁FIFO(释放动态内存)
 * @param fifo
 * @param pool
 */
void RingFifo_DeInit(RingFifo_t *fifo, mem_pool_t *pool);

//基础读写接口
/**
 * @brief 写入单个元素
 * @param fifo: FIFO句柄
 * @param p_data: 待写入的数据指针
 * @return 返回事件码
 */
EventCb_Type RingFifo_WriteOne(RingFifo_t *fifo, const void *p_data);

/**
 * @brief 读取单个元素
 * @param fifo: FIFO句柄
 * @param p_data: 读取数据的目标指针
 * @return 返回事件码
 */
EventCb_Type RingFifo_ReadOne(RingFifo_t *fifo, void *p_data);

/**
 * @brief 预览单个元素(只读不弹出)
 * @param fifo: FIFO句柄
 * @param p_data: 预览数据的目标指针
 * @return 返回事件码
 */
EventCb_Type RingFifo_PeekOne(const RingFifo_t *fifo, void *p_data);

//批量高速读写
/**
 * @brief 批量写入指定长度元素
 * @param fifo: FIFO句柄
 * @param p_src: 待写入数据的源指针
 * @param len: 要写入的元素数量
 * @return 返回写入元素长度
 */
uint32_t RingFifo_WriteBatch(RingFifo_t *fifo, const void *p_src, uint32_t len);

/**
 * @brief 批量读取指定长度元素
 * @param fifo: FIFO句柄
 * @param p_dst: 读取数据的目标指针
 * @param len: 要读取的元素数量
 * @return 返回读取元素长度
 */
uint32_t RingFifo_ReadBatch(RingFifo_t *fifo, void *p_dst, uint32_t len);

/**
 * @brief 批量预览指定长度元素
 * @param fifo: FIFO句柄
 * @param p_dst: 预览数据的目标指针
 * @param len: 要预览的元素数量
 * @return 返回预览元素长度
 */
uint32_t RingFifo_PeekBatch(const RingFifo_t *fifo, void *p_dst, uint32_t len);

/**
 * @brief 丢弃指定数量已存数据
 * @param fifo: FIFO句柄
 * @param discard_len: 要丢弃的元素数量
 * @return 返回丢弃元素长度
 */
uint32_t RingFifo_DiscardData(RingFifo_t *fifo, uint32_t discard_len);

//状态查询
/**
 * @brief 判断是否为空
 * @param fifo: FIFO句柄
 * @return 返回布尔值(TRUE为空,FALSE为非空)
 */
bool_t RingFifo_IsEmpty(const RingFifo_t *fifo);

/**
 * @brief 判断是否已满
 * @param fifo: FIFO句柄
 * @return 返回布尔值(TRUE为满,FALSE为未满)
 */
bool_t RingFifo_IsFull(const RingFifo_t *fifo);

/**
 * @brief 获取当前已存元素数量
 * @param fifo: FIFO句柄
 * @return 返回已存元素数
 */
uint32_t RingFifo_GetUsed(const RingFifo_t *fifo);

/**
 * @brief 获取剩余空闲元素数量
 * @param fifo: FIFO句柄
 * @return 返回剩余空闲元素数
 */
uint32_t RingFifo_GetFree(const RingFifo_t *fifo);

/**
 * @brief 获取FIFO总容量
 * @param fifo: FIFO句柄
 * @return 返回FIFO总容量
 */
uint32_t RingFifo_GetCapacity(const RingFifo_t *fifo);

/**
 * @brief 回调注册函数
 * @param cb: 回调函数指针
 */
void RingFifo_SetCb(RingFifoCb_t cb);

#endif //RING_FIFO_H
