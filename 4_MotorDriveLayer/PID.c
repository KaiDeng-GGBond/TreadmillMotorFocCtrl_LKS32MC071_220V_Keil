#include "PID.h"
#include "sysdef.h"

int16_t PI_Controller(int16_t ref, int16_t fb, stru_PIparams *stru_PI)
{
    int32_t InputError = 0;//输入误差
    int32_t P, PIoutput, I, DumpPortion;
	int64_t I_state ;
	int32_t tmp;//存储p项的临时值
	
    /* error (Q15) */
    InputError = (int32_t)ref - (int32_t)fb;//q15运算
	
    /* P = Kp * error >> 15 */
    tmp = (int32_t)stru_PI->hKp_Gain * InputError;//q15运算，两个q15相乘要除于一次2^15
    P = tmp >> stru_PI->hKp_Divisor;
	
	/* I accumulation */
    tmp = (int32_t)stru_PI->hKi_Gain * InputError;//q15运算，两个q15相乘要除于一次2^15
    I = tmp >> stru_PI->hKi_Divisor;
	
    /* 累计积分 I(k)=I(k-1)+Kie(k) */
    I_state  = (int64_t)I + stru_PI->wIntegral;//本次积分值加旧的积分值	//q15运算
	
    /* 累计积分限幅 */
    if (I_state  > stru_PI->wUpper_Limit_Integral)
        I = stru_PI->wUpper_Limit_Integral;
    else if (I_state  < stru_PI->wLower_Limit_Integral)
        I = stru_PI->wLower_Limit_Integral;
	else 
		I = (int32_t)I_state ;

    /* 输出等于P项+I项 */
	PIoutput = P + I;

    /* controller output limitation */
	//上限处理
    if (PIoutput > stru_PI->hUpper_Limit_Output)
    {
		//如果pioutput超出输出限幅最大值，则先计算超出了多少，再输出最大值
        DumpPortion = PIoutput - (stru_PI->hUpper_Limit_Output); /* 超出的部分会反向抵消积分，防止继续风up */
        PIoutput = stru_PI->hUpper_Limit_Output;
		
		//输出超多少，就把积分反向减掉多少
        if (I > 0)
        {
            I = I - DumpPortion;
        }
    }
	//下限处理
    else if (PIoutput < stru_PI->hLower_Limit_Output)
    {
        DumpPortion = PIoutput - (stru_PI->hLower_Limit_Output);
        PIoutput = stru_PI->hLower_Limit_Output;
		
        if (I < 0)
        {
            I = I - DumpPortion;
        }
    }

    stru_PI->wIntegral = I;
	
    return (int16_t)PIoutput;
}