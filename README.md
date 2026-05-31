# Lib Toolkit

一个轻量级的嵌入式C语言工具库，提供多种常用的数据结构和工具函数。

## 目录结构

```
lib/
├── ring_fifo.h      # 环形FIFO实现
├── ring_fifo.c
├── link_list.h      # 单链表实现
├── link_list.c
├── reg.h            # 寄存器位操作工具
├── reg.c
├── event_cb.h       # 事件回调机制
├── event_cb.c
├── mem_pool.h       # 内存池管理
├── mem_pool.c
├── critical.h       # 临界区保护
├── critical.c
└── README.md        # 项目说明文档
```

## 模块说明

### 1. 环形FIFO (ring_fifo)

一个高效的环形缓冲区实现，支持以下特性：

- **静态/动态内存模式** - 支持外部提供缓冲区或内部自动申请
- **多线程安全** - 可选的临界区保护
- **计数/指针判空满** - 两种判空满策略
- **批量高速读写** - 支持连续内存块的高效读写
- **预览功能** - 只读不弹出的查看操作
- **数据丢弃** - 快速跳过指定数量的数据

**主要接口：**

| 函数 | 功能 |
|------|------|
| `RingFifo_StaticInit()` | 静态初始化（外部缓冲区） |
| `RingFifo_DynamicInit()` | 动态初始化（自动申请内存） |
| `RingFifo_WriteOne()` | 写入单个元素 |
| `RingFifo_ReadOne()` | 读取单个元素 |
| `RingFifo_PeekOne()` | 预览单个元素 |
| `RingFifo_WriteBatch()` | 批量写入 |
| `RingFifo_ReadBatch()` | 批量读取 |
| `RingFifo_PeekBatch()` | 批量预览 |
| `RingFifo_DiscardData()` | 丢弃数据 |
| `RingFifo_Reset()` | 重置FIFO |
| `RingFifo_Clear()` | 清空FIFO（不释放内存） |
| `RingFifo_DeInit()` | 销毁FIFO（释放内存池） |

**配置宏：**

```c
#define FIFO_USE_DYNAMIC_MEM   1   // 启用动态内存
#define FIFO_USE_LOCK_SAFE     1   // 开启线程安全
#define FIFO_USE_BIT_OPT       1   // 启用2次方位运算优化
#define FIFO_USE_DATA_COUNT    1   // 使用计数判空满
```

**使用示例：**

```c
// --- 静态初始化（外部数组）---
uint8_t buf[1024];
RingFifo_t fifo;
RingFifo_StaticInit(&fifo, buf, 256, 4);

// --- 动态初始化（内存池）---
mem_pool_t *pool = mem_pool_init();
RingFifo_t dynamic_fifo;
RingFifo_DynamicInit(&dynamic_fifo, pool, 64, sizeof(uint32_t));

// --- 通用操作 ---
uint32_t data = 0x12345678;
RingFifo_WriteOne(&dynamic_fifo, &data);
RingFifo_ReadOne(&dynamic_fifo, &data);

// --- 清空 vs 销毁 ---
RingFifo_Clear(&dynamic_fifo);  // 仅清空，不释放内存
RingFifo_DeInit(&dynamic_fifo, pool);  // 销毁并释放内存池
```

### 2. 寄存器工具 (reg)

提供位操作封装，简化寄存器读写操作。

**支持的寄存器：**

| 寄存器 | 位宽 | 描述 |
|--------|------|------|
| `REG_CTRL` | 8位 | 控制寄存器 |
| `REG_STATUS` | 16位 | 状态寄存器 |
| `REG_DATA` | 32位 | 数据寄存器 |

**主要接口：**

| 函数 | 功能 |
|------|------|
| `Reg_Init()` | 初始化寄存器模块 |
| `Reg_Read()` | 读取寄存器值 |
| `Reg_SetValue()` | 写入寄存器值 |
| `reg_set1()` | 置1指定位 |
| `reg_set0()` | 清零指定位 |
| `Reg_ReadBit()` | 读取指定位 |
| `Reg_Toggle()` | 翻转指定位 |
| `Reg_ToggleBit()` | 翻转指定位 |

**位操作宏：**

```c
BIT_MASK(pos)        // 生成位掩码
BIT_SET(val, pos)    // 置1
BIT_CLEAR(val, pos)  // 清零
BIT_TOGGLE(val, pos) // 翻转
BIT_READ(val, pos)   // 读取位
```

**使用示例：**

```c
// 初始化
Reg_Init();

// 写寄存器
Reg_SetValue(REG_CTRL, 0xAA);

// 位操作
reg_set1(REG_CTRL, 3);   // 置1第3位
reg_set0(REG_CTRL, 5);   // 清零第5位
uint8_t bit = Reg_ReadBit(REG_CTRL, 3);
```

### 3. 事件回调 (event_cb)

提供统一的事件触发和回调机制，用于模块间的异步通信和错误报告。

**事件类型分类：**

| 区间 | 范围 | 描述 |
|------|------|------|
| 内存池事件 | 0-99 | 内存池初始化、分配、释放等事件 |
| 寄存器事件 | 100-199 | 寄存器读写、变化、错误等事件 |
| FIFO事件 | 200-299 | FIFO操作成功、空、满、错误等事件 |

**主要接口：**

| 函数 | 功能 |
|------|------|
| `EventCb_Register()` | 注册事件回调函数 |
| `EventCb_TriggerSimple()` | 触发简单事件 |

**事件信息结构体：**

