#include "hardware_init.h"
#include "led.h"
#include "TaskScheduler.h"
#include "lks32mc07x.h"
#include "Hall.h"
#include "SysInit.h"
#include "Clark_Park.h"

//uint32_t reg_aon_evt_rcd = 0;
//AON_EVT_RCD事件记录寄存器\
AON_EVT_RCD_IWDG_RST_BIT    BIT3  // [3] 独立看门狗复位发生记录，高表示发生过\
AON_EVT_RCD_KEY_RST_RCD_BIT BIT2  // [2] 按键复位发生记录，高表示发生过\
AON_EVT_RCD_POR_RST_RCD_BIT BIT0  // [0] POR复位发生记录，高表示发生过

int main(void)
{
	/* Hardware init */
	Hardware_init();

	/* sys_init */
	SysInit();

	SYS_DBG_CFG |= SYS_DBG_CFG_SFT_RST_PERI_BIT;//该位置为1，CPU软复位后，所有外设寄存器同时复位
	//reg_aon_evt_rcd = AON_EVT_RCD;//读取AON_EVT_RCD事件记录寄存器

    for (;;)
    {
        task_scheduler();
    }
}
