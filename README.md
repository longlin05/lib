# Lib Toolkit

一个轻量级的嵌入式C语言工具库，提供多种常用的数据结构和工具函数。

## 目录结构

```
lib/
├── api/                    # 应用编程接口
│   ├── reg.c               # 寄存器位操作工具
│   └── reg.h
├── app/                    # 应用层模块
│   ├── event_cb.c          # 事件回调机制
│   ├── event_cb.h
│   ├── field_parser.h      # 字段解析器
│   ├── protocal.c          # 协议处理
│   ├── protocal.h
│   ├── serial_monitor.c    # 串口监视器
│   └── serial_monitor.h
├── bsp/                    # 板级支持包
│   ├── serial.c            # 串口驱动封装
│   └── serial.h
├── common/                 # 公共定义
│   ├── error_code.h        # 错误码定义
│   ├── error_def.h         # 错误类型定义
│   ├── frame_def.h         # 帧格式定义
│   ├── serial_def.h        # 串口相关定义
│   ├── sys_def.h           # 系统基础定义
│   ├── sys_time.c          # 系统时间工具
│   └── sys_time.h
├── service/                # 服务层模块
│   ├── check.c             # 校验算法（CRC8/累加和）
│   ├── check.h
│   ├── critical.c          # 临界区保护
│   ├── critical.h
│   ├── finite.c            # 有限状态机
│   ├── finite.h
│   ├── frame_parser.c      # 帧解析器
│   ├── frame_parser.h
│   ├── link_list.c         # 单链表实现
│   ├── link_list.h
│   ├── mem_pool.c          # 内存池管理
│   ├── mem_pool.h
│   ├── ring_fifo.c         # 环形FIFO实现
│   └── ring_fifo.h
├── test/                   # 测试文件
│   ├── test_frame_parser.c
│   ├── test_protocal.c
│   └── test_serial_monitor.c
├── CMakeLists.txt          # CMake构建脚本
└── README.md               # 项目说明文档
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

RingFifo_StaticInit(&fifo, buf, 256, 4);

uint32_t data = 0x12345678;
RingFifo_WriteOne(&fifo, &data);

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
Reg_Init();

Reg_SetValue(REG_CTRL, 0xAA);

reg_set1(REG_CTRL, 3);
reg_set0(REG_CTRL, 5);
uint8_t bit = Reg_ReadBit(REG_CTRL, 3);
```

### 3. 事件回调 (event_cb)

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

### 4. 内存池 (mem_pool)

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

void *ptr = mem_pool_alloc(pool);

void *buf = mem_pool_alloc_continuous(pool, 1024);

mem_pool_free(pool, ptr);
mem_pool_free_page(pool, buf);

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
Critical_Enter();
// 临界区代码...
Critical_Exit();

uint32_t primask = Critical_Lock();
// 临界区代码...
Critical_Unlock(primask);
```

### 6. 单链表 (link_list)

提供通用单链表实现，支持多种操作和高级功能。

**主要特性：**

- 头插/尾插/按位置插入
- 按条件删除/安全删除
- 按索引/回调查找
- 遍历操作
- 链表反转/合并/截取/去重
- 冒泡排序
- 环检测/获取中间节点

**主要接口：**

| 函数                          | 功能            |
| --------------------------- | ------------- |
| `single_list_init()`         | 初始化链表头       |
| `single_list_batch_init()`   | 批量初始化（依赖内存池） |
| `single_list_insert_head()`  | 头插节点         |
| `single_list_insert_tail()`  | 尾插节点         |
| `single_list_insert_by_index()` | 按位置插入     |
| `single_list_delete_head()`  | 删除表头         |
| `single_list_delete_tail()`  | 删除表尾         |
| `single_list_delete_node()`  | 删除指定节点       |
| `single_list_find_by_index()`| 按索引查找       |
| `single_list_find_by_cb()`   | 按回调查找       |
| `single_list_reverse()`      | 链表反转         |
| `single_list_merge()`        | 链表合并         |
| `single_list_bubble_sort()`  | 冒泡排序         |
| `single_list_check_cycle()`  | 环检测          |

**使用示例：**

```c
single_list_t list;
single_list_init(&list);

single_list_node_t node = {NULL};
single_list_insert_tail(&list, &node);

single_list_node_t *found = single_list_find_by_index(&list, 0);

