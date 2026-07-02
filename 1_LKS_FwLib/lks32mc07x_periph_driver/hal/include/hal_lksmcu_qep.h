#ifndef __HAL_LKSMCU_QEP_H__
#define __HAL_LKSMCU_QEP_H__
#include <stdint.h>
#include "../include/hal_lksmcu_gpio.h"
typedef enum
{
    HAL_QEP_0,
    HAL_QEP_1,
    HAL_QEP_2,
    HAL_QEP_3,
} HAL_QEP_x;

typedef enum
{
    HAL_QEP_MODE_QUADRATURE,   // 正交编码计数模式
    HAL_QEP_MODE_SYMBOL_PULSE, // 符号加脉冲计数模式
    HAL_QEP_MODE_DUAL_PULSE,   // 双脉冲计数模式
} HAL_QEP_MODE_x;
void HAL_QEP_Reset(HAL_QEP_x);   // 模块复位
void HAL_QEP_Enable(HAL_QEP_x);  // 模块打开
void HAL_QEP_Disable(HAL_QEP_x); // 模块关闭

void HAL_QEP_SetTh(HAL_QEP_x, uint32_t th);      // 设置最大计数值
void HAL_QEP_SetMode(HAL_QEP_x, HAL_QEP_MODE_x); // 设置计数模式
uint32_t HAL_QEP_ReadCnt(HAL_QEP_x);             // 获取当前的计数值
void HAL_QEP_ResetCnt(HAL_QEP_x);                // 复位计数值到0

void HAL_QEP_SetGpio(HAL_QEP_x, HAL_GPIO_Px); // 设置指定的GPIO为ADC输入
#endif
