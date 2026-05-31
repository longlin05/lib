#include <stdlib.h>
#include <stdint.h>

#include "mem_pool.h"
#include "../common/error_code.h"
#include "../common/error_def.h"
#include "../common/sys_def.h"

#ifdef MEM_POOL_FOR_SINGLE_LIST
    #define MEM_POOL_BLOCK_SIZE  8U
#elif defined(MEM_POOL_FOR_FIFO)
    #define MEM_POOL_BLOCK_SIZE  32U
#else
    #define MEM_POOL_BLOCK_SIZE  64U
#endif
#define PAGE_SIZE 4096U
#define MEM_POOL_ALIGN_SIZE 4U

static MemPoolCb_t s_mem_pool_cb = NULL;

static void mem_pool_trigger_cb(uint32_t event, uint32_t status, 
                                void* data, uint32_t len) {
    if (s_mem_pool_cb != NULL) {
        s_mem_pool_cb(event, status, data, len);
    }
}

void mem_pool_set_cb(MemPoolCb_t cb) {
    s_mem_pool_cb = cb;
}

//小块空闲链表用于复用内存
typedef struct mem_block {
    struct mem_block* next; //单向链表，串联所有空闲块
}mem_block_t;

//内存池大页一次性申请的内存
typedef struct mem_page {
    struct mem_page* next;   //指向下一个页
    void* raw_ptr;           //原始malloc地址
    uint8_t* start;          //页起始地址
    uint8_t* end;            //页结束地址
    size_t free;             //当前页剩余可用大小
}mem_page_t;

//内存池核心控制结构体
typedef struct mem_pool{
    mem_page_t* first_page;  //分配的第一页
    mem_page_t* curr_page;   //正在分配的内存页
    mem_block_t* free_list;  //空闲小块内存链表
    size_t page_size;        //单个内存页大小
    uint32_t align;          //内存对齐字节数
}mem_pool_t;

//切割内存块
static void mem_page_cut(const mem_pool_t* pool, mem_page_t* page) {
    //拿到第一个块的头部
    mem_block_t* curr = (mem_block_t*)page -> start;
    while (page -> free >= MEM_POOL_BLOCK_SIZE) {
        mem_block_t* next_block = (mem_block_t*)((char*)curr + MEM_POOL_BLOCK_SIZE);
        curr -> next = next_block;
        page -> free -= MEM_POOL_BLOCK_SIZE;
        curr = next_block;
    }
    curr -> next = NULL;
    page -> free = page -> end - page -> start;
}

//创建页
static struct mem_page* mem_page_create(mem_pool_t *pool, bool_t is_first) {
    mem_page_t* page = (mem_page_t*)malloc(sizeof(mem_page_t));
    if (page == NULL) {
        mem_pool_trigger_cb(POOL_EXPAND_FAILURE, OPERATE_FAILURE, NULL, 0);
        return NULL;
    }
    void* raw_ptr = malloc(PAGE_SIZE);
    if (raw_ptr == NULL) {
        free(page);
        mem_pool_trigger_cb(POOL_EXPAND_FAILURE, OPERATE_FAILURE, NULL, 0);
        return NULL;
    }
    const uintptr_t raw_addr = (uintptr_t)raw_ptr;
    const uintptr_t aligned_addr = raw_addr + (pool -> align -  \
                        (raw_addr % pool -> align)) % pool -> align;
    page -> raw_ptr = raw_ptr;
    page -> start = (uint8_t*)aligned_addr;
    page ->end = (uint8_t*)raw_ptr + PAGE_SIZE;
    page -> free = page ->end - page ->start;
    page -> next = NULL;

    if (is_first == TRUE) {
        pool -> first_page = page;
        pool -> curr_page = page;
        pool -> free_list = (mem_block_t*)page -> start;
    } else {
        pool -> curr_page -> next = page;
        pool -> curr_page = page;
    }

    mem_page_cut(pool, page);
    mem_pool_trigger_cb(POOL_EXPAND_SUCCESS, OPERATE_SUCCESS, NULL, 0);
    return page;
}

