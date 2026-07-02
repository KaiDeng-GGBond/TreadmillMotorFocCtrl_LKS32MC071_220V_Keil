
#include "../include/hal_lksmcu_timer.h"
#include "../include/hal_lksmcu_gpio.h"
#include "../hal_lks32mc07x.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"

// 模块复位
void HAL_TIMER_Reset(HAL_TIMER_x timer_x)
{
    HAL_REG_SYS_UNLOCK();
    switch (timer_x)
    {
        case HAL_TIMER_0:
            HAL_REG_SET(SYS_SFT_RST, SYS_SFT_RST_TIMER0_SFT_RST_BIT);
            HAL_REG_RESET(SYS_SFT_RST, SYS_SFT_RST_TIMER0_SFT_RST_BIT);
            break;
        case HAL_TIMER_1:
            HAL_REG_SET(SYS_SFT_RST, SYS_SFT_RST_TIMER1_SFT_RST_BIT);
            HAL_REG_RESET(SYS_SFT_RST, SYS_SFT_RST_TIMER1_SFT_RST_BIT);
            break;
        case HAL_TIMER_2:
            HAL_REG_SET(SYS_SFT_RST, SYS_SFT_RST_TIMER2_SFT_RST_BIT);
            HAL_REG_RESET(SYS_SFT_RST, SYS_SFT_RST_TIMER2_SFT_RST_BIT);
            break;
        case HAL_TIMER_3:
            HAL_REG_SET(SYS_SFT_RST, SYS_SFT_RST_TIMER3_SFT_RST_BIT);
            HAL_REG_RESET(SYS_SFT_RST, SYS_SFT_RST_TIMER3_SFT_RST_BIT);
            break;
        default:
            break;
    }
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}
// 模块打开
void HAL_TIMER_Enable(HAL_TIMER_x timer_x)
{
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    switch (timer_x)
    {
        case HAL_TIMER_0:
            HAL_REG_SET(SYS_CLK_FEN, SYS_CLK_FEN_TIMER0_CLK_EN_BIT);
            break;
        case HAL_TIMER_1:
            HAL_REG_SET(SYS_CLK_FEN, SYS_CLK_FEN_TIMER1_CLK_EN_BIT);
            break;
        case HAL_TIMER_2:
            HAL_REG_SET(SYS_CLK_FEN, SYS_CLK_FEN_TIMER2_CLK_EN_BIT);
            break;
        case HAL_TIMER_3:
            HAL_REG_SET(SYS_CLK_FEN, SYS_CLK_FEN_TIMER3_CLK_EN_BIT);
            break;
        default:
            break;
    }
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}
// 模块关闭
void HAL_TIMER_Disable(HAL_TIMER_x timer_x)
{
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    switch (timer_x)
    {
        case HAL_TIMER_0:
            HAL_REG_RESET(SYS_CLK_FEN, SYS_CLK_FEN_TIMER0_CLK_EN_BIT);
            break;
        case HAL_TIMER_1:
            HAL_REG_RESET(SYS_CLK_FEN, SYS_CLK_FEN_TIMER1_CLK_EN_BIT);
            break;
        case HAL_TIMER_2:
            HAL_REG_RESET(SYS_CLK_FEN, SYS_CLK_FEN_TIMER2_CLK_EN_BIT);
            break;
        case HAL_TIMER_3:
            HAL_REG_RESET(SYS_CLK_FEN, SYS_CLK_FEN_TIMER3_CLK_EN_BIT);
            break;
        default:
            break;
    }
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}
/**
 * @brief 设置定时器周期
 * @param timer_x 定时器编号
 * @param count 定时器周期
 */
void HAL_TIMER_SetPeriodCount(HAL_TIMER_x timer_x, uint32_t count)
{
    if (count > 0)
    {
        count = count - 1;
    }
    if ((timer_x == HAL_TIMER_0) || (timer_x == HAL_TIMER_1))
    {

        uint32_t div = 0;
        for (; count > TIMER0_TH_TH_MAX;)
        {
            count >>= 1;
            div++;
        }
        if (div > TIMER0_CFG_CLK_DIV_MAX)
        {
            div   = TIMER0_CFG_CLK_DIV_MAX;
            count = TIMER0_TH_TH_MAX;
        }
        HAL_REG_WRITE_BIT_MASK_POS(HAL_TIMER[timer_x]->CFG, TIMER0_CFG_CLK_DIV, div);
        HAL_REG_WRITE_BIT_MASK_POS(HAL_TIMER[timer_x]->TH, TIMER0_TH_TH, count);
    }
    else
    {
        uint32_t div = 0;
        for (; count > TIMER2_TH_TH_MAX;)
        {
            count >>= 1;
            div++;
        }
        if (div > TIMER2_CFG_CLK_DIV_MAX)
        {
            div   = TIMER2_CFG_CLK_DIV_MAX;
            count = TIMER2_TH_TH_MAX;
        }
        HAL_REG_WRITE_BIT_MASK_POS(HAL_TIMER[timer_x]->CFG, TIMER2_CFG_CLK_DIV, div);
        HAL_REG_WRITE_BIT_MASK_POS(HAL_TIMER[timer_x]->TH, TIMER2_TH_TH, count);
    }
}
/**
 * @brief 设置定时器频率
 * @param timer_x 定时器编号
 * @param freq 定时器频率
 */
