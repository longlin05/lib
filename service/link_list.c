//
// Created by 祖龙 on 2026/5/18.
//

#define MEM_POOL_FOR_SINGLE_LIST

#include "link_list.h"

// 仅校验 list 非空
static EventCb_Type single_list_check_list(const single_list_t *list)
{
    if (list == NULL)
    {
        return SINGLE_LIST_NULL_PTR;
    }
    return SINGLE_LIST_OK;
}

//同时校验 list 和 node 都非空
static EventCb_Type single_list_check_list_node(
    const single_list_t *list,
    const single_list_node_t *node)
{
    if ((list == NULL) || (node == NULL))
    {
        return SINGLE_LIST_NULL_PTR;
    }
    return SINGLE_LIST_OK;
}

// 节点合法性校验（校验节点是否属于当前链表，避免非法节点）
static EventCb_Type single_list_node_check(const single_list_t *list, const single_list_node_t *node) {
    EventCb_Type status = single_list_check_list_node(list, node);
    if (status != SINGLE_LIST_OK) {
        return status;
    }

    single_list_node_t *cur = list->head;
    while (cur != NULL) {
        if (cur == node) {
            return SINGLE_LIST_OK;
        }
        cur = cur->next;
    }

    return SINGLE_LIST_NODE_NOT_EXIST;
}

// 初始化
//链表头初始化（创建空表）
EventCb_Type single_list_init(single_list_t *list) {
    // 调用静态校验函数，校验list非空（只校验list）
    EventCb_Type status = single_list_check_list(list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }

    // 初始化链表状态
    list->head = NULL;
    list->tail = NULL;
    list->node_cnt = 0u;

    return SINGLE_LIST_OK;
}

// 链表批量初始化（预创建指定数量节点，依赖内存池）
EventCb_Type single_list_batch_init(single_list_t *list, uint32_t node_num, mem_pool_t *pool) {
    // 1. 校验list非空（node传NULL，只校验list）
    EventCb_Type status = single_list_check_list(list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }

    // 2. 校验pool非空
    if (pool == NULL)
    {
        return SINGLE_LIST_INVALID_PAR;
    }

    // 3. 校验节点数量
    if (node_num == 0U)
    {
        return SINGLE_LIST_INVALID_PAR;
    }

    // 4. 从内存池申请节点并尾插
    for (uint32_t i = 0u; i < node_num; i++)
    {
        single_list_node_t *node = mem_pool_alloc(pool);
        if (node == NULL)
        {
            // 内存池申请失败，先清空已创建的节点，避免泄漏
            single_list_clear(list, pool);
            return SINGLE_LIST_MEM_ALLOC_FAIL;
        }

        // 尾插节点（直接实现，不依赖外部函数）
        node->next = NULL;
        if (list->head == NULL)
        {
            list->head = node;
            list->tail = node;
        }
        else
        {
            list->tail->next = node;
            list->tail = node;
        }
        list->node_cnt++;
    }

    return SINGLE_LIST_OK;
}

// 链表状态重置（清空节点，不销毁链表头，节点归还给内存池）
EventCb_Type single_list_reset(single_list_t *list, mem_pool_t *pool) {
    // 调用静态校验函数，校验list非空（node参数传NULL）
    EventCb_Type status = single_list_check_list(list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }

    // 校验 pool 非空
    if (pool == NULL)
    {
        return SINGLE_LIST_INVALID_PAR;
    }

    // 清空所有节点
    single_list_clear(list, pool);

    // 重置链表头状态
    single_list_init(list);

    return SINGLE_LIST_OK;
}

// 增
// 头插节点（插入到表头第一个位置）
EventCb_Type single_list_insert_head(single_list_t *list, single_list_node_t *node){
    // 1. 校验 list 和 node 都非空
    EventCb_Type status = single_list_check_list_node(list, node);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }

    // 2. 将 node 插入到表头
    node->next = list->head;   // 新节点的 next 指向原来的头节点
    list->head = node;         // 链表头指向新节点

    // 3. 如果链表为空，尾指针也需要更新
    if (list->tail == NULL)
    {
        list->tail = node;
    }

    // 4. 节点计数+1
    list->node_cnt++;

    return SINGLE_LIST_OK;
}

