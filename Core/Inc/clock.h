/*
 * clock.h
 *
 *  Created on: May 23, 2026
 *      Author: ryanm
 */

#ifndef RTOSALARM_CORE_INC_CLOCK_H_
#define RTOSALARM_CORE_INC_CLOCK_H_
#include "common.h"

osSemaphoreId_t rtc_init(RTC_HandleTypeDef * rtc);
void rtc_task(void * arg);
void rtc_set_hours(void);
void rtc_set_minutes(void);
void rtc_alarm_toggle(void);

#endif /* RTOSALARM_CORE_INC_CLOCK_H_ */
