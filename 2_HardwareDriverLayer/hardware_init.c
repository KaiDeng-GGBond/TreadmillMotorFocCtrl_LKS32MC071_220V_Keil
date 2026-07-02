#include "hardware_init.h"

extern void sortThreePhasePWMDuty(uint16_t cmprU,uint16_t cmprV,uint16_t cmprW);
extern adc_result gAdcObj;

static void Clock_Init(void)
{
    SYS_WR_PROTECT = 0x7a83;   /* 解除系统寄存器写保护 */
    SYS_AFE_REG5 = BIT15;      /* BIT15:PLLPDN 开PLL */
    SoftDelay(100);            /* 延时100us, 等待PLL稳定 21.4.17*/
    SYS_CLK_CFG = 0x000001ff;  /* BIT8:0: CLK_HS,1:PLL  | BIT[7:0]CLK_DIV  | 1ff对应96M时钟 */
    SYS_WR_PROTECT = 0;        /* 关闭系统寄存器写操作*/
}

/*******************************************************************************
 函数名称：    void SystemInit(void)
 功能描述：    硬件系统初始化，调用时钟初始化函数
 输入参数：    无
 输出参数：    无
 返 回 值：    无
 其它说明：	   上电后，先进入的是SystemInit()，然后才是main()
 *******************************************************************************/
void SystemInit(void)
{
    Clock_Init();  /* 时钟初始化 */
}

static void ADC0_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    ADC_StructInit(&ADC_InitStructure);
	
	ADC_InitStructure.IE         = ADC_SF1_IE;	// 中断使能，第一段常规采样完成中断使能
	ADC_InitStructure.RE         = 0;			// DMA请求不使能
	ADC_InitStructure.NSMP       = DISABLE;		// 两段采样不使能
	ADC_InitStructure.DATA_ALIGN = DISABLE;		// DAT右对齐使能,官方推荐统一使用左对齐方式
	ADC_InitStructure.CSMP       = DISABLE;		// 连续采样使能
	ADC_InitStructure.TCNT       = 0;			// 触发一次采样所需的事件数，0为1次事件
	ADC_InitStructure.TROVS      = DISABLE;		// 手动触发过采样使能，开启后一次采样需要多次触发
	ADC_InitStructure.OVSR       = 0;			// 过采样率
	ADC_InitStructure.TRIG       = ADC_TRIG_MCPWM0_T0;  // 触发信号，MCPWM0 T0事件触发ADC常规采样
	ADC_InitStructure.S1         = 5;			// 第一段常规采样次数
	ADC_InitStructure.S2         = 0;			// 第二段常规采样次数
	ADC_InitStructure.IS1        = 0;			// 空闲采样次数
	ADC_InitStructure.LTH        = 0;			// ADC 模拟看门狗 0 下阈值
	ADC_InitStructure.HTH        = 0;			// ADC 模拟看门狗 0 上阈值
	ADC_InitStructure.GEN        = 0;			// ADC 模拟看门狗 0 对应使能位
	ADC_Init(ADC0, &ADC_InitStructure);
	
	ADC_ClearIRQFlag(ADC0, ADC_ALL_IF);//清除所有中断标志位
	
	/* 通道配置 */
	/* 运放
	OPW+ -> P3.5	 -> OPA0_IP		-> ADC_CHANNEL_0
	OPW- -> P3.7	 -> OPA0_IN		-> ADC_CHANNEL_0
	OPV+ -> P3.0	 -> OPA1_IP		-> ADC_CHANNEL_1
	OPV- -> P3.1	 -> OPA1_IN		-> ADC_CHANNEL_1
	OPU+ -> P3.10	 -> OPA2_IP		-> ADC_CHANNEL_2
	OPU- -> P3.11	 -> OPA2_IN		-> ADC_CHANNEL_2 */
	ADC_CHN_GAIN_CFG(ADC0, CHN0, ADC_CHANNEL_0, ADC_GAIN7V2);//W
	ADC_CHN_GAIN_CFG(ADC0, CHN1, ADC_CHANNEL_1, ADC_GAIN7V2);//V
	ADC_CHN_GAIN_CFG(ADC0, CHN2, ADC_CHANNEL_2, ADC_GAIN7V2);//U
	ADC_CHN_GAIN_CFG(ADC0, CHN3, ADC_CHANNEL_4, ADC_GAIN3V6);/* BUC_ADC -> P0.0 -> ADC01_CH4 */
	ADC_CHN_GAIN_CFG(ADC0, CHN4, ADC_CHANNEL_11, ADC_GAIN3V6);/* NTC -> P2.7 -> ADC0_CH11(ADC0 通道 11) */

	ADC0_CFG |= BIT11;  //复位一下
}

