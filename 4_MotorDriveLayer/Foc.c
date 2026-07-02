#include "Foc.h"
#include "hall.h"
#include "AdcCalc.h"
#include "Clark_Park.h"
#include "Svpwm.h"
#include "PID.h"
#include "sysdef.h"
#include "hardware_init.h"

stru_RC_Def struCurrU_RC;	/* U相相电流RC滤波结构体 */	//要初始化
stru_RC_Def struCurrV_RC;	/* V相相电流RC滤波结构体 */	//要初始化
stru_RC_Def struCurrW_RC;	/* W相相电流RC滤波结构体 */	//要初始化
extern stru_RC_Def struSpeedLoopOutput_RC;	/* 速度环电流输出RC滤波结构体 */	//要初始化
extern stru_RC_Def struSpeedFeedback_RC;	/* 速度环速度输入RC滤波结构体 */	//要初始化

extern adc_result gAdcObj;						/* Adc读取结构体 */
extern strHallProcessObj gStructHallProcess;	/* Hall信号处理结构体 */

extern void rebuildThreePhaseCurr(int16_t *currU,int16_t *currV,int16_t *currW);

#if CERRENT_LOOP_BAND_WIDTH_TEST
BW_TEST cur_bw_test = {0};
extern uint8_t gTrig8;
#endif

static int16_t CurrentMa_ToQ15(int16_t current_ma)
{
	int32_t scaled_current;

	scaled_current = ((int32_t)current_ma * Q15_MAX) / FOC_CURRENT_Q15_BASE_MA;//除于满量程得到比例，再*32767，得到q15格式

	if (scaled_current > Q15_MAX)
	{
		scaled_current = Q15_MAX;
	}
	else if (scaled_current < Q15_MIN)
	{
		scaled_current = Q15_MIN;
	}

	return (int16_t)scaled_current;//返回q1.15格式
}

/* ADC中断中更新的相电流在这里进行一阶滤波，然后存入FOC电流环结构体 */
static void Get_PhaseCurrent_AndFilter(stru_FOC_CurrLoopDef *obj)
{
	int16_t hPhaseCurrU , hPhaseCurrV , hPhaseCurrW;
	
	hPhaseCurrU = gAdcObj.adc_sample_current_ma[2];//u, 获取到ma值
	hPhaseCurrV = gAdcObj.adc_sample_current_ma[1];//v
	hPhaseCurrW = gAdcObj.adc_sample_current_ma[0];//w

	hPhaseCurrU = lowPass_filter(&struCurrU_RC, hPhaseCurrU);// 低通滤波
	hPhaseCurrV = lowPass_filter(&struCurrV_RC, hPhaseCurrV);
	hPhaseCurrW = lowPass_filter(&struCurrW_RC, hPhaseCurrW);
	
#if CURRENT_RECONSTRUCTION
	/* 这里重构电流 */
	rebuildThreePhaseCurr(&hPhaseCurrU, &hPhaseCurrV, &hPhaseCurrW);
#endif
	
	/* 这里转化成q15存储运算 */
	obj->struCurr_u_v_w.u = CurrentMa_ToQ15(hPhaseCurrU);
	obj->struCurr_u_v_w.v = CurrentMa_ToQ15(hPhaseCurrV);
	obj->struCurr_u_v_w.w = CurrentMa_ToQ15(hPhaseCurrW);
}

/* 低通滤波器 */
int16_t lowPass_filter(stru_RC_Def *rc, int16_t signal)
{
	/* 一阶离散RC低通公式 */
    int32_t err;
    err = (signal - (int16_t)(rc->filt_output >> 16)) * (rc->coef);//计算误差：当前输入-上一输出（上一输出通过右移16位得到）
	rc->filt_output += err;//累加误差（带小数精度）
    return(rc->filt_output >> 16);//返回当前输出（高16位）
}

/* 低通滤波器初始化 */
void LowPass_Filter_Init(stru_FOC_CtrProcDef *this)
{
	//电流环滤波，截止频率1.6kHz
	//滤波太狠也会导致电流环的阶跃响应超调震荡
	struCurrU_RC.filt_output = 0;
    struCurrU_RC.coef = 25290;
    struCurrV_RC.filt_output = 0;
    struCurrV_RC.coef = 25290;
    struCurrW_RC.filt_output = 0;
    struCurrW_RC.coef = 25290;
	
	struSpeedLoopOutput_RC.filt_output = 0;
	struSpeedLoopOutput_RC.coef = 5000;
	struSpeedFeedback_RC.filt_output = 0;
	struSpeedFeedback_RC.coef = 5000;
}