```c
typedef struct {
    EventCb_Type event;     // 事件类型
    EventCb_Status status;  // 状态码
    void* priv_data;        // 附加数据
    uint32_t data_len;      // 数据长度
    Module_Type module_id;  // 模块编号
}EventCb_Info_t;
```

**使用示例：**

```c
void my_event_handler(EventCb_Info_t *info) {
    switch (info->module_id) {
        case MODULE_FIFO:
            if (info->event == FIFO_ERR_FULL) {
                // 处理FIFO满事件
            }
            break;
    }
}

EventCb_Register(my_event_handler);
```

### 4. 单链表 (link_list)

一个轻量级的单向链表实现，支持完整的增删查改操作，依赖内存池进行节点管理。

**主要特性：**
- 支持头插、尾插、按索引插入
- 支持头删、尾删、按节点删除、按条件批量删除
- 支持按索引查找、按回调条件查找
- 支持链表反转、合并、截取、去重、排序
- 支持循环检测和中间节点查找

**主要接口：**

| 函数 | 功能 |
|------|------|
| `single_list_init()` | 初始化链表头 |
| `single_list_batch_init()` | 批量初始化（预创建节点） |
| `single_list_reset()` | 重置链表状态 |
| `single_list_insert_head()` | 头插节点 |
| `single_list_insert_tail()` | 尾插节点 |
| `single_list_insert_by_index()` | 按索引插入 |
| `single_list_delete_head()` | 删除头节点 |
| `single_list_delete_tail()` | 删除尾节点 |
| `single_list_delete_node()` | 删除指定节点 |
| `single_list_delete_by_cb()` | 按回调条件批量删除 |
| `single_list_find_by_index()` | 按索引查找 |
| `single_list_find_by_cb()` | 按回调条件查找 |
| `single_list_reverse()` | 链表反转 |
| `single_list_merge()` | 链表合并 |
| `single_list_cut()` | 链表截取 |
| `single_list_bubble_sort()` | 冒泡排序 |
| `single_list_check_cycle()` | 循环检测 |

**使用示例：**

```c
single_list_t list;
mem_pool_t *pool = mem_pool_init();

// 初始化链表
single_list_init(&list);

// 批量创建节点
single_list_batch_init(&list, 10, pool);

// 插入节点
single_list_node_t *node = mem_pool_alloc(pool);
single_list_insert_head(&list, node);

// 遍历查找
single_list_node_t *found = single_list_find_by_index(&list, 5);

// 删除节点
single_list_delete_head(&list, pool);

// 反转链表
single_list_reverse(&list);

// 清空链表
single_list_clear(&list, pool);

// 销毁内存池
mem_pool_destroy(pool);
```

### 5. 内存池 (mem_pool)

提供内存池管理功能，支持小块内存分配和连续内存块分配。通过宏定义可配置不同工具使用不同的块大小。

**块大小配置宏：**

| 宏定义 | 块大小 | 适用工具 |
|--------|--------|----------|
| `MEM_POOL_FOR_SINGLE_LIST` | 8 字节 | 单链表节点 |
| `MEM_POOL_FOR_FIFO` | 32 字节 | FIFO（未使用，FIFO用整页分配） |
| 默认 | 64 字节 | 通用场景 |

**主要接口：**

| 函数 | 功能 |
|------|------|
| `mem_pool_init()` | 初始化内存池 |
| `mem_pool_destroy()` | 销毁内存池 |
| `mem_pool_alloc()` | 分配内存块 |
| `mem_pool_free()` | 释放内存块 |
| `mem_pool_alloc_page()` | 分配整页内存（不切块，供FIFO使用） |

**使用示例：**

```c
mem_pool_t *pool = mem_pool_init();

// 分配单个内存块
void *ptr = mem_pool_alloc(pool);

// 分配整页内存（用于FIFO，默认4KB）
void *page = mem_pool_alloc_page(pool, 4096);

// 释放
mem_pool_free(pool, ptr);

// 销毁内存池（含整页内存）
mem_pool_destroy(pool);
```

**工具专用块大小配置：**

在包含 `mem_pool.h` 之前定义对应宏即可使用专用块大小：

```c
// 单链表专用（8字节块）
#define MEM_POOL_FOR_SINGLE_LIST
#include "mem_pool.h"

// FIFO专用（32字节块）
#define MEM_POOL_FOR_FIFO
#include "mem_pool.h"
```

### 6. 临界区保护 (critical)

提供临界区保护机制，用于多线程/中断环境下的数据保护。

**主要接口：**

| 函数 | 功能 |
|------|------|
| `Critical_Enter()` | 进入临界区 |
| `Critical_Exit()` | 退出临界区 |
| `Critical_Lock()` | 锁定并返回当前状态 |
| `Critical_Unlock()` | 根据状态解锁 |

**使用示例：**

```c
// 简单临界区保护
Critical_Enter();
// 临界区代码...
Critical_Exit();

// 嵌套临界区保护
uint32_t primask = Critical_Lock();
// 临界区代码...
Critical_Unlock(primask);
```

## 编译说明

使用 CMake 进行编译：

```bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
make
```

## 测试

项目包含完整的测试用例：

- **test_ring_fifo.c** - 环形FIFO覆盖测试
- **test_reg.c** - 寄存器工具测试
- **test_link_list.c** - 单链表测试
- **test_mem_pool_fifo.c** - 内存池+FIFO综合测试

运行测试：

```bash
./test_ring_fifo
./test_reg
./test_link_list
./test_mem_pool_fifo
```

## 许可证

MIT License

## 作者

祖龙