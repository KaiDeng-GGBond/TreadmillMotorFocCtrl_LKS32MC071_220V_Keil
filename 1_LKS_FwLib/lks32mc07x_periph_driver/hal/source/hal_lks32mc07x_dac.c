#include "../include/hal_lksmcu_dac.h"
#include "../include/hal_lksmcu_trim.h"
#include "lks32mc07x.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"
// 模块复位
void HAL_DAC_Reset(HAL_DAC_x dac_x)
{
    HAL_DAC_Disable(dac_x);
}
// 模块打开
void HAL_DAC_Enable(HAL_DAC_x dac_x)
{
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    switch (dac_x)
    {
        case HAL_DAC_0:
            HAL_REG_SET(SYS_AFE_REG5, BIT12);
            break;
        case HAL_DAC_1:
            HAL_REG_SET(SYS_AFE_REG5, BIT9);
            break;
        default:
            break;
    }
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}
// 模块关闭
void HAL_DAC_Disable(HAL_DAC_x dac_x)
{
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    switch (dac_x)
    {
        case HAL_DAC_0:
            HAL_REG_RESET(SYS_AFE_REG5, BIT12);
            break;
        case HAL_DAC_1:
            HAL_REG_RESET(SYS_AFE_REG5, BIT9);
            break;
        default:
            break;
    }
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}

void HAL_DAC_SetOutVal(HAL_DAC_x dac_x, uint16_t val)
{
    switch (dac_x)
    {
        case HAL_DAC_0:
            HAL_REG_WRITE(SYS_AFE_DAC0, val);
            break;
        case HAL_DAC_1:
            HAL_REG_WRITE(SYS_AFE_DAC1, val);
            break;
        default:
            break;
    }
}
void HAL_DAC_SetOutVoltagemV(HAL_DAC_x dac_x, uint16_t vol_mV)
{
    // 系统寄存器解锁
    int32_t val;
    HAL_REG_SYS_UNLOCK();
    switch (dac_x)
    {
        case HAL_DAC_0:
            if (vol_mV < 1000)
            {
                val = vol_mV;
                val = (val * 27853 + BIT12) >> 13;
                if (val > 4095)
                {
                    val = 4095;
                }
                if (val < 0)
                {
                    val = 0;
                }
                SYS_AFE_DAC0_DC  = HAL_TRIM_Read(0x0000145C);
                SYS_AFE_DAC0_AMC = HAL_TRIM_Read(0x00001458);
                HAL_REG_SET(SYS_AFE_REG1, BIT6);
                HAL_REG_WRITE(SYS_AFE_DAC0, val);
            }
            else
            {
                val = vol_mV;
                val = (val * 27674 + BIT14) >> 15;
                if (val > 4095)
                {
                    val = 4095;
                }
                if (val < 0)
                {
                    val = 0;
                }
                SYS_AFE_DAC0_DC  = HAL_TRIM_Read(0x00001454);
                SYS_AFE_DAC0_AMC = HAL_TRIM_Read(0x00001450);
                HAL_REG_RESET(SYS_AFE_REG1, BIT6);
                HAL_REG_WRITE(SYS_AFE_DAC0, val);
            }
            break;
        case HAL_DAC_1:
            if (vol_mV < 1000)
            {
                val = vol_mV;
                val = (val * 27853 + BIT12) >> 13;
                if (val > 4095)
                {
                    val = 4095;
                }
                if (val < 0)
                {
                    val = 0;
                }
                SYS_AFE_DAC1_DC  = HAL_TRIM_Read(0x0000146C);
                SYS_AFE_DAC1_AMC = HAL_TRIM_Read(0x00001468);
                HAL_REG_SET(SYS_AFE_REG1, BIT7);
                HAL_REG_WRITE(SYS_AFE_DAC1, val);
            }
            else
            {
                val = vol_mV;
                val = (val * 27674 + BIT14) >> 15;
                if (val > 4095)
                {
                    val = 4095;
                }
                if (val < 0)
                {
                    val = 0;
                }
                SYS_AFE_DAC1_DC  = HAL_TRIM_Read(0x00001464);
                SYS_AFE_DAC1_AMC = HAL_TRIM_Read(0x00001460);
                HAL_REG_RESET(SYS_AFE_REG1, BIT7);
                HAL_REG_WRITE(SYS_AFE_DAC1, val);
            }
            break;
        default:
            break;
    }
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}

void HAL_DAC_Out2GpioEnable(HAL_DAC_x dac_x)
{
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    switch (dac_x)
    {
        case HAL_DAC_0:
            HAL_REG_SET(SYS_AFE_REG3, BIT11);
            break;
        case HAL_DAC_1:
            HAL_REG_SET(SYS_AFE_REG3, BIT15);
            break;
        default:
            break;
    }
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}
void HAL_DAC_Out2GpioDisable(HAL_DAC_x dac_x)
{
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();
    switch (dac_x)
    {
        case HAL_DAC_0:
            HAL_REG_RESET(SYS_AFE_REG3, BIT11);
            break;
        case HAL_DAC_1:
            HAL_REG_RESET(SYS_AFE_REG3, BIT15);
            break;
        default:
            break;
    }
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}

/**
 * @brief 设置DAC输出到GPIO
 * @param dac DAC模块
 * @param px GPIO
 */
void HAL_DAC_SetGpioOut(HAL_DAC_x dac, HAL_GPIO_Px px)
{
    (void)dac;

    HAL_GPIO_SetAf(px, HAL_GPIO_AF_DAC);
    HAL_GPIO_SetMode(px, HAL_GPIO_MODE_AFE);
}
