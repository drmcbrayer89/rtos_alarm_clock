/*
 * log.h
 *
 *  Created on: May 23, 2026
 *      Author: ryanm
 */

#ifndef RTOSALARM_CORE_INC_LOG_H_
#define RTOSALARM_CORE_INC_LOG_H_
#include "common.h"
#include "stdarg.h"

#define MSG_SIZE 128

void log_at_alt(char * msg, ...);
void log_at_task(void * arg);
void log_at(char * msg);
void log_at_init(UART_HandleTypeDef * uart);

#endif /* RTOSALARM_CORE_INC_LOG_H_ */
