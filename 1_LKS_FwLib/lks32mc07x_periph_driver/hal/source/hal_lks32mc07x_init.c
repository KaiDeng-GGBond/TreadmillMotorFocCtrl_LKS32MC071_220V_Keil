#include "../include/hal_lksmcu_init.h"
#include "lks32mc07x.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"
void HAL_INIT_SysInit(void)
{
    IWDG_CFG       = 0x3c00;
    SYS_WR_PROTECT = 0x7a83;     /* 解除系统寄存器写保护 */
    SYS_AFE_REG5   = BIT15;      /* BIT15:PLLPDN 开PLL */
    SYS_CLK_CFG    = 0x000001ff; /* BIT8:0: CLK_HS,1:PLL  | BIT[7:0]CLK_DIV  | 1ff对应96M时钟 */
}

void HAL_INIT_PowerReady(HAL_INIT_POWER_x powerx)
{
    switch (powerx)
    {
        case HAL_INIT_POWER_3_3V: // 等待3.3V电源就绪，判断阈值为2.8V
            __disable_irq();
            // 设置掉电检测电压
            HAL_REG_SYS_UNLOCK();
            HAL_REG_WRITE_BIT_MASK_POS(HAL_SYS[0]->REG6, SYS_AFE_REG6_PVDSEL, 3);
            HAL_REG_RESET(HAL_SYS[0]->REG6, (SYS_AFE_REG6_VSR_PDT_BIT | SYS_AFE_REG6_PD_PDT_BIT));
            HAL_REG_SYS_LOCK();
            // 等待电源就绪
            while (HAL_REG_READ_BIT(HAL_SYS[0]->DBG, SYS_AFE_DBG_PWR_WEAK_BIT))
            {
                __NOP();
            }
            break;
        case HAL_INIT_POWER_5V: // 等待5.0V电源就绪，判断阈值为4.0V
            __disable_irq();
            // 设置掉电检测电压
            HAL_REG_SYS_UNLOCK();
            HAL_REG_WRITE_BIT_MASK_POS(HAL_SYS[0]->REG6, SYS_AFE_REG6_PVDSEL, 0);
            HAL_REG_RESET(HAL_SYS[0]->REG6, (SYS_AFE_REG6_VSR_PDT_BIT | SYS_AFE_REG6_PD_PDT_BIT));
            HAL_REG_SYS_LOCK();
            // 等待电源就绪
            while (HAL_REG_READ_BIT(HAL_SYS[0]->DBG, SYS_AFE_DBG_PWR_WEAK_BIT))
            {
                __NOP();
            }
            break;
    }
    // 清除中断标志位
    NVIC_ClearPendingIRQ(PWRDN_IRQn);
}