// 尾插节点（插入到表尾最后一个位置）
EventCb_Type single_list_insert_tail(single_list_t *list, single_list_node_t *node){
    // 1. 校验 list 和 node 都非空
    EventCb_Type status = single_list_check_list_node(list, node);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }

    // 2. 将 node 的 next 置空（作为尾节点）
    node->next = NULL;

    // 3. 如果链表为空，直接让头和尾都指向 node
    if (list->head == NULL)
    {
        list->head = node;
        list->tail = node;
    }
    else
    {
        // 链表非空，将 node 接在 tail 后面，再更新 tail
        list->tail->next = node;
        list->tail = node;
    }

    // 4. 节点计数+1
    list->node_cnt++;

    return SINGLE_LIST_OK;
}

// 按位置索引插入节点（index从0开始）
EventCb_Type single_list_insert_by_index(single_list_t *list, uint32_t index, single_list_node_t *node)
{
    // 1. 校验 list 和 node 都非空
    EventCb_Type status = single_list_check_list_node(list, node);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }

    // 2. 校验索引合法性
    // 索引范围：0 ~ node_cnt（支持尾插，index == node_cnt 等价于尾插）
    if (index > list->node_cnt)
    {
        return SINGLE_LIST_INVALID_PAR;
    }

    // 3. 特殊情况：index == 0（头插）
    if (index == 0U)
    {
        // 调用已有的头插逻辑，避免重复代码
        return single_list_insert_head(list, node);
    }

    // 4. 特殊情况：index == node_cnt（尾插）
    if (index == list->node_cnt)
    {
        // 调用已有的尾插逻辑，避免重复代码
        return single_list_insert_tail(list, node);
    }

    // 5. 中间位置插入：找到 index-1 位置的前驱节点
    single_list_node_t *prev = list->head;
    for (uint32_t i = 0U; i < index - 1; i++)
    {
        prev = prev->next;
    }

    // 6. 插入节点
    node->next = prev->next;  // 新节点指向原来的后继节点
    prev->next = node;         // 前驱节点指向新节点

    // 7. 节点计数+1
    list->node_cnt++;

    return SINGLE_LIST_OK;
}

// 在指定节点之后插入节点
EventCb_Type single_list_insert_after_node(single_list_t *list, single_list_node_t *pos_node, single_list_node_t *node) {
    // 1. 校验 list、pos、node 都不能为空，且 pos_node 必须属于该链表
    EventCb_Type status = single_list_node_check(list, pos_node);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }
    // 额外校验插入节点非空
    if (node == NULL)
    {
        return SINGLE_LIST_NULL_PTR;
    }

    // 2. 执行插入（最标准的单向链表插入逻辑）
    node->next = pos_node->next;
    pos_node->next = node;

    // 3. 如果插在尾部，更新 tail 指针
    if (pos_node == list->tail)
    {
        list->tail = node;
    }

    // 4. 计数+1
    list->node_cnt++;

    return SINGLE_LIST_OK;
}

// 删
// 删除表头第一个节点
EventCb_Type single_list_delete_head(single_list_t *list, mem_pool_t *pool){
    // 1. 校验 list 和 pool 非空
    EventCb_Type status = single_list_check_list(list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }
    if (pool == NULL)
    {
        return SINGLE_LIST_INVALID_PAR;
    }

    // 2. 链表为空，无法删除
    if (list->head == NULL)
    {
        return SINGLE_LIST_EMPTY;
    }

    // 3. 保存待删除的头节点
    single_list_node_t *del_node = list->head;

    // 4. 更新头指针
    list->head = del_node->next;

    // 5. 如果链表只剩一个节点，删除后尾指针也要置空
    if (list->tail == del_node)
    {
        list->tail = NULL;
    }

    // 6. 归还节点到内存池
    mem_pool_free(pool, del_node);

    // 7. 节点计数-1
    list->node_cnt--;

    return SINGLE_LIST_OK;
}