single_list_reverse(&list);
```

### 7. 有限状态机 (finite)

提供轻量级有限状态机实现，支持状态转换和事件处理。

**状态定义：**

| 状态        | 描述   |
| --------- | ---- |
| `STATE_IDLE`  | 空闲   |
| `STATE_RUN`   | 运行   |
| `STATE_PAUSE` | 暂停   |
| `STATE_FAULT` | 故障   |

**事件定义：**

| 事件          | 值   | 描述   |
| ----------- | --- | ---- |
| `EVENT_START` | 1   | 启动   |
| `EVENT_PAUSE` | 2   | 暂停   |
| `EVENT_STOP`  | 3   | 停止   |
| `EVENT_ERROR` | 4   | 错误   |
| `EVENT_RESET` | 5   | 重置   |
| `EVENT_RESUME`| 6   | 恢复   |

**主要接口：**

| 函数                 | 功能       |
| ------------------ | -------- |
| `state_machine_run()` | 运行状态机   |

**使用示例：**

```c
state_machine_run(EVENT_START);
state_machine_run(EVENT_PAUSE);
state_machine_run(EVENT_RESUME);
```

### 8. 校验工具 (check)

提供数据校验算法，支持累加和和CRC8校验。

**主要接口：**

| 函数              | 功能        |
| --------------- | --------- |
| `checksum_calc()` | 计算累加和     |
| `crc8_calc()`     | 计算CRC8校验值  |

**使用示例：**

```c
uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
uint8_t sum = checksum_calc(data, sizeof(data));
uint8_t crc = crc8_calc(data, sizeof(data));
```

### 9. 帧解析器 (frame_parser)

提供通用帧解析器，支持状态机方式解析串行数据帧。

**解析状态：**

| 状态                     | 描述       |
| ---------------------- | -------- |
| `FRAME_PARSE_STATE_IDLE`     | 空闲状态，等待帧头 |
| `FRAME_PARSE_STATE_HEADER`   | 正在匹配帧头   |
| `FRAME_PARSE_STATE_LENGTH`   | 正在接收长度字段 |
| `FRAME_PARSE_STATE_DATA`     | 正在接收数据体   |
| `FRAME_PARSE_STATE_TAIL`     | 正在匹配帧尾   |
| `FRAME_PARSE_STATE_CHECKSUM` | 正在校验     |
| `FRAME_PARSE_STATE_COMPLETE` | 帧解析完成   |

**主要接口：**

| 函数                          | 功能           |
| --------------------------- | ------------ |
| `FrameParser_Init()`         | 初始化帧解析器     |
| `FrameParser_Reset()`        | 复位解析器       |
| `FrameParser_Feed()`         | 喂入一个字节数据    |
| `FrameParser_HasFrame()`     | 查询是否有完整帧    |
| `FrameParser_GetFrameBuffer()` | 获取完整帧缓冲区    |
| `FrameParser_UpdateFormat()` | 更新帧格式配置     |

**使用示例：**

```c
FrameParser_t parser;
FrameFormat_t format = {
    .type = FRAME_TYPE_FIXED_HEAD_TAIL,
    .frame_head = (uint8_t[]){0xAA, 0xBB},
    .head_len = 2,
    .frame_tail = (uint8_t[]){0xCC},
    .tail_len = 1,
    .checksum = CHECKSUM_CRC16
};
FrameParser_Init(&parser, &format);

uint8_t byte;
while (Serial_Recv(&byte)) {
    if (FrameParser_Feed(&parser, byte) && FrameParser_HasFrame(&parser)) {
        uint32_t len;
        const uint8_t* frame = FrameParser_GetFrameBuffer(&parser, &len);
        // 处理完整帧
        FrameParser_Reset(&parser);
    }
}
```

### 10. 串口驱动 (serial)

提供串口设备的封装接口，支持打开、关闭、发送和接收操作。

**主要接口：**

| 函数                  | 功能           |
| ------------------- | ------------ |
| `Serial_SetWriteCb()` | 注册写入回调     |
| `Serial_Open()`      | 打开串口（初始化）  |
| `Serial_Close()`     | 关闭串口        |
| `Serial_RecvToFifo()` | 接收数据到FIFO   |
| `Serial_Send()`      | 发送数据        |

**使用示例：**

```c
SerialDevice_t dev;
SerialConfig_t config = {
    .port = "COM1",
    .baudrate = 115200,
    .databit = 8,
    .parity = 0,
    .stopbit = 1
};

Serial_Open(&dev, &config);

uint8_t data[] = "Hello";
Serial_Send(&dev, data, sizeof(data));

