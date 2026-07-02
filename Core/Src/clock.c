/*
 * clock.c
 *
 *  Created on: May 23, 2026
 *      Author: ryanm
 */
#include "clock.h"
#include "log.h"
#include "stdio.h"

#include "../Inc/lcd_st7066u.h"

#define TIME_LOCATION 0,0
#define ALARM_LABEL_LOCATION 1,0
#define ALARM_TIME_LOCATION 2,0
#define ALARM_ALERT_LOCATION 3,0
#define ALARM_CLEAR_INDICATOR "   "
#define WAKE_UP_MESSAGE "WAKE UP!"
#define IDLE_MESSAGE    "SLEEPING"

static osSemaphoreId_t sem;
static RTC_HandleTypeDef * p_rtc;
static TIM_HandleTypeDef * p_timer;
static RTC_TimeTypeDef time = {0};
static RTC_DateTypeDef date = {0};
static RTC_TimeTypeDef time_start = { .Hours = 12, .Minutes = 00, .Seconds = 0};
static RTC_TimeTypeDef alarm_time = { .Hours = 11, .Minutes = 00, .Seconds = 0};
static uint32_t alarm_armed = FALSE;
static uint8_t alarm_reset = FALSE;
static uint8_t alarm_buzzer_enable = FALSE;

static void rtc_write_time(uint8_t hours, uint8_t minutes, CLOCK_MODE mode){
	//lcd_clear_display();
	static char hour_tens, hour_ones, min_tens, min_ones;
	hour_tens = (char)((hours / 10) + '0');
	hour_ones = (char)((hours % 10) + '0');
	min_tens = (char)((minutes / 10) + '0');
	min_ones = (char)((minutes % 10) + '0');

	(mode == MODE_TIME) ? lcd_set_cursor(TIME_LOCATION) : lcd_set_cursor(ALARM_TIME_LOCATION);

	lcd_write_char(hour_tens);
	lcd_write_char(hour_ones);
	lcd_write_char(':');
	lcd_write_char(min_tens);
	lcd_write_char(min_ones);
}

void rtc_set_hours(CLOCK_MODE mode) {
	switch(mode) {
		case MODE_TIME:
			/* Increment hours counter then call set time */
			if(time.Hours++ >= 24) {
				time.Hours = 0;
			}
			HAL_RTC_SetTime(p_rtc, &time, RTC_FORMAT_BIN);
			break;
		case MODE_ALARM:
			if(alarm_time.Hours++ >= 24) {
				alarm_time.Hours = 0;
			}
			break;
		default:
			break;
	}
}

void rtc_set_minutes(CLOCK_MODE mode) {
	switch(mode) {
		case MODE_TIME:
			if(time.Minutes++ >= 59) {
				time.Minutes = 0;
				rtc_set_hours(MODE_TIME);
			}
			HAL_RTC_SetTime(p_rtc, &time, RTC_FORMAT_BIN);
			break;
		case MODE_ALARM:
			if(alarm_time.Minutes++ >= 59) {
				alarm_time.Minutes = 0;
				rtc_set_hours(MODE_ALARM);
			}
			break;
		default:
			break;
	}
}

void rtc_alarm_toggle(void) {
	alarm_armed = !alarm_armed;
	if(alarm_armed) {
		lcd_write_msg(ALARM_LABEL_LOCATION, "Alarm: ON ");
	} else {
		lcd_write_msg(ALARM_ALERT_LOCATION, "        ");
		lcd_write_msg(ALARM_LABEL_LOCATION, "Alarm: OFF");
	}
}

void rtc_toggle_buzzer(void) {
	static uint8_t enable = TRUE;

	(enable == TRUE) ? HAL_TIM_PWM_Start(p_timer, TIM_CHANNEL_2) : HAL_TIM_PWM_Stop(p_timer, TIM_CHANNEL_2);
	enable = !enable;
}


void rtc_task(void * arg) {
	static RTC_TimeTypeDef time_last, alarm_last;
	static uint8_t counter_2hz = 5;
	FOREVER {
		osSemaphoreAcquire(sem, osWaitForever);
		HAL_RTC_GetTime(p_rtc, &time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(p_rtc, &date, RTC_FORMAT_BIN);
		// update time
		if((time_last.Hours != time.Hours) || (time_last.Minutes != time.Minutes)) {
			rtc_write_time(time.Hours, time.Minutes, MODE_TIME);
			// check for alarm trigger, clear if not met
			if(time.Hours == alarm_time.Hours && time.Minutes == alarm_time.Minutes) {
				if(alarm_armed) {
					alarm_buzzer_enable = TRUE;
				}
			}
		}
		// update alarm if changed
		if((alarm_last.Hours != alarm_time.Hours) || (alarm_last.Minutes != alarm_time.Minutes)) {
			rtc_write_time(alarm_time.Hours, alarm_time.Minutes, MODE_ALARM);
		}
		
		if(alarm_armed && alarm_buzzer_enable) {
			if(counter_2hz-- == 0) {
				rtc_toggle_buzzer();
				counter_2hz = 5;
			}
		} else {
			alarm_buzzer_enable = FALSE;
			HAL_TIM_PWM_Stop(p_timer, TIM_CHANNEL_2);
		}

		time_last = time;
		alarm_last = alarm_time;
	}
}

static void rtc_alarm_display_init(void) {
	rtc_write_time(alarm_time.Hours, alarm_time.Minutes, MODE_ALARM);
	(alarm_armed) ? lcd_write_msg(ALARM_LABEL_LOCATION, "Alarm: ON") : lcd_write_msg(ALARM_LABEL_LOCATION, "Alarm: OFF");
}

osSemaphoreId_t rtc_init(RTC_HandleTypeDef * rtc, TIM_HandleTypeDef * timer) {
	p_rtc = rtc;
	p_timer = timer;

	HAL_RTC_SetTime(p_rtc, &time_start, RTC_FORMAT_BIN);
	rtc_alarm_display_init();

	sem = osSemaphoreNew(1, 1, NULL);
	return sem;
}


