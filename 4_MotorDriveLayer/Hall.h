#ifndef __HALL_H__
#define __HALL_H__

#include <stdint.h>

/*
 * Hall测量模块设计说明
 *
 * 1. 本模块把“Hall边沿事件处理”和“PWM周期内角度连续插值”拆成了两部分：
 *    - Hall_IrqCallback() 只在Hall跳变/超时中断里运行，负责做离散事件更新。
 *      这里会完成 Hall状态读取、方向判定、周期测量、转速换算、扇区边界重定位。
 *    - Hall_PwmCycleCallback() 在每个PWM周期运行一次，负责做连续角度插值。
 *      这里不会重新做测速，而是利用上一拍已经算好的 AngleStepPerPwm 让电角度平滑推进。
 *
 * 2. 电角度不是直接把某个Hall状态粗暴映射成一个固定角度，而是采用“60度扇区边界 + 扇区内进度”的表示法：
 *    - Hall每变化一次，说明转子刚进入了一个新的60电角度扇区。
 *    - 于是把 SectorBoundaryAngle 置为该扇区的进入边界，把 SectorProgress 清零。
 *    - 后续每个PWM周期只累加 SectorProgress，直到下一个真实Hall边沿到来。
 *    这样做的好处是：思路直观、与六步换相一致、也便于后续继续扩展到更平滑的位置估计。
 *
 * 3. 转速测量使用 HALL 外设的 WIDTH 寄存器：
 *    - WIDTH 保存“刚刚结束的那个Hall扇区持续了多少HALL时钟tick”。
 *    - 一个Hall扇区固定对应 60 电角度，因此可直接把这个周期换算成电转速。
 *    - 为了抑制抖动，周期会进入一个长度为 HALL_SPEED_FIFO_SIZE 的滑动平均FIFO。
 *
 * 4. 机械转速 = 电转速 / 极对数。
 *    当前默认极对数由 HALL_MOTOR_POLE_PAIRS 给出；如果换了电机，最需要确认的就是这里。
 *
 * 5. 角度采用 S16 标幺表示：
 *    - 0 ~ 65535 对应 0 ~ 360 电角度。
 *    - 这样既能自然利用 uint16_t 回绕，又方便和后续控制算法统一角度表示。
 */

/* 1度角归一化处理 1/360*65536, 角度0~360度变化， 对应归一化数据0~65535 */
#define S16_360_PHASE_SHIFT                     (uint16_t)(65535)
#define S16_330_PHASE_SHIFT                     (uint16_t)(60075)
#define S16_318_PHASE_SHIFT                     (uint16_t)(58000)
#define S16_315_PHASE_SHIFT                     (uint16_t)(57344)
#define S16_300_PHASE_SHIFT                     (uint16_t)(54613)
#define S16_270_PHASE_SHIFT                     (uint16_t)(49152)
#define S16_240_PHASE_SHIFT                     (uint16_t)(43691)
#define S16_210_PHASE_SHIFT                     (uint16_t)(38220)
#define S16_180_PHASE_SHIFT                     (uint16_t)(32768)
#define S16_150_PHASE_SHIFT                     (uint16_t)(27300)
#define S16_120_PHASE_SHIFT                     (uint16_t)(21845)
#define S16_90_PHASE_SHIFT                      (uint16_t)(16384)
#define S16_75_PHASE_SHIFT                      (uint16_t)(13653)
#define S16_70_PHASE_SHIFT                      (uint16_t)(12743)
#define S16_63_PHASE_SHIFT                      (uint16_t)(11468)
#define S16_60_PHASE_SHIFT                      (uint16_t)(10923)
#define S16_45_PHASE_SHIFT                      (uint16_t)(8192)
#define S16_30_PHASE_SHIFT                      (uint16_t)(5461)
#define S16_20_PHASE_SHIFT                      (uint16_t)(3641)
#define S16_15_PHASE_SHIFT                      (uint16_t)(2731)
#define S16_10_PHASE_SHIFT                      (uint16_t)(1820)
#define S16_5_PHASE_SHIFT                       (uint16_t)(910)
#define S16_1_PHASE_SHIFT                       (uint16_t)(182)

//要计算：每个PWM周期转过多少电角度（Dpp） = ROTOR_SPEED_FACTOR / Hall周期
//65536代表360度，则每个hall状态60度对应的是65536 / 6 = 10923
//*96是因为1us内的cpu周期是96次(96MHz)
//ROTOR_SPEED_FACTOR_1US：保留这个历史换算系数，便于理解“Hall周期越短，角速度越大”这个关系
#define ROTOR_SPEED_FACTOR_1US	(int32_t)(10923 * 96)

