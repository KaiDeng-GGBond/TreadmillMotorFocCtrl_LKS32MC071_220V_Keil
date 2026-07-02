#ifndef __HAL_LKSMCU_ADC_H__
#define __HAL_LKSMCU_ADC_H__
#include "stdint.h"
#include "hal_lksmcu_gpio.h"
/// @brief 主频
typedef enum
{
    // ADC0采样通道
    // 0x0: OPA0输出
    // 0x1: OPA1输出
    // 0x2: OPA2输出
    // 0x3: OPA3输出
    // 0x4: ADC01_CH4
    // 0x5: ADC01_CH5
    // 0x6: ADC01_CH6
    // 0x7: ADC01_CH7
    // 0x8: ADC01_CH8
    // 0x9: ADC01_CH9
    // 0xA: ADC0_CH10
    // 0xB: ADC0_CH11
    // 0xC: ADC0_CH12
    // 0xD: ADC0_CH13
    // 0xE: DAC0_OUT
    // 0xF: DAC1_OUT

    // ADC1采样通道
    // 0x0: OPA0输出
    // 0x1: OPA1输出
    // 0x2: OPA2输出
    // 0x3: OPA3输出
    // 0x4: ADC01_CH4
    // 0x5: ADC01_CH5
    // 0x6: ADC01_CH6
    // 0x7: ADC01_CH7
    // 0x8: ADC01_CH8
    // 0x9: ADC01_CH9
    // 0xA: ADC1_CH10
    // 0xB: ADC1_CH11
    // 0xC: ADC1_CH12
    // 0xD: ADC1_CH13
    // 0xE:温度传感器
    // 0xF:内部电源AVDD

    HAL_ADC_PORT_CH0,  ///< ADC采样端口0
    HAL_ADC_PORT_CH1,  ///< ADC采样端口1
    HAL_ADC_PORT_CH2,  ///< ADC采样端口2
    HAL_ADC_PORT_CH3,  ///< ADC采样端口3
    HAL_ADC_PORT_CH4,  ///< ADC采样端口4
    HAL_ADC_PORT_CH5,  ///< ADC采样端口5
    HAL_ADC_PORT_CH6,  ///< ADC采样端口6
    HAL_ADC_PORT_CH7,  ///< ADC采样端口7
    HAL_ADC_PORT_CH8,  ///< ADC采样端口8
    HAL_ADC_PORT_CH9,  ///< ADC采样端口9
    HAL_ADC_PORT_CH10, ///< ADC采样端口10
    HAL_ADC_PORT_CH11, ///< ADC采样端口11
    HAL_ADC_PORT_CH12, ///< ADC采样端口12
    HAL_ADC_PORT_CH13, ///< ADC采样端口13
    HAL_ADC_PORT_CH14, ///< ADC采样端口14
    HAL_ADC_PORT_CH15, ///< ADC采样端口15

    HAL_ADC_PORT_OPA0 = HAL_ADC_PORT_CH0, ///< ADC采样端口运放0
    HAL_ADC_PORT_OPA1 = HAL_ADC_PORT_CH1, ///< ADC采样端口运放1
    HAL_ADC_PORT_OPA2 = HAL_ADC_PORT_CH2, ///< ADC采样端口运放2
    HAL_ADC_PORT_OPA3 = HAL_ADC_PORT_CH3, ///< ADC采样端口运放3

    HAL_ADC_PORT_DAC0 = HAL_ADC_PORT_CH14, ///< 仅ADC0 ADC采样DAC0输出
    HAL_ADC_PORT_DAC1 = HAL_ADC_PORT_CH15, ///< 仅ADC0 ADC采样DAC1输出

    HAL_ADC_PORT_TMP0 = HAL_ADC_PORT_CH14, ///< 仅ADC1 ADC采样端口温度传感器
    HAL_ADC_PORT_VDD  = HAL_ADC_PORT_CH15, ///< 仅ADC1 ADC采样供电

    HAL_ADC_PORT_REF,   ///< ADC采样芯片内部基准电压 不支持 这里保留接口定义
    HAL_ADC_PORT_AGND,  ///< ADC采样内部地 不支持 这里保留接口定义
    HAL_ADC_PORT_OPA4,  ///< ADC采样端口运放4 不支持 这里保留接口定义
    HAL_ADC_PORT_OPA5,  ///< ADC采样端口运放5 不支持 这里保留接口定义
    HAL_ADC_PORT_CH16,  ///< ADC采样端口16 不支持 这里保留接口定义
    HAL_ADC_PORT_CH17,  ///< ADC采样端口17 不支持 这里保留接口定义
    HAL_ADC_PORT_CH18,  ///< ADC采样端口18 不支持 这里保留接口定义
    HAL_ADC_PORT_CH19,  ///< ADC采样端口19 不支持 这里保留接口定义
    HAL_ADC_PORT_LDO12, ///< ADC采样芯片LDO12(1.2V)内核供电 不支持 这里保留接口定义
} HAL_ADC_PORT_x;
/// @brief ADC采样序列
typedef enum
{
    HAL_ADC_SAMP_SEQUENCE_0,  ///< ADC第0次采样
    HAL_ADC_SAMP_SEQUENCE_1,  ///< ADC第1次采样
    HAL_ADC_SAMP_SEQUENCE_2,  ///< ADC第2次采样
    HAL_ADC_SAMP_SEQUENCE_3,  ///< ADC第3次采样
    HAL_ADC_SAMP_SEQUENCE_4,  ///< ADC第4次采样
    HAL_ADC_SAMP_SEQUENCE_5,  ///< ADC第5次采样
    HAL_ADC_SAMP_SEQUENCE_6,  ///< ADC第6次采样
    HAL_ADC_SAMP_SEQUENCE_7,  ///< ADC第7次采样
    HAL_ADC_SAMP_SEQUENCE_8,  ///< ADC第8次采样
    HAL_ADC_SAMP_SEQUENCE_9,  ///< ADC第9次采样
    HAL_ADC_SAMP_SEQUENCE_10, ///< ADC第10次采样
    HAL_ADC_SAMP_SEQUENCE_11, ///< ADC第11次采样
    HAL_ADC_SAMP_SEQUENCE_12, ///< ADC第12次采样
    HAL_ADC_SAMP_SEQUENCE_13, ///< ADC第13次采样
    HAL_ADC_SAMP_SEQUENCE_14, ///< ADC第14次采样
    HAL_ADC_SAMP_SEQUENCE_15, ///< ADC第15次采样
    HAL_ADC_SAMP_SEQUENCE_16, ///< ADC第16次采样
    HAL_ADC_SAMP_SEQUENCE_17, ///< ADC第17次采样
    HAL_ADC_SAMP_SEQUENCE_18, ///< ADC第18次采样
    HAL_ADC_SAMP_SEQUENCE_19, ///< ADC第19次采样
} HAL_ADC_SAMP_SEQUENCE_x;
typedef enum
{
    HAL_ADC_0, ///< ADC模块0
    HAL_ADC_1, ///< ADC模块1
    HAL_ADC_2, ///< ADC模块2
    HAL_ADC_3, ///< ADC模块3
} HAL_ADC_x;
typedef enum
{
    HAL_ADC_SAMP_NUMBER_0,                            ///< ADC采样0次
    HAL_ADC_SAMP_NUMBER_1,                            ///< ADC采样1次
    HAL_ADC_SAMP_NUMBER_2,                            ///< ADC采样2次
    HAL_ADC_SAMP_NUMBER_3,                            ///< ADC采样3次
    HAL_ADC_SAMP_NUMBER_4,                            ///< ADC采样4次
    HAL_ADC_SAMP_NUMBER_5,                            ///< ADC采样5次
    HAL_ADC_SAMP_NUMBER_6,                            ///< ADC采样6次
    HAL_ADC_SAMP_NUMBER_7,                            ///< ADC采样7次
    HAL_ADC_SAMP_NUMBER_8,                            ///< ADC采样8次
    HAL_ADC_SAMP_NUMBER_9,                            ///< ADC采样9次
    HAL_ADC_SAMP_NUMBER_10,                           ///< ADC采样10次
    HAL_ADC_SAMP_NUMBER_11,                           ///< ADC采样11次
    HAL_ADC_SAMP_NUMBER_12,                           ///< ADC采样12次
    HAL_ADC_SAMP_NUMBER_13,                           ///< ADC采样13次
    HAL_ADC_SAMP_NUMBER_14,                           ///< ADC采样14次
    HAL_ADC_SAMP_NUMBER_15,                           ///< ADC采样15次
    HAL_ADC_SAMP_NUMBER_MAX = HAL_ADC_SAMP_NUMBER_15, ///< ADC采样15次
} HAL_ADC_SAMP_NUMBER_x;
typedef enum
{
    HAL_ADC_TRIGGER_MCPWM, ///< 使用MCPWM触发ADC采样
    HAL_ADC_TRIGGER_TIMER, ///< 使用TIMER触发ADC采样
    HAL_ADC_TRIGGER_SOFT,  ///< 使用软件触发ADC采样
} HAL_ADC_TRIGGER_x;
typedef enum
{
    HAL_ADC_REFERENCE_INTERNAL, ///< 使用内部基准源作为adc基准
    HAL_ADC_REFERENCE_EXTERNAL, ///< 使用芯片供电作为adc基准
} HAL_ADC_REFERENCE_x;

