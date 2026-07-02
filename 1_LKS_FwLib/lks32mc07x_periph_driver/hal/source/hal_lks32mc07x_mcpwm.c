#include "../include/hal_lksmcu_mcpwm.h"
#include "lks32mc07x.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"
#include "../include/hal_lksmcu_clk.h"
/**
 * bit0 通道交换使能 0 交换 1 交换
 * bit1 上管有效电平 0 上管低有效 1 上管高有效
 * bit2 下管有效电平 0 下管低有效 1 下管高有效
 * bit3 默认上管电平 0 关机后上管关闭 1 关机后上管打开
 * bit4 默认下管电平 0 关机后下管关闭 1 关机后下管打开
 * bit5-bit7 输出状态 HAL_MCPWM_OUTPUT_STATE_x
 */
static uint16_t mcpwm_polarity_state[1][6] = {{0, 0, 0, 0, 0, 0}};

/**
 * @brief 内部使用,输出端口的状态
 * @param mcpwm_x 操作的模块编号
 * @param mcpwm_port_x 第x个pwm输出端口
 */
void hal_MCPWM_UpdatePolarityState(HAL_MCPWM_x mcpwm_x, HAL_MCPWM_PORT_x mcpwm_port_x)
{
    (void)mcpwm_x;
    (void)mcpwm_port_x;
    uint8_t tmp[6]  = {0, 0, 0, 0, 0, 0};
    uint8_t tmp1[6] = {0, 0, 0, 0, 0, 0};
    int32_t i;
    uint32_t ch_def = 0;
    uint32_t io01   = 0;
    uint32_t io23   = 0;
    uint32_t io45   = 0;
    HAL_REG_IF_WRITE(io01, BIT6, mcpwm_polarity_state[0][0] & BIT0);  // 打开通道交换
    HAL_REG_IF_WRITE(io01, BIT14, mcpwm_polarity_state[0][1] & BIT0); // 打开通道交换
    HAL_REG_IF_WRITE(io23, BIT6, mcpwm_polarity_state[0][2] & BIT0);  // 打开通道交换
    HAL_REG_IF_WRITE(io23, BIT14, mcpwm_polarity_state[0][3] & BIT0); // 打开通道交换
    HAL_REG_IF_WRITE(io45, BIT6, mcpwm_polarity_state[0][4] & BIT0);  // 打开通道交换
    HAL_REG_IF_WRITE(io45, BIT14, mcpwm_polarity_state[0][5] & BIT0); // 打开通道交换

    HAL_REG_IF_WRITE(io01, BIT1, !(mcpwm_polarity_state[0][0] & BIT1)); // 上管高有效
    HAL_REG_IF_WRITE(io01, BIT9, !(mcpwm_polarity_state[0][1] & BIT1)); // 上管高有效
    HAL_REG_IF_WRITE(io23, BIT1, !(mcpwm_polarity_state[0][2] & BIT1)); // 上管高有效
    HAL_REG_IF_WRITE(io23, BIT9, !(mcpwm_polarity_state[0][3] & BIT1)); // 上管高有效
    HAL_REG_IF_WRITE(io45, BIT1, !(mcpwm_polarity_state[0][4] & BIT1)); // 上管高有效
    HAL_REG_IF_WRITE(io45, BIT9, !(mcpwm_polarity_state[0][5] & BIT1)); // 上管高有效

    HAL_REG_IF_WRITE(io01, BIT0, !(mcpwm_polarity_state[0][0] & BIT2)); // 下管高有效
    HAL_REG_IF_WRITE(io01, BIT8, !(mcpwm_polarity_state[0][1] & BIT2)); // 下管高有效
    HAL_REG_IF_WRITE(io23, BIT0, !(mcpwm_polarity_state[0][2] & BIT2)); // 下管高有效
    HAL_REG_IF_WRITE(io23, BIT8, !(mcpwm_polarity_state[0][3] & BIT2)); // 下管高有效
    HAL_REG_IF_WRITE(io45, BIT0, !(mcpwm_polarity_state[0][4] & BIT2)); // 下管高有效
    HAL_REG_IF_WRITE(io45, BIT8, !(mcpwm_polarity_state[0][5] & BIT2)); // 下管高有效

    for (i = 0; i < 6; i++)
    {
        if (mcpwm_polarity_state[0][i] & BIT4)
        {
            // 关机后上管打开
            if (mcpwm_polarity_state[0][i] & BIT0) // gpio交换使能
            {
                HAL_REG_IF_WRITE(tmp[i], BIT0, (mcpwm_polarity_state[0][i] & BIT1));  // CH0 N 通道默认值
                HAL_REG_IF_WRITE(tmp[i], BIT1, !(mcpwm_polarity_state[0][i] & BIT2)); // CH0 P 通道默认值
            }
            else
            {
                HAL_REG_IF_WRITE(tmp[i], BIT0, !(mcpwm_polarity_state[0][i] & BIT2)); // CH0 N 通道默认值
                HAL_REG_IF_WRITE(tmp[i], BIT1, (mcpwm_polarity_state[0][i] & BIT1));  // CH0 P 通道默认值
            }
        }
        else if (mcpwm_polarity_state[0][i] & BIT3)
        {
            // 关机后下管打开
            if (mcpwm_polarity_state[0][i] & BIT0) // gpio交换使能
            {
                HAL_REG_IF_WRITE(tmp[i], BIT0, !(mcpwm_polarity_state[0][i] & BIT1)); // CH0 N 通道默认值
                HAL_REG_IF_WRITE(tmp[i], BIT1, (mcpwm_polarity_state[0][i] & BIT2));  // CH0 P 通道默认值
            }
            else
            {
                HAL_REG_IF_WRITE(tmp[i], BIT0, (mcpwm_polarity_state[0][i] & BIT2));  // CH0 N 通道默认值
                HAL_REG_IF_WRITE(tmp[i], BIT1, !(mcpwm_polarity_state[0][i] & BIT1)); // CH0 P 通道默认值
            }
        }
        else
        {
            // 关机后gpio悬空
            // 关机后下管打开
            if (mcpwm_polarity_state[0][i] & BIT0) // gpio交换使能
            {
                HAL_REG_IF_WRITE(tmp[i], BIT0, !(mcpwm_polarity_state[0][i] & BIT1)); // CH0 N 通道默认值
                HAL_REG_IF_WRITE(tmp[i], BIT1, !(mcpwm_polarity_state[0][i] & BIT2)); // CH0 P 通道默认值
            }
            else
            {
                HAL_REG_IF_WRITE(tmp[i], BIT0, !(mcpwm_polarity_state[0][i] & BIT2)); // CH0 N 通道默认值
                HAL_REG_IF_WRITE(tmp[i], BIT1, !(mcpwm_polarity_state[0][i] & BIT1)); // CH0 P 通道默认值
            }
        }
    }
    HAL_REG_IF_WRITE(ch_def, BIT0, tmp[0] & BIT0);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT1, tmp[0] & BIT1);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT2, tmp[1] & BIT0);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT3, tmp[1] & BIT1);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT4, tmp[2] & BIT0);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT5, tmp[2] & BIT1);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT6, tmp[3] & BIT0);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT7, tmp[3] & BIT1);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT8, tmp[4] & BIT0);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT9, tmp[4] & BIT1);  // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT10, tmp[5] & BIT0); // 默认输出状态
    HAL_REG_IF_WRITE(ch_def, BIT11, tmp[5] & BIT1); // 默认输出状态

    for (i = 0; i < 6; i++)
    {
        HAL_MCPWM_OUTPUT_STATE_x out_state;
        out_state = (HAL_MCPWM_OUTPUT_STATE_x)(mcpwm_polarity_state[0][i] >> 5);
        switch (out_state)
        {
            case HAL_MCPWM_OUTPUT_STATE_PWM: ///< 上下管对称pwm
                tmp1[i] = 0;
                break;
            case HAL_MCPWM_OUTPUT_STATE_OFF: ///< 关闭输出
                tmp1[i] = BIT2 | BIT3;
                break;
            case HAL_MCPWM_OUTPUT_STATE_LOW: ///< 下管恒开
                tmp1[i] = BIT2 | BIT3 | BIT4;
                break;
            case HAL_MCPWM_OUTPUT_STATE_HIGH: ///< 上管恒开
                tmp1[i] = BIT2 | BIT3 | BIT5;
                break;
            case HAL_MCPWM_OUTPUT_STATE_LOW_PWM: ///< 下管pwm
                tmp1[i] = BIT3;
                break;
            case HAL_MCPWM_OUTPUT_STATE_HIGH_PWM: ///< 上管pwm
                tmp1[i] = BIT2;
                break;
        }
    }
    HAL_REG_WRITE_BIT(io01, 0x003c, tmp1[0]);
    HAL_REG_WRITE_BIT(io01, 0x3c00, tmp1[1] << 8);
    HAL_REG_WRITE_BIT(io23, 0x003c, tmp1[2]);
    HAL_REG_WRITE_BIT(io23, 0x3c00, tmp1[3] << 8);
    HAL_REG_WRITE_BIT(io45, 0x003c, tmp1[4]);
    HAL_REG_WRITE_BIT(io45, 0x3c00, tmp1[5] << 8);

    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_WRITE_BIT(HAL_MCPWM[mcpwm_x]->CH_DEF, 0xff00, ch_def);
    HAL_REG_WRITE_BIT(HAL_MCPWM[mcpwm_x]->IO01, 0x7f7f, io01);
    HAL_REG_WRITE_BIT(HAL_MCPWM[mcpwm_x]->IO23, 0x7f7f, io23);
    HAL_REG_WRITE_BIT(HAL_MCPWM[mcpwm_x]->IO45, 0x7f7f, io45);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}

