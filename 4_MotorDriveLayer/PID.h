#ifndef __PID_H__
#define __PID_H__

#include "basic.h"
#include <stdint.h>
#include "foc.h"

int16_t PI_Controller(int16_t ref, int16_t fb, stru_PIparams *p);
int16_t HL_PI_AntiDump(int16_t DesiredValue, int16_t MeasuredValue, stru_PIparams *pParams);

#endif
