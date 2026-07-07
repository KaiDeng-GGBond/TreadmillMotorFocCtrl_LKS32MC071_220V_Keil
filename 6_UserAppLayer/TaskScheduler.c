#include "TaskScheduler.h"
#include "hardware_init.h"
#include "lks32mc07x_hall.h"
#include "lks32mc07x_uart.h"
#include "lks32mc07x_mcpwm.h"
#include "led.h"
#include "uart.h"
#include "vofa.h"
#include "AdcCalc.h"
#include "lks32mc07x_dma.h"
#include "hall.h"
#include "foc.h"
#include "Speedloop.h"
#include "Multifunction.h"
#include "Sysinit.h"

/* 基于时基500us */
#define TASK_SCHEDU_1MS		(2)			/* 任务调度1ms计数时长 */
#define TASK_SCHEDU_10MS	(20)		/* 任务调度10ms计数时长 */
#define TASK_SCHEDU_50MS	(100)		/* 任务调度50ms计数时长 */
#define TASK_SCHEDU_500MS	(1000)		/* 任务调度500ms计数时长 */

struct_task_scheduler g_task_scheduler = {0};
extern float ntc_temp;
extern volatile int16_t adcRawValue[5];
extern strHallProcessObj gStructHallProcess;
extern volatile int32_t speed_loop_integral_acc;
extern stru_RC_Def struCurrU_RC;	/* U相相电流RC滤波结构体 */	//要初始化
extern stru_RC_Def struCurrV_RC;	/* V相相电流RC滤波结构体 */	//要初始化
extern stru_RC_Def struCurrW_RC;	/* W相相电流RC滤波结构体 */	//要初始化

#if IF_OPENLOOP
extern uint8_t IFStartOpenLoopFlag;
extern uint8_t gTrig7;
#endif

#if CERRENT_LOOP_BAND_WIDTH_TEST
uint8_t gTrig8;
#endif

static void Sys_State_Machine(void)
{
	switch (struFOC_CtrProc.MotorStateMachine)
	{
	case IDLE:
	{
		/* 空闲态下允许重新发启动命令，但必须满足：没有过流过压欠压 */
#if !LOW_VOLT_DEBUG
		if((struFOC_CtrProc.MotorRunFlag == 1) && (gAdcObj.bus_over_volt_flag == 0) && \
		(gAdcObj.phase_over_current_flag == 0) && (gAdcObj.bus_under_volt_flag == 0) && (gAdcObj.KM_connect_flag == 1))
		{
			#if IF_OPENLOOP
			if(IFStartOpenLoopFlag == 1)//开环强拖要保证角度在跑才能开波
			{
				struFOC_CtrProc.MotorStateMachine = INIT;
			}
			#endif
			struFOC_CtrProc.MotorStateMachine = INIT;
		}
#else
		if((struFOC_CtrProc.MotorRunFlag == 1) && (gAdcObj.bus_over_volt_flag == 0) && \
		(gAdcObj.phase_over_current_flag == 0) && (gAdcObj.bus_under_volt_flag == 0))
		{
			struFOC_CtrProc.MotorStateMachine = INIT;
		}
#endif
		break;
	}
	case INIT:
	{
		/* 在这里清零积分项等,目的是让下次启动从干净状态进入 */
		struFOC_CtrProc.ref_rpm = 0;									//速度参考值清零
		struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.wIntegral = 0;	//q轴电流环累计积分清零
		struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.wIntegral = 0;		//d轴电流环累计积分清零
		speed_loop_integral_acc = 0;									//速度环积分项清零
		
		struFOC_CtrProc.struFOC_CurrLoop.firstOpenMosFlgCnt = 0;//清零该计数器，保证刚开始进入foc函数时的前几拍，能稳定
		
		LowPass_Filter_Init(&struFOC_CtrProc);//低通滤波器初始化,输出清零
				
		struFOC_CtrProc.struFOC_CurrLoop.iq_Reference = 0;//400;//300;//60至100是开环强拖的合适的值//300;//给定初始iq
		struFOC_CtrProc.struFOC_CurrLoop.id_Reference = 0;//100;//60至100是开环强拖的合适的值//300;//给定初始id

		__disable_irq();//关中断，运行1拍FOC
		
		FOC_Model(&struFOC_CtrProc);//强制跑一次FOC,算出SVPWM的占空比
		
		MCPWM_UPDATE_REG();
		
		__enable_irq();
		
		PWMOutputs(ENABLE);//打开PWM输出

		/* 等待两拍PWM */
		g_task_scheduler.pwm_update_cnt = 0;
		while (g_task_scheduler.pwm_update_cnt < 2)
		{
			__nop();
		}

		struFOC_CtrProc.MotorStateMachine = RUN;
		break;
	}
	case RUN:
	{
		/* 监视是否需要退回停机流程过流、过压、欠压、堵转、过温（待补充） */
		/* 堵转也可以：堵转了，慢慢把目标转速降下来（还没做，现在是直接STOP） */
		if((struFOC_CtrProc.MotorRunFlag == 0) ||\
		   (gAdcObj.bus_over_volt_flag == 1) ||\
		   (gAdcObj.phase_over_current_flag == 1)||\
		   (stru_motorcomprehensive.motorblockflg == 1) ||\
		   (gAdcObj.bus_under_volt_flag == 1) ||\
		   (gStructHallProcess.FaultState != 0))
		{
			PWMOutputs(DISABLE);
			struFOC_CtrProc.MotorStateMachine = STOP;
		}
		
#if 0
		if(gStructHallProcess.Direction == CCW)//HALL检测到反转，马上停机关波，防止人伤
		{
			PWMOutputs(DISABLE);
			struFOC_CtrProc.MotorStateMachine = STOP;
		}
#endif
		
		break;
	}
	case STOP:
	{
		/* 目标速度置0 */
		struFOC_CtrProc.target_rpm = 0;
		struFOC_CtrProc.MotorRunFlag = 0;
		
		/* STOP过程不关波，WAIT状态下才关波 */
		/* 满足500次速度=0及目标速度=0，则置为等待状态。该函数1ms进1次 */
		if((gStructHallProcess.MechSpeedRpm == 0) && (struFOC_CtrProc.target_rpm == 0))
		{
			static uint16_t zeroMechSpeedRpmCounts= 0;
			zeroMechSpeedRpmCounts++;
			if(zeroMechSpeedRpmCounts > 500)
			{
				zeroMechSpeedRpmCounts = 0;
				struFOC_CtrProc.MotorStateMachine = WAIT;//检测到500次速度值为0后，就置为IDLE状态
			}
		}
		break;
	}
	case WAIT:
	{
		/* WAIT状态只允许从STOP状态进来 */
		/* 在这里清零积分项等,目的是让下次启动从干净状态进入 */
		struFOC_CtrProc.target_rpm = 0;
		struFOC_CtrProc.ref_rpm = 0;
		struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.wIntegral = 0;
		struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.wIntegral = 0;
		speed_loop_integral_acc = 0;
		
		/* 电机堵转应该是能恢复的，所以在这里清除标志位 */
		stru_motorcomprehensive.motorblockflg = 0;
		stru_motorcomprehensive.motorbeenblockflg = 0;
					
		/* 关波 */
		PWMOutputs(DISABLE);
	
		/* 转为IDLE状态 */
		struFOC_CtrProc.MotorStateMachine = IDLE;
		break;
	}
    default:
	{
		struFOC_CtrProc.MotorStateMachine = STOP;
        break;
	}
	}
}
/**************************************************** 定时任务函数 ************************************************************/
static void task_1ms(void)
{
	/* A. 电机状态机 */
	Sys_State_Machine();

	/* B. 记录运行时间 */
	g_task_scheduler.run_time_ms++;
	
	/* C. 发调试量 */
	vofa_pack_and_send();
}

