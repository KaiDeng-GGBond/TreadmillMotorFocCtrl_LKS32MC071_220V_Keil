#include "Multifunction.h"
#include "sysdef.h"
#include "foc.h"
#include "hall.h"
#include "AdcCalc.h"
#include "TaskScheduler.h"

#define BLOCK_CURRENT 4000u
#define BLOCK_RPM 10u

extern adc_result gAdcObj;
void Motor_Block_Protect(void)
{
    if ((struFOC_CtrProc.MotorStateMachine == RUN) && (struFOC_CtrProc.struFOC_CurrLoop.iq_Reference >= (FOC_CURRENT_Q15_BASE_MA >> 1)))//iq_Reference大于多少判定为堵转，需要调
    {
        if(gStructHallProcess.MechSpeedRpm < BLOCK_RPM)
        {
			if((ABS(gAdcObj.adc_sample_current_ma[0]) > BLOCK_CURRENT) || (ABS(gAdcObj.adc_sample_current_ma[1]) > BLOCK_CURRENT) || (ABS(gAdcObj.adc_sample_current_ma[2]) > BLOCK_CURRENT))
			{
				if(stru_motorcomprehensive.smotorblockcnt < TIME_1S6_BASE)	/* 10ms进一次，进够160次就认可 */
				{
					stru_motorcomprehensive.smotorblockcnt++;
					stru_motorcomprehensive.motorblockflg = 1;	/* 低速且大电流持续足够久，判定堵转，主状态机中停机 */
					stru_motorcomprehensive.motorbeenblockflg = 1;
				}
			}
			else stru_motorcomprehensive.smotorblockcnt = 0;
        }
		else stru_motorcomprehensive.smotorblockcnt = 0;
    }
    else stru_motorcomprehensive.smotorblockcnt = 0;
}