/**
 * @brief 复位mcpwm模块
 * @param mcpwm_x 操作的模块编号
 */
void HAL_MCPWM_Reset(HAL_MCPWM_x mcpwm_x)
{
    (void)mcpwm_x;
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    HAL_REG_SET(SYS_SFT_RST, SYS_SFT_RST_MCPWM0_SFT_RST_BIT);
    HAL_REG_RESET(SYS_SFT_RST, SYS_SFT_RST_MCPWM0_SFT_RST_BIT);
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}
/**
 * @brief mcpwm模块使能
 * @param mcpwm_x 操作的模块编号
 */
void HAL_MCPWM_Enable(HAL_MCPWM_x mcpwm_x)
{
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    // SYS_CLK_FEN[4] MCPWM 模块整体时钟控制信号
    HAL_REG_SET(SYS_CLK_FEN, SYS_CLK_FEN_MCPWM0_CLK_EN_BIT);
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();

    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_SET(HAL_MCPWM[mcpwm_x]->TCLK, BIT2);
    //    HAL_REG_WRITE_BIT(HAL_MCPWM[mcpwm_x]->AUEN, 0xffff, 0x43f);
    HAL_REG_SET(HAL_MCPWM[mcpwm_x]->SDCFG, BIT4);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
/**
 * @brief mcpwm模块关闭
 * @param mcpwm_x 操作的模块编号
 */
void HAL_MCPWM_Disable(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_RESET(HAL_MCPWM[mcpwm_x]->TCLK, BIT2);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    // SYS_CLK_FEN[4] MCPWM 模块整体时钟控制信号
    HAL_REG_RESET(SYS_CLK_FEN, SYS_CLK_FEN_MCPWM0_CLK_EN_BIT);
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}
/**
 * @brief 设置pwm频率
 * @param mcpwm_x 操作的模块编号
 */
void HAL_MCPWM_SetFreq(HAL_MCPWM_x mcpwm_x, uint32_t freq)
{
    uint32_t mclk = HAL_CLK_GetMcuFreq();
    uint32_t th   = ((mclk / freq) - 1) / 2;
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->TH0, th);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->UPDATE, BIT12);
}
/**
 * @brief 获取pwm频率
 * @param mcpwm_x 操作的模块编号
 * @return 当前pwm的频率
 */