// 删除表尾最后一个节点
EventCb_Type single_list_delete_tail(single_list_t *list, mem_pool_t *pool){
    // 1. 校验 list 和 pool 非空
    EventCb_Type status = single_list_check_list(list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }
    if (pool == NULL)
    {
        return SINGLE_LIST_INVALID_PAR;
    }

    // 2. 链表为空，无法删除
    if (list->head == NULL)
    {
        return SINGLE_LIST_EMPTY;
    }

    // 3. 链表只有一个节点，直接复用 delete_head 逻辑
    if (list->head == list->tail)
    {
        return single_list_delete_head(list, pool);
    }

    // 4. 找到尾节点的前驱节点（O(n)遍历）
    single_list_node_t *prev = list->head;
    while (prev->next != list->tail)
    {
        prev = prev->next;
    }

    // 5. 保存待删除的尾节点
    single_list_node_t *del_node = list->tail;

    // 6. 更新尾指针
    prev->next = NULL;
    list->tail = prev;

    // 7. 归还节点到内存池
    mem_pool_free(pool, del_node);

    // 8. 节点计数-1
    list->node_cnt--;

    return SINGLE_LIST_OK;
}

// 删除指定目标节点
EventCb_Type single_list_delete_node(single_list_t *list, single_list_node_t *target, mem_pool_t *pool){
    // 1. 校验 list、target 非空，且 target 必须属于该链表
    EventCb_Type status = single_list_node_check(list, target);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }
    if (pool == NULL)
    {
        return SINGLE_LIST_INVALID_PAR;
    }

    // 2. 特殊情况：删除的是头节点
    if (target == list->head)
    {
        return single_list_delete_head(list, pool);
    }

    //3. 特殊情况：删除的是尾节点（复用尾删逻辑）
    if (target == list->tail)
    {
        return single_list_delete_tail(list, pool);
    }

    // 4. 中间节点：找到目标节点的前驱节点
    single_list_node_t *prev = list->head;
    while (prev != NULL && prev->next != target)
    {
        prev = prev->next;
    }

    // 5. 找不到目标节点（不在链表中）
    if (prev == NULL)
    {
        return SINGLE_LIST_NOT_FOUND;
    }

    // 6. 从链表中移除目标节点
    prev->next = target->next;

    // 7. 归还节点到内存池
    mem_pool_free(pool, target);

    // 8. 节点计数-1
    list->node_cnt--;

    return SINGLE_LIST_OK;
}

// 按回调条件批量删除（满足回调条件的节点全部删除）
EventCb_Type single_list_delete_by_cb(single_list_t *list, ListDelJudgeCb judge_cb,
                                void *priv_data, mem_pool_t *pool)
{
    // 1. 入参检查
    EventCb_Type status = single_list_check_list(list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }
    if ((judge_cb == NULL) || (pool == NULL))
    {
        return SINGLE_LIST_ERR_PARAM;
    }

    single_list_node_t *curr = list->head;  // 当前节点
    single_list_node_t *prev = NULL;        // 前驱节点
    single_list_node_t *next = NULL;        // 保存下一个节点（关键！）
    (void)priv_data;

    // 2. 遍历链表
    while (curr != NULL)
    {
        // 必须先保存下一个节点！因为当前节点可能被删除
        next = curr->next;

        // 3. 调用回调，判断是否要删除
        if (judge_cb(curr, priv_data) == TRUE)
        {
            // 4. 从链表中断开节点
            if (prev == NULL)
            {
                // 删除的是头节点
                list->head = next;
            }
            else
            {
                // 删除的是中间/尾节点
                prev->next = next;
            }

            // 如果删除的是尾节点，更新 tail
            if (curr == list->tail)
            {
                list->tail = prev;
            }

            // 节点计数 -1
            list->node_cnt--;

            // 5. 释放节点内存
            mem_pool_free(pool, curr);
        }
        else
        {
            // 不删除 → 更新前驱节点
            prev = curr;
        }

        // 跳到下一个节点
        curr = next;
    }

    return SINGLE_LIST_OK;
}

// 安全删除（校验节点合法性，避免野指针，释放回内存池）
EventCb_Type single_list_safe_delete(single_list_t *list,
                                     const single_list_node_t *node,
                                     mem_pool_t *pool){
    single_list_node_t *prev = NULL;
    single_list_node_t *curr = NULL;

    // 1. 入参合法性校验（MISRA 强制要求）
    EventCb_Type status = single_list_node_check(list, node);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }
    if (pool == NULL)
    {
        return SINGLE_LIST_INVALID_PAR;
    }

    curr = list->head;
    prev = NULL;

    // 2. 遍历链表，查找目标节点
    while (curr != NULL)
    {
        if (curr == node)
        {
            // 找到节点，执行删除断链
            if (prev == NULL)
            {
                // 情况A：删除头节点
                list->head = curr->next;
            }
            else
            {
                // 情况B：删除中间节点
                prev->next = curr->next;
            }

            // 情况C：如果是尾节点，更新尾指针
            if (curr == list->tail)
            {
                list->tail = prev;
            }

            // 节点计数减1
            list->node_cnt--;

            // 3. 释放节点内存（调用你的内存池接口）
            mem_pool_free(pool, curr);

            // 4. 为避免野指针，调用方传入的node置空
            node = NULL;

            return  SINGLE_LIST_OK;
        }

        prev = curr;
        curr = curr->next;
    }

    return SINGLE_LIST_NOT_FOUND;
}

