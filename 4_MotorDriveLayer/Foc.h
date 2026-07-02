#ifndef __FOC_H__
#define __FOC_H__

#include "basic.h"
#include "sysdef.h"

#if CERRENT_LOOP_BAND_WIDTH_TEST
typedef struct
{
    int16_t ref[256];
    int16_t fb[256];
    uint16_t index;
    uint8_t recording;
	int16_t last_id_ref_q15;
}BW_TEST;
extern BW_TEST cur_bw_test;
#endif

/* 电流矢量 */
typedef struct
{
    int16_t qI_Value1;
    int16_t qI_Value2;
} stru_CurrVoctor;

/* 三相电流 */
typedef struct
{
    int16_t u;
    int16_t v;
    int16_t w;
} stru_CurrPhaseUVW; 

/* 电压矢量 */
typedef struct
{
    int16_t qV_Value1;
    int16_t qV_Value2;
} stru_VoltVoctor;

/* 三相电压 */
typedef struct
{
    int16_t u;
    int16_t v;
    int16_t w;
} stru_VoltPhaseUVW; 

/* 定子角度sin Cos值 */
typedef struct
{
    int16_t hCos;
    int16_t hSin;
} stru_TrigComponents;

/* 电流RC滤波结构体  */
typedef struct
{
    int32_t filt_output;	// 上一时刻的累积值，高16位为输出，低16位为小数部分
    uint16_t coef;			// 滤波系数
} stru_RC_Def;

/* PID调节结构体 */
typedef struct
{
    uint16_t hKp_Gain;		// Kp系数（整数放大）
    uint16_t hKp_Divisor;	// Kp缩小（右移）
    uint16_t hKi_Gain;		// Ki系数
    uint16_t hKi_Divisor;	// Ki缩小（右移）
    int16_t hLower_Limit_Output;	//Lower Limit for Output limitation
    int16_t hUpper_Limit_Output;	//Lower Limit for Output limitation
    int32_t wLower_Limit_Integral;	//Lower Limit for Integral term limitation
    int32_t wUpper_Limit_Integral;	//Lower Limit for Integral term limitation
    volatile int32_t wIntegral;		// 当前积分累计值（状态变量）
	volatile int32_t output;		// 输出量
} stru_PIparams;

/* FOC电流环结构体 */
typedef struct
{
	/* 电流相关 */
	volatile stru_CurrPhaseUVW struCurr_u_v_w;		/* 电机真实三相电流，由中断中的CalcPhaseCurrent()得来 */
	volatile stru_CurrVoctor struCurr_alfa_beta;	/* 电机定子alfa beta轴电流,uvw相采样后，克拉克变换而来 */
	volatile stru_CurrVoctor struCurr_q_d;			/* 电机定子d/q轴电流,克拉克变换后，帕克变换而来，转换为旋转坐标系下的电流 */
	volatile int16_t iq_Reference;					/* 目标：Q轴电流给定；速度环需求最终会映射到这里 */
	volatile int16_t id_Reference;					/* 目标：D轴电流给定；默认通常等于0，即Id目标接近 0 */

	/* 电压相关 */
	volatile stru_VoltVoctor struVolt_q_d;			/* 电机定子Q相D相电压，PID调节后给定 */
	volatile stru_VoltVoctor struVolt_alfa_beta;	/* 电机定子alfa beta轴电压，反帕克变换而来 */
	volatile stru_VoltPhaseUVW struVoltUVW_PWM;     /* SVPWM计算得UVW PWM输出电压 */

	/* 角度相关 */
	volatile stru_TrigComponents struTrigSinCos;	/* 定子角度sin Cos值 */
	volatile uint8_t bSector;						/* 当前扇区 */

	/* PI相关 */
	stru_PIparams struPI_Torque;                 	/* Q轴PI参数 */			//要初始化
	stru_PIparams struPI_Flux;						/* D轴电流环PI参数 */	//要初始化
	volatile uint8_t softwareOverCurrentFlag;		/* 软过流标志位 */
	
	/* 启动相关 */
	uint8_t firstOpenMosFlgCnt;						/* 打开MOS的前几个周期，不采样，避开干扰 */

} stru_FOC_CurrLoopDef;

/* 电机状态枚举 */
typedef enum
{
    IDLE		= 0,	/* 空闲状态 */
    INIT		= 1,	/* 初始化状态 */
    RUN			= 2,	/* 正常运行状态 */
    STOP		= 3,	/* 停止状态 */
	WAIT		= 4,	/* 等待置为IDLE状态 */
} enum_SystStatus;

/* FOC电机相关结构体 */
typedef struct
{
	/* 基础控制参数 */
	uint8_t MotorRunFlag;						/* 电机启动指令 */
	uint8_t MotorRunDir;						/* 电机运行方向，在这个跑步机上默认CW */
	int32_t target_rpm;							/* 目标速度 */
	int32_t ref_rpm;							/* 斜坡处理后，传给速度环的目标 */
	int32_t acc_step;							/* 每ms加速量（rpm/ms） */
	int32_t dec_step;							/* 每ms减速量（rpm/ms） */
	
	enum_SystStatus MotorStateMachine;			/* 当前电机运行状态机 */
	
	stru_FOC_CurrLoopDef struFOC_CurrLoop;		/* 电流内环结构体 */
	
} stru_FOC_CtrProcDef;
extern stru_FOC_CtrProcDef struFOC_CtrProc;

void LowPass_Filter_Init(stru_FOC_CtrProcDef *this);
int16_t lowPass_filter(stru_RC_Def *rc, int16_t signal);
void FOC_InitstruParama(void);
void FOC_Model(stru_FOC_CtrProcDef *obj);

#endif
