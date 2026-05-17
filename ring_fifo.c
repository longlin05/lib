#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ring_fifo.h"
#include  "event_cb.h"
#include "mem_pool.h"
#include "critical.h"

#define PAGE_SIZE 4096U

//安全锁函数声明
// 上锁
static void RingFifo_Lock(const RingFifo_t *fifo);

// 解锁
static void RingFifo_UnLock(const RingFifo_t *fifo);

// 静态FIFO初始化(外部提供数组)
EventCb_Type RingFifo_StaticInit(RingFifo_t *fifo, uint8_t *buf,
                                 uint32_t cap, uint32_t elem_size) {
    if (fifo == NULL || buf == NULL || cap == 0 || elem_size == 0) {
        return FIFO_ERR_PARAM;
    }
    memset(fifo, 0, sizeof(RingFifo_t));
    fifo->buf = buf;
    fifo->capacity = cap;
    fifo->elem_size = elem_size;
    fifo->wr_idx = 0;
    fifo->rd_idx = 0;
    fifo->is_dynamic = false;
    return FIFO_OK;
}

// 动态FIFO初始化(内部自动申请内存)
EventCb_Type RingFifo_DynamicInit(RingFifo_t *fifo,mem_pool_t *pool,
                                  uint32_t cap, uint32_t elem_size) {
    if (fifo == NULL || cap == 0 || elem_size == 0) {
        return FIFO_ERR_PARAM;
    }
    memset(fifo, 0, sizeof(RingFifo_t));
    fifo->capacity = cap;
    fifo->elem_size = elem_size;
    uint32_t total_bytes = fifo->capacity * fifo->elem_size;
    if (total_bytes > PAGE_SIZE) {
        return FIFO_ERR_PARAM;
    }
    fifo->buf = mem_pool_alloc_continuous(pool, total_bytes);
    if (fifo->buf == NULL) {return FIFO_ERR_MEM;}
    fifo->wr_idx = 0;
    fifo->rd_idx = 0;
#if FIFO_USE_DATA_COUNT
    fifo->data_cnt = 0;
#endif
    fifo->is_dynamic = true;
    return FIFO_OK;
}

// 重置FIFO(不清内存,只复位指针计数)
void RingFifo_Reset(RingFifo_t *fifo, bool clear_buf) {
    if (fifo == NULL) {return;}
    fifo->wr_idx = 0;
    fifo->rd_idx = 0;
#if FIFO_USE_DATA_COUNT
    fifo->data_cnt = 0;
#endif
#if FIFO_USE_LOCK_SAFE
    fifo->lock_flag = false;
#endif
    if (clear_buf == true && fifo->buf != NULL)
    {
        uint32_t total_bytes = fifo->capacity * fifo->elem_size;
        memset(fifo->buf, 0, total_bytes);
    }
}

// 销毁FIFO(释放动态内存)
void RingFifo_DeInit(RingFifo_t *fifo, mem_pool_t *pool) {
    // 1. 空指针校验
    if (fifo == NULL || pool == NULL)
    {
        return;
    }

    // 2. 只有动态申请的（整页）才需要释放
    if (fifo->is_dynamic == true && fifo->buf != NULL)
    {
        mem_pool_free_page(pool, fifo->buf);

        fifo->buf = NULL; // 指针清空，防止野指针
    }

    // 3. 清空整个FIFO结构体（专业必备）
    memset(fifo, 0, sizeof(RingFifo_t));
}