//初始化内存池
void *mem_pool_init(void) {
    mem_pool_t* pool = (mem_pool_t*)malloc(sizeof(mem_pool_t));
    if (pool == NULL)
    {
        mem_pool_trigger_cb(POOL_INIT_FAILURE, OPERATE_FAILURE, NULL, 0);
        return NULL;
    }
    pool -> align = MEM_POOL_ALIGN_SIZE;
    pool -> page_size = PAGE_SIZE;
    pool -> first_page = NULL;
    pool -> curr_page = NULL;
    pool -> free_list = NULL;

    mem_page_t* page = mem_page_create(pool, TRUE);
    if (page == NULL) {
        free(pool);
        mem_pool_trigger_cb(POOL_INIT_FAILURE, OPERATE_FAILURE, NULL, 0);
        return NULL;
    }
    mem_pool_trigger_cb(POOL_INIT_SUCCESS, OPERATE_SUCCESS, NULL, 0);
    return pool;
}

//释放内存块
void mem_pool_free(mem_pool_t* pool, void *ptr) {
    if (pool == NULL || ptr == NULL)
    {
        mem_pool_trigger_cb(POOL_DESTROY_FAILURE, PARA_INVALID, NULL, 0);
        return;
    }
    mem_block_t* block = (mem_block_t*)ptr;
    block -> next = pool -> free_list;
    pool -> free_list = block;
    mem_pool_trigger_cb(POOL_DESTROY_SUCCESS, OPERATE_SUCCESS, NULL, 0);
}

//分配块
void *mem_pool_alloc(mem_pool_t *pool) {
    if (pool == NULL)
    {
        mem_pool_trigger_cb(POOL_ALLOC_FAILURE, PARA_INVALID, NULL, 0);
        return NULL;
    }
    if (pool -> free_list == NULL || pool -> curr_page \
                            -> free <= 2 * MEM_POOL_BLOCK_SIZE) {
        mem_page_t* page = mem_page_create(pool, FALSE);
        if (page == NULL)
        {
            mem_pool_trigger_cb(POOL_ALLOC_FAILURE, OPERATE_FAILURE, NULL, 0);
            return NULL;
        }
        pool -> free_list = (mem_block_t*)page -> start;
    }
    mem_block_t* curr_block = pool -> free_list;
    pool -> free_list = pool -> free_list -> next;
    mem_pool_trigger_cb(POOL_ALLOC_SUCCESS, OPERATE_SUCCESS, curr_block, 0);
    return (void*)curr_block;
}

//申请整页内存（不切块）
void* mem_pool_alloc_page(mem_pool_t *pool, size_t size) {
    if (pool == NULL || size == 0)
    {
        mem_pool_trigger_cb(POOL_ALLOC_FAILURE, PARA_INVALID, NULL, 0);
        return NULL;
    }

    mem_page_t* page = (mem_page_t*)malloc(sizeof(mem_page_t));
    if (page == NULL) {
        mem_pool_trigger_cb(POOL_EXPAND_FAILURE, OPERATE_FAILURE, NULL, 0);
        return NULL;
    }

    void* raw_ptr = malloc(size);
    if (raw_ptr == NULL) {
        free(page);
        mem_pool_trigger_cb(POOL_EXPAND_FAILURE, OPERATE_FAILURE, NULL, 0);
        return NULL;
    }

    const uintptr_t raw_addr = (uintptr_t)raw_ptr;
    const uintptr_t aligned_addr = raw_addr + (pool -> align -  \
                        (raw_addr % pool -> align)) % pool -> align;

    page -> raw_ptr = raw_ptr;
    page -> start = (uint8_t*)aligned_addr;
    page -> end = (uint8_t*)raw_ptr + size;
    page -> free = page -> end - page -> start;
    page -> next = NULL;

    if (pool -> first_page == NULL) {
        pool -> first_page = page;
        pool -> curr_page = page;
    } else {
        pool -> curr_page -> next = page;
        pool -> curr_page = page;
    }

    mem_pool_trigger_cb(POOL_ALLOC_SUCCESS, OPERATE_SUCCESS, page -> start, 0);
    return page -> start;
}

//释放内存池
void mem_pool_destroy(mem_pool_t* pool) {
    if (pool == NULL)
    {
        mem_pool_trigger_cb(POOL_DESTROY_FAILURE, PARA_INVALID, NULL, 0);
        return;
    }
    mem_page_t* first_page = pool -> first_page;
    while (first_page != NULL) {
        mem_page_t* next = first_page -> next;
        free(first_page -> raw_ptr);
        free(first_page);
        first_page = next;
    }
    free(pool);
    mem_pool_trigger_cb(POOL_DESTROY_SUCCESS, OPERATE_SUCCESS, NULL, 0);
}
