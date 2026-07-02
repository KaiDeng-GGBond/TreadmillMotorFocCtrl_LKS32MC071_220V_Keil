#include "Hall.h"
#include "lks32mc07x_hall.h"
#include "lks32mc07x.h"
#include "sysdef.h"
#include "lks32mc07x_mcpwm.h"
#include "foc.h"
/*
 * Hall测量实现思路
 * 一、时间基
 * Hall外设提供了两个关键量：
 * 1. HALL->WIDTH：刚刚结束的那个Hall扇区持续了多少tick。
 * 2. HALL->INFO[2:0]：当前滤波后的Hall三位码。
 * 本文件的核心做法就是：
 * - 用 WIDTH 做“扇区周期测量”，得到速度；
 * - 用当前Hall状态 + 方向，确定“我现在进入了哪一个60度扇区”；
 * - 再在每个PWM周期里按固定 AngleStepPerPwm 累加 SectorProgress，
 *   从而得到扇区内部的连续电角度。
 * 二、为什么要分“中断更新”和“PWM周期插值”
 * 如果只在Hall跳变时更新角度，那么角度每次只会跳60度，信息太粗。
 * 因此把角度估计拆成：
 * - Hall IRQ：做离散校准。每次跳变都把角度重新拉回真实扇区边界。
 * - PWM callback：做连续推进。每个PWM周期推进一点点角度。
 * 这样即使只是六步Hall，也能得到一个“在相邻两个边沿之间连续变化”的电角度。
 * 三、为什么角度基准选“扇区边界”，不选“扇区中心”
 * 因为Hall跳变这一瞬间最确定的信息就是：转子刚刚跨过一个扇区边界。
 * 所以用 SectorBoundaryAngle + SectorProgress 来表达最直观，也最容易和
 * 六步换相的扇区概念对齐。若后续你希望把角度整体平移到别的参考系，
 * 直接调 AngleOffset 即可。
 */

#define HALL_STATE_MASK				((uint8_t)0x07u)
#define HALL_TIMER_CLOCK_HZ			((uint32_t)MCU_MCLK)	/* 必须与 hardware_init.c 里 Hall时钟分频保持一致 */
#define HALL_PWM_UPDATE_TICKS		((uint32_t)(HALL_TIMER_CLOCK_HZ / PWM_FREQ0)) /* 一个PWM整周期对应多少Hall tick */
#define HALL_ELEC_RPM_FACTOR		((uint32_t)(HALL_TIMER_CLOCK_HZ * 10u)) /* eRPM = HallClk * 60 / (period * 6) = HallClk * 10 / period */

/*
 * Hall状态 -> CW顺序步号映射表
 * 这里不直接存角度，而是先把 Hall 三位码映射成“在当前六步顺序中的第几步”。
 * 这样做的原因是：
 * 1. 方向判定只需要比较“前一步”和“后一步”的先后关系；
 * 2. 这张表直接和 6StepsMotorCtrl.c 里的换相表保持同一个顺序语义。
 * 当前工程里，CW方向的有效Hall序列是：
 * 3 -> 2 -> 6 -> 4 -> 5 -> 1 -> 3 ...
 * 这张表的含义就是：用hall值作为数组下标，去找当前hall值是在该电气圈的哪一步
 * hall_state = 3 时返回 step 1
 * hall_state = 2 时返回 step 2
 * hall_state = 6 时返回 step 3
 * hall_state = 4 时返回 step 4
 * hall_state = 5 时返回 step 5
 * hall_state = 1 时返回 step 6
 * 非法码 0 / 7 返回 0，用于后面统一做异常处理。
 * 如，cw方向实测是326451，定3为第1步，1为第6步
	hall值=1，第6步，则s_hall_step_table_cw[1]要=6
	hall值=2，第2步，则s_hall_step_table_cw[2]=2
	hall值=3，第1步，则s_hall_step_table_cw[2]=1
	hall值=4，第4步，则s_hall_step_table_cw[2]=4
	hall值=5，第5步，则s_hall_step_table_cw[2]=5
	hall值=6，第3步，则s_hall_step_table_cw[2]=3
 */
static const uint8_t s_hall_step_table_cw[8] =
{
	0u, 6u, 2u, 1u, 4u, 5u, 3u, 0u
};

/*
 * 下面两张角度表存的是“进入当前Hall扇区时的边界角度”，而不是扇区中心角。
 * 对同一个Hall状态：
 * - 如果转子按 CW 方向运动，那么它是从该扇区的起始边界进入；
 * - 如果转子按 CCW 方向运动，那么它是从该扇区的结束边界进入。
 * 所以需要分成 CW / CCW 两张表来保存“这个方向下，刚进入扇区时应当对齐到哪个边界”。
 * 这样 Hall 一跳变，就能把 SectorProgress 清零，再从这个边界开始做插值。
 */
