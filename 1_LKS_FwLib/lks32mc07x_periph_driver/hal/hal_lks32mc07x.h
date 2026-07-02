#include "basic.h"
#include "stdint.h"
#include "lks32mc07x.h"

#ifndef __HAL_LKS32MC07X_H__
#define __HAL_LKS32MC07X_H__

/**
 * @brief 统一中断函数接口
 * @ref
 * 命名方式：
 * HAL_模块名称_IRQHandler_模块序号中断序号。
 * 例如MCPWM模块有有两个CNT,会产生两个中断。
 * 这里分别对应MCPWM0_IRQHandler和MCPWM1_IRQHandler。
 * 为方便使用,末尾的_00可以省略。
 */
#define HAL_TIMER_IRQHandler_00 TIMER0_IRQHandler
#define HAL_TIMER_IRQHandler_10 TIMER1_IRQHandler
#define HAL_TIMER_IRQHandler_20 TIMER2_IRQHandler
#define HAL_TIMER_IRQHandler_30 TIMER3_IRQHandler
#define HAL_MCPWM_IRQHandler_00 MCPWM0_IRQHandler
#define HAL_MCPWM_IRQHandler_01 MCPWM1_IRQHandler
#define HAL_I2C_IRQHandler_00   I2C0_IRQHandler
#define HAL_SPI_IRQHandler_00   SPI0_IRQHandler
#define HAL_GPIO_IRQHandler_00  GPIO_IRQHandler
#define HAL_HALL_IRQHandler_00  HALL0_IRQHandler
#define HAL_UART_IRQHandler_00  UART0_IRQHandler
#define HAL_UART_IRQHandler_10  UART1_IRQHandler
#define HAL_CMP_IRQHandler_00   CMP_IRQHandler
#define HAL_ADC_IRQHandler_00   ADC0_IRQHandler
#define HAL_ADC_IRQHandler_10   ADC1_IRQHandler
#define HAL_WAKE_IRQHandler_00  WAKE_IRQHandler
#define HAL_QEP_IRQHandler_00   QEP0_IRQHandler
#define HAL_QEP_IRQHandler_10   QEP1_IRQHandler
#define HAL_DSP_IRQHandler_00   DSP0_IRQHandler
#define HAL_DMA_IRQHandler_00   DMA0_IRQHandler
#define HAL_CAN_IRQHandler_00   CAN0_IRQHandler
#define HAL_SIF_IRQHandler_00   SIF0_IRQHandler
#define HAL_SW_IRQHandler_00    SW_IRQHandler
#define HAL_PWRDN_IRQHandler_00 PWRDN_IRQHandler
#define HAL_CL_IRQHandler_00    CL0_IRQHandler
#define HAL_INIT_IRQHandler_00  PWRDN_IRQHandler

// 简化写法
#define HAL_TIMER_IRQHandler HAL_TIMER_IRQHandler_00
#define HAL_MCPWM_IRQHandler HAL_MCPWM_IRQHandler_00
#define HAL_I2C_IRQHandler   HAL_I2C_IRQHandler_00
#define HAL_SPI_IRQHandler   HAL_SPI_IRQHandler_00
#define HAL_GPIO_IRQHandler  HAL_GPIO_IRQHandler_00
#define HAL_HALL_IRQHandler  HAL_HALL_IRQHandler_00
#define HAL_UART_IRQHandler  HAL_UART_IRQHandler_00
#define HAL_CMP_IRQHandler   HAL_CMP_IRQHandler_00
#define HAL_ADC_IRQHandler   HAL_ADC_IRQHandler_00
#define HAL_DMA_IRQHandler   HAL_DMA_IRQHandler_00
#define HAL_WAKE_IRQHandler  HAL_WAKE_IRQHandler_00
#define HAL_QEP_IRQHandler   HAL_QEP_IRQHandler_00
#define HAL_DSP_IRQHandler   HAL_DSP_IRQHandler_00
#define HAL_CAN_IRQHandler   HAL_CAN_IRQHandler_00
#define HAL_SIF_IRQHandler   HAL_SIF_IRQHandler_00
#define HAL_SW_IRQHandler    HAL_SW_IRQHandler_00
#define HAL_PWRDN_IRQHandler HAL_PWRDN_IRQHandler_00
#define HAL_CL_IRQHandler    HAL_CL_IRQHandler_00
#define HAL_INIT_IRQHandler  HAL_INIT_IRQHandler_00

