

#include "../hal_lks32mc07x.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"
#include "../include/hal_lksmcu_irqfun.h"

/**
 * @brief 中断处理函数
 * @param hall 模块编号
 * @return 1表示中断发生，0表示没有中断发生
 */
uint8_t HAL_HALL_IrqChange(HAL_HALL_x hall)
{
    if ((HAL_REG_READ_BIT(HAL_HALL[hall]->CFG, HALL0_CFG_CHG_IE_BIT)) && (HAL_REG_READ_BIT(HAL_HALL[hall]->INFO, HALL0_INFO_CHG_IF_BIT)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief 中断信号（和 HAL_HALL_IrqChange 的区别是不判断中断使能）
 * @param hall 模块编号
 * @return 1表示中断发生，0表示没有中断发生
 */
uint8_t HAL_HALL_IrqChangeFlg(HAL_HALL_x hall)
{
    if (HAL_REG_READ_BIT(HAL_HALL[hall]->INFO, HALL0_INFO_CHG_IF_BIT))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief 中断使能
 * @param hall 模块编号
 */
void HAL_HALL_IrqChangeEnable(HAL_HALL_x hall)
{
    HAL_REG_SET(HAL_HALL[hall]->CFG, HALL0_CFG_CHG_IE_BIT);
}
/**
 * @brief 中断禁止
 * @param hall 模块编号
 */
void HAL_HALL_IrqChangeDisable(HAL_HALL_x hall)
{
    HAL_REG_RESET(HAL_HALL[hall]->CFG, HALL0_CFG_CHG_IE_BIT);
}
/**
 * @brief 中断清除
 * @param hall 模块编号
 */
void HAL_HALL_IrqChangeClear(HAL_HALL_x hall)
{
    HAL_REG_WRITE(HAL_HALL[hall]->INFO, HALL0_INFO_CHG_IF_BIT);
}

/**
 * @brief 中断处理函数
 * @param hall 模块编号
 * @return 1表示中断发生，0表示没有中断发生
 */
uint8_t HAL_HALL_IrqOver(HAL_HALL_x hall)
{
    if ((HAL_REG_READ_BIT(HAL_HALL[hall]->CFG, HALL0_CFG_CHG_IE_BIT)) && (HAL_REG_READ_BIT(HAL_HALL[hall]->INFO, HALL0_INFO_OV_IF_BIT)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief 中断信号（和 HAL_HALL_IrqOver 的区别是不判断中断使能）
 * @param hall 模块编号
 * @return 1表示中断发生，0表示没有中断发生
 */
uint8_t HAL_HALL_IrqOverFlg(HAL_HALL_x hall)
{
    if (HAL_REG_READ_BIT(HAL_HALL[hall]->INFO, HALL0_INFO_OV_IF_BIT))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief 中断使能
 * @param hall 模块编号
 */
void HAL_HALL_IrqOverEnable(HAL_HALL_x hall)
{
    HAL_REG_SET(HAL_HALL[hall]->CFG, HALL0_CFG_CHG_IE_BIT);
}
/**
 * @brief 中断禁止
 * @param hall 模块编号
 */
void HAL_HALL_IrqOverDisable(HAL_HALL_x hall)
{
    HAL_REG_RESET(HAL_HALL[hall]->CFG, HALL0_CFG_CHG_IE_BIT);
}
/**
 * @brief 中断清除
 * @param hall 模块编号
 */
void HAL_HALL_IrqOverClear(HAL_HALL_x hall)
{
    HAL_REG_WRITE(HAL_HALL[hall]->INFO, HALL0_INFO_OV_IF_BIT);
}

/**
 * @brief 中断处理函数
 * @param adc 模块编号
 * @return 1表示中断发生，0表示没有中断发生
 */
uint8_t HAL_ADC_IrqSamp1(HAL_ADC_x adc)
{
    if ((HAL_REG_READ_BIT(HAL_ADC[adc]->IE, ADCx_IE_SF1_IE_BIT)) && (HAL_REG_READ_BIT(HAL_ADC[adc]->IF, ADCx_IF_SF1_IF_BIT)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief 中断信号（和 HAL_ADC_IrqSamp1 的区别是不判断中断使能）
 * @param adc 模块编号
 * @return 1表示中断发生，0表示没有中断发生
 */
uint8_t HAL_ADC_IrqSamp1Flg(HAL_ADC_x adc)
{
    if (HAL_REG_READ_BIT(HAL_ADC[adc]->IF, ADCx_IF_SF1_IF_BIT))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief 中断使能
 * @param adc 模块编号
 */
void HAL_ADC_IrqSamp1Enable(HAL_ADC_x adc)
{
    HAL_REG_SET(HAL_ADC[adc]->IE, ADCx_IE_SF1_IE_BIT);
}
/**
 * @brief 中断禁止
 * @param adc 模块编号
 */
void HAL_ADC_IrqSamp1Disable(HAL_ADC_x adc)
{
    HAL_REG_RESET(HAL_ADC[adc]->IE, ADCx_IE_SF1_IE_BIT);
}
/**
 * @brief 中断清除
 * @param adc 模块编号
 */
void HAL_ADC_IrqSamp1Clear(HAL_ADC_x adc)
{
    HAL_REG_WRITE(HAL_ADC[adc]->IF, ADCx_IF_SF1_IF_BIT);
}

/**
 * @brief 中断处理函数
 * @param adc 模块编号
 * @return 1表示中断发生，0表示没有中断发生
 */
uint8_t HAL_ADC_IrqSamp2(HAL_ADC_x adc)
{
    if ((HAL_REG_READ_BIT(HAL_ADC[adc]->IE, ADCx_IE_SF2_IE_BIT)) && (HAL_REG_READ_BIT(HAL_ADC[adc]->IF, ADCx_IF_SF2_IF_BIT)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief 中断信号（和 HAL_ADC_IrqSamp2 的区别是不判断中断使能）
 * @param adc 模块编号
 * @return 1表示中断发生，0表示没有中断发生
 */
uint8_t HAL_ADC_IrqSamp2Flg(HAL_ADC_x adc)
{
    if (HAL_REG_READ_BIT(HAL_ADC[adc]->IF, ADCx_IF_SF2_IF_BIT))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief 中断使能
 * @param adc 模块编号
 */
void HAL_ADC_IrqSamp2Enable(HAL_ADC_x adc)
{
    HAL_REG_SET(HAL_ADC[adc]->IE, ADCx_IE_SF2_IE_BIT);
}
/**
 * @brief 中断禁止
 * @param adc 模块编号
 */
void HAL_ADC_IrqSamp2Disable(HAL_ADC_x adc)
{
    HAL_REG_RESET(HAL_ADC[adc]->IE, ADCx_IE_SF2_IE_BIT);
}
/**
 * @brief 中断清除
 * @param adc 模块编号
 */
void HAL_ADC_IrqSamp2Clear(HAL_ADC_x adc)
{
    HAL_REG_WRITE(HAL_ADC[adc]->IF, ADCx_IF_SF2_IF_BIT);
}
