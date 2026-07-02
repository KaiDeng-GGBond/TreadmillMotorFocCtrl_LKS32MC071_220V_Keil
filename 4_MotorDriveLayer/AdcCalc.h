#ifndef __ADC_CAL_VOLT_AND_CUR_H__
#define __ADC_CAL_VOLT_AND_CUR_H__

#include <stdint.h>


#define ADC_RANGE 0x7FF0//满量程的ADC值，12位ADC但最高位为符号位，所以满量程是2^11=2048(0x7FF),左对齐
#define PRECHARGE_MIN_V		(200)
#define PRECHARGE_RESET_V	(180)
#define PRECHARGE_MAX_V		(300)
#define BUS_OVERVOLT_V		(390)
#define BUS_UNDERVOLT_V		(240)

/*
 * ADC模块用处：
 * 1. 母线电压测量、预充判断、过压判断
 * 2. 相电流测量
 * - 三相瞬时电流都测出来；
 * - 取三相绝对值中的最大值，作为六步两相导通时的等效转矩电流，再做一阶低通
 */

typedef struct
{
    uint16_t adc_raw_vlaue;
    uint16_t volt_v;// 单位：V
} adc_volt_table;
extern const adc_volt_table my_volt_table[43];

typedef struct
{
    volatile uint16_t	bus_volt_v;
	volatile uint16_t	bus_volt_filt_v;
	volatile int16_t	adc_sample_current_ma[3];	// 三相电流原始换算结果，保留符号，便于调试每相电流波形
	volatile int16_t	max_current_record;			// 记录运行过程中见过的最大电流绝对值
	
	volatile uint8_t	bus_over_volt_flag;
	volatile uint8_t	bus_under_volt_flag;
	volatile uint8_t	KM_connect_flag;
	volatile uint8_t	KM_can_connect_flag;
	volatile uint8_t	phase_over_current_flag;
	
	/* 以下是在OPAOut_OffsetCalibration()中进行校准 */
	uint16_t opa0_real_cal_gain;
	uint16_t opa1_real_cal_gain;
	uint16_t opa2_real_cal_gain;
	int16_t PGA_ADC_offect[3];//存储PGA的差分输出零漂电压
	
	int32_t opa0CalCurrent_k_q20;//存储电流计算时的系数，让adc原始值直接*该系数即可得到电流值，在sysinit()中初始化
	int32_t opa1CalCurrent_k_q20;//存储电流计算时的系数，让adc原始值直接*该系数即可得到电流值，在sysinit()中初始化
	int32_t opa2CalCurrent_k_q20;//存储电流计算时的系数，让adc原始值直接*该系数即可得到电流值，在sysinit()中初始化
} adc_result;
extern adc_result gAdcObj;

typedef enum
{
    KM_STATE_IDLE = 0,
    KM_STATE_PRECHARGE_READY
} km_state_t;

void CalcBusVolt(void);

/* 低频调用：查表求母线电压，并顺带处理预充、过压等系统级状态。 */
void TableCalcBusVolt(void);

/* 高频调用：在ADC中断里完成三相电流换算，并构造电流环反馈值。 */
void CalcPhaseCurrent(void);

/* 温度计算 */
float ntc_temperature_calc(int16_t adc_mv);


#endif
