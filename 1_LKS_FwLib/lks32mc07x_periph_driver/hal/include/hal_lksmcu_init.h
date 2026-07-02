#ifndef __HAL_LKSMCU_INIT_H__
#define __HAL_LKSMCU_INIT_H__
typedef enum
{
    HAL_INIT_POWER_3_3V = 0,
    HAL_INIT_POWER_5V,
} HAL_INIT_POWER_x;
void HAL_INIT_SysInit(void);
void HAL_INIT_PowerReady(HAL_INIT_POWER_x powerx);

#endif