/* 该跑步机定义正常跑步的电机方向为CW，对应的hall值顺序为326451 */
/* 用hall值作为数组下标，新hall值对应的下标应为其对应的角度值 */
//IF拖d轴得到的角度
#if 0
static const uint16_t s_hall_sector_enter_angle_cw[8] =
{
	0u,
	24698u,
	46929u,
	34062u,
	1537u,
	14151u,
	57603u,
	0u
};
static const uint16_t s_hall_sector_enter_angle_ccw[8] =
{
	0u,
	30231u,
	51235u,
	41417u,
	8721u,
	18414u,
	63043u,
	0u
};
static const uint16_t s_hall_sector_center_angle[8] =
{
    0u,
    27464u,
    49082u,
    37739u,
    5129u,
    16282u,
    60323u,
    0u
};
#else	//跑步机上的电机
static const uint16_t s_hall_sector_enter_angle_cw[8] =
{
	0u,
	24357u,
	46616u,
	33437u,
	2535u,
	11956u,
	55191u,
	0u
};
static const uint16_t s_hall_sector_enter_angle_ccw[8] =
{
    0u,
    30629u,    // Hall = 0x01
    52296u,    // Hall = 0x02
    43170u,    // Hall = 0x03
    8718u,     // Hall = 0x04
    21077u,    // Hall = 0x05
    65216u,    // Hall = 0x06
    0u

};
static const uint16_t s_hall_sector_center_angle[8] =
{
    0u,
    27493u,    // Hall = 0x01
    49456u,    // Hall = 0x02
    38303u,    // Hall = 0x03
    5626u,     // Hall = 0x04
    16516u,    // Hall = 0x05
    60167u,    // Hall = 0x06
    0u
};
#endif
//ccw表会与cw表差约30度

/* 根据Hall三位码查当前落在CW序列的第几步。 */
static uint8_t Hall_GetStepFromState(uint8_t hall_state)
{
	return s_hall_step_table_cw[hall_state & HALL_STATE_MASK];
}

/*
 * 在还没有观察到足够的Hall跳变之前，方向可能暂时未知。
 * 这时候优先使用 obj 里已经测出来的方向；如果还没有，再退回到当前控制命令方向。
 * 这样可以让系统在启动初期也先把角度插值跑起来，等真实方向建立后再覆盖。
 */
static uint8_t Hall_GetFallbackDirection(const strHallProcessObj *obj)
{
	if ((obj->Direction == CW) || (obj->Direction == CCW))
	{
		return obj->Direction;
	}
	return CW;
}

/*
 * 方向判定规则
 *
 * 这里不比较原始Hall三位码，而是比较“步号”：
 * - prev -> curr 正好是顺序表里的下一步：说明是 CW
 * - curr -> prev 正好是顺序表里的下一步：说明是 CCW
 * 之所以这样做，是因为 Hall码本身并不是自然递增顺序，直接比较数值没有意义；
 * 先映射成 step 以后，前后关系就清楚了。
 * 返回 0 表示这次跳变不合法，例如：
 * - 0/7 非法Hall码
 * - 连续两次读到同一步
 * - 一次跳过了不止一个扇区
 */
static uint8_t Hall_GetDirectionFromTransition(uint8_t prev_step, uint8_t curr_step)
{
	if ((prev_step == 0u) || (curr_step == 0u) || (prev_step == curr_step))
	{
		return 0u;
	}

	if (((prev_step % 6u) + 1u) == curr_step)
	{
		return CW;
	}

	if (((curr_step % 6u) + 1u) == prev_step)
	{
		return CCW;
	}
	return 0u;
}

/* 清空周期FIFO；一般在速度失效、状态非法或超时后调用。 */
static void Hall_ClearPeriodFilter(strHallProcessObj *obj)
{
	uint8_t i;

	obj->HallPeriodTick = 0u;
	obj->HallPeriodTickAvg = 0u;
	obj->HallPeriodTickSum = 0u;
	obj->HallPeriodTickIndex = 0u;
	obj->HallPeriodTickCount = 0u;

	for (i = 0u; i < HALL_SPEED_FIFO_SIZE; i++)
	{
		obj->HallPeriodTickBuf[i] = 0u;
	}
}

/*
 * 把最新一次扇区周期压入滑动平均FIFO。
 */
