#include "../include/hal_lksmcu_clk.h"
#include "../include/hal_lksmcu_uart.h"
#include "../hal_lks32mc07x.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"
#include "stdint.h"

// 初始化
// 模块复位
void HAL_UART_Reset(HAL_UART_x UART_x)
{
    switch (UART_x)
    {
    case HAL_UART_0:

        // 系统寄存器解锁
        HAL_REG_SYS_UNLOCK();
        HAL_REG_SET(SYS_SFT_RST, SYS_SFT_RST_UART0_SFT_RST_BIT);
        HAL_REG_RESET(SYS_SFT_RST, SYS_SFT_RST_UART0_SFT_RST_BIT);
        // 系统寄存器上锁
        HAL_REG_SYS_LOCK();
        break;
    case HAL_UART_1:

        // 系统寄存器解锁
        HAL_REG_SYS_UNLOCK();
        HAL_REG_SET(SYS_SFT_RST, SYS_SFT_RST_UART1_SFT_RST_BIT);
        HAL_REG_RESET(SYS_SFT_RST, SYS_SFT_RST_UART1_SFT_RST_BIT);
        // 系统寄存器上锁
        HAL_REG_SYS_LOCK();
        break;
    default:
        break;
    }
}
// 模块打开
void HAL_UART_Enable(HAL_UART_x UART_x)
{
    switch (UART_x)
    {
    case HAL_UART_0:

        // 系统寄存器解锁
        HAL_REG_SYS_UNLOCK();
        HAL_REG_SET(SYS_CLK_FEN, SYS_CLK_FEN_UART0_CLK_EN_BIT);
        // 系统寄存器上锁
        HAL_REG_SYS_LOCK();
        break;
    case HAL_UART_1:

        // 系统寄存器解锁
        HAL_REG_SYS_UNLOCK();
        HAL_REG_SET(SYS_CLK_FEN, SYS_CLK_FEN_UART1_CLK_EN_BIT);
        // 系统寄存器上锁
        HAL_REG_SYS_LOCK();
        break;
    default:
        break;
    }
}
// 模块关闭
void HAL_UART_Disable(HAL_UART_x UART_x)
{
    switch (UART_x)
    {
    case HAL_UART_0:

        // 系统寄存器解锁
        HAL_REG_SYS_UNLOCK();
        HAL_REG_RESET(SYS_CLK_FEN, SYS_CLK_FEN_UART0_CLK_EN_BIT);
        // 系统寄存器上锁
        HAL_REG_SYS_LOCK();
        break;
    case HAL_UART_1:

        // 系统寄存器解锁
        HAL_REG_SYS_UNLOCK();
        HAL_REG_RESET(SYS_CLK_FEN, SYS_CLK_FEN_UART1_CLK_EN_BIT);
        // 系统寄存器上锁
        HAL_REG_SYS_LOCK();
        break;
    default:
        break;
    }
}
void HAL_UART_SetBaud(HAL_UART_x UART_x, uint32_t baud)
{
    switch (UART_x)
    {
    case HAL_UART_0:
    {
        uint32_t mclk = HAL_CLK_GetMcuFreq();
        uint32_t div  = mclk / baud;
        uint32_t div2 = 0;
        while (div & 0xffff0000)
        {
            div2++;
            div >>= 1;
        }
        HAL_REG_SYS_UNLOCK();
        HAL_REG_WRITE(SYS_CLK_DIV2, div2);
        HAL_REG_SYS_LOCK();
        HAL_REG_WRITE(HAL_UART[UART_x]->DIVH, div >> 8);
        HAL_REG_WRITE(HAL_UART[UART_x]->DIVL, div & 0xff);
    }
    default:
        break;
    }
}
void HAL_UART_TxData(HAL_UART_x UART_x, uint8_t *data, uint32_t len)
{
    for (; len > 0; len--)
    {
        HAL_REG_WRITE(HAL_UART[UART_x]->BUFF, *data);
        data++;
        while (HAL_REG_READ_BIT(HAL_UART[UART_x]->STT, BIT0) == 0)
        {
        }
    }
}