static void GPIO_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

	/* MCPWM P1.4~P1.9 */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_PinAFConfig(GPIO1, GPIO_PinSource_4, AF3_MCPWM);//U_Hin_PIN
	GPIO_PinAFConfig(GPIO1, GPIO_PinSource_5, AF3_MCPWM);//U_Lin_PIN
	GPIO_PinAFConfig(GPIO1, GPIO_PinSource_6, AF3_MCPWM);//V_Hin_PIN
	GPIO_PinAFConfig(GPIO1, GPIO_PinSource_7, AF3_MCPWM);//V_Lin_PIN
	GPIO_PinAFConfig(GPIO1, GPIO_PinSource_8, AF3_MCPWM);//W_Hin_PIN
	GPIO_PinAFConfig(GPIO1, GPIO_PinSource_9, AF3_MCPWM);//W_Lin_PIN

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin = U_Hin_PIN | U_Lin_PIN | V_Hin_PIN | V_Lin_PIN | W_Hin_PIN | W_Lin_PIN;
	GPIO_Init(U_Hin_PORT, &GPIO_InitStruct);//PORT都是GPIO1

	/* 继电器 UP -> P2.10 */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin = UP_PIN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(UP_PORT, &GPIO_InitStruct);
	GPIO_ResetBits(UP_PORT, UP_PIN);//默认不吸合
	
	/* 继电器 DOWN -> P2.9 */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin = DOWN_PIN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DOWN_PORT, &GPIO_InitStruct);
	GPIO_ResetBits(DOWN_PORT, DOWN_PIN);	//默认不吸合
	
	/* 继电器 KM_EN -> P3.14 */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin = KM_EN_PIN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(KM_EN_PORT, &GPIO_InitStruct);
	GPIO_ResetBits(KM_EN_PORT, KM_EN_PIN);	//默认不吸合
	gAdcObj.KM_connect_flag = 0;
	
	/*
	运放
	OPW+ -> P3.5	 -> OPA0_IP
	OPW- -> P3.7	 -> OPA0_IN
	OPV+ -> P3.0	 -> OPA1_IP
	OPV- -> P3.1	 -> OPA1_IN
	OPU+ -> P3.10	 -> OPA2_IP
	OPU- -> P3.11	 -> OPA2_IN
	*/
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Pin =	OPW_P_PIN | OPW_N_PIN | \
								OPV_P_PIN | OPV_N_PIN | \
								OPU_P_PIN | OPU_N_PIN;
	GPIO_Init(OPW_P_PORT, &GPIO_InitStruct);

	/*
	HALL
	H_U -> P0.11 -> HALL_IN0
	H_V -> P0.12 -> HALL_IN1
	H_W -> P0.13 -> HALL_IN2
	*/
	/* GPIO_P0.11/P0.12/P0.13设置为HALL_IN0/HALL_IN1/HALL_IN2模拟通道 */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Pin =  H_U_PIN | H_V_PIN | H_W_PIN;
	GPIO_Init(GPIO0, &GPIO_InitStruct);
    GPIO_PinAFConfig(H_U_PORT, GPIO_PinSource_11, AF2_HALL);	//设置复用
    GPIO_PinAFConfig(H_V_PORT, GPIO_PinSource_12, AF2_HALL);	//设置复用
    GPIO_PinAFConfig(H_W_PORT, GPIO_PinSource_13, AF2_HALL);	//设置复用

	/* NTC -> P2.7 -> ADC0_CH11(ADC0 通道 11) */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Pin = NTC_PIN;
	GPIO_Init(NTC_PORT, &GPIO_InitStruct);

	/* LED -> P0.6 */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Pin = LED_PIN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;	//默认上拉，灯灭
    GPIO_Init(LED_PORT, &GPIO_InitStruct);

	/*
	相电压采样 
	U_ADC -> P0.3 -> ADC01_CH7(ADC0/ADC1 通道 7)
	V_ADC -> P0.4 -> ADC01_CH8(ADC0/ADC1 通道 8)
	W_ADC -> P0.5 -> ADC01_CH9(ADC0/ADC1 通道 9)
	*/
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Pin = U_ADC_PIN | V_ADC_PIN | W_ADC_PIN;
	GPIO_Init(U_ADC_PORT, &GPIO_InitStruct);

	/* 母线电压采样 BUC_ADC -> P0.0 -> ADC01_CH4(ADC0/ADC1 通道 4) */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Pin = BUC_ADC_PIN;
	GPIO_Init(BUC_ADC_PORT, &GPIO_InitStruct);

	/* UART1_TX -> P2.5 ; UART1_RX -> P2.4 */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Pin = UARTx_RXD_PIN;
	GPIO_Init(UARTx_RXD_PORT, &GPIO_InitStruct);

	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin = UARTx_TXD_PIN;
	GPIO_Init(UARTx_TXD_PORT, &GPIO_InitStruct);

	GPIO_PinAFConfig(UARTx_TXD_PORT, GPIO_PinSource_5, AF4_UART); //P2.5复用为UART_TX
	GPIO_PinAFConfig(UARTx_RXD_PORT, GPIO_PinSource_4, AF4_UART); //P2.4复用为UART_RX

	/* 把P2.12设置为测试脚	*/
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIO2, &GPIO_InitStruct);
}

