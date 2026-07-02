#include "Svpwm.h"

/* 生成SVPWM波形 */
void SVPWM_2PH(stru_FOC_CtrProcDef *pCtrProc)
{
   volatile s32 wX, wY, wZ, wUAlpha, wUBeta;
	
	stru_FOC_CurrLoopDef *this;
    this = &pCtrProc->struFOC_CurrLoop;
	
	/* 先把 alpha/beta 电压转换成扇区判定坐标 */
	// struVolt_alfa_beta 存的是Q15归一化后的电压指令
	wUAlpha = (this->struVolt_alfa_beta.qV_Value1 * 28378) >> 15;//alpha电压
    wUBeta = (this->struVolt_alfa_beta.qV_Value2 >> 1);//beta电压
	
	/* 决定扇区 */
	wX = this->struVolt_alfa_beta.qV_Value2; /* REF1 = wX */
	wY = (wUBeta + wUAlpha); /* REF3 = -wY */
	wZ = (wUBeta - wUAlpha);
	
	if (wY > 0)
	{
		if (wZ > 0)
			this->bSector = 2;
		else
		{
			if (wX <= 0)
				this->bSector = 6;
			else
				this->bSector = 1;
		}
	}
	else
	{
		if (wZ <= 0)
			this->bSector = 5;
		else
		{
			if (wX <= 0)
				this->bSector = 4;
			else
				this->bSector = 3;
		}
	}
	
	switch (this->bSector) /* 根据所处扇区分配三相作用时间 */
	{
		case 1:
			this->struVoltUVW_PWM.w = 0; /* T1 = 0 */
			this->struVoltUVW_PWM.v = wX;/* T2 = T_1 */
			this->struVoltUVW_PWM.u = this->struVoltUVW_PWM.v - wZ;  /* T3 = T2 + T_2 */
			break;

		case 2:
			this->struVoltUVW_PWM.w = 0; /* T1 = 0 */
			this->struVoltUVW_PWM.u = wY;/* T2 = T_1 */
			this->struVoltUVW_PWM.v = this->struVoltUVW_PWM.u + wZ;  /* T3 = T2 + T_2 */
			break;

		case 3:
			this->struVoltUVW_PWM.u = 0; /* T1 = 0 */
			this->struVoltUVW_PWM.w = -wY;/* T2 = T_1 */
			this->struVoltUVW_PWM.v = this->struVoltUVW_PWM.w + wX;  /* T3 = T2 + T_2 */
			break;

		case 4:
			this->struVoltUVW_PWM.u = 0; /* T1 = 0 */
			this->struVoltUVW_PWM.v = wZ;/* T2 = T_1 */
			this->struVoltUVW_PWM.w = this->struVoltUVW_PWM.v - wX;  /* T3 = T2 + T_2 */
			break;

		case 5:
			this->struVoltUVW_PWM.v = 0; /* T1 = 0 */
			this->struVoltUVW_PWM.u = -wZ;/* T2 = T_1 */
			this->struVoltUVW_PWM.w = this->struVoltUVW_PWM.u - wY;  /* T3 = T2 + T_2 */
			break;

		case 6:
			this->struVoltUVW_PWM.v = 0; /* T1 = 0 */
			this->struVoltUVW_PWM.w = -wX;/* T2 = T_1 */
			this->struVoltUVW_PWM.u = this->struVoltUVW_PWM.w + wY;  /* T3 = T2 + T_2 */
			break;

		default:
			break;
	}
	
	this->struVoltUVW_PWM.u = (this->struVoltUVW_PWM.u * MCPWM_PERIOD0) >> 15; /* 归一化电压 -> PWM 计数值 */
	this->struVoltUVW_PWM.v = (this->struVoltUVW_PWM.v * MCPWM_PERIOD0) >> 15;
	this->struVoltUVW_PWM.w = (this->struVoltUVW_PWM.w * MCPWM_PERIOD0) >> 15;
}

void SVPWM_2PH_2(stru_FOC_CtrProcDef *pCtrProc)
{
    s32 wX, wY, wZ, wUAlpha, wUBeta;
    s32 Ta, Tb, Tc;
    s32 T1, T2, T0;
    
    stru_FOC_CurrLoopDef *this;
    this = &pCtrProc->struFOC_CurrLoop;

    /* 坐标变换 */
    wUAlpha = (this->struVolt_alfa_beta.qV_Value1 * 28378) >> 15; // √3/2
    wUBeta  = (this->struVolt_alfa_beta.qV_Value2 >> 1);          // 1/2

    wX = this->struVolt_alfa_beta.qV_Value2;
    wY = wUBeta + wUAlpha;
    wZ = wUBeta - wUAlpha;

    /* 扇区判断 */
    if (wY > 0)
    {
        if (wZ > 0) this->bSector = 2;
        else this->bSector = (wX <= 0) ? 6 : 1;
    }
    else
    {
        if (wZ <= 0) this->bSector = 5;
        else this->bSector = (wX <= 0) ? 4 : 3;
    }

    /* 计算 T1 T2 */
    switch (this->bSector)
    {
        case 1: T1 = wX;     T2 = -wZ;    break;
        case 2: T1 = wY;     T2 = wZ;     break;
        case 3: T1 = -wY;    T2 = wX;     break;
        case 4: T1 = wZ;     T2 = -wX;    break;
        case 5: T1 = -wZ;    T2 = -wY;    break;
        case 6: T1 = -wX;    T2 = wY;     break;
        default: T1 = 0; T2 = 0; break;
    }

    /* 归一化到PWM周期 */
    T1 = (T1 * MCPWM_PERIOD0) >> 15;
    T2 = (T2 * MCPWM_PERIOD0) >> 15;

    /* 零矢量时间 */
    T0 = MCPWM_PERIOD0 - T1 - T2;

    /* 中心对称分配 */
    Ta = T0/2;
    Tb = Ta + T1;
    Tc = Tb + T2;

    /* 根据扇区映射到UVW */
    switch (this->bSector)
    {
        case 1:
            this->struVoltUVW_PWM.u = Tc;
            this->struVoltUVW_PWM.v = Tb;
            this->struVoltUVW_PWM.w = Ta;
            break;

        case 2:
            this->struVoltUVW_PWM.u = Tb;
            this->struVoltUVW_PWM.v = Tc;
            this->struVoltUVW_PWM.w = Ta;
            break;

        case 3:
            this->struVoltUVW_PWM.u = Ta;
            this->struVoltUVW_PWM.v = Tc;
            this->struVoltUVW_PWM.w = Tb;
            break;

        case 4:
            this->struVoltUVW_PWM.u = Ta;
            this->struVoltUVW_PWM.v = Tb;
            this->struVoltUVW_PWM.w = Tc;
            break;

        case 5:
            this->struVoltUVW_PWM.u = Tb;
            this->struVoltUVW_PWM.v = Ta;
            this->struVoltUVW_PWM.w = Tc;
            break;

        case 6:
            this->struVoltUVW_PWM.u = Tc;
            this->struVoltUVW_PWM.v = Ta;
            this->struVoltUVW_PWM.w = Tb;
            break;
    }
}
