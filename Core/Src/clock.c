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
#define ALARM_ENABLE 3,7
#define ALARM_TIME 3, 11
#define ALARM_CLEAR_INDICATOR "   "

typedef enum {
	CLOCK_TIME,
	CLOCK_ALARM
} CLOCK_MODE;

static osSemaphoreId_t sem;
static RTC_HandleTypeDef * p_rtc;
static RTC_TimeTypeDef time = {0};
static RTC_DateTypeDef date = {0};
static RTC_TimeTypeDef time_start = { .Hours = 12, .Minutes = 00, .Seconds = 0};
static RTC_TimeTypeDef alarm_time = { .Hours = 12, .Minutes = 00, .Seconds = 0};
static uint32_t alarm_enabled = 0;

static void rtc_write_time(uint8_t hours, uint8_t minutes, CLOCK_MODE mode){
	//lcd_clear_display();
	static char hour_tens, hour_ones, min_tens, min_ones;
	hour_tens = (char)((hours / 10) + '0');
	hour_ones = (char)((hours % 10) + '0');
	min_tens = (char)((minutes / 10) + '0');
	min_ones = (char)((minutes % 10) + '0');

	(mode == CLOCK_TIME) ? lcd_set_cursor(0,0) : lcd_set_cursor(ALARM_TIME);

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

void rtc_set_hours(void) {
	/* Increment hours counter then call set time */
	if(time.Hours++ >= 24) {
		time.Hours = 0;
	}
	HAL_RTC_SetTime(p_rtc, &time, RTC_FORMAT_BIN);
}

void rtc_set_minutes(void) {
	if(time.Minutes++ >= 60) {
		time.Minutes = 0;
	}
	HAL_RTC_SetTime(p_rtc, &time, RTC_FORMAT_BIN);
}

void rtc_alarm_toggle(void) {
	alarm_enabled = !alarm_enabled;
	lcd_set_cursor(ALARM_ENABLE);
	lcd_write_string(ALARM_CLEAR_INDICATOR);
	lcd_set_cursor(ALARM_ENABLE);
	(alarm_enabled) ? lcd_write_string("ON") : lcd_write_string("OFF");
}

void rtc_task(void * arg) {
	static RTC_TimeTypeDef time_last;
	//static char hour_tens, hour_ones, min_tens, min_ones;
	FOREVER {
		osSemaphoreAcquire(sem, osWaitForever);
		HAL_RTC_GetTime(p_rtc, &time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(p_rtc, &date, RTC_FORMAT_BIN);

		if((time_last.Hours != time.Hours) || (time_last.Minutes != time.Minutes)) {
			rtc_write_time(time.Hours, time.Minutes, CLOCK_TIME);
		}
		time_last = time;
	}
}

osSemaphoreId_t rtc_init(RTC_HandleTypeDef * rtc) {
	p_rtc = rtc;
	sem = osSemaphoreNew(1, 1, NULL);
	rtc_set_time(time_start.Hours, time_start.Minutes, time_start.Seconds);
	/* init the clock display */
	lcd_set_cursor(ALARM_LABEL_LOCATION);
	lcd_write_string("Alarm: ");
	(alarm_enabled) ? lcd_write_string("ON ") : lcd_write_string("OFF ");
	rtc_write_time(alarm_time.Hours, alarm_time.Minutes, CLOCK_ALARM);
	return sem;
}