static void MCPWM_init(void)
{
	MCPWM_InitTypeDef MCPWM_InitStructure;
	MCPWM_StructInit(&MCPWM_InitStructure);
	
	/* 计数周期设置即MCPWM输出周期(PWM周期)，((96,000,000 / (2 * 16,000 *(1<<0))))=96,000,000/32,000=3,000*/
	MCPWM_InitStructure.TH0 = MCPWM_PERIOD0;
	MCPWM_InitStructure.CLK_DIV = PWM_PRSC;			/* MCPWM时钟分频设置，=0，不分频 */
	
	/* PWM比较值 */
    MCPWM_InitStructure.TH00 = (uint16_t)(-(MCPWM_PERIOD0 >> 2));	//CH0P比较门限值（PWM占空比）
    MCPWM_InitStructure.TH01 = (MCPWM_PERIOD0 >> 2);				//CH0N比较门限值（PWM占空比）
    MCPWM_InitStructure.TH10 = (uint16_t)(-(MCPWM_PERIOD0 >> 2));	//CH1P比较门限值（PWM占空比）
    MCPWM_InitStructure.TH11 = (MCPWM_PERIOD0 >> 2);				//CH1N比较门限值（PWM占空比）
    MCPWM_InitStructure.TH20 = (uint16_t)(-(MCPWM_PERIOD0 >> 2));	//CH2P比较门限值（PWM占空比）
    MCPWM_InitStructure.TH21 = (MCPWM_PERIOD0 >> 2);				//CH2N比较门限值（PWM占空比）
	
	/* 死区时间设置 */
    MCPWM_InitStructure.DeadTimeCH012N = DEADTIME;
    MCPWM_InitStructure.DeadTimeCH012P = DEADTIME;
	
	/* 工作模式：中心对齐PWM模式 */
    MCPWM_InitStructure.MCPWM_WorkModeCH0 = MCPWM0_CENTRAL_PWM_MODE;
    MCPWM_InitStructure.MCPWM_WorkModeCH1 = MCPWM0_CENTRAL_PWM_MODE;
    MCPWM_InitStructure.MCPWM_WorkModeCH2 = MCPWM0_CENTRAL_PWM_MODE;
	
	MCPWM_InitStructure.TMR0 = (uint16_t)(40 - MCPWM_PERIOD0);//该芯片pwm的CNT范围为-PERIOD0到+PERIOD0，MCPWM_PERIOD0=3000
	MCPWM_InitStructure.TMR1 = (uint16_t)(MCPWM_PERIOD0 - 1);
	
	/* MCPWM0_CNT0 */
	MCPWM_InitStructure.MCLK_EN = ENABLE;			/* 模块时钟开启 */
	MCPWM_InitStructure.BASE_CNT0_EN = ENABLE;		/* 主计数器开始计数使能开关 */
	MCPWM_InitStructure.CMP_CTRL_CNT0 = DISABLE;	/* CMP控制CNT0失能 */
    MCPWM_InitStructure.EVT_CNT0_EN = DISABLE;		/* MCPWM_CNT1外部触发失能 */
	MCPWM_InitStructure.TMR2_TB = 0;				/* MCPWM TMR2时基（计数器）选择 0：时基0；1：时基1 */
    MCPWM_InitStructure.TMR3_TB = 1;				/* MCPWM TMR3时基（计数器）选择 0：时基0；1：时基1 */
	MCPWM_InitStructure.EVT0 = DISABLE ;

    MCPWM_InitStructure.TR0_UP_INTV     = DISABLE;
    MCPWM_InitStructure.TR0_T0_UpdateEN = ENABLE;		/* T0事件时更新PWM寄存器 */
    MCPWM_InitStructure.TR0_T1_UpdateEN = DISABLE;
    MCPWM_InitStructure.TR0_AEC         = DISABLE;		/*自动清除MCPWM0_EIF标志位*/

    MCPWM_InitStructure.T0_Update0_INT_EN = DISABLE;	/* T0更新事件 中断关闭 */
    MCPWM_InitStructure.T1_Update0_INT_EN = DISABLE;	/* T1更新事件 中断使能 */
    MCPWM_InitStructure.Update1_INT_EN = ENABLE;		/* CNT0 更新事件 中断使能 */

#if (CNT0_PRE_DRIVER_POLARITY == P_HIGH__N_LOW)			/* CHxP 高有效， CHxN低电平有效 */
    MCPWM_InitStructure.CH0N_Polarity_INV = ENABLE;		/* CH0N通道输出极性设置 | 正常输出或取反输出*/
    MCPWM_InitStructure.CH0P_Polarity_INV = DISABLE;	/* CH0P通道输出极性设置 | 正常输出或取反输出 */
    MCPWM_InitStructure.CH1N_Polarity_INV = ENABLE;
    MCPWM_InitStructure.CH1P_Polarity_INV = DISABLE;
    MCPWM_InitStructure.CH2N_Polarity_INV = ENABLE;
    MCPWM_InitStructure.CH2P_Polarity_INV = DISABLE;

    MCPWM_InitStructure.Switch_CH0N_CH0P =  DISABLE;	/* 通道交换选择设置 | CH0P和CH0N是否选择信号交换 */
    MCPWM_InitStructure.Switch_CH1N_CH1P =  DISABLE;	/* 通道交换选择设置 */
    MCPWM_InitStructure.Switch_CH2N_CH2P =  DISABLE;	/* 通道交换选择设置 */

    /* 默认电平设置 默认电平输出不受MCPWM_IO01和MCPWM_IO23的 BIT0、BIT1、BIT8、BIT9、BIT6、BIT14通道交换和极性控制的影响，直接控制通道输出 */
    MCPWM_InitStructure.CH0P_default_output = MCPWM0_LOW_LEVEL;
    MCPWM_InitStructure.CH0N_default_output = MCPWM0_HIGH_LEVEL;
    MCPWM_InitStructure.CH1P_default_output = MCPWM0_LOW_LEVEL;	/* CH1P对应引脚在空闲状态输出低电平 */
    MCPWM_InitStructure.CH1N_default_output = MCPWM0_HIGH_LEVEL;	/* CH1N对应引脚在空闲状态输出高电平 */
    MCPWM_InitStructure.CH2P_default_output = MCPWM0_LOW_LEVEL;
    MCPWM_InitStructure.CH2N_default_output = MCPWM0_HIGH_LEVEL;
#else
#if (CNT0_PRE_DRIVER_POLARITY == P_HIGH__N_HIGH)		/* CHxP 高有效， CHxN高电平有效 */
    MCPWM_InitStructure.CH0N_Polarity_INV = DISABLE;	/* CH0N通道输出极性设置 | 正常输出或取反输出*/
    MCPWM_InitStructure.CH0P_Polarity_INV = DISABLE;	/* CH0P通道输出极性设置 | 正常输出或取反输出 */
    MCPWM_InitStructure.CH1N_Polarity_INV = DISABLE;
    MCPWM_InitStructure.CH1P_Polarity_INV = DISABLE;
    MCPWM_InitStructure.CH2N_Polarity_INV = DISABLE;
    MCPWM_InitStructure.CH2P_Polarity_INV = DISABLE;

    MCPWM_InitStructure.Switch_CH0N_CH0P =  DISABLE;	/* 通道交换选择设置 | CH0P和CH0N是否选择信号交换 */
    MCPWM_InitStructure.Switch_CH1N_CH1P =  DISABLE;	/* 通道交换选择设置 */
    MCPWM_InitStructure.Switch_CH2N_CH2P =  DISABLE;	/* 通道交换选择设置 */

    /* 默认电平设置 默认电平输出不受MCPWM_IO01和MCPWM_IO23的 BIT0、BIT1、BIT8、BIT9、BIT6、BIT14通道交换和极性控制的影响，直接控制通道输出 */
    MCPWM_InitStructure.CH0P_default_output = MCPWM0_LOW_LEVEL;
    MCPWM_InitStructure.CH0N_default_output = MCPWM0_LOW_LEVEL;
    MCPWM_InitStructure.CH1P_default_output = MCPWM0_LOW_LEVEL;/* CH1P对应引脚在空闲状态输出低电平 */
    MCPWM_InitStructure.CH1N_default_output = MCPWM0_LOW_LEVEL;/* CH1N对应引脚在空闲状态输出高电平 */
    MCPWM_InitStructure.CH2P_default_output = MCPWM0_LOW_LEVEL;
    MCPWM_InitStructure.CH2N_default_output = MCPWM0_LOW_LEVEL;
#endif
#endif

	/* 如果发生FAIL信号立即关闭PWM */
    MCPWM_InitStructure.CH0N_FAIL_EN = ENABLE;
    MCPWM_InitStructure.CH0P_FAIL_EN = ENABLE;
    MCPWM_InitStructure.CH1N_FAIL_EN = ENABLE;
    MCPWM_InitStructure.CH1P_FAIL_EN = ENABLE;
    MCPWM_InitStructure.CH2N_FAIL_EN = ENABLE;
    MCPWM_InitStructure.CH2P_FAIL_EN = ENABLE;

    MCPWM_InitStructure.FAIL0_INPUT_EN   = DISABLE;				//FAIL_0CAP
    MCPWM_InitStructure.FAIL0_INT_EN     = DISABLE;
    MCPWM_InitStructure.FAIL0_Signal_Sel = MCPWM0_FAIL_SEL_CMP;	//FAIL_0CAP
    MCPWM_InitStructure.FAIL0_Polarity   = MCPWM0_HIGH_LEVEL_ACTIVE;

    MCPWM_InitStructure.FAIL1_INPUT_EN   = DISABLE;				//FAIL_0CAP
    MCPWM_InitStructure.FAIL1_INT_EN     = DISABLE;
    MCPWM_InitStructure.FAIL1_Signal_Sel = MCPWM0_FAIL_SEL_IO;	//FAIL_0CAP
    MCPWM_InitStructure.FAIL1_Polarity   = MCPWM0_HIGH_LEVEL_ACTIVE;

    MCPWM_InitStructure.HALT_PRT0        = DISABLE;
    MCPWM_InitStructure.FAIL_0CAP        = DISABLE;

    MCPWM_Init(&MCPWM_InitStructure);
}