//基础读写接口
// 写入单个元素
EventCb_Type RingFifo_WriteOne(RingFifo_t *fifo, const void *p_data) {
    // 上锁
#if FIFO_USE_LOCK_SAFE
    RingFifo_Lock(fifo);
#endif
    // 1. 空指针校验
    if (fifo == NULL || p_data == NULL || fifo->buf == NULL)
    {
        return FIFO_ERR_PARAM;
    }

    // 2. 判断FIFO是否已满
#if FIFO_USE_DATA_COUNT
    // 计数模式下：直接用data_cnt判满
    if (fifo->data_cnt >= fifo->capacity)
    {
        EventCb_TriggerSimple(
                FIFO_ERR_FULL,
                MEM_OVERFLOW,
                fifo,
                sizeof(RingFifo_t),
                MODULE_FIFO
        );
        return FIFO_ERR_FULL;
    }
#else
    // 无计数模式下：用读写指针判满（写指针追上读指针）
    if ((fifo->wr_idx + 1) % fifo->capacity == fifo->rd_idx)
    {
        EventCb_TriggerSimple(
                FIFO_ERR_FULL,
                MEM_OVERFLOW,
                fifo,
                sizeof(RingFifo_t),
                MODULE_FIFO
        );
        return FIFO_FULL;
    }
#endif

    // 3. 计算写入的目标位置
    uint32_t write_offset = fifo->wr_idx * fifo->elem_size;
    uint8_t *dest_addr = &fifo->buf[write_offset];

    // 4. 拷贝数据（支持任意类型：uint8_t、int、float、结构体）
    memcpy(dest_addr, p_data, fifo->elem_size);

    // 5. 写指针 +1，到达尾部自动回绕
    fifo->wr_idx = (fifo->wr_idx + 1U) % fifo->capacity;

    // 6. 有效数据计数 +1
#if FIFO_USE_DATA_COUNT
    // 只有开启计数时，才更新data_cnt
    fifo->data_cnt++;
#endif

    // 解锁
#if FIFO_USE_LOCK_SAFE
    RingFifo_UnLock(fifo);
#endif

    // 7. 写入成功
    EventCb_TriggerSimple(
            FIFO_OK,
            OPERATE_SUCCESS,
            fifo,
            sizeof(RingFifo_t),
            MODULE_FIFO
    );
    return FIFO_OK;
}


// 读取单个元素
EventCb_Type RingFifo_ReadOne(RingFifo_t *fifo, void *p_data) {
    // 上锁
#if FIFO_USE_LOCK_SAFE
    RingFifo_Lock(fifo);
#endif

    // 1. 入参合法性校验
    if (fifo == NULL || p_data == NULL || fifo->buf == NULL)
    {
        return FIFO_ERR_PARAM;
    }

#if FIFO_USE_DATA_COUNT
    // 计数模式下：直接用data_cnt判空
    if (fifo->data_cnt == 0)
    {
        EventCb_TriggerSimple(
                FIFO_ERR_EMPTY,
                EMPTY,
                fifo,
                sizeof(RingFifo_t),
                MODULE_FIFO
        );
        return FIFO_ERR_EMPTY;
    }
#else
    // 无计数模式下：用读写指针判空
    if (fifo->wr_idx == fifo->rd_idx)
    {
        EventCb_TriggerSimple(
                FIFO_ERR_EMPTY,
                EMPTY,
                fifo,
                sizeof(RingFifo_t),
                MODULE_FIFO
        );
        return FIFO_EMPTY;
    }
#endif

    // 2. 计算读取的目标位置
    uint32_t read_offset = fifo->rd_idx * fifo->elem_size;
    uint8_t *src_addr = &fifo->buf[read_offset];

    // 3. 拷贝数据到用户缓冲区（支持任意类型）
    memcpy(p_data, src_addr, fifo->elem_size);

    // 4. 读指针 +1，到达尾部自动回绕
    fifo->rd_idx = (fifo->rd_idx + 1U) % fifo->capacity;

#if FIFO_USE_DATA_COUNT
    // 只有开启计数时，才更新data_cnt
    fifo->data_cnt--;
#endif

    // 解锁
#if FIFO_USE_LOCK_SAFE
    RingFifo_UnLock(fifo);
#endif

    // 5. 读取成功
    EventCb_TriggerSimple(
            FIFO_OK,
            OPERATE_SUCCESS,
            fifo,
            sizeof(RingFifo_t),
            MODULE_FIFO
    );
    return FIFO_OK;
}