// 当前芯片不支持的中断函数
#define HAL_UART_IRQHandler_20  HAL_UART_IRQHandler_20_Null
#define HAL_CMP_IRQHandler_10   HAL_CMP_IRQHandler_10_Null
#define HAL_I2C_IRQHandler_10   HAL_I2C_IRQHandler_10_Null
#define HAL_HALL_IRQHandler_10  HAL_HALL_IRQHandler_10_Null
#define HAL_CMP_IRQHandler_20   HAL_CMP_IRQHandler_20_Null
#define HAL_CMP_IRQHandler_30   HAL_CMP_IRQHandler_30_Null
#define HAL_CMP_IRQHandler_40   HAL_CMP_IRQHandler_40_Null
#define HAL_CMP_IRQHandler_50   HAL_CMP_IRQHandler_50_Null
#define HAL_MCPWM_IRQHandler_10 HAL_MCPWM_IRQHandler_10_Null
#define HAL_MCPWM_IRQHandler_11 HAL_MCPWM_IRQHandler_11_Null
#define HAL_TIMER_IRQHandler_40 HAL_TIMER_IRQHandler_40_Null
#define HAL_QEP_IRQHandler_20   HAL_QEP_IRQHandler_20_Null
#define HAL_QEP_IRQHandler_30   HAL_QEP_IRQHandler_30_Null
#define HAL_ADC_IRQHandler_20   HAL_ADC_IRQHandler_20_Null
#define HAL_SPI_IRQHandler_10   HAL_SPI_IRQHandler_10_Null
#define HAL_WWDG_IRQHandler_00  HAL_WWDG_IRQHandler_00_Null
#define HAL_FMAC_IRQHandler_00  HAL_FMAC_IRQHandler_00_Null

/**
 * @brief 芯片的默认时钟频率
 */
#define HAL_INIT_DEFAULE_MCLK    96000000
#define HAL_INIT_DEFAULE_MCLK_US 96    // 上电后的默认时钟1uS分频系数
#define HAL_INIT_DEFAULE_MCLK_MS 96000 // 上电后的默认时钟1mS分频系数

#include "include/hal_lksmcu_adc.h"
#include "include/hal_lksmcu_can.h"
#include "include/hal_lksmcu_clk.h"
#include "include/hal_lksmcu_cmp.h"
#include "include/hal_lksmcu_crc.h"
#include "include/hal_lksmcu_dac.h"
#include "include/hal_lksmcu_dma.h"
#include "include/hal_lksmcu_dsp.h"
#include "include/hal_lksmcu_flash.h"
#include "include/hal_lksmcu_gpio.h"
#include "include/hal_lksmcu_hall.h"
#include "include/hal_lksmcu_i2c.h"
#include "include/hal_lksmcu_irq.h"
#include "include/hal_lksmcu_init.h"
#include "include/hal_lksmcu_iwdg.h"
#include "include/hal_lksmcu_mcpwm.h"
#include "include/hal_lksmcu_opa.h"
#include "include/hal_lksmcu_qep.h"
#include "include/hal_lksmcu_reg.h"
#include "include/hal_lksmcu_reset.h"
#include "include/hal_lksmcu_spi.h"
#include "include/hal_lksmcu_timer.h"
#include "include/hal_lksmcu_tmp.h"
#include "include/hal_lksmcu_trim.h"
#include "include/hal_lksmcu_uart.h"
#include "include/hal_lksmcu_wake.h"
#include "include/hal_lksmcu_irqfun.h"
#endif
