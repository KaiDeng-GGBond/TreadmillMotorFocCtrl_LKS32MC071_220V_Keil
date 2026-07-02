#ifndef __HAL_LKSMCU_DAC_H__
#define __HAL_LKSMCU_DAC_H__
#include "stdint.h"
#include "hal_lksmcu_gpio.h"

typedef enum
{
    HAL_DAC_0, ///< DAC模块0
    HAL_DAC_1, ///< DAC模块1
    HAL_DAC_2, ///< DAC模块2
    HAL_DAC_3, ///< DAC模块3
} HAL_DAC_x;
void HAL_DAC_Reset(HAL_DAC_x);   // 模块复位
void HAL_DAC_Enable(HAL_DAC_x);  // 模块打开
void HAL_DAC_Disable(HAL_DAC_x); // 模块关闭

void HAL_DAC_SetOutVal(HAL_DAC_x, uint16_t val);
void HAL_DAC_SetOutVoltagemV(HAL_DAC_x, uint16_t vol_mV);
void HAL_DAC_Out2GpioEnable(HAL_DAC_x);
void HAL_DAC_Out2GpioDisable(HAL_DAC_x);
void HAL_DAC_SetGpioOut(HAL_DAC_x, HAL_GPIO_Px);
#endif
