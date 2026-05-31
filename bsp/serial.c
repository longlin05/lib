//
// Created by 祖龙 on 2026/5/28.
//

#include "serial.h"
#include <string.h>

/**
 * 串口基础开关读写接口
 * 区分不同操作系统
 * 操作硬件
 */

#ifdef _WIN32
#include <windows.h>
#elif __linux__ || __APPLE__
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

// 静态回调变量
static Serial_WriteByteCb_t g_write_cb = NULL;

// 注册写入回调
void Serial_SetWriteCb(Serial_WriteByteCb_t cb)
{
    g_write_cb = cb;
}

EventCb_Status Serial_Open(SerialDevice_t *dev, const SerialConfig_t *config)
{
    // 1. 指针判空（必须）
    if (dev == NULL || config == NULL) {
        return PARA_INVALID;
    }

    // 2. 防重复打开
    if (dev->is_open) {
        return OPERATE_FAILURE;
    }

    // 3. 打开串口（多平台）
#ifdef _WIN32
    HANDLE hCom = CreateFileA(
        config->port,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if (hCom == INVALID_HANDLE_VALUE) {
        return OPERATE_FAILURE;
    }
    dev->handle = hCom;

#elif __linux__ || __APPLE__
    int fd = open(config->port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        return OPERATE_FAILURE;
    }
    dev->handle = (void*)(intptr_t)fd;
#endif

    // 4. 设置串口参数
#ifdef _WIN32
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    GetCommState(dev->handle, &dcb);

    dcb.BaudRate = config->baudrate;
    dcb.ByteSize = config->databit;
    dcb.Parity   = config->parity;
    dcb.StopBits = config->stopbit;

    SetCommState(dev->handle, &dcb);

#elif __linux__ || __APPLE__
    struct termios attr;
    tcgetattr((int)(intptr_t)dev->handle, &attr);

    // 设置波特率
    cfsetispeed(&attr, config->baudrate);
    cfsetospeed(&attr, config->baudrate);

    // 数据位
    attr.c_cflag &= ~CSIZE;
    switch(config->databit) {
        case 5: attr.c_cflag |= CS5; break;
        case 6: attr.c_cflag |= CS6; break;
        case 7: attr.c_cflag |= CS7; break;
        case 8: attr.c_cflag |= CS8; break;
    }

    // 校验位
    switch(config->parity) {
        case 0: attr.c_cflag &= ~PARENB; break;         // 无校验
        case 1: attr.c_cflag |= PARENB; attr.c_cflag &= ~PARODD; break; // 偶校验
        case 2: attr.c_cflag |= PARENB; attr.c_cflag |= PARODD; break; // 奇校验
    }

    // 停止位
    if(config->stopbit == 1)
        attr.c_cflag &= ~CSTOPB;
    else
        attr.c_cflag |= CSTOPB;

    tcsetattr((int)(intptr_t)dev->handle, TCSANOW, &attr);
#endif

    // 5. 清空缓冲区（系统API）
#ifdef _WIN32
    PurgeComm(dev->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
#elif __linux__ || __APPLE__
    tcflush((int)(intptr_t)dev->handle, TCIOFLUSH);
#endif

    // 6. 标记已打开
    dev->is_open = 1;
    dev->config  = *config;

    return OPERATE_SUCCESS;
}

// 关闭串口
EventCb_Status Serial_Close(SerialDevice_t* dev){
    if (dev == NULL) {
        return PARA_INVALID;
    }
    if (!dev->is_open) {
        return OPERATE_FAILURE;
    }

#ifdef _WIN32
    CloseHandle(dev->handle);
#elif __linux__ || __APPLE__
    close((int)(intptr_t)dev->handle);
#endif

    dev->is_open = 0;
    dev->handle = NULL;

    return OPERATE_SUCCESS;
}

// 接收数据并通过回调写入
uint32_t Serial_RecvToFifo(const SerialDevice_t* dev) {
    if (dev == NULL) {
        return 0;
    }
    if (!dev->is_open) {
        return 0;
    }
    if (g_write_cb == NULL) {
        return 0;
    }

    uint32_t write_count = 0;
    uint8_t byte = 0;

#ifdef _WIN32
    DWORD bytes_available = 0;
    COMSTAT comstat = {0};
    
    // 查询串口缓冲区有多少数据
    if (ClearCommError(dev->handle, NULL, &comstat)) {
        bytes_available = comstat.cbInQue;
    }
    
    // 逐字节读取并通过回调写入
    while (bytes_available > 0) {
        DWORD bytes_read = 0;
        if (ReadFile(dev->handle, &byte, 1, &bytes_read, NULL) && bytes_read > 0) {
            if (g_write_cb(byte) == EVENT_FIFO_WRITE_ONE_OK) {
                write_count++;
            }
            bytes_available--;
        } else {
            break;
        }
    }
#elif __linux__ || __APPLE__
    int fd = (int)(intptr_t)dev->handle;
    uint8_t temp_buf[64];
    int bytes_read = read(fd, temp_buf, sizeof(temp_buf));
    
    if (bytes_read > 0) {
        for (int i = 0; i < bytes_read; i++) {
            byte = temp_buf[i];
            if (g_write_cb(byte) == EVENT_FIFO_WRITE_ONE_OK) {
                write_count++;
            }
        }
    }
#endif

    return write_count;
}

// 发送数据
uint32_t Serial_Send(const SerialDevice_t* dev, const uint8_t* data, uint32_t len)
{
    if (dev == NULL || data == NULL || len == 0) {
        return 0;
    }
    if (!dev->is_open) {
        return 0;
    }

    uint32_t send_count = 0;

#ifdef _WIN32
    DWORD bytes_written = 0;
    if (WriteFile(dev->handle, data, len, &bytes_written, NULL)) {
        send_count = bytes_written;
    }
#elif __linux__ || __APPLE__
    int fd = (int)(intptr_t)dev->handle;
    int bytes_written = write(fd, data, len);
    if (bytes_written > 0) {
        send_count = bytes_written;
    }
#endif

    return send_count;
}