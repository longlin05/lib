//
// Created by 祖龙 on 2026/5/17.
//

#ifndef LINK_LIST_H
#define LINK_LIST_H

#include "../common/sys_def.h"
#include "../common/error_def.h"
#include "mem_pool.h"

// 通用节点结构体（仅next指针，数据外置）
typedef struct single_list_node {
    struct single_list_node *next;  // 后继指针
} single_list_node_t;

// 链表头结构体（管理整条链表）
typedef struct single_list {
    single_list_node_t *head;       // 表头节点
    single_list_node_t *tail;       // 表尾节点（优化尾插效率）
    uint32_t node_cnt;              // 节点总数
} single_list_t;

//链表回调钩子
// 删除判断回调
typedef bool_t (*ListDelJudgeCb)(single_list_node_t *node, void *priv_data);
// 遍历处理回调
typedef void (*ListTraverseCb)(single_list_node_t *node, void *user_data);
// 节点相等判断回调
typedef bool_t (*SingleListEqualCb)(single_list_node_t *node1, single_list_node_t *node2);
// 节点比较回调
typedef bool_t (*SingleListCompareCb)(single_list_node_t *a, single_list_node_t *b);

// 初始化
/**
 * @brief 链表头初始化（创建空表）
 * @param list 链表结构体
 * @return 返回事件码
 */
EventCb_Type single_list_init(single_list_t *list);

/**
 * @brief 链表批量初始化（预创建指定数量节点，依赖内存池）
 * @param list: 链表结构体
 * @param node_num: 预创建节点数量
 * @param pool: 内存池指针
 * @return 返回事件码
 */
EventCb_Type single_list_batch_init(single_list_t *list, uint32_t node_num, mem_pool_t *pool);

/**
 * @brief 链表状态重置（清空节点，不销毁链表头，节点归还给内存池）
 * @param list 链表变量
 * @param pool 内存池变量
 * @return 返回事件码
 */
EventCb_Type single_list_reset(single_list_t *list, mem_pool_t *pool);

// 增
/**
 * @brief 头插节点（插入到表头第一个位置）
 * @param list 链表变量
 * @param node 链表节点
 * @return 返回事件码
 */
EventCb_Type single_list_insert_head(single_list_t *list, single_list_node_t *node);

/**
 * @brief 尾插节点（插入到表尾最后一个位置）
 * @param list 链表变量
 * @param node 链表节点
 * @return 返回事件码
 */
EventCb_Type single_list_insert_tail(single_list_t *list, single_list_node_t *node);

/**
 * @brief 按位置索引插入节点（index从0开始）
 * @param list 链表变量
 * @param index 索引位置
 * @param node 链表节点
 * @return 返回事件码
 */
EventCb_Type single_list_insert_by_index(single_list_t *list, uint32_t index, single_list_node_t *node);

/**
 * @brief 在指定节点之后插入节点
 * @param list: 链表变量
 * @param pos_node: 指定节点
 * @param node: 要插入的节点
 * @return 返回事件码
 */
EventCb_Type single_list_insert_after_node(single_list_t *list, single_list_node_t *pos_node, single_list_node_t *node);

// 删
/**
 * @brief 删除表头第一个节点
 * @param list 链表变量
 * @param pool 内存池变量
 * @return 返回事件码
 */
EventCb_Type single_list_delete_head(single_list_t *list, mem_pool_t *pool);

/**
 * @brief 删除表尾最后一个节点
 * @param list 链表变量
 * @param pool 内存池变量
 * @return 返回事件码
 */
EventCb_Type single_list_delete_tail(single_list_t *list, mem_pool_t *pool);

/**
 * @brief 删除指定目标节点
 * @param list 链表变量
 * @param target 目标节点
 * @param pool 内存池变量
 * @return 返回事件码
 */
EventCb_Type single_list_delete_node(single_list_t *list, single_list_node_t *target, mem_pool_t *pool);

/**
 * @brief 按回调条件批量删除（满足回调条件的节点全部删除）
 * @param list 链表变量
 * @param judge_cb 调用者传入的判断规则
 * @param priv_data 比较用的数据
 * @param pool 内存池变量
 * @return 返回事件码
 */
EventCb_Type single_list_delete_by_cb(single_list_t *list, ListDelJudgeCb judge_cb, void *priv_data, mem_pool_t *pool);