static void UART_init(void)
{
	UART_InitTypeDef UART_InitStruct;
	SYS_ModuleClockCmd(SYS_Module_UART1, ENABLE);			/* 开GPIO时钟*/
	UART_StructInit(&UART_InitStruct);
	UART_InitStruct.BaudRate = 115200;						/* 设置波特率 */
	UART_InitStruct.WordLength = UART_WORDLENGTH_8b;		/* 发送数据长度8位 */
	UART_InitStruct.StopBits = UART_STOPBITS_1b;
	UART_InitStruct.FirstSend = UART_FIRSTSEND_LSB;			/* 先发送LSB */
	UART_InitStruct.ParityMode = UART_Parity_NO;			/* 无奇偶校验 */
	UART_InitStruct.IRQEna = UART_IRQEna_RcvOver;//UART_IRQEna_SendOver | UART_IRQEna_RcvOver;
	UART_InitStruct.DMARE = TX_DMA_RE;						/* 使能DMA发送请求 */
	UART_Init(UARTx, &UART_InitStruct);
}

static void HALL_init(void)
{
	HALL_InitTypeDef HALL_InitStruct;
	HALL_StructInit(&HALL_InitStruct);

	HALL_InitStruct.FilterLen = 4096;
	HALL_InitStruct.ClockDivision = HALL_CLK_DIV1;/* HALL时钟为系统主时钟1分频-96mhz */
	HALL_InitStruct.Filter75_Ena = DISABLE;
	HALL_InitStruct.HALL_Ena = ENABLE;
	HALL_InitStruct.Capture_IRQ_Ena = ENABLE;
	HALL_InitStruct.OverFlow_IRQ_Ena = ENABLE;
	HALL_InitStruct.CountTH = 96000000;//1分频的情况，刚好一秒一次溢出中断
	HALL_InitStruct.softIE = DISABLE;

	HALL_Init(&HALL_InitStruct);
	HALL_Cmd(ENABLE);/* HALL使能 */
}

