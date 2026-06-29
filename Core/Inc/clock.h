/*
 * clock.h
 *
 *  Created on: May 23, 2026
 *      Author: ryanm
 */

#ifndef RTOSALARM_CORE_INC_CLOCK_H_
#define RTOSALARM_CORE_INC_CLOCK_H_
#include "common.h"

#define INPUT_TIME_HOURS   GPIO_PIN_8
#define INPUT_TIME_MINS    GPIO_PIN_9
#define INPUT_ALARM_HOURS  GPIO_PIN_7
#define INPUT_ALARM_MINS   GPIO_PIN_15
#define INPUT_ALARM_ENABLE GPIO_PIN_4

typedef enum {
	MODE_TIME,
	MODE_ALARM
} CLOCK_MODE;

osSemaphoreId_t rtc_init(RTC_HandleTypeDef * rtc, TIM_HandleTypeDef * timer);
void rtc_task(void * arg);
void rtc_set_hours(CLOCK_MODE mode);
void rtc_set_minutes(CLOCK_MODE mode);
void rtc_alarm_toggle(void);

#endif /* RTOSALARM_CORE_INC_CLOCK_H_ */