/**
 * @brief 安全删除（校验节点合法性，避免野指针，释放回内存池）
 * @param list 链表变量
 * @param node 节点变量
 * @param pool 内存池变量
 * @return 返回事件码
 */
EventCb_Type single_list_safe_delete(single_list_t *list, const single_list_node_t *node, mem_pool_t *pool);

/**
 * @brief 清空节点
 * @param list 链表变量
 * @param pool 内存池变量
 * @return 返回事件码
 */
EventCb_Type single_list_clear(single_list_t *list, mem_pool_t *pool);

// 查
/**
 * @brief 按位置索引查找节点（返回节点指针，找不到返回NULL）
 * @param list 链表变量
 * @param index 索引
 * @return 返回结点指针
 */
single_list_node_t *single_list_find_by_index(const single_list_t *list, uint32_t index);

/**
 * @brief 按回调比对查找匹配节点（返回第一个匹配节点，找不到返回NULL）
 * @param list 链表变量
 * @param cb 比较逻辑
 * @param arg 对比用节点
 * @return 返回节点指针
 */
single_list_node_t *single_list_find_by_cb(const single_list_t *list, ListDelJudgeCb cb, void *arg);

/**
 * @brief 获取链表总节点个数
 * @param list: 链表变量
 * @return 返回节点数量
 */
uint32_t single_list_get_node_cnt(const single_list_t *list);

/**
 * @brief 判断链表是否为空（0=非空，1=空）
 * @param list 链表变量
 * @return 返回布尔值
 */
int8_t single_list_is_empty(const single_list_t *list);

// 遍历
/**
 * @brief 遍历执行自定义操作（打印、赋值、统计等，通过回调实现）
 * @param list 链表变量
 * @param cb 回调停止遍历判断
 * @param arg 操作对象
 */
void single_list_traverse_do(const single_list_t *list, ListDelJudgeCb cb, void *arg);

/**
 * @brief 遍历销毁业务数据（通过回调释放节点绑定的业务数据）
 * @param list: 链表变量
 * @param cb: 回调销毁函数
 * @param priv: 用户私有数据
 */
void single_list_traverse_destroy(const single_list_t *list, ListTraverseCb cb, void *priv);

// 高级
/**
 * @brief 链表反转（正向变反向，不改变节点数据）
 * @param list 链表变量
 * @return 返回事件码
 */
EventCb_Type single_list_reverse(single_list_t *list);

/**
 * @brief 两条链表拼接合并（将list2拼接到list1尾部，list2清空）
 * @param dest 目标链表
 * @param src 原链表
 * @return 返回事件码
 */
EventCb_Type single_list_merge(single_list_t *dest, single_list_t *src);

/**
 * @brief 链表截取拆分（将指定区间的节点拆分到新链表）
 * @param src: 原链表
 * @param start_idx: 拆分起始索引
 * @param end_idx: 拆分结束索引
 * @param new_list: 拆分出的新链表
 * @return 无
 */
void single_list_cut(single_list_t *src, uint32_t start_idx, uint32_t end_idx, single_list_t *new_list);

/**
 * @brief 链表去重（通过回调判重，保留第一个相同节点）
 * @param list 链表变量
 * @param equal_cb 判断是否相等（回调钩子）
 * @param pool 内存池变量
 * @return 返回事件码
 */
EventCb_Type single_list_remove_duplicate(const single_list_t *list, SingleListEqualCb equal_cb, mem_pool_t *pool);

/**
 * @brief 简单冒泡排序（通过回调比对节点大小，升序/降序由回调控制）
 * @param list 链表变量
 * @param cmp_cb 比较两数大小（比较钩子）
 * @param arg 扩展用参数，不用即置空
 * @return 返回事件码
 */
EventCb_Type single_list_bubble_sort(single_list_t *list, SingleListCompareCb cmp_cb, const void *arg);

/**
 * @brief 判断链表是否成环
 * @param list: 链表变量
 * @return 返回判断结果(0=无环,1=有环)
 */
int8_t single_list_check_cycle(const single_list_t *list);

/**
 * @brief 获取链表中间节点（偶数个节点返回第n/2个，奇数个返回中间个）
 * @param list 链表变量
 * @return 返回节点指针
 */
single_list_node_t *single_list_get_mid_node(const single_list_t *list);

#endif //LINK_LIST_H
