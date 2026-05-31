//
// Created by 祖龙 on 2026/5/27.
//

#ifndef LIB_ERRO_CODE_H
#define LIB_ERRO_CODE_H

//状态码枚举
typedef enum {
    OPERATE_SUCCESS = 0,    //操作成功
    OPERATE_FAILURE,        //操作失败
    PARA_INVALID,           //参数非法
    MEM_OVERFLOW,           //内存溢出
    EMPTY,                  //为空 / 无资源
    OUT_OF_RANGE,           //越界访问
    UNINITIALIZED,          //未初始化
    UNKNOWN_ERROR,          //未知错误
    OPERATE_PARTIAL,        //批量操作成功
}EventCb_Status;

#endif //LIB_ERRO_CODE_H
