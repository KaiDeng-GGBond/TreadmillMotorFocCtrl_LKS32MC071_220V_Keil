#include "../include/hal_lksmcu_clk.h"
#include "lks32mc07x.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"
static volatile uint32_t HAL_CLK_mclk = 96000000;
/**
 * @brief 设置主频
 * @param FREQ_x 主频
 */
void HAL_CLK_SetMcuFreq(HAL_CLK_FREQ_x FREQ_x)
{
    HAL_REG_SYS_UNLOCK();
    switch (FREQ_x)
    {
        case HAL_CLK_FREQ_RC_4MHZ: ///< 主频4Mhz
            HAL_CLK_mclk = 4000000;
            HAL_REG_WRITE(SYS_CLK_CFG, 0);
            break;
        case HAL_CLK_FREQ_RC_12MHZ: ///< 主频12Mhz
            HAL_CLK_mclk = 12000000;
            HAL_REG_WRITE(SYS_CLK_CFG, 0x101);
            break;
        case HAL_CLK_FREQ_RC_24MHZ: ///< 主频24Mhz
            HAL_CLK_mclk = 24000000;
            HAL_REG_WRITE(SYS_CLK_CFG, 0x111);
            break;
        case HAL_CLK_FREQ_RC_48MHZ: ///< 主频48Mhz
            HAL_CLK_mclk = 48000000;
            HAL_REG_WRITE(SYS_CLK_CFG, 0x155);
            break;
        default:
        case HAL_CLK_FREQ_RC_96MHZ: ///< 主频96Mhz
            HAL_CLK_mclk = 96000000;
            HAL_REG_WRITE(SYS_CLK_CFG, 0x1ff);
            break;
    }
    HAL_REG_SYS_LOCK();
}

/**
 * @brief 获取主频
 * @return 主频
 */
uint32_t HAL_CLK_GetMcuFreq(void)
{
    return HAL_CLK_mclk;
}
