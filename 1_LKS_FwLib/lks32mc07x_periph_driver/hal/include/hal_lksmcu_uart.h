
#ifndef __HAL_LKSMCU_UART_H__
#define __HAL_LKSMCU_UART_H__
#include "stdint.h"
#include "../include/hal_lksmcu_gpio.h"
typedef enum
{
    HAL_UART_0,
    HAL_UART_1,
    HAL_UART_2,
} HAL_UART_x;
typedef enum
{
    HAL_UART_MODE_LSB_STOP1_NONE, ///< 低位先发 1位停止位 无校验
    HAL_UART_MODE_LSB_STOP2_NONE, ///< 低位先发 2位停止位 无校验
    HAL_UART_MODE_LSB_STOP1_ODD,  ///< 低位先发 1位停止位 奇校验
    HAL_UART_MODE_LSB_STOP2_ODD,  ///< 低位先发 2位停止位 奇校验
    HAL_UART_MODE_LSB_STOP1_EVEN, ///< 低位先发 1位停止位 偶校验
    HAL_UART_MODE_LSB_STOP2_EVEN, ///< 低位先发 2位停止位 偶校验
    HAL_UART_MODE_MSB_STOP1_NONE, ///< 高位先发 1位停止位 无校验
    HAL_UART_MODE_MSB_STOP2_NONE, ///< 高位先发 2位停止位 无校验
    HAL_UART_MODE_MSB_STOP1_ODD,  ///< 高位先发 1位停止位 奇校验
    HAL_UART_MODE_MSB_STOP2_ODD,  ///< 高位先发 2位停止位 奇校验
    HAL_UART_MODE_MSB_STOP1_EVEN, ///< 高位先发 1位停止位 偶校验
    HAL_UART_MODE_MSB_STOP2_EVEN, ///< 高位先发 2位停止位 偶校验
} HAL_UART_MODE_x;
// 初始化
void HAL_UART_Reset(HAL_UART_x);   // 模块复位
void HAL_UART_Enable(HAL_UART_x);  // 模块打开
void HAL_UART_Disable(HAL_UART_x); // 模块关闭
//
void HAL_UART_SetBaud(HAL_UART_x, uint32_t baud);
void HAL_UART_TxData(HAL_UART_x, uint8_t *data, uint32_t len);
void HAL_UART_SetMode(HAL_UART_x, HAL_UART_MODE_x);
uint8_t HAL_UART_ReadBuffer(HAL_UART_x);
void HAL_UART_WriteBuffer(HAL_UART_x, uint8_t buff);
void HAL_UART_SetGpioTx(HAL_UART_x, HAL_GPIO_Px); // 设置用于输出的gpio
void HAL_UART_SetGpioRx(HAL_UART_x, HAL_GPIO_Px); // 设置用于输出的gpio

// 中断
// 仅支持发送完成、接受完成、停止位错误、校验错误
//
// 以下功能不提供接口，如果需要使用，可以使用标准库实现
// 发送缓冲区空
// 发送缓冲区溢出
// 接受缓冲区溢出
// LIN相关

// uart发送完成中断
uint8_t HAL_UART_IrqSendOver(HAL_UART_x);
void HAL_UART_IrqSendOverEnable(HAL_UART_x);
void HAL_UART_IrqSendOverDisable(HAL_UART_x);
void HAL_UART_IrqSendOverClear(HAL_UART_x);

// uart接受完成中断
uint8_t HAL_UART_IrqReceiveOver(HAL_UART_x);
void HAL_UART_IrqReceiveOverEnable(HAL_UART_x);
void HAL_UART_IrqReceiveOverDisable(HAL_UART_x);
void HAL_UART_IrqReceiveOverClear(HAL_UART_x);

// uart停止位错误中断
uint8_t HAL_UART_IrqStopErr(HAL_UART_x);
void HAL_UART_IrqStopErrEnable(HAL_UART_x);
void HAL_UART_IrqStopErrDisable(HAL_UART_x);
void HAL_UART_IrqStopErrClear(HAL_UART_x);

// uart校验错误中断
uint8_t HAL_UART_IrqCheckErr(HAL_UART_x);
void HAL_UART_IrqCheckErrEnable(HAL_UART_x);
void HAL_UART_IrqCheckErrDisable(HAL_UART_x);
void HAL_UART_IrqCheckErrClear(HAL_UART_x);
#endif
