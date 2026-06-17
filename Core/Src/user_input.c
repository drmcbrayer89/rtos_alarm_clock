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
			case INPUT_TIME_HOURS:
				rtc_set_hours(CLOCK_TIME);
				break;
			case INPUT_TIME_MINS:
				rtc_set_minutes(CLOCK_TIME);
				break;
			case INPUT_ALARM_ENABLE:
				rtc_alarm_toggle();
				break;
			case INPUT_ALARM_HOURS:
				rtc_set_hours(CLOCK_ALARM);
				log_at_alt("hi!\r\n");
				break;
			case INPUT_ALARM_MINS:
				rtc_set_minutes(CLOCK_ALARM);
			default:
				break;
		}
		last_press_time = press_time;
	}
}



