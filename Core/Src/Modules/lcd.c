/*
 * lcd.c
 *
 *  Created on: May 15, 2026
 *      Author: clara
 */

/*
 * lcd.c
 * Created on: May 15, 2026
 * Author: clara
 */

#include "lcd.h"

extern SPI_HandleTypeDef hspi1;

// --- Funções Internas (Privadas) ---

static void LCD_Command(uint8_t cmd) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET); // IHM_DC = 0 (Comando no PA12)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET); // IHM_CS = 0
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);   // IHM_CS = 1
}

static void LCD_Data(uint8_t data) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);   // IHM_DC = 1 (Dado no PA12)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET); // IHM_CS = 0
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);   // IHM_CS = 1
}

void LCD_SetAddressWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h) {
    uint16_t x2 = x1 + w - 1;
    uint16_t y2 = y1 + h - 1;

    LCD_Command(0x2A); // Column Address Set
    LCD_Data(x1 >> 8); LCD_Data(x1 & 0xFF);
    LCD_Data(x2 >> 8); LCD_Data(x2 & 0xFF);

    LCD_Command(0x2B); // Page Address Set
    LCD_Data(y1 >> 8); LCD_Data(y1 & 0xFF);
    LCD_Data(y2 >> 8); LCD_Data(y2 & 0xFF);

    LCD_Command(0x2C); // Memory Write
}

// --- Funções Públicas ---

void LCD_Init(void) {
    // 1. Reset por Hardware (IHM_RESET no pino PA8 conforme sua imagem)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_Delay(200);

    // 2. Sequência de comandos ILI9341
    LCD_Command(0x01); // Software Reset
    HAL_Delay(150);

    LCD_Command(0xCF); LCD_Data(0x00); LCD_Data(0xC1); LCD_Data(0X30);
    LCD_Command(0xED); LCD_Data(0x64); LCD_Data(0x03); LCD_Data(0X12); LCD_Data(0X81);
    LCD_Command(0xE8); LCD_Data(0x85); LCD_Data(0x00); LCD_Data(0x78);
    LCD_Command(0xCB); LCD_Data(0x39); LCD_Data(0x2C); LCD_Data(0x00); LCD_Data(0x34); LCD_Data(0x02);
    LCD_Command(0xF7); LCD_Data(0x20);
    LCD_Command(0xEA); LCD_Data(0x00); LCD_Data(0x00);

    LCD_Command(0xC0); LCD_Data(0x23); // Power Control 1
    LCD_Command(0xC1); LCD_Data(0x10); // Power Control 2
    LCD_Command(0xC5); LCD_Data(0x3E); LCD_Data(0x28); // VCOM Control 1
    LCD_Command(0xC7); LCD_Data(0x86); // VCOM Control 2

    LCD_Command(0x36); LCD_Data(0x48); // Orientação (BGR mode)
    LCD_Command(0x3A); LCD_Data(0x55); // Pixel Format (16-bit)

    LCD_Command(0x11); // Exit Sleep
    HAL_Delay(120);
    LCD_Command(0x29); // Display ON

    // 3. Ligar Backlight (IHM_LED no PB1)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
}

void LCD_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    LCD_SetAddressWindow(x, y, w, h);
    uint8_t data[2] = {color >> 8, color & 0xFF};

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET); // CS = 0
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);   // DC = 1 (Modo Dados)

    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        HAL_SPI_Transmit(&hspi1, data, 2, HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);   // CS = 1
}

