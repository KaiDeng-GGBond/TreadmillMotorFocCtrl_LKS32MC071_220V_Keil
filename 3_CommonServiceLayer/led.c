#include "led.h"
#include "sysdef.h"
#include "lks32mc07x_gpio.h"
#include "hardware_init.h"

void led_on(void)
{
	GPIO_ResetBits(LED_PORT, LED_PIN);
}

void led_off(void)
{
	GPIO_SetBits(LED_PORT, LED_PIN);
}

void led_blink(void)
{
	static uint16_t led_cnt = 0;
	static uint8_t led_state = 0;
	led_cnt = 0;
	if(led_state)
	{
		led_off();
		led_state = 0;
	}
	else
	{
		led_on();
		led_state = 1;
	}
}
