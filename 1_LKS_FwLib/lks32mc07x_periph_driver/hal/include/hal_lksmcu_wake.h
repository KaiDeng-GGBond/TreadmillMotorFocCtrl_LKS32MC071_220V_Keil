
#ifndef __HAL_LKSMCU_WEAK_H__
#define __HAL_LKSMCU_WEAK_H__
#include "stdint.h"
typedef enum
{
    HAL_WAKE_UP_IO_0_Low  = 0x0001,
    HAL_WAKE_UP_IO_0_High = 0x0101,
    HAL_WAKE_UP_IO_1_Low  = 0x0002,
    HAL_WAKE_UP_IO_1_High = 0x0102,
    HAL_WAKE_UP_IO_2_Low  = 0x0004,
    HAL_WAKE_UP_IO_2_High = 0x0104,
    HAL_WAKE_UP_IO_3_Low  = 0x0008,
    HAL_WAKE_UP_IO_3_High = 0x0108,
    HAL_WAKE_UP_IO_4_Low  = 0x0010,
    HAL_WAKE_UP_IO_4_High = 0x0110,
    HAL_WAKE_UP_IO_5_Low  = 0x0020,
    HAL_WAKE_UP_IO_5_High = 0x0120,
    HAL_WAKE_UP_IO_6_Low  = 0x0040,
    HAL_WAKE_UP_IO_6_High = 0x0140,
    HAL_WAKE_UP_IO_7_Low  = 0x0080,
    HAL_WAKE_UP_IO_7_High = 0x0180,
} HAL_WAKE_UP_IO_x;

void HAL_WAKE_DeepSleepIoWake(HAL_WAKE_UP_IO_x);
void HAL_WAKE_DeepSleepTimeWakeUp(uint32_t ms);
void HAL_WAKE_DeepSleepIoTimeWakeUp(HAL_WAKE_UP_IO_x, uint32_t ms);
#endif
