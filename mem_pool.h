//
// Created by 祖龙 on 2026/5/13.
//

#ifndef LIB_MEM_POOL_H
#define LIB_MEM_POOL_H


typedef struct mem_block mem_block_t;
typedef struct mem_page mem_page_t;
typedef struct mem_pool mem_pool_t;

void *mem_pool_init(void);
void mem_pool_destroy(mem_pool_t *pool);
void* mem_pool_alloc(mem_pool_t *pool);
void mem_pool_free(mem_pool_t *pool, void *ptr);

//专供环形FIFO
void *mem_pool_alloc_continuous(mem_pool_t *pool, uint32_t bytes);
void mem_pool_free_page(mem_pool_t* pool, void *page_ptr);

#endif //LIB_MEM_POOL_H