static void PGA_init(void)
{
	OPA_InitTypeDef OPA_InitStruct;
	OPA_StructInit(&OPA_InitStruct);
	
	SYS_AnalogModuleClockCmd(SYS_AnalogModule_OPA0, ENABLE);
	SYS_AnalogModuleClockCmd(SYS_AnalogModule_OPA1, ENABLE);
	SYS_AnalogModuleClockCmd(SYS_AnalogModule_OPA2, ENABLE);

	OPA_InitStruct.OPA_IT = PGA_IT_DISABLE;	/* opa偏置电流调节禁止 */
	OPA_InitStruct.OPA_Gain = PGA_GAIN_8;	/* 选择R2:R1 = 80k:10k */
	OPA_InitStruct.OPA_CLEna = ENABLE;		/* 使能 */
	
	OPA_Init(OPA0 ,&OPA_InitStruct);
	OPA_Init(OPA1 ,&OPA_InitStruct);
	OPA_Init(OPA2 ,&OPA_InitStruct);
}

static void UTimer_init(void)
{
	TIM_TimerInitTypeDef TIM_InitStruct;
	TIM_TimerStrutInit(&TIM_InitStruct);					/* Timer结构体初始化*/
	TIM_InitStruct.Timer_CH0_WorkMode = TIMER_OPMode_CMP;	/* 设置Timer CH0 为比较模式 */
	TIM_InitStruct.Timer_CH0Output = 0;						/* 计数器回零时，比较模式输出极性控制 */
	TIM_InitStruct.Timer_CH1_WorkMode = TIMER_OPMode_CMP;	/* 设置Timer CH1 为比较模式 */
	TIM_InitStruct.Timer_CH1Output = 0;						/* 计数器回零时，比较模式输出极性控制 */
	TIM_InitStruct.Timer_TH = 48000;			/* 定时器计数门限初始值48000*/
	TIM_InitStruct.Timer_CMP0 = 24000;			/* 设置比较模式的CH0比较初始值24000 */
	TIM_InitStruct.Timer_CMP1 = 24000;			/* 设置比较模式的CH1比较初始值24000 */
	TIM_InitStruct.Timer_FLT = 0;				/* 设置捕捉模式或编码器模式下对应通道的数字滤波值 */
	TIM_InitStruct.Timer_ClockDiv = TIMER_CLK_DIV1;		/* 设置Timer模块时钟2分频系数 */
	TIM_InitStruct.Timer_IRQEna = Timer_IRQEna_ZC;		/* 开启Timer模块比较中断和过零中断 Timer_IRQEna_CH0 | Timer_IRQEna_CH1 | */
	TIM_TimerInit(UTIMER0, &TIM_InitStruct);
	TIM_TimerCmd(UTIMER0, ENABLE);						/* Timer0 模块使能 */
}

