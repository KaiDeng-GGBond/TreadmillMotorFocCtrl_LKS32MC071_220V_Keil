#ifndef __HARDWARE_INIT_H
#define __HARDWARE_INIT_H

#include "basic.h"
#include "lks32mc07x_lib.h"
#include "sysdef.h"
#include "lks32mc07x_dsp.h"
#include "lks32mc07x_sys.h"
#include "Foc.h"
#include "AdcCalc.h"
#include "uart.h"

/* MOS驱动极性定义 */
#define P_HIGH__N_HIGH		0		/* 预驱预动极性设置 上管高电平有效，下管高电平有效 */
#define P_HIGH__N_LOW		1		/* 预驱预动极性设置 上管高电平有效，下管低电平有效 */

#define  CNT0_PRE_DRIVER_POLARITY           P_HIGH__N_HIGH
#define  CNT1_PRE_DRIVER_POLARITY           P_HIGH__N_LOW

/* 引脚定义 */
/* 串口 UART1_TX -> P2.5 ; UART1_RX -> P2.4 */
#define UARTx 			UART1
#define UARTx_IRQn		UART1_IRQn
#define UARTx_RXD_PORT 	GPIO2
#define UARTx_RXD_PIN 	GPIO_Pin_4
#define UARTx_TXD_PORT 	GPIO2
#define UARTx_TXD_PIN 	GPIO_Pin_5

/* 母线电压采样 BUC_ADC -> P0.0 -> ADC01_CH4(ADC0/ADC1 通道 4) */
#define BUC_ADC_PORT 	GPIO0
#define BUC_ADC_PIN 	GPIO_Pin_0

/*
相电压采样 
U_ADC -> P0.3 -> ADC01_CH7(ADC0/ADC1 通道 7)
V_ADC -> P0.4 -> ADC01_CH8(ADC0/ADC1 通道 8)
W_ADC -> P0.5 -> ADC01_CH9(ADC0/ADC1 通道 9)
*/
#define U_ADC_PORT 	GPIO0
#define U_ADC_PIN 	GPIO_Pin_3

#define V_ADC_PORT 	GPIO0
#define V_ADC_PIN 	GPIO_Pin_4

#define W_ADC_PORT 	GPIO0
#define W_ADC_PIN 	GPIO_Pin_5

/* LED -> P0.6 */
#define LED_PORT 	GPIO0
#define LED_PIN		GPIO_Pin_6

/* NTC -> P2.7 -> ADC0_CH11(ADC0 通道 11) */
#define NTC_PORT 	GPIO2
#define NTC_PIN		GPIO_Pin_7

/*
HALL
H_U -> P0.11 -> HALL_IN0
H_V -> P0.12 -> HALL_IN1
H_W -> P0.13 -> HALL_IN2
*/
#define H_U_PORT 	GPIO0
#define H_U_PIN		GPIO_Pin_11

#define H_V_PORT 	GPIO0
#define H_V_PIN		GPIO_Pin_12

#define H_W_PORT 	GPIO0
#define H_W_PIN		GPIO_Pin_13

/*
运放
OPW+ -> P3.5	 -> OPA0_IP		-> ADC_CHANNEL_0
OPW- -> P3.7	 -> OPA0_IN		-> ADC_CHANNEL_0
OPV+ -> P3.0	 -> OPA1_IP		-> ADC_CHANNEL_1
OPV- -> P3.1	 -> OPA1_IN		-> ADC_CHANNEL_1
OPU+ -> P3.10	 -> OPA2_IP		-> ADC_CHANNEL_2
OPU- -> P3.11	 -> OPA2_IN		-> ADC_CHANNEL_2
*/
#define OPW_P_PORT	GPIO3
#define OPW_P_PIN	GPIO_Pin_5
#define OPW_N_PORT	GPIO3
#define OPW_N_PIN	GPIO_Pin_7

#define OPV_P_PORT	GPIO3
#define OPV_P_PIN	GPIO_Pin_0
#define OPV_N_PORT	GPIO3
#define OPV_N_PIN	GPIO_Pin_1

#define OPU_P_PORT	GPIO3
#define OPU_P_PIN	GPIO_Pin_10
#define OPU_N_PORT	GPIO3
#define OPU_N_PIN	GPIO_Pin_11

/* 继电器 KM_EN -> P3.14 */
#define KM_EN_PORT	GPIO3
#define KM_EN_PIN	GPIO_Pin_14

/* 继电器 DOWN -> P2.9 */
#define DOWN_PORT	GPIO2
#define DOWN_PIN	GPIO_Pin_9

/* 继电器 UP -> P2.10 */
#define UP_PORT		GPIO2
#define UP_PIN		GPIO_Pin_10

/*
PWM驱动
W_Lin -> P1.9	 -> MCPWM_CH2N
W_Hin -> P1.8	 -> MCPWM_CH2P
V_Lin -> P1.7	 -> MCPWM_CH1N
V_Hin -> P1.6	 -> MCPWM_CH1P
U_Lin -> P1.5	 -> MCPWM_CH0N
U_Hin -> P1.4	 -> MCPWM_CH0P
*/
#define W_Lin_PORT	GPIO1
#define W_Lin_PIN	GPIO_Pin_9
#define W_Hin_PORT	GPIO1
#define W_Hin_PIN	GPIO_Pin_8

#define V_Lin_PORT	GPIO1
#define V_Lin_PIN	GPIO_Pin_7
#define V_Hin_PORT	GPIO1
#define V_Hin_PIN	GPIO_Pin_6

#define U_Lin_PORT	GPIO1
#define U_Lin_PIN	GPIO_Pin_5
#define U_Hin_PORT	GPIO1
#define U_Hin_PIN	GPIO_Pin_4

/* API FUNCTIONS */
void SoftDelay(u32 cnt);
void currentCalcOffsetInit(void);
void OPAOut_OffsetCalibration(void);
void GPIO_ToggleP212(void);
void MCPWM0_RegUpdate(stru_FOC_CurrLoopDef *this);
void Hardware_init(void);

#endif
