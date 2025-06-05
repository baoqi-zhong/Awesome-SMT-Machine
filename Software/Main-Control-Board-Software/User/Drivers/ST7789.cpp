/**
 * @file ST7789.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "ST7789.hpp"
#include "string.h"
#include "font.hpp"

#include "FreeRTOS.h"
#include "task.h"

extern DMA_HandleTypeDef hdma_spi1_tx;

namespace ST7789
{
uint8_t txBuffer[8];
uint8_t rxBuffer[8];
uint8_t buffer[ST7789_WIDTH*ST7789_HEIGHT*2 / 16];



uint8_t INIT_SEQUENCE[][2] = {

    {ST7789_DISPLAY_OFF, 0}, // Display OFF
    {ST7789_SLEEP_OUT, 0}, // Sleep OUT

	{0xB2, 0}, // Porch Control
	{0x01, 1}, 
	{0x01, 1},
	{0x00, 1},
	{0x33, 1},
	{0x33, 1},

    {0xB7, 0}, // Gate Control
	{0x35, 1}, 

	{0xBB, 0}, // VCOM Setting
	{0x35, 1},

    {0xC0, 0}, // LCM Control.
	{0x2C, 1},

	{0xC2, 0}, // Power Control.
	{0x01, 1},

    {0xC3, 0}, // Power Control.
	{0x13, 1},  

	{0xC4, 0}, // Power Control.
	{0x20, 1},

    {0xB3, 0}, // Frame Rate Conrtrol 1
    {0x00, 1}, // frame rate div: 0x00 fastest; 0x03 slowest
    {0x00, 1}, // unconcerned
    {0x00, 1}, // unconcerned

	{0xC6, 0}, // Frame Rate Conrtrol 2
	{0x01, 1}, // 0x00 fastest; 0x1F slowest

	{0xD0, 0}, // Power Control.
	{0xA4, 1},
	{0xA1, 1},

    {0x36, 0}, // Memory Access Direction
    {ST7789_MEMORY_ACCCESS_CONTROL_ROW_REVERSE | ST7789_MEMORY_ACCCESS_CONTROL_COL_NORMAL | ST7789_MEMORY_ACCCESS_CONTROL_ROW_COL_SWAP, 1},
    
    {0xE0, 0}, // Positive Gamma Correction
	{0xF0, 1},
	{0x00, 1},
	{0x04, 1},
	{0x04, 1},
	{0x04, 1},
	{0x05, 1},
	{0x29, 1},
	{0x33, 1},
	{0x3E, 1},
	{0x38, 1},
	{0x12, 1},
	{0x12, 1},
	{0x28, 1},
	{0x30, 1},

	{0xE1, 0}, // Negative Gamma Correction
	{0xF0, 1},
	{0x07, 1},
	{0x0A, 1},
	{0x0D, 1},
	{0x0B, 1},
	{0x07, 1},
	{0x28, 1},
	{0x33, 1},
	{0x3E, 1},
	{0x36, 1},
	{0x14, 1},
	{0x14, 1},
	{0x29, 1},
	{0x32, 1},

    {0x3A, 0},  // RGB565
    {0x05, 1},
    {0x20, 0}, // Display Inversion OFF
    {0x29, 0}, // Display ON
};


void init()
{
    HAL_GPIO_WritePin(LCD_BG_GPIO_Port, LCD_BG_Pin, GPIO_PIN_RESET);
    
    // Ӳ����λ
    HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_RESET);
    vTaskDelay(200); // no less then 120.  
    HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_SET);
    vTaskDelay(200);

    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    for(uint8_t i = 0; i < sizeof(INIT_SEQUENCE) / sizeof(INIT_SEQUENCE[0]); i++)
    {
        HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, (GPIO_PinState)INIT_SEQUENCE[i][1]);
        HAL_SPI_Transmit(&hspi1, &INIT_SEQUENCE[i][0], 1, 100);
    }
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

    fillScreen(COLOR_WHITE);
    HAL_GPIO_WritePin(LCD_BG_GPIO_Port, LCD_BG_Pin, GPIO_PIN_SET);
}

void readDisplayID()
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);

    txBuffer[0] = 0x04;
    
    HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer, 1, 100);
    txBuffer[0] = 0x00;
    txBuffer[1] = 0x00;
    txBuffer[2] = 0x00;

    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer, 3, 100);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void readDisplayStatus()
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);

    txBuffer[0] = 0x09;
    
    HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer, 1, 100);
    txBuffer[0] = 0x00;
    txBuffer[1] = 0x00;
    txBuffer[2] = 0x00;
    txBuffer[3] = 0x00;

    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer, 4, 100);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}


void MY_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size)
{
    hspi->pTxBuffPtr  = (uint8_t *)pData;
    hspi->TxXferCount = Size;

    if (Size == 0x01U)
    {
        *((__IO uint8_t *)&hspi->Instance->DR) = (*hspi->pTxBuffPtr);
        hspi->pTxBuffPtr += sizeof(uint8_t);
        hspi->TxXferCount--;
    }
    while (hspi->TxXferCount > 0U)
    {
        /* Wait until TXE flag is set to send data */
        if (__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_TXE))
        {
            *((__IO uint8_t *)&hspi->Instance->DR) = (*hspi->pTxBuffPtr);
            hspi->pTxBuffPtr += sizeof(uint8_t);
            hspi->TxXferCount--;
        }
    }

    __HAL_SPI_CLEAR_OVRFLAG(hspi);
}

