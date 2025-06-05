/**
 * @file UIManager.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "UIManager.hpp"
#include "ST7789.hpp"
#include "stdio.h"


namespace UIManager
{
UIManagerStatus_t UIManagerStatus;

void init()
{
    UIManagerStatus.newComponent = 0;
    ST7789::init();

    ST7789::printLineWithBox(6, 6, "Auto SMT Machine", COLOR_BLUE, 1);
    ST7789::printLine(6, 35, "ELEC3300 Group4");
    ST7789::drawXLine(6, 65, 200, COLOR_GRAY);
    ST7789::drawXLine(6, 66, 200, COLOR_GRAY);

    ST7789::printLineWithBox(ST7789_WIDTH - 10 - FONT_WIDTH * 3, 38, "Go!", COLOR_BLUE, 1);
    // ST7789::printLineWithBox(ST7789_WIDTH - 10 - FONT_WIDTH * 2, 93, "Up", COLOR_BLACK, 1);
    // ST7789::printLineWithBox(ST7789_WIDTH - 10 - FONT_WIDTH * 3, 143, "DWN", COLOR_BLACK, 1);
    ST7789::printLineWithBox(ST7789_WIDTH - 10 - FONT_WIDTH * 4, 195, "Stop", COLOR_RED, 1);
    newComponent("R12", 0);
    ST7789::printLine(30, 130, "0805");
}

void newComponent(const char* name, uint8_t index)
{
    for(uint8_t i = 0; i < 4; i++)
    {
        UIManagerStatus.name[i] = name[i];
    }
    UIManagerStatus.index = index;
    UIManagerStatus.newComponent = 1;
}

void drawNewComponent(uint16_t start_x, uint16_t start_y, const char* name)
{
    uint16_t componentWidth = 80;
    uint16_t componentHeight = 40;
    // 边框
    ST7789::drawXLine(start_x, start_y, componentWidth, COLOR_BLACK);
    ST7789::drawXLine(start_x, start_y + 1, componentWidth, COLOR_BLACK);
    ST7789::drawXLine(start_x, start_y + componentHeight, componentWidth, COLOR_BLACK);
    ST7789::drawXLine(start_x, start_y + componentHeight - 1, componentWidth, COLOR_BLACK);
    ST7789::drawYLine(start_x, start_y, componentHeight, COLOR_BLACK);
    ST7789::drawYLine(start_x + 1, start_y, componentHeight, COLOR_BLACK);
    ST7789::drawYLine(start_x + componentWidth, start_y, componentHeight, COLOR_BLACK);
    ST7789::drawYLine(start_x + componentWidth - 1, start_y, componentHeight, COLOR_BLACK);

    uint16_t color;
    if(name[0] == 'C')
        color = COLOR_YELLOW;
    else
        color = COLOR_BLACK;
    ST7789::drawRectangle(start_x + componentWidth / 6, start_y + 2, componentWidth * 2 / 3, componentHeight - 3, color);

    ST7789::printLineWithBox(start_x + 20, start_y + 10, name, color, 1);
}

void drawProgressBar(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, uint8_t progress)
{
    ST7789::drawXLine(start_x, start_y, width, COLOR_GRAY);
    ST7789::drawXLine(start_x, start_y + 1, width, COLOR_GRAY);
    ST7789::drawXLine(start_x, start_y + height, width, COLOR_GRAY);
    ST7789::drawXLine(start_x, start_y + height - 1, width, COLOR_GRAY);
    ST7789::drawYLine(start_x, start_y, height, COLOR_GRAY);
    ST7789::drawYLine(start_x + 1, start_y, height, COLOR_GRAY);
    ST7789::drawYLine(start_x + width, start_y, height, COLOR_GRAY);
    ST7789::drawYLine(start_x + width - 1, start_y, height, COLOR_GRAY);

    ST7789::drawRectangle(start_x + 2, start_y + 2, (width - 3) * progress / 10, height - 3, COLOR_BLUE);
    uint8_t progressStr[8];
    sprintf((char *)progressStr, "%3d%%", (uint16_t)(progress) * 100 / 10);
    ST7789::printLine(start_x + width + 10, start_y - 1, (char *)progressStr);
}

void update()
{
if(UIManagerStatus.newComponent)
{
    UIManagerStatus.newComponent = 0;
    drawNewComponent(15, 80, UIManagerStatus.name);

    // 进度条
    drawProgressBar(10, 210, 130, 15, UIManagerStatus.index);
}
}
} // namespace UIManager