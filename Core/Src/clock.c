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

#define ALARM_LABEL_LOCATION 3,0
#define ALARM_ENABLE_LOCATION 3,7
#define ALARM_TIME_LOCATION 3,11
#define ALARM_ALERT_LOCATION 2,0
#define ALARM_CLEAR_INDICATOR "   "
#define WAKE_UP_MESSAGE "WAKE UP!"

static osSemaphoreId_t sem;
static RTC_HandleTypeDef * p_rtc;
static RTC_TimeTypeDef time = {0};
static RTC_DateTypeDef date = {0};
static RTC_TimeTypeDef time_start = { .Hours = 12, .Minutes = 00, .Seconds = 0};
static RTC_TimeTypeDef alarm_time = { .Hours = 11, .Minutes = 00, .Seconds = 0};
static uint32_t alarm_armed = 0;
static uint8_t alarm_triggered = 0;

static void rtc_alarm(void) {
	lcd_set_cursor(ALARM_ALERT_LOCATION);
	lcd_write_string(WAKE_UP_MESSAGE);
}

static void rtc_write_time(uint8_t hours, uint8_t minutes, CLOCK_MODE mode){
	//lcd_clear_display();
	static char hour_tens, hour_ones, min_tens, min_ones;
	hour_tens = (char)((hours / 10) + '0');
	hour_ones = (char)((hours % 10) + '0');
	min_tens = (char)((minutes / 10) + '0');
	min_ones = (char)((minutes % 10) + '0');

	(mode == CLOCK_TIME) ? lcd_set_cursor(0,0) : lcd_set_cursor(ALARM_TIME_LOCATION);

	lcd_write_char(hour_tens);
	lcd_write_char(hour_ones);
	lcd_write_char(':');
	lcd_write_char(min_tens);
	lcd_write_char(min_ones);
}

static void rtc_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds) {
	RTC_TimeTypeDef new_time = time;

	if(hours != -1) new_time.Hours = hours;

	if(minutes != -1) new_time.Minutes = minutes;

	if(seconds != -1) new_time.Seconds = seconds;

	HAL_RTC_SetTime(p_rtc, &new_time, RTC_FORMAT_BIN);
}

void rtc_set_hours(CLOCK_MODE mode) {
	switch(mode) {
		case CLOCK_TIME:
			/* Increment hours counter then call set time */
			if(time.Hours++ >= 24) {
				time.Hours = 0;
			}
			HAL_RTC_SetTime(p_rtc, &time, RTC_FORMAT_BIN);
			break;
		case CLOCK_ALARM:
			if(alarm_time.Hours++ >= 24) {
				alarm_time.Hours = 0;
			}
			//rtc_write_time(alarm_time.Hours, alarm_time.Minutes, mode);
			break;
		default:
			break;
	}
}

void rtc_set_minutes(CLOCK_MODE mode) {
	switch(mode) {
		case CLOCK_TIME:
			if(time.Minutes++ >= 59) {
				time.Minutes = 0;
			}
			HAL_RTC_SetTime(p_rtc, &time, RTC_FORMAT_BIN);
			break;
		case CLOCK_ALARM:
			if(alarm_time.Minutes++ >= 59) {
				alarm_time.Minutes = 0;
			}
			//rtc_write_time(alarm_time.Hours, alarm_time.Minutes, mode);
			break;
		default:
			break;
	}
}

void rtc_alarm_toggle(void) {
	alarm_armed = !alarm_armed;
	if(alarm_armed == FALSE && alarm_triggered == TRUE) {
		alarm_triggered = FALSE;
		lcd_set_cursor(ALARM_ALERT_LOCATION);
		lcd_write_string("        ");
	}
	lcd_set_cursor(ALARM_ENABLE_LOCATION);
	lcd_write_string(ALARM_CLEAR_INDICATOR);
	lcd_set_cursor(ALARM_ENABLE_LOCATION);
	(alarm_armed) ? lcd_write_string("ON") : lcd_write_string("OFF");
}

void rtc_task(void * arg) {
	static RTC_TimeTypeDef time_last, alarm_last;
	//static char hour_tens, hour_ones, min_tens, min_ones;
	FOREVER {
		osSemaphoreAcquire(sem, osWaitForever);
		HAL_RTC_GetTime(p_rtc, &time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(p_rtc, &date, RTC_FORMAT_BIN);
		// update time
		if((time_last.Hours != time.Hours) || (time_last.Minutes != time.Minutes)) {
			rtc_write_time(time.Hours, time.Minutes, CLOCK_TIME);
		}
		// update alarm if changed
		if((alarm_last.Hours != alarm_time.Hours) || (alarm_last.Minutes != alarm_time.Minutes)) {
			rtc_write_time(alarm_time.Hours, alarm_time.Minutes, CLOCK_ALARM);
		}

		if(time.Hours == alarm_time.Hours && time.Minutes == alarm_time.Minutes) {
			alarm_triggered = TRUE;
		}

		// Continuously trigger alarm alert
		if(alarm_armed && alarm_triggered) {
			rtc_alarm();
		}
		time_last = time;
		alarm_last = alarm_time;
	}
}

osSemaphoreId_t rtc_init(RTC_HandleTypeDef * rtc) {
	p_rtc = rtc;
	sem = osSemaphoreNew(1, 1, NULL);
	rtc_set_time(time_start.Hours, time_start.Minutes, time_start.Seconds);
	/* init the clock display */
	lcd_set_cursor(ALARM_LABEL_LOCATION);
	lcd_write_string("Alarm: ");
	(alarm_armed) ? lcd_write_string("ON ") : lcd_write_string("OFF ");
	rtc_write_time(alarm_time.Hours, alarm_time.Minutes, CLOCK_ALARM);
	return sem;
}