// void MY_SPI_DMATransmitCplt(DMA_HandleTypeDef *hdma)
// {
//     SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)(((DMA_HandleTypeDef *)hdma)->Parent); /* Derogation MISRAC2012-Rule-11.5 */

//     /* Disable Tx DMA Request */
//     CLEAR_BIT(hspi->Instance->CR2, SPI_CR2_TXDMAEN);
//     __HAL_SPI_CLEAR_OVRFLAG(hspi);

//     hspi->State = HAL_SPI_STATE_READY;
//     HAL_SPI_TxCpltCallback(hspi);
// }

// inline void MY_SPI_Transmit_DMA(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size)
// {
//     hspi->hdmatx->XferCpltCallback = MY_SPI_DMATransmitCplt;

//     hspi->hdmatx->Instance->CR &= (uint32_t)(~DMA_SxCR_DBM);
//     hspi->hdmatx->Instance->NDTR = Size;
//     hspi->hdmatx->Instance->PAR = (uint32_t)&hspi->Instance->DR;
//     hspi->hdmatx->Instance->M0AR = (uint32_t)pData;
//     hspi->hdmatx->Instance->CR  |= DMA_IT_TC | DMA_IT_TE | DMA_IT_DME;

//     __HAL_DMA_ENABLE(hspi->hdmatx);

//     /* Enable Tx DMA Request */
//     SET_BIT(hspi->Instance->CR2, SPI_CR2_TXDMAEN);
// }

void openWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    txBuffer[0] = 0x2A; // Column Address Set
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    MY_SPI_Transmit(&hspi1, txBuffer, 1);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);


    txBuffer[0] = x >> 8;
    txBuffer[1] = x & 0xFF;
    txBuffer[2] = (x + w -1) >> 8;
    txBuffer[3] = (x + w -1) & 0xFF;
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    MY_SPI_Transmit(&hspi1, txBuffer, 4);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

    txBuffer[0] = 0x2B; // Row Address Set
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    MY_SPI_Transmit(&hspi1, txBuffer, 1);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

    txBuffer[0] = y >> 8;
    txBuffer[1] = y & 0xFF;
    txBuffer[2] = (y + h - 1 + 200) >> 8;
    txBuffer[3] = (y + h - 1 + 200) & 0xFF;
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    MY_SPI_Transmit(&hspi1, txBuffer, 4);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

    txBuffer[0] = 0x2C; // Memory Write
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    MY_SPI_Transmit(&hspi1, txBuffer, 1);
}