static void task_10ms(void)
{
#if SPEED_LOOP_ENABLE
	/* A. 转速环，梯形加减速，平滑改变struFOC_CtrProc.ref_rpm */
	speed_ref_ramp(&struFOC_CtrProc);
	SpeedLoopCallback(&struFOC_CtrProc);
#endif

	/* B. 母线电压检测，以及预充过压欠压判定。 */
	TableCalcBusVolt();

	/* C. 堵转保护 */
	Motor_Block_Protect();
}

static void task_50ms(void)
{

}

static void task_500ms(void)
{
	/* A.闪灯 */
	led_blink();

	/* B.电机停止时（保证在关波下）采集偏置 */
	if(struFOC_CtrProc.MotorStateMachine == IDLE)
	{
		OPAOut_OffsetCalibration();
		currentCalcOffsetInit();
	}

#if 1
	/* C.温度计算 */
	adcRawValue[4] = ADC_GetConversionValue(ADC0, DAT4);/* NTC -> P2.7 -> ADC0_CH11(ADC0 通道 11) */
	ntc_temp = ntc_temperature_calc(((int32_t)adcRawValue[4] * 3600 / ADC_RANGE));//ntc_temperature_calc()的传参是mv的电压值
#endif

#if IF_OPENLOOP
	/* IF强拖后发送多组角度学习数据 */
    if(gTrig7)
    {
        gTrig7 = 0;  // 清除标志
        
        // 选择一种发送方式：
        uart_send_hall_learning_data();      // 详细文本格式
        // uart_send_hall_learning_data_csv();  // CSV格式
        // uart_send_hall_learning_data_binary(); // 二进制格式
        // uart_send_hall_learning_data_compact(); // 紧凑格式
    }
#endif
	
#if CERRENT_LOOP_BAND_WIDTH_TEST
	/* 记录完电流上升时间后发送数据 */
    if(gTrig8)
    {
        gTrig8 = 0;
        uart_send_bw_test_data_csv();
    }
#endif
}

/****************************************************** 对外函数 **************************************************************/

void task_scheduler(void)
{
	if(g_task_scheduler.time_base_flag_500us)
	{
		g_task_scheduler.time_base_flag_500us = 0;

		if (g_task_scheduler.cnt_1 >= TASK_SCHEDU_1MS)
		{
			/* 1毫秒事件，任务调度 */
			g_task_scheduler.cnt_1 = 0;
			task_1ms();
		}	
		if (g_task_scheduler.cnt_2 >= TASK_SCHEDU_50MS)
		{
			/* 50毫秒事件，任务调度 */
			g_task_scheduler.cnt_2 = 0;
			task_50ms();
		}
		if (g_task_scheduler.cnt_3 >= TASK_SCHEDU_10MS)
		{
			/* 10毫秒事件，任务调度 */
			g_task_scheduler.cnt_3 = 0;
			task_10ms();
		}
		if (g_task_scheduler.cnt_4 >= TASK_SCHEDU_500MS)
		{
			/* 500毫秒事件，任务调度 */
			g_task_scheduler.cnt_4 = 0;
			task_500ms();
		}
	}
	else if(g_task_scheduler.pwm_update_flag)
	{	
		/* 每个PWM周期更新一次 */
		g_task_scheduler.pwm_update_flag = 0;
		
		/* 没轮到任务时，不空转等待，而是利用 PWM 更新标志补查 Hall 状态。 */
		//Verify_Hall_State();
	}
}