#define CURRENT_PID_PUTPUT_LIMIT 		(10000)
#define CURRENT_PID_INTEGRAL_LIMIT 		(10000)
/* FOC参数初始化，上电用，主要包括PI参数，电机状态机状态，相电流RC滤波参数 */
void FOC_InitstruParama(void)
{
	/* Q轴PI初始化 */
	struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.hKp_Gain = 2000;//5000;//Q15
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.hKp_Divisor = 8;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.hKi_Gain = 100;//Q15
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.hKi_Divisor = 8;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.hLower_Limit_Output = -CURRENT_PID_PUTPUT_LIMIT;//-32000;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.hUpper_Limit_Output = CURRENT_PID_PUTPUT_LIMIT;//32000;
	//pid输出到uq中，测试：7800、7900的uq输出限幅约对应1570rpm
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.wLower_Limit_Integral = -CURRENT_PID_INTEGRAL_LIMIT;//-20000;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.wUpper_Limit_Integral = CURRENT_PID_INTEGRAL_LIMIT;//30000;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.wIntegral = 0;
	
	/* D轴PI初始化 */
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.hKp_Gain = 2000;//Q15(0-32768)
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.hKp_Divisor = 8;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.hKi_Gain = 100;//800;//4000;//Q15
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.hKi_Divisor = 8;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.hLower_Limit_Output = -CURRENT_PID_PUTPUT_LIMIT;//-15000;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.hUpper_Limit_Output = CURRENT_PID_PUTPUT_LIMIT;//15000;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.wLower_Limit_Integral = -CURRENT_PID_INTEGRAL_LIMIT;//-20000;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.wUpper_Limit_Integral = CURRENT_PID_INTEGRAL_LIMIT;//20000;
    struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.wIntegral = 0;

	struFOC_CtrProc.MotorStateMachine = IDLE;
	struFOC_CtrProc.MotorRunDir = CW;

	struFOC_CtrProc.struFOC_CurrLoop.softwareOverCurrentFlag = 1;

	//每ms加速量（rpm/ms）,如想1s加速到3000rpm.就是每进去一次speed_ref_ramp()函数，\
	  目标rpm加3，进1000次是1s，转速也变为了3000rpm。注意这与speed_ref_ramp()的周  \
	  期有关，目前10ms进一次，进一次加2rpm。减速同理
	struFOC_CtrProc.acc_step = 2;
	struFOC_CtrProc.dec_step = 2;

	LowPass_Filter_Init(&struFOC_CtrProc);/* 把所有滤波器对象清零并装载系数 */

	//gStructHallProcess.PhaseShiftOffset = S16_15_PHASE_SHIFT;
}