// 预览单个元素(只读不弹出)
EventCb_Type RingFifo_PeekOne(const RingFifo_t *fifo, void *p_data) {
    // 1. 入参合法性校验
    if (fifo == NULL || p_data == NULL || fifo->buf == NULL)
    {
        return FIFO_ERR_PARAM;
    }

#if FIFO_USE_DATA_COUNT
    // 计数模式下：判空
    if (fifo->data_cnt == 0)
    {
        EventCb_TriggerSimple(
                FIFO_ERR_EMPTY,
                EMPTY,
                (void*)fifo,
                sizeof(RingFifo_t),
                MODULE_FIFO
        );
        return FIFO_ERR_EMPTY;
    }
#else
    // 无计数模式下：判空
    if (fifo->wr_idx == fifo->rd_idx)
    {
        EventCb_TriggerSimple(
                FIFO_ERR_EMPTY,
                EMPTY,
                (void *)fifo,
                sizeof(RingFifo_t),
                MODULE_FIFO
        );
        return FIFO_EMPTY;
    }
#endif

    // 2. 计算读取地址（和ReadOne完全一样）
    uint32_t read_offset = fifo->rd_idx * fifo->elem_size;
    uint8_t *src_addr = &fifo->buf[read_offset];

    // 3. 拷贝数据到用户缓冲区
    memcpy(p_data, src_addr, fifo->elem_size);

    return FIFO_OK;
}

//批量高速读写
// 批量写入指定长度元素
uint32_t RingFifo_WriteBatch(RingFifo_t *fifo, const void *p_src, uint32_t len) {
    // 上锁
#if FIFO_USE_LOCK_SAFE
    RingFifo_Lock(fifo);
#endif
    if (fifo == NULL || p_src == NULL || fifo->buf == NULL || len == 0)
    {
        return 0;
    }

    const uint8_t *p_data = p_src;
    uint32_t elem_size = fifo->elem_size;
    uint32_t write_cnt = 0;

#if FIFO_USE_DATA_COUNT
    // 计数模式：直接计算可用空间
    uint32_t available = fifo->capacity - fifo->data_cnt;
#else
    // 无计数模式：用读写指针计算可用空间
    uint32_t available;
    if (fifo->wr_idx >= fifo->rd_idx)
    {
        available = fifo->capacity - (fifo->wr_idx - fifo->rd_idx);
    }
    else
    {
        available = fifo->rd_idx - fifo->wr_idx;
    }
#endif

    if (available == 0)
    {
        EventCb_TriggerSimple(
                FIFO_ERR_FULL,
                MEM_OVERFLOW,
                fifo,
                sizeof(RingFifo_t),
                MODULE_FIFO
        );
        return 0;
    }

    // 实际写入的元素数，取请求长度和可用空间的最小值
    write_cnt = MIN(len, available);

    // 计算写指针到缓冲区末尾能连续写多少个元素
    uint32_t tail_cnt = fifo->capacity - fifo->wr_idx;
    uint32_t write1_cnt = MIN(write_cnt, tail_cnt);
    uint32_t write1_bytes = write1_cnt * elem_size;

    // 第一次拷贝：写入到缓冲区的连续尾部
    memcpy(&fifo->buf[fifo->wr_idx * elem_size], p_data, write1_bytes);

    // 更新源数据指针和剩余长度
    p_data += write1_bytes;
    uint32_t remaining = write_cnt - write1_cnt;

    if (remaining > 0)
    {
        // 第二次拷贝：数据回绕到缓冲区开头写入
        uint32_t write2_bytes = remaining * elem_size;
        memcpy(&fifo->buf[0], p_data, write2_bytes);
    }

    // 更新写指针
    fifo->wr_idx = (fifo->wr_idx + write_cnt) % fifo->capacity;

#if FIFO_USE_DATA_COUNT
    // 更新数据计数
    fifo->data_cnt += write_cnt;
#endif

    // 解锁
#if FIFO_USE_LOCK_SAFE
    RingFifo_UnLock(fifo);
#endif

    EventCb_TriggerSimple(
            FIFO_OK,
            OPERATE_SUCCESS,
            fifo,
            sizeof(RingFifo_t),
            MODULE_FIFO
    );
    return write_cnt;
}

