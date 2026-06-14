/*
 * user_input.c
 *
 *  Created on: May 24, 2026
 *      Author: ryanm
 */
#include "common.h"
#include "clock.h"
#include "lcd_st7066u.h"
#include "log.h"

#define DEBOUNCE_DELAY 250

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
	static uint32_t last_press_time = 0;
	uint32_t press_time = osKernelGetTickCount();
	if(press_time - last_press_time > DEBOUNCE_DELAY) {
		switch(GPIO_Pin) {
			case GPIO_PIN_8:
				rtc_set_hours();
				break;
			case GPIO_PIN_9:
				rtc_set_minutes();
				break;
			case GPIO_PIN_4:
				rtc_alarm_toggle();
				break;
			default:
				break;
		}
		last_press_time = press_time;
	}
}