/****************************************************** 对外函数 **************************************************************/

void SoftDelay(uint32_t cnt)
{
    volatile uint32_t t_cnt;

    for (t_cnt = 0; t_cnt < cnt; t_cnt++)
    {
        __NOP();
    }
}

void currentCalcOffsetInit(void)
{
	/* 电流计算的系数初始化 */
	// 除于ADC_RANGE=2048而不是4096因为该芯片的12位ADC值，最高位是符号位 \
	   ADC_RANGE=0x7FF0而不是7FF是因为该寄存器左对齐，16'h7FF0 = 12'f7FF \
	   (int32_t)防止溢出 \
	   *50是因为采样电阻为0.02欧，x / 0.02 = x * 50; *25是因为采样电阻为0.04欧，x / 0.04 = x * 25 \
	   *1000是因为opa0_real_cal_gain放大了1000倍，所以在除放大倍数之前*1000
	gAdcObj.opa0CalCurrent_k_q20 = (int32_t)((7200.0f * 1000 * 25 / ADC_RANGE / gAdcObj.opa0_real_cal_gain) * (1 << 20));
	gAdcObj.opa1CalCurrent_k_q20 = (int32_t)((7200.0f * 1000 * 25 / ADC_RANGE / gAdcObj.opa1_real_cal_gain) * (1 << 20));
	gAdcObj.opa2CalCurrent_k_q20 = (int32_t)((7200.0f * 1000 * 25 / ADC_RANGE / gAdcObj.opa2_real_cal_gain) * (1 << 20));
}