uint32_t HAL_MCPWM_GetFreq(HAL_MCPWM_x mcpwm_x)
{
    uint32_t mclk = HAL_CLK_GetMcuFreq();
    uint16_t th   = HAL_MCPWM[mcpwm_x]->TH0;
    uint32_t freq = mclk / (th * 2 + 1);
    return freq;
}
/**
 * @brief 获取计数器最大值
 * @param mcpwm_x 操作的模块编号
 */
uint32_t HAL_MCPWM_GetThCnt(HAL_MCPWM_x mcpwm_x)
{
    return HAL_MCPWM[mcpwm_x]->TH0;
}
/**
 * @brief 设置死区
 * @param mcpwm_x 操作的模块编号
 * @param pdead_nS 下管关闭上管打开的死区宽度,单位nS
 * @param ndead_nS 上管关闭下管打开的死区宽度,单位nS
 */
void HAL_MCPWM_SetDead(HAL_MCPWM_x mcpwm_x, uint32_t pdead_nS, uint32_t ndead_nS)
{
    uint32_t mclk  = HAL_CLK_GetMcuFreq();
    uint16_t pdead = (int64_t)pdead_nS * mclk / 1000000000;
    uint16_t ndead = (int64_t)ndead_nS * mclk / 1000000000;
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->DTH00, pdead);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->DTH01, ndead);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}

