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
static RTC_TimeTypeDef time = {0};
static RTC_DateTypeDef date = {0};
static RTC_TimeTypeDef time_start = { .Hours = 12, .Minutes = 00, .Seconds = 0};
static RTC_TimeTypeDef alarm_time = { .Hours = 11, .Minutes = 00, .Seconds = 0};
static uint32_t g_alarm_armed = FALSE;
static uint8_t g_alarm_triggered = FALSE;
static uint8_t g_alarm_reset = FALSE;

static void rtc_alarm(void) {
	lcd_write_msg(ALARM_ALERT_LOCATION, WAKE_UP_MESSAGE);
}

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
	g_alarm_armed = !g_alarm_armed;
	g_alarm_reset = TRUE;
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
			rtc_write_time(time.Hours, time.Minutes, MODE_TIME);
		}
		// update alarm if changed
		if((alarm_last.Hours != alarm_time.Hours) || (alarm_last.Minutes != alarm_time.Minutes)) {
			rtc_write_time(alarm_time.Hours, alarm_time.Minutes, MODE_ALARM);
		}
		// check for alarm trigger, clear if not met
		if(time.Hours == alarm_time.Hours && time.Minutes == alarm_time.Minutes) {
			g_alarm_triggered = TRUE;
		}
		else {
			g_alarm_triggered = FALSE;
		}
		// Continuously trigger alarm alert
		if(g_alarm_armed && g_alarm_triggered) {
			rtc_alarm();
		}
		// check if alarm has been reset (snooze)
		if(g_alarm_reset) {
			if(g_alarm_armed) {
				lcd_write_msg(ALARM_LABEL_LOCATION, "Alarm: ON ");
			} else {
				lcd_write_msg(ALARM_ALERT_LOCATION, "        ");
				lcd_write_msg(ALARM_LABEL_LOCATION, "Alarm: OFF");
			}
			g_alarm_triggered = FALSE;
			g_alarm_reset = FALSE;
		}
		time_last = time;
		alarm_last = alarm_time;
	}
}

static void rtc_alarm_display_init(void) {
	rtc_write_time(alarm_time.Hours, alarm_time.Minutes, MODE_ALARM);
	(g_alarm_armed) ? lcd_write_msg(ALARM_LABEL_LOCATION, "Alarm: ON") : lcd_write_msg(ALARM_LABEL_LOCATION, "Alarm: OFF");
}

osSemaphoreId_t rtc_init(RTC_HandleTypeDef * rtc) {
	p_rtc = rtc;

	HAL_RTC_SetTime(p_rtc, &time_start, RTC_FORMAT_BIN);
	rtc_alarm_display_init();
	
	sem = osSemaphoreNew(1, 1, NULL);
	return sem;
}