//在芯片上电之初，电机尚未工作时，运放输入信号为0V，此时通过ADC对该路运放进行采样,
//采到的值就是Gain * Voffset;（已经包含Gain了?）
void OPAOut_OffsetCalibration(void)
{
	uint16_t CalibCnt = 0;
	int CalibSum[3] = {0};
	uint32_t opa0_real_register , opa1_real_register , opa2_real_register;
	uint16_t opa0_real_register_R2_raw , opa0_real_register_R1_raw , \
			 opa1_real_register_R2_raw , opa1_real_register_R1_raw , \
			 opa2_real_register_R2_raw , opa2_real_register_R1_raw ;

	PWMOutputs(DISABLE);		/* 先关波，保证电流为0 */

	SoftDelay(100);		/* 延时等待稳定 */

	__disable_irq();

	/*	一 芯片的校准参数读取（每个芯片都不同，出厂时写入）,这里的地址存的是opa gain=8时的实际阻值，\
		若改了opa放大倍数，要从别的地址读取，详情见凌欧官网wiki */
	//opa gain = 8
	opa0_real_register = Read_Trim(0x00001508);//高16位存放R2实际阻值，低16位存放R1实际阻值（扩大100 倍结果）
	opa1_real_register = Read_Trim(0x00001518);//高16位存放R2实际阻值，低16位存放R1实际阻值（扩大100 倍结果）
	opa2_real_register = Read_Trim(0x00001528);//高16位存放R2实际阻值，低16位存放R1实际阻值（扩大100 倍结果）
	//opa gain = 16
	//opa0_real_register = Read_Trim(0x00001504);//高16位存放R2实际阻值，低16位存放R1实际阻值（扩大100 倍结果）
	//opa1_real_register = Read_Trim(0x00001514);//高16位存放R2实际阻值，低16位存放R1实际阻值（扩大100 倍结果）
	//opa2_real_register = Read_Trim(0x00001524);//高16位存放R2实际阻值，低16位存放R1实际阻值（扩大100 倍结果）
	/* 获取OPA的R1R2的真实阻值 */
	opa0_real_register_R2_raw = (opa0_real_register >> 16) & 0xFFFF; // 高16位
	opa0_real_register_R1_raw = opa0_real_register & 0xFFFF;         // 低16位
	opa1_real_register_R2_raw = (opa1_real_register >> 16) & 0xFFFF; // 高16位
	opa1_real_register_R1_raw = opa1_real_register & 0xFFFF;         // 低16位
	opa2_real_register_R2_raw = (opa2_real_register >> 16) & 0xFFFF; // 高16位
	opa2_real_register_R1_raw = opa2_real_register & 0xFFFF;         // 低16位
	
	/* 计算真实放大倍数（真实倍数放大1000倍） */
	gAdcObj.opa0_real_cal_gain = 1000*(opa0_real_register_R2_raw/(opa0_real_register_R1_raw + PGA_RO_KOHM*100.0f));//扩大100倍为真实倍数，真实倍数再扩大1000倍
	gAdcObj.opa1_real_cal_gain = 1000*(opa1_real_register_R2_raw/(opa1_real_register_R1_raw + PGA_RO_KOHM*100.0f));//扩大100倍为真实倍数，真实倍数再扩大1000倍
	gAdcObj.opa2_real_cal_gain = 1000*(opa2_real_register_R2_raw/(opa2_real_register_R1_raw + PGA_RO_KOHM*100.0f));//扩大100倍为真实倍数，真实倍数再扩大1000倍

	/* 二 读512次ADC/OPA零漂电压并算均值 */
	for(CalibCnt = 0; CalibCnt<(1<<9); CalibCnt++)
	{
		ADC_SoftTrgEN(ADC0 , ENABLE);

		while(ADC_GetIRQFlag(ADC0 , ADC_SERR_IF))//即使上面关了中断，但标志位仍会置位，所以不影响，详见芯片手册
			ADC_ClearIRQFlag(ADC0 , ADC_SERR_IF);

		CalibSum[0] += (int32_t)ADC_GetConversionValue(ADC0, DAT0);/* OPA0 */
		CalibSum[1] += (int32_t)ADC_GetConversionValue(ADC0, DAT1);/* OPA1 */
		CalibSum[2] += (int32_t)ADC_GetConversionValue(ADC0, DAT2);/* OPA2 */
	}

	/* 算平均值 */
	gAdcObj.PGA_ADC_offect[0] = (int16_t)(CalibSum[0] >> 9);
	gAdcObj.PGA_ADC_offect[1] = (int16_t)(CalibSum[1] >> 9);
	gAdcObj.PGA_ADC_offect[2] = (int16_t)(CalibSum[2] >> 9);

	__enable_irq();
}

