#include "hardware_init.h"
#include "led.h"
#include "hall.h"
#include "TaskScheduler.h"
#include "AdcCalc.h"
#include "uart.h"
#include "vofa.h"
#include "Foc.h"
#include "sysdef.h"

#if IF_OPENLOOP
volatile uint16_t gOpenLoopFakeElecAngle = 0;
uint8_t IFStartOpenLoopFlag = 0;
uint8_t IFOpenLoopDoneFlag = 0;
int8_t openLOopPerPWMAngle = 12;//220v市电空载下开环拖，\
								  id_ref给80，ud限制在1400以下，\
								  openLOopPerPWMAngle设2到12之间都可以
#endif
extern struct_task_scheduler g_task_scheduler;
extern strHallProcessObj gStructHallProcess;
extern stru_FOC_CtrProcDef struFOC_CtrProc;
//int16_t pwm_count;
void ADC0_IRQHandler(void)
{
    if (ADC_GetIRQFlag(ADC0, ADC_SF1_IF))
    {
        ADC_ClearIRQFlag(ADC0, ADC_SF1_IF);
				
//		pwm_count = (int16_t)MCPWM0->CNT0;//大约在-2686至-2690之间。\
											该工程mcpwm从-3072计数到3072。\
											adc t0触发点是cnt=40 - MCPWM_PERIOD0，即-3032\
											可由此算出采样时间：采样延迟 = 344 个 PWM 计数值，约3.58us
		
        /* ADC中断和PWM节拍同步，所以在这记“PWM 已更新”的标志及中断计数 */
        g_task_scheduler.pwm_update_flag = 1;
        g_task_scheduler.pwm_update_cnt++;
		
#if IF_OPENLOOP /* IF强拖 */
		if(IFStartOpenLoopFlag)
		{
			//gOpenLoopFakeElecAngle += 170;//约500rpm
			gOpenLoopFakeElecAngle += openLOopPerPWMAngle;
			//开环拖：220v下。id_ref给80，对应ud=1228。把ud限制在1400左右。对应即电流环输出\
			  上跑步机带皮带的情况下，id_ref给70，对应ud=2300。\
			  openLOopPerPWMAngle给到12\
			  注意：foc内部，给d轴通过电流环电流，q轴电流不赋值
		}
#endif

		/* FOC流程 */
		// 相电流计算，计算出来的放到gAdcObj.adc_sample_current_ma[0-2];执行时间约24-26us。优化到15us
		CalcPhaseCurrent();
		
#if HALL_ENABLE
		Hall_PwmCycleCallback(&gStructHallProcess);//电角度在两Hall边沿之间按PWM周期连续推进;执行时间约3us
#endif

		FOC_Model(&struFOC_CtrProc);// 高频电流环主流程在这里执行，频率 = PWM 频率;执行时间约16us
    }
    else
	{
        ADC_ClearIRQFlag(ADC0, ADC_ALL_IF);
	}
}

void MCPWM0_IRQHandler(void)
{
    if (MCPWM_GetIRQFlag(MCPWM0CNT0, MCPWM0_FAIL1_IRQ_IF))
    {
		PWMOutputs(DISABLE);//立即关闭PWM输出
        MCPWM_ClearIRQFlag(MCPWM0CNT0, MCPWM0_FAIL1_IRQ_IF);
    }
}