// 批量读取指定长度元素
uint32_t RingFifo_ReadBatch(RingFifo_t *fifo, void *p_dst, uint32_t len) {
    // 上锁
#if FIFO_USE_LOCK_SAFE
    RingFifo_Lock(fifo);
#endif
    if (fifo == NULL || p_dst == NULL || fifo->buf == NULL || len == 0)
    {
        return 0;
    }

    uint8_t *p_data = (uint8_t *)p_dst;
    uint32_t elem_size = fifo->elem_size;
    uint32_t read_cnt = 0;

#if FIFO_USE_DATA_COUNT
    // 计数模式：直接获取当前数据量
    uint32_t available = fifo->data_cnt;
#else
    // 无计数模式：用读写指针计算已存数据量
    uint32_t available;
    if (fifo->wr_idx >= fifo->rd_idx)
    {
        available = fifo->wr_idx - fifo->rd_idx;
    }
    else
    {
        available = fifo->capacity - (fifo->rd_idx - fifo->wr_idx);
    }
#endif

    if (available == 0)
    {
        EventCb_TriggerSimple(
                FIFO_ERR_EMPTY,
                EMPTY,
                fifo,
                sizeof(RingFifo_t),
                MODULE_FIFO
        );
        return 0;
    }

    // 实际读取的元素数，取请求长度和可用数据量的最小值
    read_cnt = MIN(len, available);

    // 计算读指针到缓冲区末尾能连续读多少个元素
    uint32_t tail_cnt = fifo->capacity - fifo->rd_idx;
    uint32_t read1_cnt = MIN(read_cnt, tail_cnt);
    uint32_t read1_bytes = read1_cnt * elem_size;

    // 第一次拷贝：从缓冲区的连续尾部读取
    memcpy(p_data, &fifo->buf[fifo->rd_idx * elem_size], read1_bytes);

    // 更新目标数据指针和剩余长度
    p_data += read1_bytes;
    uint32_t remaining = read_cnt - read1_cnt;

    if (remaining > 0)
    {
        // 第二次拷贝：数据回绕到缓冲区开头读取
        uint32_t read2_bytes = remaining * elem_size;
        memcpy(p_data, &fifo->buf[0], read2_bytes);
    }

    // 更新读指针
    fifo->rd_idx = (fifo->rd_idx + read_cnt) % fifo->capacity;

#if FIFO_USE_DATA_COUNT
    // 更新数据计数
    fifo->data_cnt -= read_cnt;
#endif

    // 解锁
#if FIFO_USE_LOCK_SAFE
    RingFifo_UnLock(fifo);
#endif

    EventCb_TriggerSimple(
            FIFO_OK,
            OPERATE_SUCCESS,
            fifo,
            sizeof(RingFifo_t),
            MODULE_FIFO
    );
    return read_cnt;
}

//批量预览长度元素（不消费）
uint32_t RingFifo_PeekBatch(const RingFifo_t *fifo, void *p_dst, uint32_t len)
{
    if (fifo == NULL || p_dst == NULL || fifo->buf == NULL || len == 0)
    {
        return 0;
    }

    uint8_t *p_data = (uint8_t *)p_dst;
    uint32_t elem_size = fifo->elem_size;
    uint32_t peek_cnt = 0;

#if FIFO_USE_DATA_COUNT
    uint32_t available = fifo->data_cnt;
#else
    uint32_t available;
    if (fifo->wr_idx >= fifo->rd_idx)
    {
        available = fifo->wr_idx - fifo->rd_idx;
    }
    else
    {
        available = fifo->capacity - (fifo->rd_idx - fifo->wr_idx);
    }
#endif

    if (available == 0)
    {
        EventCb_TriggerSimple(
                FIFO_ERR_EMPTY,
                EMPTY,
                (void *)fifo,
                sizeof(RingFifo_t),
                MODULE_FIFO
        );
        return 0;
    }

    // 实际能预览多少
    peek_cnt = MIN(len, available);

    // 计算尾部连续可预览数量
    uint32_t tail_cnt = fifo->capacity - fifo->rd_idx;
    uint32_t peek1_cnt = MIN(peek_cnt, tail_cnt);
    uint32_t peek1_bytes = peek1_cnt * elem_size;

    // 第一次拷贝
    memcpy(p_data, &fifo->buf[fifo->rd_idx * elem_size], peek1_bytes);

    p_data += peek1_bytes;
    uint32_t remaining = peek_cnt - peek1_cnt;

    if (remaining > 0)
    {
        // 第二次拷贝（回绕）
        uint32_t peek2_bytes = remaining * elem_size;
        memcpy(p_data, &fifo->buf[0], peek2_bytes);
    }

    return peek_cnt;
}