static uint32_t Hall_PushPeriodSample(strHallProcessObj *obj, uint32_t hall_period_tick)
{
	uint8_t idx;

	idx = obj->HallPeriodTickIndex;

	if (obj->HallPeriodTickCount < HALL_SPEED_FIFO_SIZE)
	{
		obj->HallPeriodTickCount++;
	}
	else
	{
		obj->HallPeriodTickSum -= obj->HallPeriodTickBuf[idx];
	}

	obj->HallPeriodTickBuf[idx] = hall_period_tick;
	obj->HallPeriodTickSum += hall_period_tick;

	idx++;
	if (idx >= HALL_SPEED_FIFO_SIZE)
	{
		idx = 0u;
	}
	obj->HallPeriodTickIndex = idx;

	return (obj->HallPeriodTickSum / obj->HallPeriodTickCount);
}

/*
 * 安装补偿本质上是“最终输出角度整体加一个相位偏置”。
 * 角度采用 uint16 表示，因此直接按 16bit 回绕即可。
 */
static uint16_t Hall_ApplyAngleOffset(uint16_t angle, int32_t angle_offset)
{
	return (uint16_t)(((uint32_t)angle + (uint32_t)((uint16_t)angle_offset)) & 0xFFFFu);
}

/*
 * 根据当前扇区边界 + 当前扇区进度，重建最终电角度。
 * - CW：角度应该从边界开始正向增加
 * - CCW：角度应该从边界开始反向减小
 * 这里先算未补偿角度，再统一叠加安装偏置。
 */
extern stru_FOC_CtrProcDef struFOC_CtrProc;
static void Hall_RebuildElectricalAngle(strHallProcessObj *obj)
{
	uint16_t angle_no_offset;
	uint8_t dir;

	dir = Hall_GetFallbackDirection(obj);
	if (dir == CCW)
	{
		angle_no_offset = (uint16_t)(obj->SectorBoundaryAngle - obj->SectorProgress) + \
		obj->PhaseShiftOffset;
	}
	else
	{
		angle_no_offset = (uint16_t)(obj->SectorBoundaryAngle + obj->SectorProgress) + \
		obj->PhaseShiftOffset;
	}

	obj->ElecAngleNoOffset = angle_no_offset;
	obj->ElecAngle = Hall_ApplyAngleOffset(angle_no_offset, obj->MechAngleOffset);//AngleOffset=32768
}

/*
 * Hall每跳变一次，就说明“刚刚进入了一个新的60度扇区”。
 * 所以这里要做两件事：
 * 1. 把扇区边界切换到新的进入边界；
 * 2. 把扇区内进度清零。
 */
static void Hall_SetSectorBoundary(strHallProcessObj *obj, uint8_t hall_state, uint8_t dir)
{
	if (dir == CCW)
	{
		obj->SectorBoundaryAngle = s_hall_sector_enter_angle_ccw[hall_state & HALL_STATE_MASK];
	}
	else
	{
		obj->SectorBoundaryAngle = s_hall_sector_enter_angle_cw[hall_state & HALL_STATE_MASK];
	}

#if 1
	//希望在起步时，在没有hall变化之前，用该hall扇区的中心角度替代Hall_PwmCycleCallback()获取的边界角度
	if((obj->SpeedValid == 0u) || (ABS(obj->AngleStepPerPwm) <= 10))
	{
		obj->SectorBoundaryAngle = s_hall_sector_center_angle[hall_state & HALL_STATE_MASK];
	}
#endif

	obj->SectorProgress = 0u;
	
	Hall_RebuildElectricalAngle(obj);
}

/*
 * 利用平均扇区周期同时更新“速度”和“每PWM周期角度增量”。
 * 公式说明：
 * 1. 一个Hall扇区固定对应 60 电角度。
 *    所以每个PWM周期推进多少电角度：
 *      AngleStepPerPwm = 60deg * PWM周期 / Hall扇区周期
 * 2. 一个电周包含 6 个Hall扇区。
 *    所以电转速：
 *      eRPM = Hall时钟频率 * 60 / (Hall扇区周期 * 6)
 *           = Hall时钟频率 * 10 / Hall扇区周期
 * 3. 机械转速：
 *      RPM = eRPM / 极对数
 * 为什么速度和角度增量都基于 HallPeriodTickAvg，而不是一个用瞬时值一个用平均值？
 * 因为这样能保证“转速显示”和“角度插值速度”来自同一个时间基，避免两者抖动特性不一致。
 */