void HAL_UART_SetMode(HAL_UART_x uart_x, HAL_UART_MODE_x mode_x)
{

    uint32_t mask = (UARTx_CTRL_CK_EN_BIT |
                     UARTx_CTRL_CK_TYPE_BIT |
                     UARTx_CTRL_BIT_ORDER_BIT |
                     UARTx_CTRL_STOP_LEN_BIT |
                     UARTx_CTRL_BYTE_LEN_BIT);
    switch (mode_x)
    {
    case HAL_UART_MODE_LSB_STOP1_NONE: ///< 低位先发 1位停止位 无校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, 0);
        break;
    }
    case HAL_UART_MODE_LSB_STOP2_NONE: ///< 低位先发 2位停止位 无校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_STOP_LEN_BIT);
        break;
    }
    case HAL_UART_MODE_LSB_STOP1_ODD: ///< 低位先发 1位停止位 奇校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_CK_EN_BIT | UARTx_CTRL_CK_TYPE_BIT | UARTx_CTRL_BYTE_LEN_BIT);
        break;
    }
    case HAL_UART_MODE_LSB_STOP2_ODD: ///< 低位先发 2位停止位 奇校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_CK_EN_BIT | UARTx_CTRL_CK_TYPE_BIT | UARTx_CTRL_STOP_LEN_BIT | UARTx_CTRL_BYTE_LEN_BIT);
        break;
    }
    case HAL_UART_MODE_LSB_STOP1_EVEN: ///< 低位先发 1位停止位 偶校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_CK_EN_BIT | UARTx_CTRL_BYTE_LEN_BIT);
        break;
    }
    case HAL_UART_MODE_LSB_STOP2_EVEN: ///< 低位先发 2位停止位 偶校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_CK_EN_BIT | UARTx_CTRL_STOP_LEN_BIT | UARTx_CTRL_BYTE_LEN_BIT);
        break;
    }
    case HAL_UART_MODE_MSB_STOP1_NONE: ///< 高位先发 1位停止位 无校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_BIT_ORDER_BIT);
        break;
    }
    case HAL_UART_MODE_MSB_STOP2_NONE: ///< 高位先发 2位停止位 无校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_BIT_ORDER_BIT | UARTx_CTRL_STOP_LEN_BIT);
        break;
    }
    case HAL_UART_MODE_MSB_STOP1_ODD: ///< 高位先发 1位停止位 奇校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_CK_EN_BIT | UARTx_CTRL_CK_TYPE_BIT | UARTx_CTRL_BIT_ORDER_BIT | UARTx_CTRL_BYTE_LEN_BIT);
        break;
    }
    case HAL_UART_MODE_MSB_STOP2_ODD: ///< 高位先发 2位停止位 奇校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_CK_EN_BIT | UARTx_CTRL_CK_TYPE_BIT | UARTx_CTRL_BIT_ORDER_BIT | UARTx_CTRL_STOP_LEN_BIT | UARTx_CTRL_BYTE_LEN_BIT);
        break;
    }
    case HAL_UART_MODE_MSB_STOP1_EVEN: ///< 高位先发 1位停止位 偶校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_CK_EN_BIT | UARTx_CTRL_BIT_ORDER_BIT | UARTx_CTRL_BYTE_LEN_BIT);
        break;
    }
    case HAL_UART_MODE_MSB_STOP2_EVEN: ///< 高位先发 2位停止位 偶校验
    {
        HAL_REG_WRITE_BIT(HAL_UART[uart_x]->CTRL, mask, UARTx_CTRL_CK_EN_BIT | UARTx_CTRL_BIT_ORDER_BIT | UARTx_CTRL_STOP_LEN_BIT | UARTx_CTRL_BYTE_LEN_BIT);
        break;
    }
    }
}
uint8_t HAL_UART_ReadBuffer(HAL_UART_x uart_x)
{
    return HAL_REG_READ(HAL_UART[uart_x]->BUFF);
}
void HAL_UART_WriteBuffer(HAL_UART_x uart_x, uint8_t buff)
{
    HAL_REG_WRITE(HAL_UART[uart_x]->BUFF, buff);
}
void HAL_UART_SetGpioTx(HAL_UART_x uart_x, HAL_GPIO_Px px)
{
    (void)uart_x;
    HAL_GPIO_SetAf(px, HAL_GPIO_AF_UART);
    HAL_GPIO_SetMode(px, HAL_GPIO_MODE_OUT);
}
void HAL_UART_SetGpioRx(HAL_UART_x uart_x, HAL_GPIO_Px px)
{
    (void)uart_x;
    HAL_GPIO_SetAf(px, HAL_GPIO_AF_UART);
    HAL_GPIO_SetMode(px, HAL_GPIO_MODE_IN);
}

