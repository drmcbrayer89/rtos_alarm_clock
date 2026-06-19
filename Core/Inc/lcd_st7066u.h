/*
 * lcd.h
 *
 *  Created on: May 26, 2026
 *      Author: ryanm
 */

#ifndef RTOSALARM_CORE_INC_LCD_ST7066U_H_
#define RTOSALARM_CORE_INC_LCD_ST7066U_H_
#include "common.h"

typedef enum {
	LCD_PIN_RS,
	LCD_PIN_RW,
	LCD_PIN_E,
	LCD_PIN_DB0,
	LCD_PIN_DB1,
	LCD_PIN_DB2,
	LCD_PIN_DB3,
	LCD_PIN_DB4,
	LCD_PIN_DB5,
	LCD_PIN_DB6,
	LCD_PIN_DB7,
	LCD_PIN_COUNT
} LCD_PIN_ENUM;

typedef struct {
	uint8_t rs;
	uint8_t rw;
	uint8_t data;
} LCD_INSTR;

typedef enum {
	LCD_DB0 = 0,
	LCD_DB1,
	LCD_DB2,
	LCD_DB3,
	LCD_DB4,
	LCD_DB5,
	LCD_DB6,
	LCD_DB7,
} LCD_DATA_PINS;

typedef enum {
	LCD_4BIT,
	LCD_8BIT
} LCD_MODE;

void lcd_write(LCD_INSTR ins);
void lcd_clear_display(void);
void lcd_init(LCD_MODE mode);
void lcd_write_char(char c);
void lcd_write_time(char h, char m1, char m2);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_write_string(char * str);
void lcd_write_msg(uint8_t row, uint8_t col, char * str);

#endif /* RTOSALARM_CORE_INC_LCD_ST7066U_H_ */