// 丢弃指定数量已存数据
uint32_t RingFifo_DiscardData(RingFifo_t *fifo, uint32_t discard_len) {
    // 上锁
#if FIFO_USE_LOCK_SAFE
    RingFifo_Lock(fifo);
#endif
    if (fifo == NULL || discard_len == 0)
    {
        return 0;
    }

    uint32_t discard_cnt = 0;

#if FIFO_USE_DATA_COUNT
    // 计数模式：直接获取当前数据量
    uint32_t available = fifo->data_cnt;
#else
    // 无计数模式：用读写指针计算已存数据量
    uint32_t available;
    if (fifo->wr_idx >= fifo->rd_idx)
    {
        available = fifo->wr_idx - fifo->rd_idx;
    }
    else
    {
        available = fifo->capacity - (fifo->rd_idx - fifo->wr_idx);
    }
#endif

    if (available == 0)
    {
        return 0;
    }

    // 实际能丢弃的数量，取请求长度和可用数据量的最小值
    discard_cnt = MIN(discard_len, available);

    // 更新读指针（直接跳过数据，不拷贝）
    fifo->rd_idx = (fifo->rd_idx + discard_cnt) % fifo->capacity;

#if FIFO_USE_DATA_COUNT
    // 更新数据计数
    fifo->data_cnt -= discard_cnt;
#endif

    return discard_cnt;
}

//状态查询
// 判断是否为空
bool RingFifo_IsEmpty(const RingFifo_t *fifo) {
    if (fifo == NULL)
    {
        return true;
    }

#if FIFO_USE_DATA_COUNT
    // 计数模式：直接判断数据计数是否为0
    return (fifo->data_cnt == 0);
#else
    // 无计数模式：判断读写指针是否重合
    return (fifo->wr_idx == fifo->rd_idx);
#endif
}

// 判断是否已满
bool RingFifo_IsFull(const RingFifo_t *fifo) {
    if (fifo == NULL)
    {
        return false;
    }

#if FIFO_USE_DATA_COUNT
    // 计数模式：直接判断数据计数是否等于容量
    return (fifo->data_cnt >= fifo->capacity);
#else
    // 无计数模式：判断(写指针+1)是否追上读指针
    uint32_t next_wr = (fifo->wr_idx + 1) % fifo->capacity;
    return (next_wr == fifo->rd_idx);
#endif
}

// 获取当前已存元素数量
uint32_t RingFifo_GetUsed(const RingFifo_t *fifo) {
    if (fifo == NULL)
    {
        return 0;
    }

#if FIFO_USE_DATA_COUNT
    // 计数模式：直接返回data_cnt
    return fifo->data_cnt;
#else
    // 无计数模式：用读写指针计算已存数据量
    if (fifo->wr_idx >= fifo->rd_idx)
    {
        return fifo->wr_idx - fifo->rd_idx;
    }
    else
    {
        return fifo->capacity - (fifo->rd_idx - fifo->wr_idx);
    }
#endif
}

// 获取剩余空闲元素数量
uint32_t RingFifo_GetFree(const RingFifo_t *fifo) {
    if (fifo == NULL)
    {
        return 0;
    }

#if FIFO_USE_DATA_COUNT
    // 计数模式：容量 - 已用数量
    return fifo->capacity - fifo->data_cnt;
#else
    // 无计数模式：用读写指针计算空闲数量
    if (fifo->wr_idx >= fifo->rd_idx)
    {
        return fifo->capacity - (fifo->wr_idx - fifo->rd_idx);
    }
    else
    {
        return fifo->rd_idx - fifo->wr_idx;
    }
#endif
}

// 获取FIFO总容量
uint32_t RingFifo_GetCapacity(const RingFifo_t *fifo) {
    if (fifo == NULL)
    {
        return 0;
    }
    return fifo->capacity;
}

#if FIFO_USE_LOCK_SAFE

//安全锁
// 上锁
static void RingFifo_Lock(const RingFifo_t *fifo) {
    (void)fifo;
    Critical_Enter();

}

// 解锁
static void RingFifo_UnLock(const RingFifo_t *fifo) {
    (void)fifo;
    Critical_Exit();

}

#endif /* FIFO_USE_LOCK_SAFE */