/*
 * lcd.h
 *
 *  Created on: May 15, 2026
 *      Author: clara
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f4xx_hal.h"

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF

void LCD_Init(void);
void LCD_SetAddressWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h);
void LCD_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

#endif /* INC_LCD_H_ */