/**
 * @brief 关闭输出
 * @param mcpwm_x 操作的模块编号
 */
void HAL_MCPWM_SetOutputDisable(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_RESET(HAL_MCPWM[mcpwm_x]->FAIL012, BIT6);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
/**
 * @brief 打开输出
 * @param mcpwm_x 操作的模块编号
 */
void HAL_MCPWM_SetOutputEnable(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_SET(HAL_MCPWM[mcpwm_x]->FAIL012, BIT6);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
/**
 * @brief 开始计数
 * @param mcpwm_x 操作的模块编号
 */
void HAL_MCPWM_StartCounting(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_SET(HAL_MCPWM[mcpwm_x]->TCLK, BIT6);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
/**
 * @brief 停止计数(同时计数器回到-th)
 * @param mcpwm_x 操作的模块编号
 */
void HAL_MCPWM_StopCounting(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_RESET(HAL_MCPWM[mcpwm_x]->TCLK, BIT6);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
/**
 * @brief 设置占空比
 * @param mcpwm_x 操作的模块编号
 * @param mcpwm_port_x 第x个pwm输出端口
 * @param t0 pwm打开时刻
 * @param t1 pwm关闭时刻
 */
void HAL_MCPWM_SetPwm(HAL_MCPWM_x mcpwm_x, HAL_MCPWM_PORT_x mcpwm_port_x, int32_t t0, int32_t t1)
{
    // 使用公式写可以在内联的情况下,实现和直接配置寄存器一样的效果
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->TH[mcpwm_port_x * 2], t0);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->TH[mcpwm_port_x * 2 + 1], t1);
}
void HAL_MCPWM_SetPwmQ15(HAL_MCPWM_x mcpwm_x, HAL_MCPWM_PORT_x mcpwm_port_x, int16_t t0, int16_t t1)
{
    uint32_t th  = HAL_MCPWM_GetThCnt(mcpwm_x);
    int32_t cnt0 = (t0 * th) >> 15;
    int32_t cnt1 = (t1 * th) >> 15;
    HAL_MCPWM_SetPwm(mcpwm_x, mcpwm_port_x, cnt0, cnt1);
}
/**
 * @brief 设置输出状态
 * @param mcpwm_x 操作的模块编号
 * @param mcpwm_port_x 第x个pwm输出端口
 * @param mcpwm_output_state_x 输出状态
 */
void HAL_MCPWM_SetOutState(HAL_MCPWM_x mcpwm_x, HAL_MCPWM_PORT_x mcpwm_port_x, HAL_MCPWM_OUTPUT_STATE_x mcpwm_output_state_x)
{
    /**
     * bit0 通道交换使能 0 交换 1 交换
     * bit1 上管有效电平 0 上管低有效 1 上管高有效
     * bit2 下管有效电平 0 下管低有效 1 下管高有效
     * bit3 默认上管电平 0 关机后上管关闭 1 关机后上管打开
     * bit4 默认下管电平 0 关机后下管关闭 1 关机后下管打开
     * bit5-bit7 输出状态 HAL_MCPWM_OUTPUT_STATE_x
     */
    HAL_REG_WRITE_BIT(mcpwm_polarity_state[mcpwm_x][mcpwm_port_x], BIT5 | BIT6 | BIT7, mcpwm_output_state_x << 5);
    hal_MCPWM_UpdatePolarityState(mcpwm_x, mcpwm_port_x);
}

/**
 * @brief 设置pwm极性
 * @param mcpwm_x 操作的模块编号
 * @param mcpwm_polarity_x 第x个pwm输出端口
 */
void HAL_MCPWM_SetDrivePolarity(HAL_MCPWM_x mcpwm_x, HAL_MCPWM_POLARITY_x mcpwm_polarity_x)
{
    /**
     * bit0 通道交换使能 0 交换 1 交换
     * bit1 上管有效电平 0 上管低有效 1 上管高有效
     * bit2 下管有效电平 0 下管低有效 1 下管高有效
     * bit3 默认上管电平 0 关机后上管关闭 1 关机后上管打开
     * bit4 默认下管电平 0 关机后下管关闭 1 关机后下管打开
     * bit5-bit7 输出状态 HAL_MCPWM_OUTPUT_STATE_x
     */
    HAL_REG_WRITE_BIT(mcpwm_polarity_state[mcpwm_x][HAL_MCPWM_PORT_0], BIT1 | BIT2, mcpwm_polarity_x << 1);
    HAL_REG_WRITE_BIT(mcpwm_polarity_state[mcpwm_x][HAL_MCPWM_PORT_1], BIT1 | BIT2, mcpwm_polarity_x << 1);
    HAL_REG_WRITE_BIT(mcpwm_polarity_state[mcpwm_x][HAL_MCPWM_PORT_2], BIT1 | BIT2, mcpwm_polarity_x << 1);
    HAL_REG_WRITE_BIT(mcpwm_polarity_state[mcpwm_x][HAL_MCPWM_PORT_3], BIT1 | BIT2, mcpwm_polarity_x << 1);
    hal_MCPWM_UpdatePolarityState(mcpwm_x, HAL_MCPWM_PORT_0);
    hal_MCPWM_UpdatePolarityState(mcpwm_x, HAL_MCPWM_PORT_1);
    hal_MCPWM_UpdatePolarityState(mcpwm_x, HAL_MCPWM_PORT_2);
    hal_MCPWM_UpdatePolarityState(mcpwm_x, HAL_MCPWM_PORT_3);
}
/**
 * @brief 设置默认状态
 * @param mcpwm_x 操作的模块编号
 * @param mcpwm_default_output_state_x 默认输出状态
 */
void HAL_MCPWM_SetDefaultState(HAL_MCPWM_x mcpwm_x, HAL_MCPWM_DEFAULT_OUTPUT_STATE_x mcpwm_default_output_state_x)
{
    uint32_t val = 0;
    switch (mcpwm_default_output_state_x)
    {
        case HAL_MCPWM_DEFAULT_OUTPUT_STATE_OFF:
            break;
        case HAL_MCPWM_DEFAULT_OUTPUT_STATE_LOW:
            HAL_REG_SET(val, BIT4);
            break;
        case HAL_MCPWM_DEFAULT_OUTPUT_STATE_HIGH:
            HAL_REG_SET(val, BIT3);
            break;
    }
    /**
     * bit0 通道交换使能 0 交换 1 交换
     * bit1 上管有效电平 0 上管低有效 1 上管高有效
     * bit2 下管有效电平 0 下管低有效 1 下管高有效
     * bit3 默认上管电平 0 关机后上管关闭 1 关机后上管打开
     * bit4 默认下管电平 0 关机后下管关闭 1 关机后下管打开
     * bit5-bit7 输出状态 HAL_MCPWM_OUTPUT_STATE_x
     */
    HAL_REG_WRITE_BIT(mcpwm_polarity_state[mcpwm_x][HAL_MCPWM_PORT_0], BIT3 | BIT4, val);
    HAL_REG_WRITE_BIT(mcpwm_polarity_state[mcpwm_x][HAL_MCPWM_PORT_1], BIT3 | BIT4, val);
    HAL_REG_WRITE_BIT(mcpwm_polarity_state[mcpwm_x][HAL_MCPWM_PORT_2], BIT3 | BIT4, val);
    HAL_REG_WRITE_BIT(mcpwm_polarity_state[mcpwm_x][HAL_MCPWM_PORT_3], BIT3 | BIT4, val);
    hal_MCPWM_UpdatePolarityState(mcpwm_x, HAL_MCPWM_PORT_0);
    hal_MCPWM_UpdatePolarityState(mcpwm_x, HAL_MCPWM_PORT_1);
    hal_MCPWM_UpdatePolarityState(mcpwm_x, HAL_MCPWM_PORT_2);
    hal_MCPWM_UpdatePolarityState(mcpwm_x, HAL_MCPWM_PORT_3);
}
/**
 * @brief 设置用于输出的gpio
 * @param mcpwm_x MCPWM模块编号
 * @param px 使用的GPIO口
 */
void HAL_MCPWM_SetGpioOut(HAL_MCPWM_x mcpwm_x, HAL_GPIO_Px px)
{
    (void)mcpwm_x;
    HAL_GPIO_SetAf(px, HAL_GPIO_AF_MCPWM);
    HAL_GPIO_SetMode(px, HAL_GPIO_MODE_OUT);
}
void HAL_MCPWM_SetGpioFail(HAL_MCPWM_x mcpwm_x, HAL_GPIO_Px px) // 设置用于故障输入的gpio
{
    (void)mcpwm_x;
    HAL_GPIO_SetAf(px, HAL_GPIO_AF_MCPWM);
    HAL_GPIO_SetMode(px, HAL_GPIO_MODE_IN);
}

/**
 * @brief 设置FAIL信号来源
 * @param mcpwm_x 操作的模块编号
 * @param fail_x FAIL信号来源
 */
void HAL_MCPWM_SetFailSignal(HAL_MCPWM_x mcpwm_x, HAL_MCPWM_FAIL_x fail_x)
{
    (void)mcpwm_x;
    (void)fail_x;
}
// 设置pwm输出ADC采样时间
void HAL_MCPWM_SetAdcTrig(HAL_MCPWM_x mcpwm_x, int32_t t0, int32_t t1)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->TMR0, t0);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->TMR1, t1);
}
// 设置pwm输出ADC采样时间q15格式输入
void HAL_MCPWM_SetAdcTrigQ15(HAL_MCPWM_x mcpwm_x, int32_t t0, int32_t t1)
{
    uint32_t th  = HAL_MCPWM_GetThCnt(mcpwm_x);
    int16_t tmr0 = ((int32_t)t0 * th) >> 15;
    int16_t tmr1 = ((int32_t)t1 * th) >> 15;
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->TMR0, tmr0);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->TMR1, tmr1);
}

