#ifndef __SVPWM_H__
#define __SVPWM_H__

#include <stdint.h>
#include "Foc.h"
#include "sysdef.h"

void SVPWM_2PH(stru_FOC_CtrProcDef *pCtrProc);
void SVPWM_2PH_2(stru_FOC_CtrProcDef *pCtrProc);

#endif