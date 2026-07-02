
#include "../include/hal_lksmcu_hall.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"
#include "../hal_lks32mc07x.h"

/**
 * @brief 模块复位
 * @param hall 模块号
 */
void HAL_HALL_Reset(HAL_HALL_x hall)
{
    (void)hall;
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    HAL_REG_SET(SYS_SFT_RST, SYS_SFT_RST_HALL0_SFT_RST_BIT);
    HAL_REG_RESET(SYS_SFT_RST, SYS_SFT_RST_HALL0_SFT_RST_BIT);
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}

/**
 * @brief 模块打开
 * @param hall 模块号
 */
void HAL_HALL_Enable(HAL_HALL_x hall)
{
    HAL_REG_SYS_UNLOCK();
    HAL_REG_SET(SYS_CLK_FEN, SYS_CLK_FEN_HALL0_CLK_EN_BIT);
    HAL_REG_SET(HAL_HALL[hall]->CFG, HALL0_CFG_HALL_EN_BIT);
    HAL_REG_SYS_LOCK();
}

/**
 * @brief 模块关闭
 * @param hall 模块号
 */
void HAL_HALL_Disable(HAL_HALL_x hall)
{
    HAL_REG_SYS_UNLOCK();
    HAL_REG_RESET(HAL_HALL[hall]->CFG, HALL0_CFG_HALL_EN_BIT);
    HAL_REG_RESET(SYS_CLK_FEN, SYS_CLK_FEN_HALL0_CLK_EN_BIT);
    HAL_REG_SYS_LOCK();
}

/**
 * @brief 设置hall频率
 * @param hall 模块号
 * @param freq 频率值
 */
void HAL_HALL_SetFreq(HAL_HALL_x hall, uint32_t freq)
{
    uint32_t mclk = HAL_CLK_GetMcuFreq();
    uint32_t th   = (mclk / freq);
    HAL_REG_WRITE(HAL_HALL[hall]->TH, th);
}

/**
 * @brief 设置hall阈值
 * @param hall 模块号
 * @param th 阈值
 */
void HAL_HALL_SetTh(HAL_HALL_x hall, uint32_t th)
{
    HAL_REG_WRITE(HAL_HALL[hall]->TH, th);
}

/**
 * @brief 获取当前hall的周期计数值
 * @param hall 模块号
 * @return 周期计数值
 */
uint32_t HAL_HALL_ReadTh(HAL_HALL_x hall)
{
    return HAL_REG_READ(HAL_HALL[hall]->TH);
}

/**
 * @brief 获取当前hall的数据
 * @param hall 模块号
 * @return 数据值
 */
uint8_t HAL_HALL_ReadHallData(HAL_HALL_x hall)
{
    return HAL_REG_READ(HAL_HALL[hall]->INFO);
}

/**
 * @brief 获取当前hall的宽度
 * @param hall 模块号
 * @return 宽度值
 */
uint32_t HAL_HALL_ReadHallWidth(HAL_HALL_x hall)
{
    return HAL_REG_READ(HAL_HALL[hall]->WIDTH);
}

/**
 * @brief 设置hall输入引脚
 * @param hall 模块号
 * @param px 引脚
 */
void HAL_HALL_SetGpio(HAL_HALL_x hall, HAL_GPIO_Px px)
{
    (void)hall;
    HAL_GPIO_SetMode(px, HAL_GPIO_MODE_IN);
    HAL_GPIO_SetAf(px, HAL_GPIO_AF_HALL);
}

/**
 * @brief 开始计数 HAL_HALL_Enable后默认开始计数
 * @param hall 模块号
 */
void HAL_HALL_StartCounting(HAL_HALL_x hall)
{
    HAL_REG_SET(HAL_HALL[hall]->CFG, HALL0_CFG_HALL_EN_BIT);
}
/**
 * @brief 停止计数(同时计数器回到0)
 * @param hall 模块号
 */
void HAL_HALL_StopCounting(HAL_HALL_x hall)
{
    HAL_REG_RESET(HAL_HALL[hall]->CFG, HALL0_CFG_HALL_EN_BIT);
    HAL_REG_WRITE(HAL_HALL[hall]->CNT, 0);
}

/**
 * @brief 设置滤波
 * @param hall 模块号
 * @param filter_mclk 滤波值
 */
void HAL_HALL_SetFilter(HAL_HALL_x hall, uint32_t filter_mclk)
{
    if (filter_mclk < 5)
    {
        HAL_REG_RESET(HAL_HALL[hall]->CFG, HALL0_CFG_FIL_75_BIT); // 小于5个cnt关闭7/5滤波
        HAL_REG_WRITE_BIT_MASK_POS(HAL_HALL[hall]->CFG, HALL0_CFG_CLK_DIV, 0);
        HAL_REG_WRITE_BIT_MASK_POS(HAL_HALL[hall]->CFG, HALL0_CFG_FIL_LEN, filter_mclk);
    }
    else
    {
        uint32_t div = 0;
        HAL_REG_SET(HAL_HALL[hall]->CFG, HALL0_CFG_FIL_75_BIT); // 大于5个cnt打开7/5滤波
        // 自动计算分配系数
        for (; filter_mclk > HALL0_CFG_FIL_LEN_MAX;)
        {
            filter_mclk >>= 1;
            div++;
        }
        if (div > HALL0_CFG_CLK_DIV_MAX)
        {
            div         = HALL0_CFG_CLK_DIV_MAX;
            filter_mclk = HALL0_CFG_FIL_LEN_MAX;
        }
        HAL_REG_WRITE_BIT_MASK_POS(HAL_HALL[hall]->CFG, HALL0_CFG_CLK_DIV, div);
        HAL_REG_WRITE_BIT_MASK_POS(HAL_HALL[hall]->CFG, HALL0_CFG_FIL_LEN, filter_mclk);
    }
}
/**
 * @brief 设置滤波，单位：uS
 * @param hall 模块号
 * @param filter_uS 滤波值
 */
void HAL_HALL_SetFilteruS(HAL_HALL_x hall, uint32_t filter_uS)
{
    uint32_t clk = HAL_CLK_GetMcuFreq();
    HAL_HALL_SetFilter(hall, (clk / 1000000) * filter_uS);
}
