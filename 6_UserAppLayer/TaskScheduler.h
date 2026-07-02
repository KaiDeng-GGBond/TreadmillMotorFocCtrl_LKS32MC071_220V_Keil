#ifndef __TASH_SCHEDULER_H__
#define __TASH_SCHEDULER_H__

#include <stdint.h>

typedef struct
{
	uint8_t		time_base_flag_500us;	/* 每次进入中断就置1 */
	uint8_t		pwm_update_flag;		/* PWM周期更新标志，一次间隔为一个PWM周期 */
	uint32_t	pwm_update_cnt;			/* PWM周期更新次数计数 */
	uint8_t		cnt_1;					/* 计数器1 */
	uint16_t	cnt_2;					/* 计数器2 */
	uint16_t	cnt_3;					/* 计数器3 */
	uint16_t	cnt_4;					/* 计数器4 */
	uint32_t	run_time_ms;			/* 系统总运行时间(ms) */
} struct_task_scheduler;
extern struct_task_scheduler g_task_scheduler;

void task_cal_phase_current(void);
void task_scheduler(void);

#endif
