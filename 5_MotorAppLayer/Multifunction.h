#ifndef __MULTIFUNCTION_H__
#define __MULTIFUNCTION_H__

#include "basic.h"
#include "foc.h"

typedef struct
{
	uint8_t  motorblockflg;			/* 电机堵转标志 */ 
	uint8_t  motorbeenblockflg;		/* 电机堵转过标志 */ 
	uint16_t smotorblockcnt;		/* 进入电机堵转时间计数 */ 
} stru_motor_comprehensive;
extern stru_motor_comprehensive stru_motorcomprehensive;

void Motor_Block_Protect(void);

#endif