void GPIO_ToggleP212(void)//test pin
{
    if ((GPIO2->PDO & GPIO_Pin_12) != 0) GPIO2->BRR = GPIO_Pin_12;		//为1，则GPIO_ResetBits(GPIOx, GPIO_Pin);
    else GPIO2->BSRR = GPIO_Pin_12;		//GPIO_SetBits(GPIOx, GPIO_Pin);
}

void MCPWM0_RegUpdate(stru_FOC_CurrLoopDef *this)
{
	MCPWM0_TH00 = -this->struVoltUVW_PWM.u; /* 中心对齐PWM需要同时写正负比较值 */
	MCPWM0_TH01 = this->struVoltUVW_PWM.u;

	MCPWM0_TH10 = -this->struVoltUVW_PWM.v;
	MCPWM0_TH11 = this->struVoltUVW_PWM.v;

	MCPWM0_TH20 = -this->struVoltUVW_PWM.w;
	MCPWM0_TH21 = this->struVoltUVW_PWM.w;
	
#if CURRENT_RECONSTRUCTION
	sortThreePhasePWMDuty((uint16_t)this->struVoltUVW_PWM.u, \
						  (uint16_t)this->struVoltUVW_PWM.v, \
						  (uint16_t)this->struVoltUVW_PWM.w);
#endif
}

void Hardware_init(void)
{
	__disable_irq();			/* 关闭中断 中断总开关 */
	SYS_WR_PROTECT = 0x7a83;	/* 写保护寄存器。系统寄存器有写保护，写入前输入密码 0x7A83解除写保护 */
	IWDG_DISABLE();				/* 关闭看门狗*/

	/* 因FLASH存储体的速度限制，无法达到96MHz的速度。对FLASH进行读取操作时，需要大于一个96MHz的时钟周期 */
	/* 打开预取功能：FLASH控制器完成当前的读取操作后，在不影响正常程序执行的前提下，顺序预取下一个WORD的数据 */
	FLASH_CFG |= 0x00080000;	/* enable prefetch，FLASH预取使能 */

	DSP_Init();				/* 一些底层数学/DSP 资源初始化 */
	ADC0_init();			/* ADC初始化，配置ADC */
	MCPWM_init();			/* PWM初始化 */
	UTimer_init();			/* 500us定时器初始化 */
	GPIO_init();			/* GPIO初始化 */
	uart_1_dma_init();		/* UART1 DMA初始化 */
	UART_init();			/* UART初始化 */
	PGA_init();				/* PGA初始化 */
	HALL_init();			/* HALL初始化 */
	
	SoftDelay(100);			/* 延时等待硬件初始化稳定 */

	NVIC_SetPriority(UARTx_IRQn, 4);	/* UART_IRQn外部中断优先级设置为4*/
    NVIC_EnableIRQ(UARTx_IRQn);			/* 使能UART_IRQn外部中断*/
	
	NVIC_SetPriority(HALL0_IRQn, 2);
	NVIC_EnableIRQ(HALL0_IRQn);			/*使能HALL中断*/
	
	NVIC_SetPriority(ADC0_IRQn, 0);
	NVIC_EnableIRQ(ADC0_IRQn);			/* enable the ADC0 interrupt */
	
	NVIC_SetPriority(TIMER0_IRQn, 2);
	NVIC_EnableIRQ(TIMER0_IRQn);		/* UTIMER0 */
	
	NVIC_SetPriority(MCPWM0_IRQn, 0);
	NVIC_EnableIRQ(MCPWM0_IRQn);
	
	NVIC_EnableIRQ(DMA0_IRQn);
	NVIC_SetPriority(DMA0_IRQn, 3);
		
//	SYS_WR_PROTECT = 0x0;				/*关闭系统寄存器写操作*/
	
	__enable_irq();						/* 开启总中断 */
}