void HAL_TIMER_SetFreq(HAL_TIMER_x timer_x, uint32_t freq)
{
    uint32_t mclk = HAL_CLK_GetMcuFreq();
    uint32_t th   = (mclk / freq);
    HAL_TIMER_SetPeriodCount(timer_x, th);
}

/**
 * @brief 设置定时器周期(ms)
 * @param timer_x 定时器编号
 * @param ms 定时器周期(ms)
 */
void HAL_TIMER_SetPeriodmS(HAL_TIMER_x timer_x, uint32_t ms)
{
    uint32_t mclk = HAL_CLK_GetMcuFreq();
    uint32_t th   = (mclk / 1000) * ms;
    HAL_TIMER_SetPeriodCount(timer_x, th);
}
/**
 * @brief 设置定时器周期(us)
 * @param timer_x 定时器编号
 * @param us 定时器周期(us)
 */
void HAL_TIMER_SetPerioduS(HAL_TIMER_x timer_x, uint32_t us)
{
    uint32_t mclk = HAL_CLK_GetMcuFreq();
    uint32_t th   = (mclk / 1000000) * us;
    HAL_TIMER_SetPeriodCount(timer_x, th);
}
// 获取当前的pwm频率
uint32_t HAL_TIMER_GetFreq(HAL_TIMER_x timer_x)
{
    uint32_t mclk = HAL_CLK_GetMcuFreq();
    uint32_t freq = (mclk / HAL_TIMER[timer_x]->TH);
    return freq;
}
// 获取计数器最大值
uint32_t HAL_TIMER_GetThCnt(HAL_TIMER_x timer_x)
{
    return HAL_TIMER[timer_x]->TH;
}
// 开始计数
void HAL_TIMER_StartCounting(HAL_TIMER_x timer_x)
{
    HAL_REG_SET(HAL_TIMER[timer_x]->CFG, BIT31);
}
// 停止计数(同时计数器回到0)
void HAL_TIMER_StopCounting(HAL_TIMER_x timer_x)
{
    HAL_REG_RESET(HAL_TIMER[timer_x]->CFG, BIT31);
    HAL_TIMER[timer_x]->CNT = 0;
}
// 设置pwm波形 仅PWM模式下有效
void HAL_TIMER_SetPwm(HAL_TIMER_x timer_x, int32_t t0, int32_t t1)
{
    HAL_REG_WRITE(HAL_TIMER[timer_x]->CMP0, t0);
    HAL_REG_WRITE(HAL_TIMER[timer_x]->CMP1, t1);
}
// 设置pwm波形q15格式输入 仅PWM模式下有效
void HAL_TIMER_SetPwmQ15(HAL_TIMER_x timer_x, int16_t t0, int16_t t1)
{
    uint32_t th = HAL_TIMER_GetThCnt(timer_x);
    t0          = t0 * th >> 15;
    t1          = t1 * th >> 15;
    HAL_REG_WRITE(HAL_TIMER[timer_x]->CMP0, t0);
    HAL_REG_WRITE(HAL_TIMER[timer_x]->CMP1, t1);
}
// 设置工作模式
void HAL_TIMER_SetWorkMode(HAL_TIMER_x timer_x, HAL_TIMER_MODE_x mode_x)
{
    switch (mode_x)
    {
        case HAL_TIMER_MODE_COUNT: ///< 计数模式
            HAL_REG_WRITE_BIT(HAL_TIMER[timer_x]->CFG, 0xff, 0);
            break;
        case HAL_TIMER_MODE_PWMOUT: ///< PWM输出模式
            HAL_REG_WRITE_BIT(HAL_TIMER[timer_x]->CFG, 0xff, BIT7 | BIT3);
            break;
        case HAL_TIMER_MODE_PWMIN: ///< PWM输入模式
            HAL_REG_WRITE_BIT(HAL_TIMER[timer_x]->CFG, 0xff, BIT6 | BIT5 | BIT2 | BIT0);
            break;
        default:
            break;
    }
}

