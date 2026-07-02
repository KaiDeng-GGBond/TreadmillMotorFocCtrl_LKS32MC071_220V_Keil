#ifndef __HAL_LKSMCU_IRQ_H__
#define __HAL_LKSMCU_IRQ_H__

#include "stdint.h"
#include "../hal_lks32mc07x.h"

/**
 * @brief 各个模块的中断编号
 * @ref 如果调用不存在中断的模块,则不做任何操作
 */
typedef enum
{
    // 来自arm内核的模块
    HAL_IRQ_MODULE_NonMaskableInt = NonMaskableInt_IRQn,
    HAL_IRQ_MODULE_HardFault      = HardFault_IRQn,
    HAL_IRQ_MODULE_SVCall         = SVCall_IRQn,
    HAL_IRQ_MODULE_PendSV         = PendSV_IRQn,
    HAL_IRQ_MODULE_SysTick        = SysTick_IRQn,
    // 芯片外设
    HAL_IRQ_MODULE_TIMER_00 = TIMER0_IRQn,
    HAL_IRQ_MODULE_TIMER_10 = TIMER1_IRQn,
    HAL_IRQ_MODULE_TIMER_20 = TIMER2_IRQn,
    HAL_IRQ_MODULE_MCPWM_00 = MCPWM0_IRQn,
    HAL_IRQ_MODULE_MCPWM_01 = MCPWM1_IRQn,
    HAL_IRQ_MODULE_I2C_00   = I2C0_IRQn,
    HAL_IRQ_MODULE_SPI_00   = SPI0_IRQn,
    HAL_IRQ_MODULE_GPIO_00  = GPIO_IRQn,
    HAL_IRQ_MODULE_HALL_00  = HALL0_IRQn,
    HAL_IRQ_MODULE_UART_00  = UART0_IRQn,
    HAL_IRQ_MODULE_UART_10  = UART1_IRQn,
    HAL_IRQ_MODULE_CMP_00   = CMP_IRQn,
    HAL_IRQ_MODULE_ADC_00   = ADC0_IRQn,
    HAL_IRQ_MODULE_ADC_10   = ADC1_IRQn,
    HAL_IRQ_MODULE_WAKE_00  = WAKE_IRQn,
    HAL_IRQ_MODULE_QEP_00   = QEP0_IRQn,
    HAL_IRQ_MODULE_DMA_00   = DMA0_IRQn,
    HAL_IRQ_MODULE_SIF_00   = SIF0_IRQn,
    HAL_IRQ_MODULE_SW_00    = SW_IRQn,
    HAL_IRQ_MODULE_INIT_00  = PWRDN_IRQn,
    HAL_IRQ_MODULE_CL_00    = CL0_IRQn,

    HAL_IRQ_MODULE_UART_20, // 不支持
    HAL_IRQ_MODULE_CMP_10,  // 不支持
} HAL_IRQ_MODULE_x;

void HAL_IRQ_Reset(void);   // 重置中断(等效于关闭中断)
void HAL_IRQ_Enable(void);  // 使能全局中断
void HAL_IRQ_Disable(void); // 关闭全局中断

void HAL_IRQ_ModuleEnable(HAL_IRQ_MODULE_x, uint8_t priority); // 使能某个模块的中断
void HAL_IRQ_ModuleDisable(HAL_IRQ_MODULE_x);                  // 关闭某个模块的中断

#endif
