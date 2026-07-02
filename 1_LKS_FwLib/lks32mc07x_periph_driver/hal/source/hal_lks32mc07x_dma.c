#include "../include/hal_lksmcu_dma.h"
#include "../hal_lks32mc07x.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"

// 初始化
// 模块复位
static uint8_t HAL_DMA_ena = 0;
void HAL_DMA_Reset(HAL_DMA_x dma_x)
{
    (void)dma_x;
    // 系统寄存器解锁
    HAL_REG_SYS_UNLOCK();

    HAL_REG_SET(SYS_SFT_RST, SYS_SFT_RST_DMA0_SFT_RST_BIT);
    HAL_REG_RESET(SYS_SFT_RST, SYS_SFT_RST_DMA0_SFT_RST_BIT);
    // 系统寄存器上锁
    HAL_REG_SYS_LOCK();
}
// 模块打开
void HAL_DMA_Enable(HAL_DMA_x dma_x)
{
    (void)dma_x;
    HAL_DMA_ena |= (1 << dma_x);
    HAL_REG_SET(DMA0_CTRL, DMA0_CTRL_EN_BIT);
}
// 模块关闭
void HAL_DMA_Disable(HAL_DMA_x dma_x)
{
    HAL_DMA_ena &= ~(1 << dma_x);
    if (HAL_DMA_ena == 0)
    {
        HAL_REG_RESET(DMA0_CTRL, DMA0_CTRL_EN_BIT);
    }
}
// 内存复制
void HAL_DMA_MemCopy(HAL_DMA_x dma_x, void *start, void *end, uint32_t len)
{
    HAL_DMA[dma_x]->CCR  = 0;
    HAL_DMA[dma_x]->SADR = (uint32_t)(uintptr_t)start;
    HAL_DMA[dma_x]->DADR = (uint32_t)(uintptr_t)end;
    HAL_DMA[dma_x]->CTMS = len;
    HAL_DMA[dma_x]->CCR  = DMA0_CCRx_EN_BIT | DMA0_CCRx_DINC_BIT | DMA0_CCRx_SINC_BIT;
    HAL_DMA[dma_x]->REN  = BIT31;
}
// dma使用uart发送数据,需要先初始化好对应的uart模块
void HAL_DMA_UartTx(HAL_DMA_x dma_x, HAL_UART_x UART_x, uint8_t *start, uint32_t len)
{
    HAL_DMA[dma_x]->CCR  = 0;
    HAL_DMA[dma_x]->SADR = (uint32_t)(uintptr_t)start;
    HAL_DMA[dma_x]->DADR = (uint32_t)(uintptr_t)(&(HAL_UART[UART_x]->BUFF));
    HAL_DMA[dma_x]->CTMS = len;
    HAL_DMA[dma_x]->CCR  = DMA0_CCRx_EN_BIT | BIT1 | DMA0_CCRx_SINC_BIT;
    HAL_DMA[dma_x]->REN  = BIT31 | BIT7;
}
// dma使用spi发送数据,需要先初始化好对应的spi模块
void HAL_DMA_SpiTx(HAL_DMA_x dma_x, HAL_SPI_x SPI_x, uint8_t *start, uint32_t len)
{
    HAL_DMA[dma_x]->CCR  = 0;
    HAL_DMA[dma_x]->SADR = (uint32_t)(uintptr_t)start;
    HAL_DMA[dma_x]->DADR = (uint32_t)(uintptr_t)(&(HAL_SPI[SPI_x]->TXDATA));
    HAL_DMA[dma_x]->CTMS = len;
    HAL_DMA[dma_x]->CCR  = DMA0_CCRx_EN_BIT | BIT1 | DMA0_CCRx_SINC_BIT;
    HAL_DMA[dma_x]->REN  = BIT31 | BIT11;
}
// dma使用i2c发送数据,需要先初始化好对应的i2c模块
void HAL_DMA_I2cTx(HAL_DMA_x dma_x, HAL_I2C_x I2Cx, uint8_t *start, uint32_t len)
{
    HAL_DMA[dma_x]->CCR  = 0;
    HAL_DMA[dma_x]->SADR = (uint32_t)(uintptr_t)start;
    HAL_DMA[dma_x]->DADR = (uint32_t)(uintptr_t)(&(HAL_I2C[I2Cx]->DATA));
    HAL_DMA[dma_x]->CTMS = len;
    HAL_DMA[dma_x]->CCR  = DMA0_CCRx_EN_BIT | BIT1 | DMA0_CCRx_SINC_BIT;
    HAL_DMA[dma_x]->REN  = BIT31 | BIT11;
}
// dma使用uart发送数据,需要先初始化好对应的uart模块
void HAL_DMA_UartRx(HAL_DMA_x dma_x, HAL_UART_x UART_x, uint8_t *start, uint32_t len)
{
    HAL_DMA[dma_x]->CCR  = 0;
    HAL_DMA[dma_x]->SADR = (uint32_t)(uintptr_t)(&(HAL_UART[UART_x]->BUFF));
    HAL_DMA[dma_x]->DADR = (uint32_t)(uintptr_t)start;
    HAL_DMA[dma_x]->CTMS = len;
    HAL_DMA[dma_x]->CCR  = DMA0_CCRx_EN_BIT | DMA0_CCRx_RMODE_BIT | DMA0_CCRx_DINC_BIT;
    HAL_DMA[dma_x]->REN  = BIT6;
}
// dma使用spi发送数据,需要先初始化好对应的spi模块
void HAL_DMA_SpiRx(HAL_DMA_x dma_x, HAL_SPI_x SPI_x, uint8_t *start, uint32_t len)
{
    HAL_DMA[dma_x]->CCR  = 0;
    HAL_DMA[dma_x]->SADR = (uint32_t)(uintptr_t)(&(HAL_SPI[SPI_x]->RXDATA));
    HAL_DMA[dma_x]->DADR = (uint32_t)(uintptr_t)start;
    HAL_DMA[dma_x]->CTMS = len;
    HAL_DMA[dma_x]->CCR  = DMA0_CCRx_EN_BIT | DMA0_CCRx_RMODE_BIT | DMA0_CCRx_DINC_BIT;
    HAL_DMA[dma_x]->REN  = BIT6;
}
// dma使用i2c发送数据,需要先初始化好对应的i2c模块
void HAL_DMA_I2cRx(HAL_DMA_x dma_x, HAL_I2C_x I2Cx, uint8_t *start, uint32_t len)
{
    HAL_DMA[dma_x]->CCR  = 0;
    HAL_DMA[dma_x]->SADR = (uint32_t)(uintptr_t)(&(HAL_I2C[I2Cx]->DATA));
    HAL_DMA[dma_x]->DADR = (uint32_t)(uintptr_t)start;
    HAL_DMA[dma_x]->CTMS = len;
    HAL_DMA[dma_x]->CCR  = DMA0_CCRx_EN_BIT | DMA0_CCRx_RMODE_BIT | DMA0_CCRx_DINC_BIT;
    HAL_DMA[dma_x]->REN  = BIT6;
}
// 使用两个dma模块和一个spi模块,同时发送&接收数据,需要先初始化好对应的spi模块
void HAL_DMA_SpiRxTx(HAL_DMA_x DMA1_x, HAL_DMA_x DMA2_x, HAL_SPI_x SPI_x, uint8_t *txdata, uint8_t *rxdata, uint32_t len)
{
    HAL_DMA[DMA1_x]->CCR  = 0;
    HAL_DMA[DMA1_x]->SADR = (uint32_t)(uintptr_t)(&(HAL_SPI[SPI_x]->RXDATA));
    HAL_DMA[DMA1_x]->DADR = (uint32_t)(uintptr_t)rxdata;
    HAL_DMA[DMA1_x]->CTMS = len;
    HAL_DMA[DMA1_x]->CCR  = DMA0_CCRx_EN_BIT | DMA0_CCRx_RMODE_BIT | DMA0_CCRx_DINC_BIT;
    HAL_DMA[DMA1_x]->REN  = BIT6;

    HAL_DMA[DMA2_x]->CCR  = 0;
    HAL_DMA[DMA2_x]->SADR = (uint32_t)(uintptr_t)txdata;
    HAL_DMA[DMA2_x]->DADR = (uint32_t)(uintptr_t)(&(HAL_SPI[SPI_x]->TXDATA));
    HAL_DMA[DMA2_x]->CTMS = len;
    HAL_DMA[DMA2_x]->CCR  = DMA0_CCRx_EN_BIT | DMA0_CCRx_RMODE_BIT | DMA0_CCRx_SINC_BIT;
    HAL_DMA[DMA2_x]->REN  = BIT6;
}