/* FOC主控制函数，16Khz的ADC中断中调用 */
extern volatile uint16_t gOpenLoopFakeElecAngle;
extern void MCPWM0_RegUpdate(stru_FOC_CurrLoopDef *this);
uint16_t gFakeElecAngle = 0;
void FOC_Model(stru_FOC_CtrProcDef *pCtrObj)
{
	/* 指代控制结构体内的电流环结构体 */
	stru_FOC_CurrLoopDef *CurrentObj;
	CurrentObj = &pCtrObj->struFOC_CurrLoop;
	int16_t theatae;
	
	/* 从这里开始归一化 */
	/* 读出相电流并做滤波，把电流值放入struFOC_CtrProc.struFOC_CurrLoop.struCurr_u_v_w */
	Get_PhaseCurrent_AndFilter(CurrentObj);
	
	/* 获取电角度 */
	//注：ElecAngleNoOffset是uint16_t，而theatae是int16_t，会被截断，但是theatae传入Park()中的Trig_Functions()中，会被重新加上32768，相当于恢复了
	//ElecAngleNoOffset = SectorBoundaryAngle - SectorProgress + PhaseShiftOffset\
	  测试中发现，给PhaseShiftOffset加大到10-15度，即1820至2731(Q15),同转速下所需电流会变小
	theatae = gStructHallProcess.ElecAngleNoOffset;//该值在AC中断的Hall_PwmCycleCallback()中被更新

#if IF_OPENLOOP
	theatae = gOpenLoopFakeElecAngle;//开环强拖
#endif

	/* 克拉克变换，把三相系统中的采样值映射到两相静止坐标系 alpha/beta */
	CurrentObj->struCurr_alfa_beta = Clarke(CurrentObj->struCurr_u_v_w);

	/* 帕克变换，把静止坐标系电流映射到随转子旋转的 dq 坐标系。d轴：励磁分量，q轴：转矩分量 */
	CurrentObj->struCurr_q_d = Park(CurrentObj->struCurr_alfa_beta , theatae , CurrentObj);
	vofa_justfloat_data[0] = CurrentObj->struCurr_q_d.qI_Value1;
	vofa_justfloat_data[1] = CurrentObj->struCurr_q_d.qI_Value2;
	
	int16_t iq_ref_q15, id_ref_q15;

    if (CurrentObj->firstOpenMosFlgCnt < 3)
    {
		/* 启动电机运行的前3拍滤波器初始化 直接给uduq */
		CurrentObj->firstOpenMosFlgCnt++;

		CurrentObj->struCurr_u_v_w.u = 0;//电机定子uvw相电流置0
		CurrentObj->struCurr_u_v_w.v = 0;
		CurrentObj->struCurr_u_v_w.w = 0;
		
		//克拉克变换后的qd相电流置0
		CurrentObj->struCurr_q_d.qI_Value1 = 0;
		CurrentObj->struCurr_q_d.qI_Value2 = 0;
		
		/* 刚开 MOS 的前几拍通常会有开关干扰，所以先把滤波器和上次采样值预充好。 */
		CurrentObj->struVolt_q_d.qV_Value2 = 0;//d轴目标电压置0
		CurrentObj->struVolt_q_d.qV_Value1 = 0;//2000;//q轴目标电压
    }
	else
	{
		/* 参考值在这里从mA转成Q15，反馈值已经在前面Get_PhaseCurrent_AndFilter(CurrentObj)中已转换为Q15 */
		id_ref_q15 = CurrentMa_ToQ15(CurrentObj->id_Reference);//归一化，q1.15格式的电流值
		iq_ref_q15 = CurrentMa_ToQ15(CurrentObj->iq_Reference);//归一化，q1.15格式的电流值
		vofa_justfloat_data[2] = id_ref_q15;

#if CERRENT_LOOP_BAND_WIDTH_TEST
	/* 在这里弄个循环缓冲区，把id_ref_q15和真实id_q15存起来，真实id_q15接近了id_ref_q15就暂停存 */
	if((cur_bw_test.recording == 0) && (ABS(id_ref_q15 - cur_bw_test.last_id_ref_q15)>100))//有阶跃输入了
	{
		cur_bw_test.recording = 1;
		cur_bw_test.index = 0;
	}
	cur_bw_test.last_id_ref_q15 = id_ref_q15;
	
	if(cur_bw_test.recording)
	{
		cur_bw_test.ref[cur_bw_test.index] = id_ref_q15;
		cur_bw_test.fb[cur_bw_test.index]  = CurrentObj->struCurr_q_d.qI_Value2;

		cur_bw_test.index++;

		if(cur_bw_test.index >= 256)
		{
			cur_bw_test.recording = 0;
			gTrig8 = 1;
		}
	}
#endif
		
		/* 赋值期望的Vd/Vq */
#if !IF_OPENLOOP
		CurrentObj->struVolt_q_d.qV_Value2 = PI_Controller(id_ref_q15,\
											 CurrentObj->struCurr_q_d.qI_Value2,\
											 &CurrentObj->struPI_Flux);//d轴目标
		//CurrentObj->struVolt_q_d.qV_Value1 = PI_Controller(iq_ref_q15,\
											 CurrentObj->struCurr_q_d.qI_Value1,\
											 &CurrentObj->struPI_Torque);//Iq轴目标\
																		   约60v下开环空载给到6000开始转\
																		   220v下空载，给定300，电流基值=8000ma下，uq=1250,空载约1200rpm\
																		   220v下上跑步机，给定700，起步稳定
#else
		CurrentObj->struVolt_q_d.qV_Value1 = iq_ref_q15;//接220市电，给定1250,空载下能转起来
		CurrentObj->struVolt_q_d.qV_Value2 = id_ref_q15;
#endif
		vofa_justfloat_data[3] = CurrentObj->struVolt_q_d.qV_Value2;
		
		/* 做约束：d 轴电压不能大到超过当前q轴总给定,避免弱磁/励磁分量过大，挤占转矩输出空间 */
		//不能用，给负iq会转不起来
#if 0
		if(ABS(CurrentObj->struVolt_q_d.qV_Value2) > (CurrentObj->struVolt_q_d.qV_Value1))
		{
			if(CurrentObj->struVolt_q_d.qV_Value2 > 0)
			{
				CurrentObj->struVolt_q_d.qV_Value2 = (CurrentObj->struVolt_q_d.qV_Value1);
			}
			else
			{
				CurrentObj->struVolt_q_d.qV_Value2 = -(CurrentObj->struVolt_q_d.qV_Value1);
			}
		}
#endif
	}

	/* 电压极限圆限制，如果 Vd/Vq 的矢量长度超过逆变器可输出范围，就按比例缩放回圆内。 */
	RevPark_Circle_Limitation(pCtrObj);

	/* 反帕克变换，dq坐标下的电压指令变回静止坐标系 */
	CurrentObj->struVolt_alfa_beta = Rev_Park(CurrentObj->struVolt_q_d , CurrentObj);
	
	/* SVPWM根据Valpha/Vbeta计算三相PWM占空比 */
	SVPWM_2PH(pCtrObj);//适配lks的mcpwm的计数模式的svpwm

	MCPWM0_RegUpdate(CurrentObj);//写到PWM外设
}