void drawRectangle(uint16_t start_x, uint16_t start_y, uint8_t width, uint8_t height, uint16_t color)
{
    assert_param(width * height * 2 <= sizeof(buffer));
    openWindow(start_x, start_y, width, height);
    for(uint16_t i = 0; i < width * height; i++)
    {
        buffer[i*2] = color >> 8;
        buffer[i*2+1] = color & 0xFF;
    }
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    MY_SPI_Transmit(&hspi1, buffer, width * height * 2 );
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void fillScreen(uint16_t color)
{
    openWindow(0, 0, ST7789_WIDTH, ST7789_HEIGHT);

    for(uint16_t i = 0; i < ST7789_WIDTH * ST7789_HEIGHT * 2 / 16 / 2; i++)
    {
        buffer[i*2] = color >> 8;
        buffer[i*2+1] = color & 0xFF;
    }
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    for(int i = 0; i < 16; i++)
    {
        MY_SPI_Transmit(&hspi1, buffer, ST7789_WIDTH * ST7789_HEIGHT * 2 / 16 );
        // while(HAL_DMA_GetState(&hdma_spi1_tx) != HAL_DMA_STATE_READY);
    }
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void drawImage(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, const uint8_t* image)
{
    openWindow(start_x, start_y, width, height);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    MY_SPI_Transmit(&hspi1, (uint8_t*)image, width * height * 2);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}


void printChar(uint16_t start_x, uint16_t start_y, const char char_)
{
    if(char_ == ' ') 
    {
        drawRectangle(start_x, start_y, FONT_WIDTH, FONT_HEIGHT, COLOR_WHITE);
        return;
    }
    openWindow(start_x, start_y, FONT_WIDTH, FONT_HEIGHT);
    unsigned int font_start_add = (char_ - '!') * FONT_WIDTH * FONT_HEIGHT;
    uint16_t processed_color = 0;
    uint16_t brightness = 0;

    for(int i = 0; i < FONT_WIDTH * FONT_HEIGHT; i++)
    {
        brightness = fontConsolas[font_start_add+i];
        processed_color = ((brightness & 0xF8) << 8) | ((brightness & 0xFC) << 3) | ((brightness & 0xF8) >> 3);
        buffer[i*2] = processed_color >> 8;
        buffer[i*2+1] = processed_color & 0xFF;
    }
    

    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    MY_SPI_Transmit(&hspi1, buffer, 440);
}


void printLine(uint16_t start_x, uint16_t start_y, const char *str)
{
    int i = 0;
    while(str[i])
    {
        if(start_x + FONT_WIDTH < 0)
        {
            continue;
        }
        if(start_x > ST7789_WIDTH - FONT_WIDTH) break;
        printChar(start_x, start_y, str[i]);
        start_x += FONT_WIDTH;
        i++;
    }
}


void drawYLine(uint16_t start_x, uint16_t start_y, uint16_t length, uint16_t color)
{
    openWindow(start_x, start_y, 1, length);

    for(int i = 0; i < length; i++)
    {
        buffer[i*2] = color >> 8;
        buffer[i*2+1] = color & 0xFF;
    }
    
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    MY_SPI_Transmit(&hspi1, buffer, length*2);
}

void drawXLine(uint16_t start_x, uint16_t start_y, uint16_t length, uint16_t color)
{
    openWindow(start_x, start_y, length, 1);

    for(int i = 0; i < length; i++)
    {
        buffer[i*2] = color >> 8;
        buffer[i*2+1] = color & 0xFF;
    }
    
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    MY_SPI_Transmit(&hspi1, buffer, length*2);
}

void printCharWithBackgroundColor(uint16_t start_x, uint16_t start_y, const char char_, uint16_t background_color, uint8_t inverse_font_color)
{
    if(char_ == ' ') 
    {
        drawRectangle(start_x, start_y, FONT_WIDTH, FONT_HEIGHT, background_color);
        return;
    }

    openWindow(start_x, start_y, FONT_WIDTH, FONT_HEIGHT);
    unsigned int font_start_add = (char_ - '!') * FONT_WIDTH * FONT_HEIGHT;
    uint16_t processed_color = 0;
    int brightness = 0;
    int background_color_r = background_color >> 11;
    int background_color_g = (background_color >> 5) & 0x3F;
    int background_color_b = background_color & 0x1F;

    if(inverse_font_color)
    {
        for(int i = 0; i < FONT_WIDTH * FONT_HEIGHT; i++)
        {
            brightness = 0xFF - (fontConsolas[font_start_add+i]);
            processed_color = ((background_color_r + (31-background_color_r) * brightness / 0xFF) << 11) | ((background_color_g + (63-background_color_g) * brightness / 0xFF) << 5) | (background_color_b + (31-background_color_b) * brightness / 0xFF);
            buffer[i*2] = processed_color >> 8;
            buffer[i*2+1] = processed_color & 0xFF;
        }
    }
    else
    {
        for(int i = 0; i < FONT_WIDTH * FONT_HEIGHT; i++)
        {
            brightness = fontConsolas[font_start_add+i];
            processed_color = ((background_color_r * brightness / 0xFF) << 11) | ((background_color_g * brightness / 0xFF) << 5) | (background_color_b * brightness / 0xFF);
            buffer[i*2] = processed_color >> 8;
            buffer[i*2+1] = processed_color & 0xFF;
        }
    }

    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    MY_SPI_Transmit(&hspi1, buffer, FONT_WIDTH * FONT_HEIGHT * 2);
}


void printLineWithBox(uint16_t start_x, uint16_t start_y, const char *str, uint16_t background_color, uint8_t inverse_font_color)
{
    drawYLine(start_x + 1, start_y + 1, 18, background_color);
    drawYLine(start_x, start_y + 2, 16, background_color);

    int i = 0;
    while(str[i])
    {
        printCharWithBackgroundColor(start_x + 2 + FONT_WIDTH * i, start_y, str[i], background_color, inverse_font_color);
        i++;
    }
    drawYLine(start_x + FONT_WIDTH * strlen(str) + 2, start_y + 1, 18, background_color);
    drawYLine(start_x + FONT_WIDTH * strlen(str) + 3, start_y + 2, 16, background_color);
}


} // namespace ST7789