static void Hall_UpdateSpeedInfo(strHallProcessObj *obj)
{
	uint32_t abs_elec_rpm;
	int32_t signed_elec_rpm;
	uint8_t dir;

	if (obj->HallPeriodTickAvg == 0u)
	{
		obj->AngleStepPerPwm = 0u;
		obj->ElecSpeedRpm = 0;
		obj->MechSpeedRpm = 0;
		return;
	}

	obj->AngleStepPerPwm = (uint16_t)((uint32_t)(S16_60_PHASE_SHIFT * HALL_PWM_UPDATE_TICKS) / obj->HallPeriodTickAvg);
	if (obj->AngleStepPerPwm > S16_60_PHASE_SHIFT)
	{
		obj->AngleStepPerPwm = S16_60_PHASE_SHIFT;
	}

	abs_elec_rpm = HALL_ELEC_RPM_FACTOR / obj->HallPeriodTickAvg;
	dir = Hall_GetFallbackDirection(obj);
	signed_elec_rpm = (dir == CCW) ? (int32_t)abs_elec_rpm : -(int32_t)abs_elec_rpm;

	obj->ElecSpeedRpm = signed_elec_rpm;
	obj->MechSpeedRpm = signed_elec_rpm / (int32_t)HALL_MOTOR_POLE_PAIRS;
}

/*
 * 当出现以下情况时，当前速度估计应当作废：
 * - Hall超时（长时间没有新边沿）
 * - 读到非法Hall码
 *
 * 这里有意“不去改动最后一个角度”，只把速度和插值步长清零。
 * 这样做的含义是：角度保持在最后一次可信估计的位置，不再继续外推。
 */
static void Hall_InvalidateMeasurement(strHallProcessObj *obj)
{
	obj->SpeedValid = 0u;
	obj->AngleStepPerPwm = 0u;
	obj->ElecSpeedRpm = 0;
	obj->MechSpeedRpm = 0;
	Hall_ClearPeriodFilter(obj);
}

/*
 * 启动初期可能还没来得及等到第一次Hall边沿，但主循环/中断已经开始调用角度插值接口。
 * 这时先从硬件寄存器同步一次“当前Hall状态”，至少先建立当前扇区。
 *
 * 注意：
 * 这里只能建立“当前位置所在扇区”，还不能可靠建立速度；
 * 真正的速度仍然要等第一个有效扇区周期（WIDTH）到来后才能计算。
 */
static void Hall_SyncStateFromHardware(strHallProcessObj *obj)
{
	uint8_t hall_state;
	uint8_t hall_step;

	hall_state = (uint8_t)(HALL_GetFilterValue() & HALL_STATE_MASK);
	hall_step = Hall_GetStepFromState(hall_state);

	if (hall_step == 0u)
	{
		return;
	}

	obj->HallState = hall_state;
	obj->HallStateRaw = (uint8_t)(HALL_GetCaptureValue() & HALL_STATE_MASK);
	obj->HallPrevState = hall_state;
	obj->HallStep = hall_step;
	obj->Direction = Hall_GetFallbackDirection(obj);
	obj->StateValid = 1u;
	Hall_SetSectorBoundary(obj, hall_state, obj->Direction);
}


/****************************************************** 对外函数 **************************************************************/
/**
 * @brief   lks32mc071的hall中断回调函数，包含hall更新中断及超时中断的处理
 * @param   strHallProcessObj *obj Hall处理结构体指针，包含状态、方向、错误计数等信息
 * @return  none
 * @retval  none
 * @note    hall中断中调用，读取hall状态，计算对应的电角度、方向、速度
 * @warning 该函数涉及全局表项操作，不可重入
 * @see 	strHallProcessObj
 */