//清空节点
EventCb_Type single_list_clear(single_list_t *list, mem_pool_t *pool) {
    // 校验list非空
    EventCb_Type status = single_list_check_list(list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }

    single_list_node_t *cur = list->head;
    single_list_node_t *next_node = NULL;

    // 遍历所有节点，逐个释放回内存池
    while (cur != NULL)
    {
        next_node = cur->next;  // 先保存下一个节点
        mem_pool_free(pool, cur);     // 归还当前节点（按块释放）
        cur = next_node;
    }

    // 重置链表状态
    list->head = NULL;
    list->tail = NULL;
    list->node_cnt = 0u;

    return SINGLE_LIST_OK;
}

// 查
// 按位置索引查找节点（返回节点指针，找不到返回NULL）
single_list_node_t *single_list_find_by_index(const single_list_t *list, uint32_t index) {
    single_list_node_t *curr = NULL;
    uint32_t i = 0U;

    // 1. 入参合法性校验（MISRA 强制要求）
    if (list == NULL)
    {
        return NULL;
    }

    // 2. 索引合法性校验（防止越界）
    if (index >= list->node_cnt)
    {
        return NULL;
    }

    curr = list->head;

    // 3. 遍历链表，找到对应索引的节点
    while ((curr != NULL) && (i < index))
    {
        curr = curr->next;
        i++;
    }

    // 4. 返回节点指针（找不到的话循环结束时curr为NULL）
    return curr;
}

// 按回调比对查找匹配节点（返回第一个匹配节点，找不到返回NULL）
single_list_node_t *single_list_find_by_cb(const single_list_t *list, ListDelJudgeCb cb, void *arg) {
    single_list_node_t *curr = NULL;

    // 1. 入参合法性校验
    if ((list == NULL) || (cb == NULL))
    {
        return NULL;
    }

    curr = list->head;

    // 2. 遍历链表，调用回调判断
    while (curr != NULL)
    {
        // 调用上层回调，判断当前节点是否匹配
        if (cb(curr, arg) == TRUE)
        {
            // 找到第一个匹配节点，直接返回
            return curr;
        }

        curr = curr->next;
    }

    // 遍历结束，未找到匹配节点
    return NULL;
}

// 获取链表总节点个数（返回节点数量）
uint32_t single_list_get_node_cnt(const single_list_t *list) {
    if (list == NULL)
    {
        return 0U;
    }

    return list->node_cnt;
}

// 判断链表是否为空（0=非空，1=空）为确保不引入stdbool库不使用布尔类型
int8_t single_list_is_empty(const single_list_t *list) {
    if ((list == NULL) || (list->node_cnt == 0U))
    {
        // 链表为空（或入参无效）
        return 1;
    }
    else
    {
        // 链表非空
        return 0;
    }
}

// 遍历
// 遍历执行自定义操作（打印、赋值、统计等，通过回调实现）
void single_list_traverse_do(const single_list_t *list, ListDelJudgeCb cb, void *arg){
    single_list_node_t *curr = NULL;

    if ((list == NULL) || (cb == NULL))
    {
        return;
    }

    curr = list->head;
    while (curr != NULL)
    {
        // 回调返回 TRUE 就停止遍历
        if (cb(curr, arg) == TRUE)
        {
            break;
        }
        curr = curr->next;
    }
}

// 遍历销毁业务数据（通过回调释放节点绑定的业务数据,不释放节点）
void single_list_traverse_destroy(const single_list_t *list, ListTraverseCb cb, void *priv){
    single_list_node_t *curr = NULL;

    // 1. MISRA 标准入参检查
    if ((list == NULL) || (cb == NULL))
    {
        return;
    }

    curr = list->head;

    // 2. 遍历所有节点
    while (curr != NULL)
    {
        // 3. 调用回调，销毁业务数据
        cb(curr, priv);

        // 4. 下一个节点
        curr = curr->next;
    }
}

