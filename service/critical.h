//
// Created by 祖龙 on 2026/5/17.
//

#ifndef LIB_CRITICAL_H
#define LIB_CRITICAL_H

#include "../common/sys_def.h"

/**
 * @brief 进入临界区（禁用所有中断）
 */
void Critical_Enter(void);

/**
 * @brief 退出临界区（使能所有中断）
 */
void Critical_Exit(void);

/**
 * @brief 锁定临界区（保存中断状态并禁用中断）
 * @return 返回之前的中断状态(用于恢复)
 */
uint32_t Critical_Lock(void);

/**
 * @brief 解锁临界区（恢复之前的中断状态）
 * @param primask: Critical_Lock返回的中断状态
 */
void Critical_Unlock(uint32_t primask);

#endif //LIB_CRITICAL_H
