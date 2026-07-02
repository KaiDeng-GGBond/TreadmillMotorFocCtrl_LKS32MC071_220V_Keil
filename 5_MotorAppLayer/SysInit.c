#include "SysInit.h"
#include "Hall.h"
#include "hardware_init.h"

/* 初始化系统 */
void SysInit(void)
{
	/* FOC参数初始化 */
	FOC_InitstruParama();
	
	/* 计算PGA的差分输出零漂电压,OPA放大倍数原厂校准值 */
	OPAOut_OffsetCalibration();
	
	/* 电流计算的系数初始化 */
	currentCalcOffsetInit();
	
	/* hall错误状态清除 */
	gStructHallProcess.FaultState = 0;
}