// 高级
// 链表反转（正向变反向，不改变节点数据）
EventCb_Type single_list_reverse(single_list_t *list) {
    // 入参合法性检查
    EventCb_Type status = single_list_check_list(list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }

    if (list->head == NULL)
    {
        return SINGLE_LIST_EMPTY;
    }

    single_list_node_t *prev = NULL;
    single_list_node_t *curr = list->head;
    single_list_node_t *next = NULL;
    single_list_node_t *old_head = list->head;

    // 遍历并反转每个节点的 next 指针
    while (curr != NULL)
    {
        // 1. 先保存下一个节点（防止链表断裂）
        next = curr->next;

        // 2. 反转当前节点的 next 指针
        curr->next = prev;

        // 3. 指针整体向后移动
        prev = curr;
        curr = next;
    }

    // 4. 遍历结束后，prev 指向新的头节点
    list->head = prev;
    // 5. 更新尾指针为原来的头节点
    list->tail = old_head;

    return SINGLE_LIST_OK; // 反转成功
}

// 两条链表拼接合并（将list2拼接到list1尾部，list2清空）
EventCb_Type single_list_merge(single_list_t *dest, single_list_t *src){
    single_list_node_t *tail = NULL;

    // 1. 入参检查
    EventCb_Type status = single_list_check_list(dest);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }
    status = single_list_check_list(src);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }

    // 源链表为空，直接返回成功
    if (src->head == NULL)
    {
        return SINGLE_LIST_OK;
    }

    // 2. 如果目标链表为空，直接把源链表赋值给目标链表
    if (dest->head == NULL)
    {
        dest->head = src->head;
        dest->tail = src->tail;
        dest->node_cnt = src->node_cnt;
    }
    else
    {
        // 3. 找到目标链表的尾节点
        tail = dest->head;
        while (tail->next != NULL)
        {
            tail = tail->next;
        }

        // 4. 将目标链表的尾节点 next 指向源链表的头节点
        tail->next = src->head;
        // 更新目标链表的尾指针和节点计数
        dest->tail = src->tail;
        dest->node_cnt += src->node_cnt;
    }

    // 5. 清空源链表
    src->head = NULL;
    src->tail = NULL;
    src->node_cnt = 0U;

    return SINGLE_LIST_OK;
}

