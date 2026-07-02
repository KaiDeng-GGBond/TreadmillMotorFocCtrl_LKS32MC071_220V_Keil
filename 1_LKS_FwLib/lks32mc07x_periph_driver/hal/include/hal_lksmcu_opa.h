#ifndef __HAL_LKSMCU_OPA_H__
#define __HAL_LKSMCU_OPA_H__
#include "stdint.h"
#include "hal_lksmcu_adc.h"
typedef enum
{
    HAL_OPA_0,  ///< ADC模块0
    HAL_OPA_1,  ///< ADC模块1
    HAL_OPA_2,  ///< ADC模块2
    HAL_OPA_3,  ///< ADC模块3
    HAL_OPA_4,  ///< ADC模块4
    HAL_OPA_5,  ///< ADC模块5
    HAL_OPA_0B, ///< ADC模块0B
} HAL_OPA_x;

typedef enum
{
    // 07
    HAL_OPA_GAIN_320_10 = 0,
    HAL_OPA_GAIN_160_10 = 1,
    HAL_OPA_GAIN_80_10  = 2,
    HAL_OPA_GAIN_40_10  = 3,

    HAL_OPA_GAIN_HIGH   = HAL_OPA_GAIN_160_10, ///< 高增益
    HAL_OPA_GAIN_MEDIUM = HAL_OPA_GAIN_80_10,  ///< 中等增益
    HAL_OPA_GAIN_LOW    = HAL_OPA_GAIN_40_10,  ///< 低增益

    // 03
    HAL_OPA_GAIN_200_10,
    HAL_OPA_GAIN_190_20,
    HAL_OPA_GAIN_180_30,
    HAL_OPA_GAIN_170_40,
    // 05
    HAL_OPA_GAIN_200_10_6,
    HAL_OPA_GAIN_190_20_6,
    HAL_OPA_GAIN_180_30_6,
    HAL_OPA_GAIN_170_40_6,

    // 08
    HAL_OPA_GAIN_200_10_4,
    HAL_OPA_GAIN_190_20_4,
    HAL_OPA_GAIN_180_30_4,
    HAL_OPA_GAIN_170_40_4,

    // 09
    HAL_OPA_GAIN_660_20,
    HAL_OPA_GAIN_640_40,
    HAL_OPA_GAIN_320_40,
    HAL_OPA_GAIN_160_40,

    // 45
    HAL_OPA_GAIN_20_10,
    HAL_OPA_GAIN_320_5,

} HAL_OPA_GAIN_x;

// 初始化
void HAL_OPA_Reset(HAL_OPA_x);   // 模块复位
void HAL_OPA_Enable(HAL_OPA_x);  // 模块打开
void HAL_OPA_Disable(HAL_OPA_x); // 模块关闭

void HAL_OPA_SetGain(HAL_OPA_x, HAL_OPA_GAIN_x); // 设置增益
void HAL_OPA_Out2GpioEnable(HAL_OPA_x);
void HAL_OPA_Out2GpioDisable(HAL_OPA_x);
void HAL_OPA_SetGpioIn(HAL_OPA_x, HAL_GPIO_Px);
void HAL_OPA_SetGpioOut(HAL_OPA_x, HAL_GPIO_Px);

#define HAL_OPA_OPAX_2_ADCCHN(OPAX) ((OPAX) == HAL_OPA_0B ? HAL_ADC_PORT_0 : (HAL_ADC_PORT_x)(OPAX))
/**
 * @brief 芯片的运放放大倍数
 */
#define HAL_OPA_READ_GAIN(GAIN_x, res) (GAIN_x == HAL_OPA_GAIN_HIGH ? (160.0f / (10.0f + (float)res)) : (GAIN_x == HAL_OPA_GAIN_MEDIUM) ? (80.0f / (10.0f + (float)res)) \
                                                                                                                                        : (40.0f / (10.0f + (float)res)))

#endif
