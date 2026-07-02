#ifndef __CLARK_PARK_H__
#define __CLARK_PARK_H__

#include <stdint.h>
#include "Foc.h"

#define AXIS_TRANSFORMATION_TYPE 0  //1是标准坐标轴

stru_TrigComponents Trig_Functions(int16_t hAngle);
stru_CurrVoctor Clarke(stru_CurrPhaseUVW Curr_Input);
stru_CurrVoctor Clarke_3Shunt(stru_CurrPhaseUVW Curr_Input);
stru_CurrVoctor Park(stru_CurrVoctor Curr_Input, int16_t Theta, stru_FOC_CurrLoopDef *this);
stru_VoltVoctor Rev_Park(stru_VoltVoctor Volt_Input, stru_FOC_CurrLoopDef *this);
void RevPark_Circle_Limitation(stru_FOC_CtrProcDef *pCtrProc);

#endif
