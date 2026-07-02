#ifndef __HAL_LKSMCU_MCPWM_H__
#define __HAL_LKSMCU_MCPWM_H__
#include "hal_lksmcu_gpio.h"
#include "stdint.h"
typedef enum
{
    HAL_MCPWM_0, ///< MCPWM模块0
    HAL_MCPWM_1, ///< MCPWM模块1
} HAL_MCPWM_x;

typedef enum
{
    HAL_MCPWM_PORT_0, ///< 输出端口
    HAL_MCPWM_PORT_1, ///< 输出端口
    HAL_MCPWM_PORT_2, ///< 输出端口
    HAL_MCPWM_PORT_3, ///< 输出端口
    HAL_MCPWM_PORT_4, ///< 输出端口
    HAL_MCPWM_PORT_5, ///< 输出端口
} HAL_MCPWM_PORT_x;
typedef enum
{
    HAL_MCPWM_POLARITY_PL_NL, ///< 上管低有效,下管低有效
    HAL_MCPWM_POLARITY_PH_NL, ///< 上管高有效,下管低有效
    HAL_MCPWM_POLARITY_PL_NH, ///< 上管低有效,下管高有效
    HAL_MCPWM_POLARITY_PH_NH, ///< 上管高有效,下管高有效
} HAL_MCPWM_POLARITY_x;
typedef enum
{
    HAL_MCPWM_OUTPUT_STATE_PWM,      ///< 上下管对称pwm
    HAL_MCPWM_OUTPUT_STATE_OFF,      ///< 关闭输出
    HAL_MCPWM_OUTPUT_STATE_LOW,      ///< 下管恒开
    HAL_MCPWM_OUTPUT_STATE_HIGH,     ///< 上管恒开
    HAL_MCPWM_OUTPUT_STATE_LOW_PWM,  ///< 下管pwm
    HAL_MCPWM_OUTPUT_STATE_HIGH_PWM, ///< 上管pwm
} HAL_MCPWM_OUTPUT_STATE_x;
typedef enum
{
    HAL_MCPWM_DEFAULT_OUTPUT_STATE_OFF,  ///< 关闭输出
    HAL_MCPWM_DEFAULT_OUTPUT_STATE_LOW,  ///< 下管恒开
    HAL_MCPWM_DEFAULT_OUTPUT_STATE_HIGH, ///< 上管恒开
} HAL_MCPWM_DEFAULT_OUTPUT_STATE_x;
typedef enum
{
    HAL_MCPWM_FAIL_IO0_HIGH,
    HAL_MCPWM_FAIL_IO1_HIGH,
    HAL_MCPWM_FAIL_CMP0_HIGH,
    HAL_MCPWM_FAIL_CMP1_HIGH,
    HAL_MCPWM_FAIL_IO0_LOW,
    HAL_MCPWM_FAIL_IO1_LOW,
    HAL_MCPWM_FAIL_CMP0_LOW,
    HAL_MCPWM_FAIL_CMP1_LOW,
} HAL_MCPWM_FAIL_x;
// 初始化
void HAL_MCPWM_Reset(HAL_MCPWM_x);   // 模块复位
void HAL_MCPWM_Enable(HAL_MCPWM_x);  // 模块打开
void HAL_MCPWM_Disable(HAL_MCPWM_x); // 模块关闭

void HAL_MCPWM_SetFreq(HAL_MCPWM_x, uint32_t freq);                        // 设置pwm频率
uint32_t HAL_MCPWM_GetFreq(HAL_MCPWM_x);                                   // 获取当前的pwm频率
uint32_t HAL_MCPWM_GetThCnt(HAL_MCPWM_x);                                  // 获取计数器最大值
void HAL_MCPWM_SetDead(HAL_MCPWM_x, uint32_t pdead_nS, uint32_t ndead_nS); // 设置死区

void HAL_MCPWM_SetOutputDisable(HAL_MCPWM_x);                                        // 关闭输出
void HAL_MCPWM_SetOutputEnable(HAL_MCPWM_x);                                         // 打开输出
void HAL_MCPWM_StartCounting(HAL_MCPWM_x);                                           // 开始计数
void HAL_MCPWM_StopCounting(HAL_MCPWM_x);                                            // 停止计数(同时计数器回到-th)
void HAL_MCPWM_SetPwm(HAL_MCPWM_x, HAL_MCPWM_PORT_x, int32_t t0, int32_t t1);        // 设置pwm波形
void HAL_MCPWM_SetPwmQ15(HAL_MCPWM_x, HAL_MCPWM_PORT_x, int16_t t0, int16_t t1);     // 设置pwm波形q15格式输入
void HAL_MCPWM_SetOutState(HAL_MCPWM_x, HAL_MCPWM_PORT_x, HAL_MCPWM_OUTPUT_STATE_x); // 设置输出状态
void HAL_MCPWM_SetAdcTrig(HAL_MCPWM_x, int32_t t0, int32_t t1);                      // 设置pwm输出ADC采样时间
void HAL_MCPWM_SetAdcTrigQ15(HAL_MCPWM_x, int32_t t0, int32_t t1);                   // 设置pwm输出ADC采样时间q15格式输入

void HAL_MCPWM_SetDrivePolarity(HAL_MCPWM_x, HAL_MCPWM_POLARITY_x);            // 设置pwm极性
void HAL_MCPWM_SetDefaultState(HAL_MCPWM_x, HAL_MCPWM_DEFAULT_OUTPUT_STATE_x); // 设置默认电平
void HAL_MCPWM_SetGpioOut(HAL_MCPWM_x, HAL_GPIO_Px);                           // 设置用于输出的gpio
void HAL_MCPWM_SetGpioFail(HAL_MCPWM_x, HAL_GPIO_Px);                          // 设置用于故障输入的gpio
void HAL_MCPWM_SetFailSignal(HAL_MCPWM_x, HAL_MCPWM_FAIL_x);                   // 设置FAIL信号来源

uint8_t HAL_MCPWM_IrqOverflow(HAL_MCPWM_x);
void HAL_MCPWM_IrqOverflowEnable(HAL_MCPWM_x);
void HAL_MCPWM_IrqOverflowDisable(HAL_MCPWM_x);
void HAL_MCPWM_IrqOverflowClear(HAL_MCPWM_x);

uint8_t HAL_MCPWM_IrqZeroCross(HAL_MCPWM_x);
void HAL_MCPWM_IrqZeroCrossEnable(HAL_MCPWM_x);
void HAL_MCPWM_IrqZeroCrossDisable(HAL_MCPWM_x);
void HAL_MCPWM_IrqZeroCrossClear(HAL_MCPWM_x);

uint8_t HAL_MCPWM_IrqFail(HAL_MCPWM_x);
void HAL_MCPWM_IrqFailEnable(HAL_MCPWM_x);
void HAL_MCPWM_IrqFailDisable(HAL_MCPWM_x);
void HAL_MCPWM_IrqFailClear(HAL_MCPWM_x);
#endif
