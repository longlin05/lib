//
// Created by 祖龙 on 2026/5/27.
//

#ifndef LIB_CHECK_H
#define LIB_CHECK_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief 累加和校验
 * @param data 校验所需累加的数据
 * @param len 数据长度
 * @return 返回累加和
 */
uint8_t checksum_calc(const uint8_t *data, size_t len);

/**
 * @brief CRC8 标准多项式校验
 * @param data 校验所需累加的数据
 * @param len 数据长度
 * @return 返回校验值
 */
uint8_t crc8_calc(const uint8_t *data, size_t len);

#endif //LIB_CHECK_H
