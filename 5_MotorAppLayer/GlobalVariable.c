#include "hall.h"
#include "AdcCalc.h"
#include "Foc.h"
#include "sysdef.h"
#include "Multifunction.h"

/* Adc读取结构体 */
adc_result gAdcObj = {0};

/* Hall信号处理结构体 */
strHallProcessObj gStructHallProcess = {0};

/* Foc处理结构体 */
stru_FOC_CtrProcDef struFOC_CtrProc;

/* 存储，堵转功能结构体 */
stru_motor_comprehensive stru_motorcomprehensive;