#ifndef HALL_MOTOR_POLE_PAIRS
#define HALL_MOTOR_POLE_PAIRS	((uint8_t)5)  /* 机械转速换算默认极对数，需按实际电机修改 */
#endif

#define HALL_SPEED_FIFO_SIZE	((uint8_t)6)/* Hall周期均值滤波FIFO长度，6个扇区做一次滑动平均 */

typedef struct
{
	volatile uint8_t HallState;			/* 当前滤波后的Hall状态；后续所有查表优先用它，而不是原始毛刺状态 */

	volatile uint8_t HallStateRaw;		/* 当前未滤波的Hall状态；主要用于调试/对比硬件滤波效果 */
	volatile uint8_t HallPrevState;		/* 上一次有效Hall状态；用于观察状态切换和定位异常跳变 */
	volatile uint8_t HallStep;			/* 当前Hall在“CW顺序表”中的步号，1~6有效，0表示非法状态 */
	volatile uint8_t Direction;			/* 实测方向：CW / CCW；在还没观察到有效跳变前可能为0 */
	volatile uint8_t StateValid;		/* 当前Hall状态是否可信；遇到非法码（0/7）时会清零 */
	volatile uint8_t SpeedValid;		/* 当前速度是否可信；超时或状态非法时会清零 */
	
	/* 周期/速度测量 */
	volatile uint32_t HallPeriodTick;								/* 最新一次Hall扇区周期，来自 HALL->WIDTH，单位：HALL时钟tick */
	volatile uint32_t HallPeriodTickAvg;							/* Hall扇区周期均值；速度和角度插值都基于它，避免两套时间基不一致 */
	volatile uint32_t HallPeriodTickBuf[HALL_SPEED_FIFO_SIZE];		/* Hall周期FIFO，用于做滑动平均 */
	volatile uint32_t HallPeriodTickSum;							/* FIFO求和缓存，避免每次都遍历整段Buffer */
	volatile uint8_t HallPeriodTickIndex;							/* FIFO写指针 */
	volatile uint8_t HallPeriodTickCount;							/* FIFO当前有效样本数量；启动初期可能小于 HALL_SPEED_FIFO_SIZE */

	/* 电角度 */
	volatile uint16_t ElecAngle;			/* 带安装补偿后的最终电角度，通常给外部模块直接读取 */
	volatile uint16_t ElecAngleNoOffset;	/* 未加安装补偿的电角度，便于区分“算法本体”和“安装修正” */
	volatile uint16_t SectorBoundaryAngle;	/* 当前60度扇区的进入边界角度；Hall一跳变就会重置到新的边界 */
	volatile uint16_t SectorProgress;		/* 当前扇区内已经推进的电角度；每个PWM周期累加一次 */
	volatile uint16_t AngleStepPerPwm;		/* 每个PWM周期推进的电角度；本质上是“离散化后的电角速度” */
	volatile uint16_t PhaseShiftOffset;		/* hall换相时的电角度偏移值 */
	
	/* 转速 */
	volatile int32_t ElecSpeedRpm;			/* 电转速，单位：eRPM；1个电周对应6个Hall扇区 */
	volatile int32_t MechSpeedRpm;			/* 机械转速，单位：RPM；由 eRPM / 极对数得到 */
	
	/* 角度偏移（安装误差补偿） */
	volatile int32_t MechAngleOffset;		/* Hall安装误差补偿，单位：S16角度；正负都可通过 uint16 回绕自然表达 */
	uint16_t PhaseShift;					/* 当前Hall角度计算角度偏移值 */
	volatile uint16_t ErrorCount;			/* Hall非法状态/非法跳变计数；用于调试接线/滤波/抗干扰效果 */
	
	uint16_t FaultState;					/* 记录错误信息 */

} strHallProcessObj;
extern strHallProcessObj gStructHallProcess;/* Hall信号处理结构体 */

/*
 * Hall_IrqCallback()
 * 作用：
 * 1. 在Hall跳变时读取当前状态；
 * 2. 根据前后两个有效状态推断旋转方向；
 * 3. 读取刚结束扇区的周期，更新平均周期；
 * 4. 根据平均周期计算转速与每PWM周期角度增量；
 * 5. 把角度重新对齐到“新扇区的进入边界”。
 */
void Hall_IrqCallback(strHallProcessObj *obj);

/*
 * Hall_PwmCycleCallback()
 * 作用：
 * 1. 在相邻两个Hall边沿之间做连续角度插值；
 * 2. 如果速度当前无效，则保持角度冻结，不继续外推；
 * 3. 如果还没有建立有效状态，则先从硬件寄存器同步一次当前Hall状态。
 */
void Hall_PwmCycleCallback(strHallProcessObj *obj);



#endif
