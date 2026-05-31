# Lib Toolkit

一个轻量级的嵌入式C语言工具库，提供多种常用的数据结构和工具函数。

## 目录结构

```
lib/
├── ring_fifo.h      # 环形FIFO实现
├── ring_fifo.c
├── reg.h            # 寄存器位操作工具
├── reg.c
├── event_cb.h       # 事件回调机制
├── event_cb.c
├── mem_pool.h       # 内存池管理
├── mem_pool.c
├── critical.h       # 临界区保护
├── critical.c
├── test_ring_fifo.c # FIFO测试用例
├── test_reg.c       # 寄存器测试用例
└── README.md        # 项目说明文档
```

## 模块说明

### 1. 环形FIFO (ring\_fifo)

一个高效的环形缓冲区实现，支持以下特性：

- **静态/动态内存模式** - 支持外部提供缓冲区或内部自动申请
- **多线程安全** - 可选的临界区保护
- **计数/指针判空满** - 两种判空满策略
- **批量高速读写** - 支持连续内存块的高效读写
- **预览功能** - 只读不弹出的查看操作
- **数据丢弃** - 快速跳过指定数量的数据

**主要接口：**

| 函数                       | 功能            |
| ------------------------ | ------------- |
| `RingFifo_StaticInit()`  | 静态初始化（外部缓冲区）  |
| `RingFifo_DynamicInit()` | 动态初始化（自动申请内存） |
| `RingFifo_WriteOne()`    | 写入单个元素        |
| `RingFifo_ReadOne()`     | 读取单个元素        |
| `RingFifo_PeekOne()`     | 预览单个元素        |
| `RingFifo_WriteBatch()`  | 批量写入          |
| `RingFifo_ReadBatch()`   | 批量读取          |
| `RingFifo_PeekBatch()`   | 批量预览          |
| `RingFifo_DiscardData()` | 丢弃数据          |
| `RingFifo_Reset()`       | 重置FIFO        |
| `RingFifo_DeInit()`      | 销毁FIFO        |

**配置宏：**

```c
#define FIFO_USE_DYNAMIC_MEM   1   // 启用动态内存
#define FIFO_USE_LOCK_SAFE     1   // 开启线程安全
#define FIFO_USE_BIT_OPT       1   // 启用2次方位运算优化
#define FIFO_USE_DATA_COUNT    1   // 使用计数判空满
```

**使用示例：**

```c
uint8_t buf[1024];
RingFifo_t fifo;

// 初始化
RingFifo_StaticInit(&fifo, buf, 256, 4);

// 写入数据
uint32_t data = 0x12345678;
RingFifo_WriteOne(&fifo, &data);

// 读取数据
uint32_t read_data;
RingFifo_ReadOne(&fifo, &read_data);
```

### 2. 寄存器工具 (reg)

提供位操作封装，简化寄存器读写操作。

**支持的寄存器：**

| 寄存器          | 位宽  | 描述    |
| ------------ | --- | ----- |
| `REG_CTRL`   | 8位  | 控制寄存器 |
| `REG_STATUS` | 16位 | 状态寄存器 |
| `REG_DATA`   | 32位 | 数据寄存器 |

**主要接口：**

| 函数                | 功能       |
| ----------------- | -------- |
| `Reg_Init()`      | 初始化寄存器模块 |
| `Reg_Read()`      | 读取寄存器值   |
| `Reg_SetValue()`  | 写入寄存器值   |
| `reg_set1()`      | 置1指定位    |
| `reg_set0()`      | 清零指定位    |
| `Reg_ReadBit()`   | 读取指定位    |
| `Reg_Toggle()`    | 翻转指定位    |
| `Reg_ToggleBit()` | 翻转指定位    |

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

### 3. 事件回调 (event\_cb)

提供统一的事件触发和回调机制，用于模块间的异步通信和错误报告。

**事件类型分类：**

| 区间     | 范围      | 描述                 |
| ------ | ------- | ------------------ |
| 内存池事件  | 0-99    | 内存池初始化、分配、释放等事件    |
| 寄存器事件  | 100-199 | 寄存器读写、变化、错误等事件     |
| FIFO事件 | 200-299 | FIFO操作成功、空、满、错误等事件 |

**主要接口：**

| 函数                        | 功能       |
| ------------------------- | -------- |
| `EventCb_Register()`      | 注册事件回调函数 |
| `EventCb_TriggerSimple()` | 触发简单事件   |

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

### 4. 内存池 (mem\_pool)

提供内存池管理功能，支持小块内存分配和连续内存块分配。

**主要接口：**

| 函数                            | 功能               |
| ----------------------------- | ---------------- |
| `mem_pool_init()`             | 初始化内存池           |
| `mem_pool_destroy()`          | 销毁内存池            |
| `mem_pool_alloc()`            | 分配内存块            |
| `mem_pool_free()`             | 释放内存块            |
| `mem_pool_alloc_continuous()` | 分配连续内存块（供FIFO使用） |
| `mem_pool_free_page()`        | 释放整页内存           |

**使用示例：**

```c
mem_pool_t *pool = mem_pool_init();

// 分配单个内存块
void *ptr = mem_pool_alloc(pool);

// 分配连续内存（用于FIFO）
void *buf = mem_pool_alloc_continuous(pool, 1024);

// 释放
mem_pool_free(pool, ptr);
mem_pool_free_page(pool, buf);

// 销毁内存池
mem_pool_destroy(pool);
```

### 5. 临界区保护 (critical)

提供临界区保护机制，用于多线程/中断环境下的数据保护。

**主要接口：**

| 函数                  | 功能        |
| ------------------- | --------- |
| `Critical_Enter()`  | 进入临界区     |
| `Critical_Exit()`   | 退出临界区     |
| `Critical_Lock()`   | 锁定并返回当前状态 |
| `Critical_Unlock()` | 根据状态解锁    |

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

不建议使用 CMake 进行编译，该程序是使用GDB编译的

## 许可证

MIT License

## 作者

祖龙
