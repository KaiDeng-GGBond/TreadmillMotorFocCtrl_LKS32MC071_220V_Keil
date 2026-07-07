#include "Multifunction.h"
#include "sysdef.h"
#include "foc.h"
#include "hall.h"
#include "AdcCalc.h"
#include "TaskScheduler.h"

#define BLOCK_CURRENT 4000u
#define BLOCK_RPM 10u


static inline void BlockCnt_Dec(void)
{
    if(stru_motorcomprehensive.smotorblockcnt > 0)
    {
        stru_motorcomprehensive.smotorblockcnt--;
    }
}

extern adc_result gAdcObj;
extern uint16_t g_abs_current_max;
void Motor_Block_Protect(void)
{
	if (struFOC_CtrProc.MotorStateMachine != RUN)
	{
		BlockCnt_Dec();
		return;
	}

	if(gStructHallProcess.MechSpeedRpm >= BLOCK_RPM)
	{
		BlockCnt_Dec();
		return;
	}

	if(g_abs_current_max <= BLOCK_CURRENT)
	{
		BlockCnt_Dec();
		return;
	}

	if(stru_motorcomprehensive.smotorblockcnt < TIME_1S6_BASE)
	{
		stru_motorcomprehensive.smotorblockcnt++;
	}

	if(stru_motorcomprehensive.smotorblockcnt >= TIME_1S6_BASE)
	{
		stru_motorcomprehensive.motorblockflg = 1;
		stru_motorcomprehensive.motorbeenblockflg = 1;
	}
}