// 链表截取拆分（将指定区间的节点拆分到新链表）
void single_list_cut(single_list_t *src, uint32_t start_idx, uint32_t end_idx, single_list_t *new_list) {
    // 1. 入参检查（src、new_list 不能为 NULL，索引必须合法）
    if (src == NULL || new_list == NULL || src->head == NULL || start_idx > end_idx)
    {
        return;
    }

    single_list_node_t *prev = NULL;   // 起点的前一个节点
    single_list_node_t *start = NULL;  // 起点节点
    single_list_node_t *end = NULL;    // 终点节点
    single_list_node_t *next = NULL;   // 终点的下一个节点
    uint32_t idx = 0;

    // 2. 遍历链表，定位四个关键节点
    single_list_node_t *curr = src->head;
    while (curr != NULL)
    {
        if (idx + 1 == start_idx)
        {
            prev = curr;
        }
        if (idx == start_idx)
        {
            start = curr;
        }
        if (idx == end_idx)
        {
            end = curr;
            break;
        }
        curr = curr->next;
        idx++;
    }

    // 索引越界，直接返回
    if (start == NULL || end == NULL)
    {
        return;
    }

    // 3. 记录终点的下一个节点
    next = end->next;

    // 4. 原链表断链与拼接
    if (prev != NULL)
    {
        // 起点不是头节点，把前半段和后半段接起来
        prev->next = next;
    }
    else
    {
        // 起点是头节点，更新原链表头指针
        src->head = next;
    }

    // 5. 更新源链表的尾指针和节点计数
    if (next == NULL)
    {
        // 截取到末尾，更新源链表尾指针
        src->tail = prev;
    }
    uint32_t cut_cnt = end_idx - start_idx + 1;
    src->node_cnt -= cut_cnt;

    // 6. 终点节点置空 next，防止新链表带尾巴
    end->next = NULL;

    // 7. 设置新链表的头、尾指针和节点计数
    new_list->head = start;
    new_list->tail = end;
    new_list->node_cnt = cut_cnt;
}
// 链表去重（通过回调判重，保留第一个相同节点）
EventCb_Type single_list_remove_duplicate(const single_list_t *list, SingleListEqualCb equal_cb, mem_pool_t *pool) {
    EventCb_Type status = single_list_check_list((single_list_t *)list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }
    if (equal_cb == NULL)
    {
        return SINGLE_LIST_ERR_PARAM;
    }

    if (list->head == NULL)
    {
        return SINGLE_LIST_OK;
    }

    single_list_node_t *base = list->head;
    single_list_node_t *curr = NULL;
    single_list_node_t *prev = NULL;
    single_list_node_t *tmp = NULL;
    single_list_t *mut_list = (single_list_t *)list;

    while (base != NULL && base->next != NULL)
    {
        curr = base->next;
        prev = base;

        while (curr != NULL)
        {
            if (equal_cb(base, curr) == TRUE)
            {
                tmp = curr;
                prev->next = curr->next;
                // 如果删除的是尾节点，更新尾指针
                if (curr == mut_list->tail)
                {
                    mut_list->tail = prev;
                }
                curr = curr->next;
                mem_pool_free(pool, tmp);
                // 更新节点计数
                mut_list->node_cnt--;
            }
            else
            {
                prev = curr;
                curr = curr->next;
            }
        }

        base = base->next;
    }

    return SINGLE_LIST_OK;
}
// 简单冒泡排序（通过回调比对节点大小，升序/降序由回调控制）
EventCb_Type single_list_bubble_sort(single_list_t *list, SingleListCompareCb cmp_cb, const void *arg){
    (void)arg;  // 消除 unused parameter 警告
    EventCb_Type status = single_list_check_list(list);
    if (status != SINGLE_LIST_OK)
    {
        return status;
    }
    if (cmp_cb == NULL)
    {
        return SINGLE_LIST_ERR_PARAM;
    }
    if (list->head == NULL || list->head->next == NULL)
    {
        return SINGLE_LIST_OK; // 空链表或单节点无需排序
    }

    single_list_node_t *curr = NULL;
    single_list_node_t *prev = NULL;
    single_list_node_t *tail = NULL;
    bool_t swapped = FALSE;

    // 冒泡排序核心逻辑
    do
    {
        swapped = FALSE;
        curr = list->head;
        prev = NULL;

        while (curr->next != tail)
        {
            if (cmp_cb(curr, curr->next) == TRUE)
            {
                // 交换 curr 和 curr->next
                single_list_node_t *tmp = curr->next;
                curr->next = tmp->next;
                tmp->next = curr;

                if (prev == NULL)
                {
                    list->head = tmp;
                }
                else
                {
                    prev->next = tmp;
                }

                prev = tmp;
                swapped = TRUE;
            }
            else
            {
                prev = curr;
                curr = curr->next;
            }
        }
        tail = curr;
    } while (swapped);

    // 更新尾指针
    list->tail = tail;

    return SINGLE_LIST_OK;
}

// 判断链表是否成环（0=无环，1=有环）
int8_t single_list_check_cycle(const single_list_t *list) {
    // 1. 边界检查
    if (list == NULL || list->head == NULL)
    {
        return 0; // 空链表，无环
    }

    single_list_node_t *slow = list->head;
    single_list_node_t *fast = list->head;

    // 2. 快慢指针遍历
    while (fast != NULL && fast->next != NULL)
    {
        slow = slow->next;          // 慢指针走 1 步
        fast = fast->next->next;    // 快指针走 2 步

        if (slow == fast)
        {
            return 1; // 指针相遇，说明链表有环
        }
    }

    // 3. 快指针到达链表尾部，无环
    return 0;
}

// 获取链表中间节点（偶数个节点返回第n/2个，奇数个返回中间个）
single_list_node_t *single_list_get_mid_node(const single_list_t *list) {
    // 1. 边界检查
    if (list == NULL || list->head == NULL)
    {
        return NULL; // 空链表，无中间节点
    }

    single_list_node_t *slow = list->head;
    single_list_node_t *fast = list->head;

    // 2. 快慢指针遍历
    while (fast != NULL && fast->next != NULL)
    {
        slow = slow->next;          // 慢指针走 1 步
        fast = fast->next->next;    // 快指针走 2 步
    }

    // 3. 此时 slow 指向中间节点
    return slow;
}