#include "../include/hal_lksmcu_crc.h"
#include "lks32mc07x_user_manual.h"
#include "../include/hal_lksmcu_reg.h"
#include "../include/hal_lks32mc07x_reg.h"
// 模块复位
void HAL_CRC_Reset(HAL_CRC_x crc_x)
{
    (void)crc_x;
}
// 模块打开
void HAL_CRC_Enable(HAL_CRC_x crc_x)
{
    (void)crc_x;
}
// 模块关闭
void HAL_CRC_Disable(HAL_CRC_x crc_x)
{
    (void)crc_x;
}
typedef struct
{
    uint32_t polynomial;
    uint32_t initValue;
    uint32_t xorOut;
    uint8_t reflectIn;
    uint8_t reflectOut;
} CRC_Model_Params;

static const CRC_Model_Params crcModels[] = {
    {0x07, 0x00, 0x00, 0, 0},                   // CRC8
    {0x07, 0x00, 0x55, 0, 0},                   // CRC8_ITU
    {0x07, 0xFF, 0x00, 1, 1},                   // CRC8_ROHC
    {0x31, 0x00, 0x00, 1, 1},                   // CRC8_MAXIMx
    {0x8005, 0x0000, 0x0000, 1, 1},             // CRC16_IBMx1
    {0x8005, 0x0000, 0xFFFF, 1, 1},             // CRC16_MAXIM
    {0x8005, 0xFFFF, 0xFFFF, 1, 1},             // CRC16_USBx1
    {0x8005, 0xFFFF, 0x0000, 1, 1},             // CRC16_MODBU
    {0x1021, 0x0000, 0x0000, 1, 1},             // CRC16_CCITT
    {0x1021, 0xFFFF, 0x0000, 0, 0},             // CRC16_CCITT_FALSE
    {0x1021, 0xFFFF, 0xFFFF, 1, 1},             // CRC16_X25
    {0x1021, 0x0000, 0x0000, 0, 0},             // CRC16_XMODEM
    {0x3D65, 0x0000, 0xFFFF, 1, 1},             // CRC16_DNP
    {0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, 1, 1}, // CRC32
    {0x04C11DB7, 0xFFFFFFFF, 0x00000000, 0, 0}, // CRC32_MPEG_2
};

static uint32_t HAL_CRC_Reflect(uint32_t data, int width)
{
    uint32_t result = 0;
    for (int i = 0; i < width; i++)
    {
        if (data & (1 << i))
        {
            result |= 1 << (width - 1 - i);
        }
    }
    return result;
}
uint32_t HAL_CRC_Calc(HAL_CRC_x crc_x, HAL_CRC_MODEL_x model, uint8_t *data, uint32_t len)
{
    (void)crc_x;
    uint32_t crc;
    const CRC_Model_Params *params;

    if (model >= sizeof(crcModels) / sizeof(crcModels[0]))
    {
        return 0; // Invalid model
    }

    params = &crcModels[model];
    crc    = params->initValue;

    for (uint32_t i = 0; i < len; i++)
    {
        uint8_t byte = data[i];
        if (params->reflectIn)
        {
            byte = HAL_CRC_Reflect(byte, 8);
        }

        crc ^= (uint32_t)byte << 24;

        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x80000000)
            {
                crc = (crc << 1) ^ params->polynomial;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    if (params->reflectOut)
    {
        crc = HAL_CRC_Reflect(crc, (params->polynomial >> 24) + 1); // Assuming polynomial width indicates CRC width
    }

    crc ^= params->xorOut;
    return crc;
}
