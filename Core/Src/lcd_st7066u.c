/*
 * lcd.c
 *
 *  Created on: May 26, 2026
 *      Author: ryanm
 */
#include "../Inc/lcd_st7066u.h"

#include "log.h"

typedef struct {
	GPIO_TypeDef * port;
	uint32_t pin;
} LCD_GPIO_PIN;

LCD_GPIO_PIN db[16] = {
		[LCD_PIN_RS] 	= {.port = GPIOC, .pin = GPIO_PIN_7},
		[LCD_PIN_E]  	= {.port = GPIOC, .pin = GPIO_PIN_8},
		[LCD_PIN_DB0] 	= {.port = GPIOC, .pin = GPIO_PIN_10}, // 10
		[LCD_PIN_DB1] 	= {.port = GPIOC, .pin = GPIO_PIN_5},
		[LCD_PIN_DB2] 	= {.port = GPIOC, .pin = GPIO_PIN_3}, // 3
		[LCD_PIN_DB3] 	= {.port = GPIOC, .pin = GPIO_PIN_2},
		[LCD_PIN_DB4] 	= {.port = GPIOC, .pin = GPIO_PIN_12}, // 11
		[LCD_PIN_DB5] 	= {.port = GPIOC, .pin = GPIO_PIN_11}, // 12
		[LCD_PIN_DB6] 	= {.port = GPIOC, .pin = GPIO_PIN_9},  // 9
		[LCD_PIN_DB7] 	= {.port = GPIOC, .pin = GPIO_PIN_6} // 6
};

void lcd_write(LCD_INSTR ins) {
	HAL_GPIO_WritePin(db[LCD_PIN_RS].port, db[LCD_PIN_RS].pin, ins.rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
	// Write to DB0-DB7 output pins
	HAL_GPIO_WritePin(db[LCD_PIN_DB7].port, db[LCD_PIN_DB7].pin, (ins.data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET); 								//GPIO_PIN_SET);
	HAL_GPIO_WritePin(db[LCD_PIN_DB6].port, db[LCD_PIN_DB6].pin, (ins.data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(db[LCD_PIN_DB5].port, db[LCD_PIN_DB5].pin, (ins.data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(db[LCD_PIN_DB4].port, db[LCD_PIN_DB4].pin, (ins.data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(db[LCD_PIN_DB3].port, db[LCD_PIN_DB3].pin, (ins.data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(db[LCD_PIN_DB2].port, db[LCD_PIN_DB2].pin, (ins.data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(db[LCD_PIN_DB1].port, db[LCD_PIN_DB1].pin, (ins.data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(db[LCD_PIN_DB0].port, db[LCD_PIN_DB0].pin, (ins.data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

	osDelay(5);
	// Enable E
	HAL_GPIO_WritePin(db[LCD_PIN_E].port, db[LCD_PIN_E].pin, GPIO_PIN_SET);
	osDelay(5);
	// Clear E
	HAL_GPIO_WritePin(db[LCD_PIN_E].port, db[LCD_PIN_E].pin, GPIO_PIN_RESET);
	osDelay(5);
}
void lcd_clear_display(void) {
	LCD_INSTR instr = { .rs = 0, .rw = 0, .data = 0x01 };
	lcd_write(instr);
}

void lcd_return(void) {
	LCD_INSTR instr = { .rs = 0, .rw = 0, .data = 0x02 };
	lcd_write(instr);
}

void lcd_entry_mode_set(uint8_t i_d, uint8_t s) {
	LCD_INSTR instr = { .rs = 0, .rw = 0, .data = 0x04 };
	instr.data |= (1 << LCD_DB2) | (i_d << LCD_DB1) | s;
	lcd_write(instr);
}

void lcd_display_on_off(uint8_t d, uint8_t c, uint8_t b) {
	LCD_INSTR instr = { .rs = 0, .rw = 0, .data = 0x08 };
	instr.data |= (d << LCD_DB2) | (c << LCD_DB1) | b;
	lcd_write(instr);
}

void lcd_cursor_display_shift(uint8_t sc, uint8_t rl) {
	LCD_INSTR instr = { .rs = 0, .rw = 0, .data = 0x00 };
	instr.data |= (sc << LCD_DB3) | (rl << LCD_DB2);
	lcd_write(instr);
}

void lcd_func_set(uint8_t dl, uint8_t n, uint8_t f) {
	LCD_INSTR instr = { .rs = 0, .rw = 0, .data = 0x20 };
	instr.data |= (dl << LCD_DB4) | (n << LCD_DB3) | (f << LCD_DB2);
	lcd_write(instr);
}

void lcd_write_ram(uint8_t data) {
	LCD_INSTR instr = { .rs = 1, .rw = 0, .data = 0x00 };
	instr.data |= data;
	lcd_write(instr);
}

uint8_t lcd_read_ram(void) {
	LCD_INSTR instr = { .rs = 1, .rw = 1, .data = 0x00 };
	lcd_write(instr);
	return 0;
}

void lcd_write_char(char c) {
	LCD_INSTR ins;
	ins.rs = 1;
	ins.data = (uint8_t)c;

	lcd_write(ins);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
	uint8_t addr = 0x80;

	switch(row) {
		case 0:
			addr |= 0x00 + col;
			break;
		case 1:
			addr |= 0x40 + col;
			break;
		case 2:
			addr |= 0x14 + col;
			break;
		case 3:
			addr |= 0x54 + col;
			break;
		default:
			addr = 0x00;
			break;
	}

	LCD_INSTR instr = { .rs = 0, .rw = 0, .data = addr };
	lcd_write(instr);
	osDelay(50);
}

void lcd_write_string(char * str) {
	size_t len = strlen(str);
	for(int i = 0; i < len; i++) {
		lcd_write_char(str[i]);
	}
}

void lcd_write_msg(uint8_t row, uint8_t col, char * str) {
	lcd_set_cursor(row, col);
	lcd_write_string(str);
}

void lcd_init(void) {
	osDelay(200);
	// func set #1-#3
	lcd_func_set(1, 1, 0);
	lcd_func_set(1, 1, 0);
	lcd_func_set(1, 1, 0);
	// turn on display, cursor OFF, blink OFF
	lcd_display_on_off(1, 0, 0);
	osDelay(5);
	// clear display
	lcd_clear_display();
	osDelay(5);
	// cursor/blinker moves RIGHT when DDRAM is increased, shift display
	lcd_entry_mode_set(1, 0);
	lcd_write_string("FreeRTOS Clock");

	lcd_set_cursor(1,0);
	lcd_write_string("Ryan McBrayer");

	lcd_set_cursor(2,0);
	lcd_write_string("2026");

	osDelay(2000);
	lcd_clear_display();
}