// uart发送完成中断
uint8_t HAL_UART_IrqSendOver(HAL_UART_x uart_x)
{
    if (HAL_UART[uart_x]->IE & HAL_UART[uart_x]->IF & BIT0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_UART_IrqSendOverEnable(HAL_UART_x uart_x)
{
    HAL_REG_SET(HAL_UART[uart_x]->IE, BIT0);
}
void HAL_UART_IrqSendOverDisable(HAL_UART_x uart_x)
{
    HAL_REG_RESET(HAL_UART[uart_x]->IE, BIT0);
}
void HAL_UART_IrqSendOverClear(HAL_UART_x uart_x)
{
    HAL_REG_WRITE(HAL_UART[uart_x]->IF, BIT0);
}

// uart接受完成中断
uint8_t HAL_UART_IrqReceiveOver(HAL_UART_x uart_x)
{
    if (HAL_UART[uart_x]->IE & HAL_UART[uart_x]->IF & UARTx_IF_RX_DONE_BIT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_UART_IrqReceiveOverEnable(HAL_UART_x uart_x)
{
    HAL_REG_SET(HAL_UART[uart_x]->IE, UARTx_IE_RX_DONE_BIT);
}
void HAL_UART_IrqReceiveOverDisable(HAL_UART_x uart_x)
{
    HAL_REG_RESET(HAL_UART[uart_x]->IE, UARTx_IE_RX_DONE_BIT);
}
void HAL_UART_IrqReceiveOverClear(HAL_UART_x uart_x)
{
    HAL_REG_WRITE(HAL_UART[uart_x]->IF, UARTx_IF_RX_DONE_BIT);
}

// uart停止位错误中断
uint8_t HAL_UART_IrqStopErr(HAL_UART_x uart_x)
{
    if (HAL_UART[uart_x]->IE & HAL_UART[uart_x]->IF & UARTx_IF_STOP_ERR_BIT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_UART_IrqStopErrEnable(HAL_UART_x uart_x)
{
    HAL_REG_SET(HAL_UART[uart_x]->IE, UARTx_IE_STOP_ERR_BIT);
}
void HAL_UART_IrqStopErrDisable(HAL_UART_x uart_x)
{
    HAL_REG_RESET(HAL_UART[uart_x]->IE, UARTx_IE_STOP_ERR_BIT);
}
void HAL_UART_IrqStopErrClear(HAL_UART_x uart_x)
{
    HAL_REG_WRITE(HAL_UART[uart_x]->IF, UARTx_IF_STOP_ERR_BIT);
}

// uart校验错误中断
uint8_t HAL_UART_IrqCheckErr(HAL_UART_x uart_x)
{
    if (HAL_UART[uart_x]->IE & HAL_UART[uart_x]->IF & BIT4)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void HAL_UART_IrqCheckErrEnable(HAL_UART_x uart_x)
{
    HAL_REG_SET(HAL_UART[uart_x]->IE, BIT4);
}
void HAL_UART_IrqCheckErrDisable(HAL_UART_x uart_x)
{
    HAL_REG_RESET(HAL_UART[uart_x]->IE, BIT4);
}
void HAL_UART_IrqCheckErrClear(HAL_UART_x uart_x)
{
    HAL_REG_WRITE(HAL_UART[uart_x]->IF, BIT4);
}
