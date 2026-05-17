//
// Created by 祖龙 on 2026/5/17.
//

#ifndef RING_FIFO_H
#define RING_FIFO_H

#include <stdint.h>
#include <stdbool.h>

#include "event_cb.h"
#include "mem_pool.h"

// ===================== 功能裁剪配置宏 =====================
#define FIFO_USE_DYNAMIC_MEM      1   // 1启用动态内存 0仅静态
#define FIFO_USE_LOCK_SAFE        1   // 1开启多线程/中断安全
#define FIFO_USE_BIT_OPT          1   // 1启用2次方位运算优化
#define FIFO_USE_DATA_COUNT       1   // 1用计数判空满 0用空余一格

// 默认FIFO元素大小(字节)
#define FIFO_DEFAULT_ELEM_SIZE    1U

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
    bool        lock_flag;      // 简易互斥锁标记
#endif
    bool        is_dynamic;     // 是否动态申请内存

    // 回调相关
    void (*callback)(EventCb_Info_t *info); // 通用回调
    void *user_data;                       // 用户私有数据
}RingFifo_t;

// 静态FIFO初始化(外部提供数组)
EventCb_Type RingFifo_StaticInit(RingFifo_t *fifo, uint8_t *buf,
                                 uint32_t cap, uint32_t elem_size);

// 动态FIFO初始化(内部自动申请内存)
EventCb_Type RingFifo_DynamicInit(RingFifo_t *fifo,mem_pool_t *pool,
                                  uint32_t cap, uint32_t elem_size);

// 重置FIFO(不清内存,只复位指针计数)
void RingFifo_Reset(RingFifo_t *fifo, bool clear_buf);

// 销毁FIFO(释放动态内存)
void RingFifo_DeInit(RingFifo_t *fifo, mem_pool_t *pool);

//基础读写接口
// 写入单个元素
EventCb_Type RingFifo_WriteOne(RingFifo_t *fifo, const void *p_data);

// 读取单个元素
EventCb_Type RingFifo_ReadOne(RingFifo_t *fifo, void *p_data);

// 预览单个元素(只读不弹出)
EventCb_Type RingFifo_PeekOne(const RingFifo_t *fifo, void *p_data);

//批量高速读写
// 批量写入指定长度元素
uint32_t RingFifo_WriteBatch(RingFifo_t *fifo, const void *p_src, uint32_t len);

// 批量读取指定长度元素
uint32_t RingFifo_ReadBatch(RingFifo_t *fifo, void *p_dst, uint32_t len);

//批量预览指定长度元素
uint32_t RingFifo_PeekBatch(const RingFifo_t *fifo, void *p_dst, uint32_t len);

// 丢弃指定数量已存数据
uint32_t RingFifo_DiscardData(RingFifo_t *fifo, uint32_t discard_len);

//状态查询
// 判断是否为空
bool RingFifo_IsEmpty(const RingFifo_t *fifo);

// 判断是否已满
bool RingFifo_IsFull(const RingFifo_t *fifo);

// 获取当前已存元素数量
uint32_t RingFifo_GetUsed(const RingFifo_t *fifo);

// 获取剩余空闲元素数量
uint32_t RingFifo_GetFree(const RingFifo_t *fifo);

// 获取FIFO总容量
uint32_t RingFifo_GetCapacity(const RingFifo_t *fifo);

#endif //RING_FIFO_H
