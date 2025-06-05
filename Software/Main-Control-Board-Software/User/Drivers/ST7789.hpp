/**
 * @file ST7789.hpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "stdint.h"

namespace ST7789
{

#define ST7789_WIDTH 320
#define ST7789_HEIGHT 240

#define FONT_WIDTH 11
#define FONT_HEIGHT 20

#define ST7789_DC_PORT LCD_DC_GPIO_Port
#define ST7789_DC_PIN LCD_DC_Pin
#define ST7789_RST_PORT LCD_RST_GPIO_Port
#define ST7789_RST_PIN LCD_RST_Pin

#define COLOR_BLACK 0x4208
#define COLOR_GRAY 0xDEFB
#define COLOR_WHITE 0xFFFF
#define COLOR_BLUE 0x35FA
#define COLOR_RED 0xE222
#define COLOR_GREEN 0x1701
#define COLOR_YELLOW 0xEED0


#define ST7789_NOP                  0x00
#define ST7789_SOFTWARE_RESET       0x01

#define ST7789_SLEEP_IN                 0x10
#define ST7789_SLEEP_OUT                0x11
#define ST7789_PARTIAL_ON               0x12 
#define ST7789_NORMAL_ON                0x13  // Normal mode ON
#define ST7789_INVERT_OFF               0x20
#define ST7789_INVERT_ON                0x21
#define ST7789_DISPLAY_OFF              0x28
#define ST7789_DISPLAY_ON               0x29
#define ST7789_IDLE_OFF                 0x38
#define ST7789_IDLE_ON                  0x39

#define ST7789_SET_COLUMN_ADDRESS       0x2A
#define ST7789_SET_ROW_ADDRESS          0x2B
#define ST7789_RAM_WRITE                0x2C
#define ST7789_RAM_READ                 0x2E

#define ST7789_COLOR_SETTING            0x3A
#define ST7789_MEMORY_ACCESS_CONTROL    0x36

#define ST7789_PARTIAL_AREA             0x30   // partial start/end
#define ST7789_VSCRDEF                  0x33   // Set Scroll Area
#define ST7789_VSCRSADD                 0x37

#define ST7789_MEMORY_ACCCESS_CONTROL_ROW_NORMAL        0x00
#define ST7789_MEMORY_ACCCESS_CONTROL_ROW_REVERSE       0x80
#define ST7789_MEMORY_ACCCESS_CONTROL_COL_NORMAL        0x00
#define ST7789_MEMORY_ACCCESS_CONTROL_COL_REVERSE       0x40
#define ST7789_MEMORY_ACCCESS_CONTROL_ROW_COL_NORMAL    0x00
#define ST7789_MEMORY_ACCCESS_CONTROL_ROW_COL_SWAP      0x20

extern uint8_t INIT_SEQUENCE[][2];
extern uint8_t buffer[ST7789_WIDTH*ST7789_HEIGHT*2 / 16];

void readDisplayID();
void readDisplayStatus();

void init();
void openWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void MY_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size);
void MY_SPI_Transmit_DMA(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size);

void fillScreen(uint16_t color);

void drawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* image);

void drawXLine(uint16_t start_x, uint16_t start_y, uint16_t length, uint16_t color);
void drawYLine(uint16_t start_x, uint16_t start_y, uint16_t length, uint16_t color);

void printLine(uint16_t start_x, uint16_t start_y, const char *str);
void printLineWithBox(uint16_t start_x, uint16_t start_y, const char *str, uint16_t background_color, uint8_t inverse_font_color);

void drawRectangle(uint16_t start_x, uint16_t start_y, uint8_t width, uint8_t height, uint16_t color);
} // namespace ST7789