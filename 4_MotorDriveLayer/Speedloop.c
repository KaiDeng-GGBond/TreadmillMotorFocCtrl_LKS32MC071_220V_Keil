#include "Speedloop.h"
#include "Multifunction.h"
#include "Foc.h"

stru_RC_Def struSpeedLoopOutput_RC;	/* 速度环电流输出RC滤波结构体 */	//要初始化
stru_RC_Def struSpeedFeedback_RC;	/* 速度环电流输出RC滤波结构体 */	//要初始化

#define IQ_STEP_MAX    100
#define SPEEDLOOP_I_MAX_VALUE    53000

/*
 * 斜率限制器
 * 功能：
 *   限制输出值每次调用的最大变化量
 *
 * 参数：
 *   current : 当前输出值
 *   target  : 目标值
 *   stepMax : 每次允许的最大变化量
 * 返回：
 *   限幅后的新输出值
 * 示例：
 * iq_Reference = RampLimitS16( iq_Reference, (int16_t)current_cmd_ma_2, IQ_STEP_MAX);
 */
static int16_t RampLimitS16(int16_t current, int16_t target, int16_t stepMax)
{
    int32_t delta;

    delta = (int32_t)target - (int32_t)current;

    /* 限制变化率 */
    if (delta > stepMax)
    {
        delta = stepMax;
    }
    else if (delta < -stepMax)
    {
        delta = -stepMax;
    }

    return (int16_t)(current + delta);
}

/* 速度决策,由最终目标obj->target_rpm梯形改变输出的obj->ref_rpm的值，再传给速度环。只在target_rpm变化时起作用 */
/* 该函数10ms进一次，每次进入判断targetrpm与传给速度环的refrpm的差值，做梯形加减速，refrpm加到terget后，后面就没这个函数的事了，是速度环在调节 */
extern void PWMOutputs(FuncState t_state);
void speed_ref_ramp(stru_FOC_CtrProcDef *obj)
{
	if(struFOC_CtrProc.MotorRunFlag == 0)
	{
		return;
	}
	
    int32_t diff;

    /* 计算目标与当前斜坡值的差 */
    diff = obj->target_rpm - obj->ref_rpm;

    /* 加速 */
    if (diff > 0)
    {
        if (diff > obj->acc_step)
            obj->ref_rpm += obj->acc_step;
        else
            obj->ref_rpm = obj->target_rpm;
    }
    /* 减速 */
    else if (diff < 0)
    {
        if (diff < -obj->dec_step)
            obj->ref_rpm -= obj->dec_step;
        else
            obj->ref_rpm = obj->target_rpm;
    }
    /* diff == 0，不处理 */

    /* obj->ref_rpm输出给速度环 */
}

/*
 * 外转速环：
 * - 输入：Hall估算出来的机械转速
 * - 输出：ref_current
 * - 执行频率：10ms任务
 * 改速度环pi中，输入的是真实的rpm，输出的却是q15归一化的电流值，量纲有点混乱。目前设置的归一化的基准电流是10A，对应的在该环里的输出就是32768。
 * 若希望积分决定50%的输出，则积分值的限幅应该是((32768/2)*32768)/motorCtrlSpeedLoopKi->(（输出的50%）* 2^15) / Ki
 */
int32_t	motorCtrlSpeedLoopKp_maPerRpm = 64768;//647680;//10	/* Q15速度环比例 */
int32_t	motorCtrlSpeedLoopKi = 1400;//14000;//0.12			/* Q15速度环积分 */
//以上的Kp与Ki是跑步机上调出来的值，空载下可能偏大
volatile int32_t speed_loop_integral_acc = 0;
extern stru_motor_comprehensive stru_motorcomprehensive;
void SpeedLoopCallback(stru_FOC_CtrProcDef *obj)
{
	int32_t speed_ref_rpm;
	int32_t speed_error_rpm;
	int32_t speed_feedback_rpm , speed_feedback_rpm_filt;
	int64_t current_cmd_ma;
	
	if (obj->MotorStateMachine != RUN)
	{
		return;
	}

	/* 获取目标转速、当前转速、转速误差 */
	speed_ref_rpm = obj->ref_rpm;
	speed_feedback_rpm = gStructHallProcess.MechSpeedRpm;
	//speed_feedback_rpm = lowPass_filter(&struSpeedFeedback_RC , speed_feedback_rpm);//会造成起步卡顿
	speed_error_rpm = speed_ref_rpm - speed_feedback_rpm;
	
	/* 不同转速赋不同pi参数处理。目标转速为0，则说明停机（这里存疑，要是运行过程中目标转速被设置为0怎么办） */
	if (speed_ref_rpm == 0)
	{
		//积分清零
		struFOC_CtrProc.struFOC_CurrLoop.struPI_Torque.wIntegral = 0;
		struFOC_CtrProc.struFOC_CurrLoop.struPI_Flux.wIntegral = 0;
		speed_loop_integral_acc = 0;
		obj->struFOC_CurrLoop.id_Reference = 0;		//d目标电流为0
		obj->struFOC_CurrLoop.iq_Reference = 0;		//q目标电流为0
		return;
	}
	
	/* 积分累加并限幅 */
	speed_loop_integral_acc += speed_error_rpm;
	
	//53000由计算得出，在ki=4000情况下，希望积分项占总输出的20%
	if(speed_loop_integral_acc >= SPEEDLOOP_I_MAX_VALUE)
		speed_loop_integral_acc = SPEEDLOOP_I_MAX_VALUE;
	else if(speed_loop_integral_acc <= -SPEEDLOOP_I_MAX_VALUE)
		speed_loop_integral_acc = -SPEEDLOOP_I_MAX_VALUE;
	
	/* 速度误差先换算成电流命令，再交给电流环去做快调节 */
	current_cmd_ma = (((int64_t)speed_error_rpm * motorCtrlSpeedLoopKp_maPerRpm) >> 15) + \
					  (((int64_t)speed_loop_integral_acc * motorCtrlSpeedLoopKi) >> 15);

	/* 速度环输出的是正向电流命令，所以积分器下限直接设为 0，代表不允许速度环自己生成反向制动电流 */
#if 0
	if(current_cmd_ma >= 32000)
		current_cmd_ma = 32000;
	else if(current_cmd_ma <= -20000)
		current_cmd_ma = -20000;//可以设置为不允许反向电流
#else
	//速度方向定义取反后
	if(current_cmd_ma >= 0)
		current_cmd_ma = 0;//暂时不允许反向电流
	else if(current_cmd_ma <= -2000)
		current_cmd_ma = -2000;
#endif
	current_cmd_ma = lowPass_filter(&struSpeedLoopOutput_RC , current_cmd_ma);//脚上的效果不太好

	/* 给电流环输出值 */
#if SPEED_LOOP_ENABLE
	obj->struFOC_CurrLoop.id_Reference = 0;
	obj->struFOC_CurrLoop.iq_Reference = (int16_t)(current_cmd_ma);
#endif
}