uint8_t HAL_MCPWM_IrqOverflow(HAL_MCPWM_x mcpwm_x)
{
    if (HAL_MCPWM[mcpwm_x]->IE0 & HAL_MCPWM[mcpwm_x]->IF0 & BIT0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_MCPWM_IrqOverflowEnable(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_SET(HAL_MCPWM[mcpwm_x]->IE0, BIT0);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
void HAL_MCPWM_IrqOverflowDisable(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_RESET(HAL_MCPWM[mcpwm_x]->IE0, BIT0);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
void HAL_MCPWM_IrqOverflowClear(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->IF0, BIT0);
}

uint8_t HAL_MCPWM_IrqZeroCross(HAL_MCPWM_x mcpwm_x)
{
    if (HAL_MCPWM[mcpwm_x]->IE0 & HAL_MCPWM[mcpwm_x]->IF0 & BIT1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_MCPWM_IrqZeroCrossEnable(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_SET(HAL_MCPWM[mcpwm_x]->IE0, BIT1);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
void HAL_MCPWM_IrqZeroCrossDisable(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_RESET(HAL_MCPWM[mcpwm_x]->IE0, BIT1);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
void HAL_MCPWM_IrqZeroCrossClear(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->IF0, BIT1);
}

uint8_t HAL_MCPWM_IrqFail(HAL_MCPWM_x mcpwm_x)
{
    if (HAL_MCPWM[mcpwm_x]->EIE & HAL_MCPWM[mcpwm_x]->EIF & (BIT4 | BIT5))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_MCPWM_IrqFailEnable(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_SET(HAL_MCPWM[mcpwm_x]->IE0, BIT4 | BIT5);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
void HAL_MCPWM_IrqFailDisable(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0xdead);
    HAL_REG_RESET(HAL_MCPWM[mcpwm_x]->IE0, BIT4 | BIT5);
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->PRT, 0);
}
void HAL_MCPWM_IrqFailClear(HAL_MCPWM_x mcpwm_x)
{
    HAL_REG_WRITE(HAL_MCPWM[mcpwm_x]->IF0, BIT4 | BIT5);
}
