#ifndef __HAL_LKSMCU_REG_H__
#define __HAL_LKSMCU_REG_H__

/**
 * @file
 * @brief HAL库配置和内部参数
 */
#define HAL_REG_WRITE(reg, mask)                   reg = (mask)
#define HAL_REG_READ(reg)                          (reg)
#define HAL_REG_WRITE_BIT(reg, mask, val)          reg = ((reg) & ~(mask)) | ((val) & (mask))
#define HAL_REG_WRITE_BIT_MASK_POS(reg, name, val) reg = ((reg) & ~(name##_MASK)) | (name(val))
#define HAL_REG_READ_BIT_POS_MASK(reg, mask)       (((reg) & (mask##_MASK)) >> mask##_POS)
#define HAL_REG_READ_BIT(reg, mask)                ((reg) & (mask))
#define HAL_REG_SET(reg, mask)                     reg |= (mask)
#define HAL_REG_RESET(reg, mask)                   reg &= ~(mask)
#define HAL_REG_INVERT(reg, mask)                  reg = ((reg) & ~(mask)) | ((~((reg) & (mask))) & (mask))
#define HAL_REG_SYS_LOCK()                         SYS_WR_PROTECT = 0
#define HAL_REG_SYS_UNLOCK()                       SYS_WR_PROTECT = 0x7a83
#define HAL_REG_IF_WRITE(reg, mask, condition)     reg = (condition) ? ((reg) | (mask)) : ((reg) & ~(mask))

#endif