// 初始化
void HAL_ADC_Reset(HAL_ADC_x);   // 模块复位
void HAL_ADC_Enable(HAL_ADC_x);  // 模块打开
void HAL_ADC_Disable(HAL_ADC_x); // 模块关闭

// 自动采样配置
int16_t HAL_ADC_GetSamplingValues(HAL_ADC_x, HAL_ADC_SAMP_SEQUENCE_x);                     // 获取采样值 返回第n次采样结果
void HAL_ADC_SetSamplingChannel(HAL_ADC_x, HAL_ADC_SAMP_SEQUENCE_x, HAL_ADC_PORT_x);       // 设置采样通道,输入参数 第n次采样channel通道
void HAL_ADC_SetSamplingQuantity(HAL_ADC_x, HAL_ADC_SAMP_NUMBER_x, HAL_ADC_SAMP_NUMBER_x); // 设置采样数量
void HAL_ADC_SetTriggerSignal(HAL_ADC_x, HAL_ADC_TRIGGER_x);                               // 设置触发信号
void HAL_ADC_EndSampling(HAL_ADC_x);                                                       // 中止当前采样
void HAL_ADC_SetReference(HAL_ADC_x, HAL_ADC_REFERENCE_x);                                 // 设置ADC基准源
float HAL_ADC_GetRange(HAL_ADC_x);                                                         // 获取adc量程,使用芯片供电作为adc基准时固定返回5.0
float HAL_ADC_GetValuesUnit(HAL_ADC_x);                                                    // 获取采样值单位
// 手动采样配置
void HAL_ADC_SoftTrigger(HAL_ADC_x);                       // 软件触发ADC开始采样
int16_t HAL_ADC_ManualSampling(HAL_ADC_x, HAL_ADC_PORT_x); // 手动采样 需要先设置采样通道为软件触发

// IO配置
void HAL_ADC_SetGpioIn(HAL_ADC_x, HAL_GPIO_Px);         // 设置指定的GPIO为ADC输入
void HAL_ADC_SetGpioTriggerOut(HAL_ADC_x, HAL_GPIO_Px); // 设置指定的GPIO为ADC触发信号输出

#endif
