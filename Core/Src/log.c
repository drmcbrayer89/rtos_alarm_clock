/*
 * log.c
 *
 *  Created on: May 23, 2026
 *      Author: ryanm
 */
#include "common.h"
#include "log.h"

static osMessageQueueId_t q_id;
static UART_HandleTypeDef * p_uart;

void log_at_alt(char * msg, ...) {
	char dest[128];
	va_list args;
	va_start(args, msg);
	vsnprintf(dest, sizeof(dest), msg, args);

	osMessageQueuePut(q_id, dest, 0, 0);

	va_end(args);
}

void log_at(char * msg) {
	osMessageQueuePut(q_id, msg, 0, 0);
}

void log_at_task(void * arg) {
	char buffer[MSG_SIZE];
	FOREVER {
		if(osMessageQueueGet(q_id, &buffer, NULL, osWaitForever) == osOK) {
			HAL_UART_Transmit(p_uart, buffer, (uint32_t)strlen(buffer), 200);
		}
	}
}

void log_at_init(UART_HandleTypeDef * uart) {
	q_id = osMessageQueueNew(5, MSG_SIZE, NULL);
	p_uart = uart;
}
