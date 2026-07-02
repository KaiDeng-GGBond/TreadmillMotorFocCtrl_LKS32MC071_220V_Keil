#ifndef __SYSDEF_H__
#define __SYSDEF_H__

extern float vofa_justfloat_data[9];

/************************************PWM 频率及死区定义************************************/
#define MCU_MCLK	(96000000LL)				/* PWM模块运行主频 */
#define PWM_MCLK	((uint32_t)MCU_MCLK)		/* PWM模块运行主频 */
#define PWM_PRSC	((uint8_t)0)				/* PWM模块运行预分频器 0:MCU_MCLK; 1:MCU_MCLK/2; 2:MCU_MCLK/4; 3:MCU_MCLK/8  */
#define PWM_FREQ0	((uint16_t)15625)			/* MCPWM_CNT0PWM斩波频率 */	//1,000,000/15,625 = 64us

/* 电机控制PWM周期计数器值，2 * PWM_FREQ0是因为使用了中心对齐模式 */
#define MCPWM_PERIOD0		((uint16_t) (PWM_MCLK / (uint32_t)(2 * PWM_FREQ0 *(1<<PWM_PRSC))))

#define DEADTIME_NS			((uint16_t)1500)			/**<  死区时间 */
#define DEADTIME			(uint16_t)(((unsigned long long)PWM_MCLK * (unsigned long long)DEADTIME_NS)/1000000000uL)

/************************************电流采样相关*************************************/
/* 
 * 采样电阻40毫欧，模块最大值15A，15A下压降0.6V，放大到接近5V（ADC量程选择为7V2）,OPA放大倍数选择GAIN_8，此时OPA的R2:R1 = 80K:10K,外置电阻R0=3K 
 * 实际GAIN = R2/(R1+R0) = 80,000/(3,000+10,000) = 6.15...
 * 实测：采样电阻两端施加电压0.61v时，adc_raw_vlaue是15200左右，计算得出电流值是13200ma左右（预期是15000ma）
*/
#define PHASE_PGA_REAL_GAIN					(6)							/* 设置GAIN=8情况下->R2:R1=80H:10K，实际放大倍数=R2/(R1+R0)=80,000/(3,000+10,000)=6.1538461538 */
#define PHASE_SHUNT_RESIS_NUMBER			(1)							/* 母线取样电阻个数 */
#define PHASE_SHUNT_RESISTANCE				(0.04f)						/* 单个取样电阻阻值（欧）*/
#define MAX_PHASE_HARDWARE_CURRENT_MA		((uint16_t)8000)			/* 最大相线限流电流（ma） */
#define MAX_PHASE_SOFTWARE_CURRENT_MA		((uint16_t)6000)			/* 软过流限流电流（ma） */
#define PGA_RO_KOHM							((uint16_t)3)				/* PGA放大器的R0电阻的阻值，单位千欧 */

/************************************电压采样相关************************************************/
#define R_HIGH								(1120000)					//2*560k欧
#define R_LOW								(9100)						//9.1k欧
#define DIV_RATIO_BUS						(124.07692f)				//((R_HIGH + R_LOW) / R_LOW) = 62.538461538
#define DIV_RATIO_PHASE						(124.07692f)				//((R_HIGH + R_LOW) / R_LOW) = 62.538461538

/************************************电机相关数据************************************************/
//U		Halla		黄
//V		Hallb		蓝
//W		Hallc		绿

#define	CCW		(1)	/*（从电机出线的方向看）电机轴逆时针，本电机是外转子，外壳顺时针
						跑步机跑带往前转
						电角度减小,iq为+
						hall值：315462 */
#define	CW		(2)	/*（从电机出线的方向看）电机轴顺时针，本电机是外转子，外壳逆时针
						跑步机跑带往后转，正常走
						电角度增大,iq为-
						hall值：326451 */

#define TIME_1S6_BASE	((uint16_t)50)	/* 堵转时间计数 */

/******************************************标幺************************************************/
/* 电流环Q15归一化基准电流，按8A为满量程 */
#define		FOC_CURRENT_Q15_BASE_MA						((int32_t)10000)

#define Q15_MAX    INT16_MAX
#define Q15_MIN    INT16_MIN

/**************************************各种使能************************************************/
#define IF_OPENLOOP							0							/* 是否开环强拖 */
#if IF_OPENLOOP
#define HALL_ENABLE							0							/* 不使用hall计算角度 */
#define SPEED_LOOP_ENABLE					0							/* 是否打开速度环 */
#else
#define HALL_ENABLE							1							/* 使用hall计算角度 */
#define SPEED_LOOP_ENABLE					0							/* 是否打开速度环 */
#endif

#define LOW_VOLT_DEBUG						1							/* 低压调试 */

#define CURRENT_RECONSTRUCTION				0							/* 是否重构电流 */

#define CERRENT_LOOP_BAND_WIDTH_TEST		1							/* 电流环带宽测试 */

#define FOC_DEBUG_PRINT						0							/* 是否打印foc运算过程的数据 */
#define ADC_DEBUG_PRINT						0							/* 是否打印ADC采样中的数据 */
#define PID_DEBUG_PRINT						0							/* 是否打印PID计算中的数据 */
/*********************************************其他********************************************/
#define ABS(X)								( ( (X) >= 0 ) ? (X) : -(X) )

#define MCPWM_UPDATE_REG()      \
    {                           \
        MCPWM0_PRT = 0x0000DEAD; \
        MCPWM0_UPDATE = 0x00ff;  \
        MCPWM0_PRT = 0x0000CAFE; \
    }

#define OPENLOOP_BUFFER_SIZE  35  // 环形缓冲区配置，30-40组

#endif