#if IF_OPENLOOP
uint8_t gOpenLoopHallBuffer[OPENLOOP_BUFFER_SIZE][6];	//Hall值环形缓冲区 (每组6个Hall值)
uint16_t gOpenLoopAngleBuffer[OPENLOOP_BUFFER_SIZE][6];	//角度环形缓冲区 (每组6个角度值)
uint8_t gBufferWritePointer = 0;		//写指针
uint16_t gAngleTemp[6] = {0};			//临时存角度
uint8_t gHallTemp[6] = {0};				//临时存hall值
uint8_t gOpenLoopStudyPointer = 0;		//临时存放的指针
uint8_t gBufferValidGroups = 0;			//有效数据组数
uint8_t gBufferOverflowFlag = 0;		//是否发生过溢出（环形覆盖）
uint8_t gTrig7 = 0;						//触发发送
#endif
void HALL0_IRQHandler(void)
{
#if HALL_ENABLE
    Hall_IrqCallback(&gStructHallProcess);
#endif
#if IF_OPENLOOP		//开环拖动时，hall变化到来，记录下变化时刻的hall值及其对应的假角度值。正反转也需要区分
	/* Hall变化中断 */
	if (HALL_GetIRQFlag(HALL_CAPTURE_EVENT))
	{
		HALL_Clear_IRQ(HALL_CAPTURE_EVENT);//清除HALL信号变化中断标志位
		
		/* 开环拖动 */
#if IF_OPENLOOP	
		if(IFStartOpenLoopFlag)
		{
			gHallTemp[gOpenLoopStudyPointer] = (uint8_t)HALL_GetFilterValue();
			gAngleTemp[gOpenLoopStudyPointer] = gOpenLoopFakeElecAngle;
			//gOpenLoopAngleStudy[]：进入gOpenLoopHallValueStudy[]相同下标下的hall值时的角度值
			
			gOpenLoopStudyPointer++;
			if(gOpenLoopStudyPointer >= 6)
			{
				gOpenLoopStudyPointer = 0;// 完成一组6个点的采集
				
				IFOpenLoopDoneFlag = 1;
				for(uint8_t i = 0; i < 6; i++)//放到环形缓冲区
				{
					gOpenLoopHallBuffer[gBufferWritePointer][i] = gHallTemp[i];
					gOpenLoopAngleBuffer[gBufferWritePointer][i] = gAngleTemp[i];
				}
				
				gBufferWritePointer++;// 更新写指针（环形）
				if(gBufferWritePointer >= OPENLOOP_BUFFER_SIZE)
				{
					gBufferWritePointer = 0;
					gBufferOverflowFlag = 1;  // 标记发生过环形覆盖
				}
				
				if(gBufferValidGroups < OPENLOOP_BUFFER_SIZE)// 更新有效组数
				{
					gBufferValidGroups++;
				}

			}
		}
#endif
	}
	/* Hall超时中断 */
	if (HALL_GetIRQFlag(HALL_OVERFLOW_EVENT))
	{
		HALL_Clear_IRQ(HALL_OVERFLOW_EVENT);//清除HALL超时中断标志位
	}
#endif
}

extern struct_task_scheduler g_task_scheduler;
void TIMER0_IRQHandler(void)
{
    /* 时基500us */
    if (TIM_GetIRQFlag(UTIMER0, TIMER_IF_ZERO))
    {
        TIM_ClearIRQFlag(UTIMER0, Timer_IRQEna_ZC);
        g_task_scheduler.time_base_flag_500us = 1;
        g_task_scheduler.cnt_1++;
        g_task_scheduler.cnt_2++;
        g_task_scheduler.cnt_3++;
        g_task_scheduler.cnt_4++;
    }
}

uint8_t uart1_buff = 0;
void UART1_IRQHandler(void)
{
    if (UART_GetIRQFlag(UART1, UART_IF_SendOver))   // 发送完成中断
    {
        UART_ClearIRQFlag(UART1, UART_IF_SendOver); // 清除发送完成标志位
    }
    if (UART_GetIRQFlag(UART1, UART_IF_RcvOver))    // 接收完成中断
    {
        UART_ClearIRQFlag(UART1, UART_IF_RcvOver);  // 清除接收完成标志位
        uart1_buff = UART_ReadData(UART1);          // 接收 1 Byte数据
    }
}

void DMA0_IRQHandler(void)
{
    uart_1_dma_tx_irq_handler();
}

void HardFault_Handler(void)
{
    MCPWM0_PRT = 0x0000DEAD;
    PWMOutputs(DISABLE);						/* 硬故障时首先关闭功率输出，避免半桥保持未知导通状态 */
	GPIO_ResetBits(KM_EN_PORT, KM_EN_PIN);		/* 主接触器断开，避免故障后母线仍持续加载到功率级 */
	gAdcObj.KM_connect_flag = 0;
    NVIC_SystemReset();							/* 复位 MCU，回到受控上电状态 */
}
