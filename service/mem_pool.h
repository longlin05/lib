//
// Created by 祖龙 on 2026/5/13.
//
#ifndef LIB_MEM_POOL_H
#define LIB_MEM_POOL_H

#include "../common/sys_def.h"

#ifdef MEM_POOL_FOR_SINGLE_LIST
    #define MEM_POOL_BLOCK_SIZE  8U   
#elif defined(MEM_POOL_FOR_FIFO)
    #define MEM_POOL_BLOCK_SIZE  32U  
#else
    #define MEM_POOL_BLOCK_SIZE  64U  
#endif

typedef struct mem_block mem_block_t;
typedef struct mem_page mem_page_t;
typedef struct mem_pool mem_pool_t;

typedef void (*MemPoolCb_t)(uint32_t event, uint32_t status, 
                            void* data, uint32_t len);

/**
 * @brief 回调注册
 * @param cb: 回调函数指针
 */
void mem_pool_set_cb(MemPoolCb_t cb);

/**
 * @brief 内存池初始化
 * @return 返回内存池变量
 */
void *mem_pool_init(void);

/**
 * @brief 销毁内存池
 * @param pool: 内存池指针
 */
void mem_pool_destroy(mem_pool_t *pool);

/**
 * @brief 创建页并切块后分配块
 * @param pool: 内存池指针
 * @return 返回当前块地址
 */
void* mem_pool_alloc(mem_pool_t *pool);

/**
 * @brief 释放块（块释放后拼回空闲块链表）
 * @param pool: 内存池指针
 * @param ptr: 要释放的块地址
 */
void mem_pool_free(mem_pool_t *pool, void *ptr);

/**
 * @brief 分配整页（不切块）
 * @param pool: 内存池指针
 * @param size: 所需内存大小(字节)
 * @return 返回页起始地址
 */
void* mem_pool_alloc_page(mem_pool_t *pool, size_t size);

#endif //LIB_MEM_POOL_H
