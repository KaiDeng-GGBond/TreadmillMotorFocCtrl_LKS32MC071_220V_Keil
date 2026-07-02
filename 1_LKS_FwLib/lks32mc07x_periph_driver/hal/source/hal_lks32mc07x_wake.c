#include "../include/hal_lksmcu_iwdg.h"
#include "../include/hal_lksmcu_wake.h"
#include "../hal_lks32mc07x.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"

/**
 * @brief 进入深度休眠,并且使用IO唤醒
 * @param wake_up_io 指定唤醒IO引脚及其电平
 */
void HAL_WAKE_DeepSleepIoWake(HAL_WAKE_UP_IO_x wake_up_io)
{
    (void)wake_up_io;
    u32 t_cnt;
    uint32_t reg5;         // 模拟配置
    uint32_t clkcfg;       // 时钟配置
    uint32_t clk_fen;      // 模块使能配置
    uint32_t saved_enable; // 中断配置

    __disable_irq();              // 关闭全局中断，确保执行休眠期间，不会进入其他任何中断
    reg5         = SYS_AFE_REG5;  // 保存模拟使能配置
    clkcfg       = SYS_CLK_CFG;   // 保存时钟配置
    clk_fen      = SYS_CLK_FEN;   // 保存模块时钟使能配置
    saved_enable = NVIC->ISER[0]; // 保存当前中断配置

    SYS_WR_PROTECT = 0x7a83;               // 开启寄存器写使能
    NVIC->ICER[0]  = 0xFFFFFFFF;           // 关闭所有中断
    NVIC_EnableIRQ(WAKE_IRQn);             // 打开唤醒中断
    IWDG_PSW    = 0xA6B4;                  // 打开看门狗使能
    IWDG_CLR    = 0x798D;                  // 喂狗
    SYS_CLK_CFG = 0;                       // 切换到HRC时钟
    for (t_cnt = 0; t_cnt < 4000; t_cnt++) // 等待时钟稳定
    {
        __NOP();
    }
    SYS_CLK_FEN    = 0;            // 关闭数字模块时钟
    SYS_AFE_REG5   = BIT10 | BIT8; // 准备关闭 PLL,BGP,以及其它模拟电路使能
    SYS_WR_PROTECT = 0x0;          // 关闭系统寄存器写使能
    __enable_irq();                // 打开全局中断
    SCB->SCR |= (1UL << 2);        // 使能深度休眠

    __WFI();                    // 休眠 wakeak
    __disable_irq();            // 唤醒后关闭全局中断
    NVIC_DisableIRQ(WAKE_IRQn); // 关闭唤醒中断

    SYS_WR_PROTECT = 0x7a83; // 开启寄存器写使能
    SYS_AFE_REG5   = reg5;   // 开启 PLL,BGP

    for (t_cnt = 0; t_cnt < 400; t_cnt++) // 此时使用hrc，等待>1mS，确保PLL稳定
    {
        __NOP();
    }
    SYS_CLK_CFG    = clkcfg;       // 恢复时钟配置
    NVIC->ISER[0]  = saved_enable; // 恢复中断使能
    SYS_CLK_FEN    = clk_fen;      // 打开其他模块
    SYS_WR_PROTECT = 0x0;          // 打开写保护
    __enable_irq();                // 使能全局中断
}

/**
 * @brief 进入深度休眠,并且使用定时唤醒
 * @param ms 指定定时唤醒的时间,单位为毫秒
 */
void HAL_WAKE_DeepSleepTimeWakeUp(uint32_t ms)
{
    u32 t_cnt;
    uint32_t reg5;         // 模拟配置
    uint32_t clkcfg;       // 时钟配置
    uint32_t clk_fen;      // 模块使能配置
    uint32_t saved_enable; // 中断配置

    __disable_irq();              // 关闭全局中断，确保执行休眠期间，不会进入其他任何中断
    reg5         = SYS_AFE_REG5;  // 保存模拟使能配置
    clkcfg       = SYS_CLK_CFG;   // 保存时钟配置
    clk_fen      = SYS_CLK_FEN;   // 保存模块时钟使能配置
    saved_enable = NVIC->ISER[0]; // 保存当前中断配置

    SYS_WR_PROTECT = 0x7a83;     // 开启寄存器写使能
    NVIC->ICER[0]  = 0xFFFFFFFF; // 关闭所有中断
    NVIC_EnableIRQ(WAKE_IRQn);   // 打开唤醒中断
    {
        int32_t th;
        th = ((uint32_t)(32 * ms + 0x800) & 0x1ff000);
        th = HAL_REG_READ(HAL_IWDG[HAL_IWDG_0]->RTH) - th;
        if (th < 0)
        {
            th = 0;
        }
        HAL_REG_WRITE(HAL_IWDG[HAL_IWDG_0]->WTH, th);
    }
    HAL_IWDG_Feed(HAL_IWDG_0);             // 喂狗
    SYS_CLK_CFG = 0;                       // 切换到HRC时钟
    for (t_cnt = 0; t_cnt < 4000; t_cnt++) // 等待时钟稳定
    {
        __NOP();
    }
    SYS_CLK_FEN    = 0;            // 关闭数字模块时钟
    SYS_AFE_REG5   = BIT10 | BIT8; // 准备关闭 PLL,BGP,以及其它模拟电路使能
    SYS_WR_PROTECT = 0x0;          // 关闭系统寄存器写使能

    __enable_irq();         // 打开全局中断
    SCB->SCR |= (1UL << 2); // 使能深度休眠

    __WFI();                    // 休眠 wakeak
    __disable_irq();            // 唤醒后关闭全局中断
    NVIC_DisableIRQ(WAKE_IRQn); // 关闭唤醒中断

    SYS_WR_PROTECT = 0x7a83; // 开启寄存器写使能
    SYS_AFE_REG5   = reg5;   // 开启 PLL,BGP

    for (t_cnt = 0; t_cnt < 400; t_cnt++) // 此时使用hrc，等待>1mS，确保PLL稳定
    {
        __NOP();
    }
    SYS_CLK_CFG    = clkcfg;       // 恢复时钟配置
    NVIC->ISER[0]  = saved_enable; // 恢复中断使能
    SYS_CLK_FEN    = clk_fen;      // 打开其他模块
    SYS_WR_PROTECT = 0x0;          // 打开写保护
    __enable_irq();                // 使能全局中断
}

