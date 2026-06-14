/*
 * app.h
 *
 *  Created on: May 22, 2026
 *      Author: ryanm
 */

#ifndef RTOSALARMCLOCK_CORE_INC_APP_H_
#define RTOSALARMCLOCK_CORE_INC_APP_H_

#include "common.h"

typedef struct {
	UART_HandleTypeDef  * p_uart;
	RTC_HandleTypeDef   * p_rtc;
} APP_INTERFACES;

typedef enum {
	tLOG = 0,
	tCLOCK,
	tTASK_NUM
} EXEC_TASK_ENUM;

typedef struct {
	void (*func)(void *);
	uint32_t timeout;
	uint32_t ticks;
	osSemaphoreId_t sem;
} TASK;

void app_entry(void * arg);
void app_init_interfaces(UART_HandleTypeDef * uart, RTC_HandleTypeDef * rtc);

#endif /* RTOSALARMCLOCK_CORE_INC_APP_H_ */