Serial_Close(&dev);
```

### 11. 系统时间 (sys_time)

提供系统时间相关的工具函数。

**主要接口：**

| 函数              | 功能       |
| --------------- | -------- |
| `Sys_GetTickMs()` | 获取当前时间（毫秒） |
| `Sys_DelayMs()`   | 延时指定毫秒   |

**使用示例：**

```c
uint64_t start = Sys_GetTickMs();

Sys_DelayMs(100);

uint64_t elapsed = Sys_GetTickMs() - start;
```

### 12. 串口监视器 (serial_monitor)

提供完整的串口监控功能，整合串口驱动、帧解析器和协议处理。

**主要特性：**

- 接收数据解析和帧提取
- 单次发送和循环发送支持
- 发送间隔控制和循环计数
- 显示模式设置（十六进制/ASCII）
- 帧格式和校验方式配置

**主要接口：**

| 函数                       | 功能           |
| ------------------------ | ------------ |
| `Serial_Create()`        | 创建串口监视器实例   |
| `Serial_Destroy()`       | 销毁串口监视器实例   |
| `Serial_Start()`         | 启动串口监视器     |
| `Serial_Stop()`          | 停止串口监视器     |
| `Serial_GetState()`      | 获取串口状态      |
| `Serial_ProcessRecv()`   | 处理接收数据      |
| `Serial_ProcessSend()`   | 处理发送数据      |
| `Serial_SendOnce()`      | 单次发送数据      |
| `Serial_SendLoop()`      | 循环发送数据（启动/处理） |
| `Serial_StopSend()`      | 停止发送        |
| `Serial_SetFrameFormat()` | 设置帧格式       |
| `Serial_SetChecksum()`   | 设置校验方式      |
| `Serial_SetDisplayMode()` | 设置显示模式      |

**发送配置结构体：**

```c
typedef struct {
    SendMode_e    mode;         // 发送模式：单次/循环
    uint32_t      interval_ms;  // 循环发送间隔（毫秒）
    uint32_t      loop_count;   // 循环次数（0表示无限循环）
    const uint8_t* data;        // 要发送的数据指针
    uint32_t      data_len;     // 数据长度
} SendConfig_t;
```

**使用示例：**

```c
SerialMonitor_t monitor;
Serial_Create(&monitor);

SerialConfig_t serial_config = {
    .port = "COM1",
    .baudrate = 115200,
    .databit = 8,
    .parity = 0,
    .stopbit = 1
};
Serial_Start(&monitor, &serial_config);

// 单次发送
uint8_t data[] = {0x01, 0x02, 0x03};
Serial_SendOnce(&monitor, data, sizeof(data));

// 循环发送配置
SendConfig_t loop_config = {
    .mode = SEND_MODE_LOOP,
    .interval_ms = 100,
    .loop_count = 10,
    .data = data,
    .data_len = sizeof(data)
};
Serial_SendLoop(&monitor, &loop_config);

// 主循环处理
while (1) {
    Serial_ProcessRecv(&monitor);
    Serial_ProcessSend(&monitor);
    Serial_SendLoop(&monitor, NULL);  // 处理循环发送
}

Serial_Stop(&monitor);
Serial_Destroy(&monitor);
```

### 13. 协议处理 (protocal)

提供协议解析和处理功能，支持多种协议类型的自动检测。

**支持的协议类型：**

| 协议类型           | 值   | 描述     |
| ---------------- | --- | ------ |
| `PROTOCAL_TYPE_CUSTOM` | 0   | 自定义协议   |
| `PROTOCAL_TYPE_MODBUS` | 1   | Modbus协议  |

**主要接口：**

| 函数                  | 功能           |
| ------------------- | ------------ |
| `Protocal_Process()` | 处理协议数据     |

**使用示例：**

```c
uint8_t frame_data[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
Protocal_Process(frame_data, sizeof(frame_data), PROTOCAL_TYPE_CUSTOM);
```

### 14. 字段解析器 (field_parser)

提供字段解析功能，用于从数据帧中提取特定字段。

**预留接口说明：**

该模块用于定义和解析协议中的各个字段，支持数据类型转换和字段映射。

## 编译说明

使用 CMake 进行编译：

```bash
mkdir build
cd build
cmake ..
make
```

**Windows 环境编译：**

```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

## 测试说明

项目包含三个测试模块：

```bash
# 运行所有测试
./test_frame_parser.exe   # 帧解析器测试
./test_protocal.exe      # 协议处理测试
./test_serial_monitor.exe # 串口监视器测试
```

## 许可证

MIT License

## 作者

祖龙
