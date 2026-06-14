/*
 * app.c
 *
 *  Created on: May 22, 2026
 *      Author: ryanm
 */
#include "common.h"
#include "app.h"

#include "../Inc/lcd_st7066u.h"
#include "log.h"
#include "clock.h"

#define ONESHOT -1
#define BASECLK 100
#define HZ_1    (BASECLK / 1)
#define HZ_2    (BASECLK / 2)
#define HZ_5    (BASECLK / 5)
#define HZ_10   (BASECLK / 10)

#define APP_POWER_ON 1
#define APP_INIT     2
#define APP_RUNNING  3

TASK task_list[tTASK_NUM] = {
		[tLOG] 		= 	{.func = log_at_task, .timeout = ONESHOT, .ticks = 0,    .sem = NULL},
		[tCLOCK] 	= 	{.func = rtc_task,    .timeout = HZ_5,    .ticks = HZ_5, .sem = NULL}
};

typedef enum {
	POWERUP = 0,
	INIT_HW,
	INIT_TASKS,
	RUN,
	STATES
} APP_STATE_ENUM;

APP_INTERFACES iface;

static void app_init_tasks(void) {
	/* Init log_at_task */
	const osThreadAttr_t log_at_attr = { .name = "log_at", .stack_size = 2048};
	log_at_init(iface.p_uart);
	osThreadNew(log_at_task, NULL, &log_at_attr);

	/* Init RTC task */
	const osThreadAttr_t rtc_attr = { .name = "rtc"};
	task_list[tCLOCK].sem = rtc_init(iface.p_rtc);
	osThreadNew(rtc_task, NULL, &rtc_attr);
}

static void app_init_hw(void) {
	lcd_init();
}

void app_entry(void * arg) {
	static APP_STATE_ENUM state = POWERUP;
	switch(state) {
		case POWERUP:
			state = INIT_HW;
			break;
		case INIT_HW:
			app_init_hw();
			state = INIT_TASKS;
			break;
		case INIT_TASKS:
			app_init_tasks();
			state = RUN;
			break;
		case RUN:
			for(uint8_t i = 1; i < tTASK_NUM; i++) {
				if(task_list[i].timeout != ONESHOT && --task_list[i].ticks == 0) {
					task_list[i].ticks = task_list[i].timeout;
					osSemaphoreRelease(task_list[i].sem);
				}
			}
			break;
		default:
			state = INIT_HW;
	}
}

void app_init_interfaces(UART_HandleTypeDef * uart, RTC_HandleTypeDef * rtc) {
	iface.p_uart = uart;
	iface.p_rtc = rtc;
}
