#ifndef __HAL_LKSMCU_I2C_H__
#define __HAL_LKSMCU_I2C_H__
#include "hal_lksmcu_gpio.h"
#include "stdint.h"
typedef enum
{
    HAL_I2C_0,
    HAL_I2C_1,
} HAL_I2C_x;
// 初始化
void HAL_I2C_Reset(HAL_I2C_x);   // 模块复位
void HAL_I2C_Enable(HAL_I2C_x);  // 模块打开
void HAL_I2C_Disable(HAL_I2C_x); // 模块关闭
//
void HAL_I2C_SetBaud(HAL_I2C_x, uint32_t baud);
uint8_t HAL_I2C_TxData(HAL_I2C_x, uint8_t addr, uint8_t *data, uint32_t len);
uint8_t HAL_I2C_RxData(HAL_I2C_x, uint8_t addr, uint8_t *data, uint32_t len);

void HAL_I2C_SetGpioScl(HAL_I2C_x, HAL_GPIO_Px i2cgpio);
void HAL_I2C_SetGpioSda(HAL_I2C_x, HAL_GPIO_Px i2cgpio);

void HAL_I2C_BusReset(HAL_I2C_x); // 复位总线

#endif
