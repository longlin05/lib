//
// Created by 祖龙 on 2026/5/27.
//

#include "check.h"

// 累加和
uint8_t checksum_calc(const uint8_t *data, size_t len)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return sum;
}

// CRC8
uint8_t crc8_calc(const uint8_t *data, size_t len)
{
    uint8_t crc = 0x00;

    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}