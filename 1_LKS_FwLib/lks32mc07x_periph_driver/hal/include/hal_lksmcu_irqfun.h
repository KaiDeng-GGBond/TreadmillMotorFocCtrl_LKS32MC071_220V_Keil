
#ifndef __HAL_LKSMCU_IRQFUN_H__
#define __HAL_LKSMCU_IRQFUN_H__

#include "../hal_lks32mc07x.h"

uint8_t HAL_HALL_IrqChange(HAL_HALL_x);
uint8_t HAL_HALL_IrqChangeFlg(HAL_HALL_x hall);
void HAL_HALL_IrqChangeEnable(HAL_HALL_x);
void HAL_HALL_IrqChangeDisable(HAL_HALL_x hall);
void HAL_HALL_IrqChangeClear(HAL_HALL_x hall);

uint8_t HAL_HALL_IrqOver(HAL_HALL_x);
uint8_t HAL_HALL_IrqOverFlg(HAL_HALL_x hall);
void HAL_HALL_IrqOverEnable(HAL_HALL_x);
void HAL_HALL_IrqOverDisable(HAL_HALL_x hall);
void HAL_HALL_IrqOverClear(HAL_HALL_x hall);

uint8_t HAL_ADC_IrqSamp1(HAL_ADC_x);
uint8_t HAL_ADC_IrqSamp1Flg(HAL_ADC_x adc);
void HAL_ADC_IrqSamp1Enable(HAL_ADC_x);
void HAL_ADC_IrqSamp1Disable(HAL_ADC_x adc);
void HAL_ADC_IrqSamp1Clear(HAL_ADC_x adc);

uint8_t HAL_ADC_IrqSamp2(HAL_ADC_x);
uint8_t HAL_ADC_IrqSamp2Flg(HAL_ADC_x adc);
void HAL_ADC_IrqSamp2Enable(HAL_ADC_x);
void HAL_ADC_IrqSamp2Disable(HAL_ADC_x adc);
void HAL_ADC_IrqSamp2Clear(HAL_ADC_x adc);

#endif
