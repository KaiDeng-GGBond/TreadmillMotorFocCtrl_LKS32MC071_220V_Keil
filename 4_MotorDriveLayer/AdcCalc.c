#include "AdcCalc.h"
#include <math.h>
#include "lks32mc07x.h"
#include "lks32mc07x_adc.h"
#include "lks32mc07x_mcpwm.h"
#include "sysdef.h"
#include "foc.h"
#include "hardware_init.h"

/* 把带符号电流转成绝对值 */
static uint16_t adc_abs_s16(int16_t value)
{
	return (uint16_t)((value < 0) ? (-value) : value);
}

/* 电压查表 */
const adc_volt_table my_volt_table[43] =
{
    {733, 10},
    {1466, 20},
    {2200, 30},
    {2933, 40},
    {3666, 50},
    {4399, 60},
    {5133, 70},
    {5866, 80},
    {6599, 90},
    {7332, 100},
    {8066, 110},
    {8799, 120},
    {9532, 130},
    {10265, 140},
    {10999, 150},
    {11732, 160},
    {12465, 170},
    {13198, 180},
    {13932, 190},
    {14665, 200},
    {15398, 210},
    {16131, 220},
    {16864, 230},
    {17598, 240},
    {18331, 250},
    {19064, 260},
    {19797, 270},
    {20531, 280},
    {21264, 290},
    {21997, 300},
    {22730, 310},
    {23464, 320},
    {24197, 330},
    {24930, 340},
    {25663, 350},
    {26397, 360},
    {27130, 370},
    {27863, 380},
    {28596, 390},
    {29329, 400},
    {30063, 410},
    {30796, 420},
    {31529, 430},
};

/**************************************************** 温度计算 ***************************************************************/
float ntc_temperature_calc(int16_t adc_mv)
{
    float v = adc_mv / 1000.0f;

    if (v <= 0.01f || v >= 4.99f) return -100.0f; // 防异常

    /* 1. 算电阻 */
    float r_ntc = 4700.0f * v / (5.0f - v);

    /* 2. Beta公式 */
    float T0 = 298.15f;     // 25℃
    float R0 = 10000.0f;    // 10k
    float B  = 3950.0f;

    float tempK = 1.0f / ( (1.0f / T0) + (logf(r_ntc / R0) / B) );

    return tempK - 273.15f; // 转摄氏度
}

/**************************************** 由adc原始值查表获取电压值并插值 ***************************************************/
static uint16_t adc_table_to_volt(uint16_t adc_raw)
{
    int i;
	
    // 小于第一个点，直接返回
    if (adc_raw <= my_volt_table[0].adc_raw_vlaue)
        return my_volt_table[0].volt_v;

    // 大于最后一个点，直接返回
    if (adc_raw >= my_volt_table[42].adc_raw_vlaue)
        return my_volt_table[42].volt_v;
	
	// 找到区间
    for(i = 0; i < 42; i++)
    {
        if(adc_raw < my_volt_table[i+1].adc_raw_vlaue)
        {
            uint16_t adc_low  = my_volt_table[i].adc_raw_vlaue;
            uint16_t adc_high = my_volt_table[i+1].adc_raw_vlaue;
            uint16_t volt_low  = my_volt_table[i].volt_v;
            uint16_t volt_high = my_volt_table[i+1].volt_v;
			// 线性插值，整数运算
			uint16_t volt = volt_low + (uint32_t)(adc_raw - adc_low) * (volt_high - volt_low) / (adc_high - adc_low);
            return volt;
        }
    }
    // 走到这里就报错误
    return 0;
}


/**************************************** 查表获取电压值并做了过压和预充的判断 ***************************************************/
volatile int16_t adcRawValue[5] = {0};		//存储ADC原始读数;
float ntc_temp;
//此函数可以较低频，放在5/10ms定时任务中，电流测量转换见CalcPhaseCurrent()
void TableCalcBusVolt(void)
{
	/* 获取原始采样ADC值，输出为12bit补码 */
	adcRawValue[3] = ADC_GetConversionValue(ADC0, DAT3);/* BUC_ADC -> P0.0 -> ADC01_CH4 */
	
	/* 查表查电压 */
	gAdcObj.bus_volt_v = adc_table_to_volt((uint16_t)adcRawValue[3]);
	
	/* 给gAdcObj.bus_volt_v加个滤波 */
	static uint16_t adc_buf[8];
	static uint8_t idx = 0;
	adc_buf[idx++] = gAdcObj.bus_volt_v;
	if(idx > 7) idx = 0;
	uint32_t sum = 0;
	for(int i=0;i<8;i++)
	{
		sum += adc_buf[i];
	}
	gAdcObj.bus_volt_filt_v = sum >> 3;
	
	/* 知道电压值后，做一系列判断、标志位处理，如预充、过压、欠压等... */
	/* 过压判断 */
	if(gAdcObj.bus_volt_v >= BUS_OVERVOLT_V)
	{
		gAdcObj.bus_over_volt_flag = 1;
		
		//过压后，一定要断电重启才行
		PWMOutputs(DISABLE);//关波
		GPIO_ResetBits(KM_EN_PORT, KM_EN_PIN);//KM不吸合
		gAdcObj.KM_connect_flag = 0;	
	}
	
#if !LOW_VOLT_DEBUG
	/* 欠压判断 */
	if(gAdcObj.bus_volt_v <= BUS_UNDERVOLT_V)
	{
		gAdcObj.bus_under_volt_flag = 1;
		
		//欠压后，一定要断电重启才行
		PWMOutputs(DISABLE);//关波，一定要关波再管继电器
		GPIO_ResetBits(KM_EN_PORT, KM_EN_PIN);//KM不吸合
		gAdcObj.KM_connect_flag = 0;
	}
#endif

	/* 预充是否达到电压判断 */
	static uint16_t km_delay_cnt = 0;
	if((gAdcObj.bus_volt_filt_v > PRECHARGE_MIN_V) && (gAdcObj.bus_volt_filt_v < PRECHARGE_MAX_V))
	{
		//进入延时等待，该函数进入周期为10ms，电压达标后的1s后才允许吸合，即进入100次
        if (km_delay_cnt < 100)
        {
            km_delay_cnt++;
            gAdcObj.KM_can_connect_flag = KM_STATE_IDLE; // 还没到2秒
        }
        else
		{
			gAdcObj.KM_can_connect_flag = KM_STATE_PRECHARGE_READY; // 满200次，允许吸合
			GPIO_SetBits(KM_EN_PORT, KM_EN_PIN);//KM吸合
			gAdcObj.KM_connect_flag = 1;
		}
	}
	else if (gAdcObj.bus_volt_v < PRECHARGE_RESET_V) // 滞回
	{
		gAdcObj.KM_can_connect_flag = KM_STATE_IDLE;//在继电器吸合的函数中判断，flag为0不吸合，但已吸合的就不再受影响
		km_delay_cnt = 0;// 条件不满足，清零
	}
}