/**
 * @brief 进入深度休眠,并且使用IO唤醒和定时唤醒
 * @param wake_up_io 指定唤醒IO引脚及其电平
 * @param ms 指定定时唤醒的时间,单位为毫秒
 */
void HAL_WAKE_DeepSleepIoTimeWakeUp(HAL_WAKE_UP_IO_x wake_up_io, uint32_t ms)
{
    u32 t_cnt;
    uint32_t reg5;         // 模拟配置
    uint32_t clkcfg;       // 时钟配置
    uint32_t clk_fen;      // 模块使能配置
    uint32_t saved_enable; // 中断配置

    __disable_irq();              // 关闭全局中断，确保执行休眠期间，不会进入其他任何中断
    reg5         = SYS_AFE_REG5;  // 保存模拟使能配置
    clkcfg       = SYS_CLK_CFG;   // 保存时钟配置
    clk_fen      = SYS_CLK_FEN;   // 保存模块时钟使能配置
    saved_enable = NVIC->ISER[0]; // 保存当前中断配置

    SYS_WR_PROTECT = 0x7a83;     // 开启寄存器写使能
    NVIC->ICER[0]  = 0xFFFFFFFF; // 关闭所有中断
    NVIC_EnableIRQ(WAKE_IRQn);   // 打开唤醒中断
    {
        int32_t th;
        th = ((uint32_t)(64 * ms + 0x800) & 0x1ff000);
        th = HAL_REG_READ(HAL_IWDG[HAL_IWDG_0]->RTH) - th;
        if (th < 0)
        {
            th = 0;
        }
        HAL_REG_WRITE(HAL_IWDG[HAL_IWDG_0]->WTH, th);
    }
    HAL_IWDG_Feed(HAL_IWDG_0);             // 喂狗
    SYS_CLK_CFG = 0;                       // 切换到HRC时钟
    for (t_cnt = 0; t_cnt < 4000; t_cnt++) // 等待时钟稳定
    {
        __NOP();
    }
    SYS_CLK_FEN    = 0;            // 关闭数字模块时钟
    SYS_AFE_REG5   = BIT10 | BIT8; // 准备关闭 PLL,BGP,以及其它模拟电路使能
    SYS_WR_PROTECT = 0x0;          // 关闭系统寄存器写使能

    AON_IO_WAKE_POL = wake_up_io >> 8;   // 设置IO唤醒电平
    AON_IO_WAKE_EN  = wake_up_io & 0xFF; // 设置IO唤醒引脚

    __enable_irq();         // 打开全局中断
    SCB->SCR |= (1UL << 2); // 使能深度休眠

    __WFI();                    // 休眠 wakeak
    __disable_irq();            // 唤醒后关闭全局中断
    NVIC_DisableIRQ(WAKE_IRQn); // 关闭唤醒中断

    SYS_WR_PROTECT = 0x7a83; // 开启寄存器写使能
    SYS_AFE_REG5   = reg5;   // 开启 PLL,BGP

    for (t_cnt = 0; t_cnt < 400; t_cnt++) // 此时使用hrc，等待>1mS，确保PLL稳定
    {
        __NOP();
    }
    SYS_CLK_CFG    = clkcfg;       // 恢复时钟配置
    NVIC->ISER[0]  = saved_enable; // 恢复中断使能
    SYS_CLK_FEN    = clk_fen;      // 打开其他模块
    SYS_WR_PROTECT = 0x0;          // 打开写保护
    __enable_irq();                // 使能全局中断
}