uint8_t HAL_TIMER_IrqOverflow(HAL_TIMER_x timer_x)
{
    uint32_t val1;

    val1 = HAL_TIMER[timer_x]->IE & HAL_TIMER[timer_x]->IF;
    if (val1 & BIT2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_TIMER_IrqOverflowEnable(HAL_TIMER_x timer_x)
{
    HAL_REG_SET(HAL_TIMER[timer_x]->IE, BIT2);
}
void HAL_TIMER_IrqOverflowDisable(HAL_TIMER_x timer_x)
{
    HAL_REG_RESET(HAL_TIMER[timer_x]->IE, BIT2);
}
void HAL_TIMER_IrqOverflowClear(HAL_TIMER_x timer_x)
{
    HAL_REG_WRITE(HAL_TIMER[timer_x]->IF, BIT2);
}

uint8_t HAL_TIMER_IrqCompare0(HAL_TIMER_x timer_x)
{
    uint32_t val1;

    val1 = HAL_TIMER[timer_x]->IE & HAL_TIMER[timer_x]->IF;
    if (val1 & BIT0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_TIMER_IrqCompare0Enable(HAL_TIMER_x timer_x)
{
    HAL_REG_SET(HAL_TIMER[timer_x]->IE, BIT0);
}
void HAL_TIMER_IrqCompare0Disable(HAL_TIMER_x timer_x)
{
    HAL_REG_RESET(HAL_TIMER[timer_x]->IE, BIT0);
}
void HAL_TIMER_IrqCompare0Clear(HAL_TIMER_x timer_x)
{
    HAL_REG_WRITE(HAL_TIMER[timer_x]->IF, BIT0);
}

uint8_t HAL_TIMER_IrqCompare1(HAL_TIMER_x timer_x)
{
    uint32_t val1;

    val1 = HAL_TIMER[timer_x]->IE & HAL_TIMER[timer_x]->IF;
    if (val1 & BIT1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_TIMER_IrqCompare1Enable(HAL_TIMER_x timer_x)
{
    HAL_REG_SET(HAL_TIMER[timer_x]->IE, BIT1);
}
void HAL_TIMER_IrqCompare1Disable(HAL_TIMER_x timer_x)
{
    HAL_REG_RESET(HAL_TIMER[timer_x]->IE, BIT1);
}
void HAL_TIMER_IrqCompare1Clear(HAL_TIMER_x timer_x)
{
    HAL_REG_WRITE(HAL_TIMER[timer_x]->IF, BIT1);
}

/**
 * @brief 设置定时器输出gpio
 * @param timer_x 定时器编号
 * @param px gpio编号
 */
void HAL_TIMER_SetGpioOut(HAL_TIMER_x timer_x, HAL_GPIO_Px px)
{
    switch (timer_x)
    {
        case HAL_TIMER_0:
            HAL_GPIO_SetAf(px, HAL_GPIO_AF_TIMER0);
            HAL_GPIO_SetMode(px, HAL_GPIO_MODE_OUT);
            break;
        case HAL_TIMER_1:
            HAL_GPIO_SetAf(px, HAL_GPIO_AF_TIMER1);
            HAL_GPIO_SetMode(px, HAL_GPIO_MODE_OUT);
            break;
        case HAL_TIMER_2:
            HAL_GPIO_SetAf(px, HAL_GPIO_AF_TIMER2);
            HAL_GPIO_SetMode(px, HAL_GPIO_MODE_OUT);
            break;
        case HAL_TIMER_3:
            HAL_GPIO_SetAf(px, HAL_GPIO_AF_TIMER3);
            HAL_GPIO_SetMode(px, HAL_GPIO_MODE_OUT);
            break;
        default:
            break;
    }
}
/**
 * @brief 设置定时器输入gpio
 * @param timer_x 定时器编号
 * @param px gpio编号
 */
void HAL_TIMER_SetGpioIn(HAL_TIMER_x timer_x, HAL_GPIO_Px px)
{
    switch (timer_x)
    {
        case HAL_TIMER_0:
            HAL_GPIO_SetAf(px, HAL_GPIO_AF_TIMER0);
            HAL_GPIO_SetMode(px, HAL_GPIO_MODE_IN);
            break;
        case HAL_TIMER_1:
            HAL_GPIO_SetAf(px, HAL_GPIO_AF_TIMER1);
            HAL_GPIO_SetMode(px, HAL_GPIO_MODE_IN);
            break;
        case HAL_TIMER_2:
            HAL_GPIO_SetAf(px, HAL_GPIO_AF_TIMER2);
            HAL_GPIO_SetMode(px, HAL_GPIO_MODE_IN);
            break;
        case HAL_TIMER_3:
            HAL_GPIO_SetAf(px, HAL_GPIO_AF_TIMER3);
            HAL_GPIO_SetMode(px, HAL_GPIO_MODE_IN);
            break;
        default:
            break;
    }
}