/*********************************************** 中断回调函数 **********************************************************/
/*
 * 此函数放在 ADC 中断中
 * 关键思路：
 * 1. 先把三相电流都算出来，保留符号，便于观察真实波形；
 * 2. 六步两相导通时，真正产生转矩的是当前导通回路中的那一路电流幅值；
 * 3. 在没有做精确扇区电流重构的前提下，取三相绝对值最大值，是一种简单、稳定、计算量很小的近似；
 * 4. 再做一阶低通，把换相边沿的毛刺和 ADC 抖动压一压，再交给电流环。
 */
volatile int16_t adc_sample_volt_mv[3];		//存储ADC转换后的电压值（单位mv）
uint16_t g_abs_current_max;
void CalcPhaseCurrent(void)
{
	uint16_t abs_current_u;
	uint16_t abs_current_v;
	uint16_t abs_current_w;
	
	/* 获取原始采样ADC值，输出为12bit补码 */
	adcRawValue[0] = ADC_GetConversionValue(ADC0, DAT0);/* OPA0 -> W相 */
	adcRawValue[1] = ADC_GetConversionValue(ADC0, DAT1);/* OPA1 -> V相 */
	adcRawValue[2] = ADC_GetConversionValue(ADC0, DAT2);/* OPA2 -> U相 */

	/* 计算对应的采样电阻的压降（放大器输入侧） */
	//	除于ADC_RANGE=2048而不是4096因为该芯片的12位ADC值，最高位是符号位 \
		ADC_RANGE=0x7FF0而不是7FF是因为该寄存器左对齐，16'h7FF0 = 12'f7FF \
	 	(int32_t)防止溢出 \
		*50是因为采样电阻为0.04欧，x / 0.04 = x * 25
	gAdcObj.adc_sample_current_ma[0] = (int32_t)(((int64_t)(adcRawValue[0] - gAdcObj.PGA_ADC_offect[0]) * gAdcObj.opa0CalCurrent_k_q20) >> 20);
	gAdcObj.adc_sample_current_ma[1] = (int32_t)(((int64_t)(adcRawValue[1] - gAdcObj.PGA_ADC_offect[1]) * gAdcObj.opa1CalCurrent_k_q20) >> 20);
	gAdcObj.adc_sample_current_ma[2] = (int32_t)(((int64_t)(adcRawValue[2] - gAdcObj.PGA_ADC_offect[2]) * gAdcObj.opa2CalCurrent_k_q20) >> 20);

	/* 获取绝对值电流 */
	abs_current_w = adc_abs_s16(gAdcObj.adc_sample_current_ma[0]);
	abs_current_v = adc_abs_s16(gAdcObj.adc_sample_current_ma[1]);
	abs_current_u = adc_abs_s16(gAdcObj.adc_sample_current_ma[2]);
	
	/* 获取最大电流 */
	g_abs_current_max = abs_current_w;
	if(abs_current_v > g_abs_current_max)
	{
		g_abs_current_max = abs_current_v;
	}
	if(abs_current_u > g_abs_current_max)
	{
		g_abs_current_max = abs_current_u;
	}
	
	/* 软过流保护 */
	if(g_abs_current_max >= MAX_PHASE_SOFTWARE_CURRENT_MA)
	{
		struFOC_CtrProc.struFOC_CurrLoop.softwareOverCurrentFlag = 1;//软过流标志位置1
		
		/* 过流保护 */
		if(g_abs_current_max >= MAX_PHASE_HARDWARE_CURRENT_MA)
		{
			PWMOutputs(DISABLE);//关波
			
			GPIO_ResetBits(KM_EN_PORT, KM_EN_PIN);//主接触器断开
			gAdcObj.KM_connect_flag = 0;
			
			gAdcObj.phase_over_current_flag = 1;//过流标志位置1,状态机检测到标志位变化，会置为STOP状态的
		}
	}
	else
	{
		struFOC_CtrProc.struFOC_CurrLoop.softwareOverCurrentFlag = 0;//软过流标志位置0
	}
	
	/* 记录最大电流 */
	if(g_abs_current_max > gAdcObj.max_current_record)
	{
		gAdcObj.max_current_record = g_abs_current_max;
		vofa_justfloat_data[2] = g_abs_current_max;
	}
}