void Hall_IrqCallback(strHallProcessObj *obj)
{
	uint8_t temp_hall_state;
	uint8_t temp_hall_step;
	uint8_t temp_dir;
	uint32_t temp_hall_period_tick;
	
	/* Hall 变化中断 */
	if (HALL_GetIRQFlag(HALL_CAPTURE_EVENT))
	{
		HALL_Clear_IRQ(HALL_CAPTURE_EVENT);//清除HALL信号变化中断标志位
		
		/* 1. 读取当前滤波后的Hall状态，以及“刚刚结束的那个扇区周期”。 */
		temp_hall_state = (uint8_t)(HALL_GetFilterValue() & HALL_STATE_MASK);
		temp_hall_step = Hall_GetStepFromState(temp_hall_state);
		temp_hall_period_tick = (HALL->WIDTH); /* WIDTH 的有效位在硬件上是24bit，这里直接取寄存器值即可。 */
		obj->HallStateRaw = (uint8_t)(HALL_GetCaptureValue() & HALL_STATE_MASK);
		
		/* 2. 非法Hall码（通常是0或7）直接判为当前状态无效
		 *    这里不尝试“修复”它，而是明确进入失效状态，方便外部更早发现接线/噪声问题 */
		if (temp_hall_step == 0u)
		{
			obj->HallPrevState = obj->HallState;
			obj->HallState = temp_hall_state;
			obj->StateValid = 0u;
			obj->ErrorCount++;
			Hall_InvalidateMeasurement(obj);
			switch (obj->ErrorCount)
			{
				case 1:  // 第1次：警告，继续用上次有效状态
					obj->HallState = obj->HallPrevState;  // 保持之前的状态
					obj->StateValid = 0u;
					break;
				case 2:  // 第2次：警告，继续用上次有效状态
					obj->HallState = obj->HallPrevState;  // 保持之前的状态
					obj->StateValid = 0u;
					break;
				default:  // 3次以上：报错,停机
					
					obj->FaultState = 1;
					Hall_InvalidateMeasurement(obj);
					return;
			}
			return;
		}

		/* 判方向
		 *    - 如果这是第一次建立有效状态，还没有前后两拍可比较，只能先用fallback方向
		 *    - 如果已经有上一拍，就用前后步号关系判断真实方向 */
		if (obj->StateValid == 0u)
		{
			obj->Direction = Hall_GetFallbackDirection(obj);
		}
		else
		{
			temp_dir = Hall_GetDirectionFromTransition(obj->HallStep, temp_hall_step);
			if (temp_dir == 0u)
			{
				obj->ErrorCount++;
			}
			else
			{
				obj->Direction = temp_dir;
			}
		}

		/* 更新当前有效状态，供后续角度重建和调试使用 */
		obj->HallPrevState = obj->HallState;
		obj->HallState = temp_hall_state;
		obj->HallStep = temp_hall_step;
		obj->StateValid = 1u;

		/* 用刚结束扇区的周期更新速度。这里更新出来的不仅是转速，还包括每个PWM周期应该推进多少电角度 */
		if (temp_hall_period_tick > 0u)
		{
			obj->HallPeriodTick = temp_hall_period_tick;
			obj->HallPeriodTickAvg = Hall_PushPeriodSample(obj, temp_hall_period_tick);
			obj->SpeedValid = 1u;
			Hall_UpdateSpeedInfo(obj);
		}

		/*  Hall跳变意味着进入一个新的60度扇区,所以把扇区边界切换到当前扇区入口，并把 SectorProgress 归零 */
		Hall_SetSectorBoundary(obj, temp_hall_state, Hall_GetFallbackDirection(obj));
	}
	
	/* Hall超时中断 */
	if (HALL_GetIRQFlag(HALL_OVERFLOW_EVENT))
	{
		/* 超时的意义是：当前速度已经不再可信，但最后一次角度可以保留，不继续向前外推。 */
		HALL_Clear_IRQ(HALL_OVERFLOW_EVENT);//清除HALL超时中断标志位
		Hall_InvalidateMeasurement(obj);//hall超时中断会不会进入太快，影响正常的低速旋转?
	}
}

/*
 * 每个PWM周期调用一次，用于在相邻两个Hall边沿之间做连续角度插值。
 * - 如果还没有建立有效Hall状态，先从硬件同步当前状态；
 * - 如果当前速度无效，就不继续外推，只保持最后可信角度；
 * - 如果速度有效，就用 AngleStepPerPwm 推进 SectorProgress；
 * - 但 SectorProgress 最多只能推进到 60 电角度，不越过当前扇区。
 */
void Hall_PwmCycleCallback(strHallProcessObj *obj)
{
	uint16_t next_progress;

	if (obj->StateValid == 0u)//没有建立有效Hall状态，先从硬件同步当前状态
	{
		Hall_SyncStateFromHardware(obj);
		return;
	}

	if ((obj->SpeedValid == 0u) || (obj->AngleStepPerPwm == 0u))//速度无效，就不继续外推，只保持最后可信角度
	{
		Hall_RebuildElectricalAngle(obj);
		return;
	}

	/* 正常在跑，速度有效，就用 AngleStepPerPwm 推进 SectorProgress */
	next_progress = (uint16_t)(obj->SectorProgress + obj->AngleStepPerPwm);
	
	if (next_progress >= S16_60_PHASE_SHIFT)//不允许加超过60度
	{
		next_progress = S16_60_PHASE_SHIFT;
	}

	obj->SectorProgress = next_progress;
	
	Hall_RebuildElectricalAngle(obj);
}
