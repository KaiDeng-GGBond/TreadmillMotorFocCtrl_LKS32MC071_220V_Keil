#ifndef __SPEEDLOOP_H__
#define __SPEEDLOOP_H__

#include <stdint.h>
#include "basic.h"
#include "sysdef.h"
#include "Foc.h"
#include "Hall.h"

void speed_ref_ramp(stru_FOC_CtrProcDef *obj);
void SpeedLoopCallback(stru_FOC_CtrProcDef *obj);

#endif
