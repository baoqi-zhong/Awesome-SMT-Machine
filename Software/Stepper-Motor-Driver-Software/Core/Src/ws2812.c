/**
 * @file ws2812.c
 * @author Baoqi (zzhongas@connect.ust.hk); Guo Zilin
 *
 * @copyright Copyright (c) 2025
 */

#include "ws2812.h"

#include "string.h"

#define BIT1_WIDTH 137
#define BIT0_WIDTH 69
#define DMA_TX_BUFFER_LENGTH 500

typedef struct
{
    RGB rgbs[LED_NUM];
    unsigned char updatedFlag;
    unsigned char txFlag;
} RGBStatus;

static volatile RGBStatus rgbStatus;
static volatile int ws2812_isInit = 0;

void PWM_DMA_TransmitFinshed_Callback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2)
    {
        HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_2);
        rgbStatus.txFlag = 0;
    }
}

static unsigned int txFailCnt = 0;
static long unsigned int CCRDMABuff[LED_NUM * sizeof(RGB) * 8 + 1];
void PWM_DMA_TransmitError_Callback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2)
    {
        HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_2);
        if (++txFailCnt < 10)
            HAL_TIM_PWM_Start_DMA(htim, TIM_CHANNEL_2, CCRDMABuff, LED_NUM * sizeof(RGB) * 8 + 1);
        else
        {
            txFailCnt        = 0;
            rgbStatus.txFlag = 0;
        }
    }
}

void WS2812_Init()
{
    if (ws2812_isInit)
        return;
    ws2812_isInit = 1;
    HAL_TIM_RegisterCallback(&htim2, HAL_TIM_PWM_PULSE_FINISHED_CB_ID, PWM_DMA_TransmitFinshed_Callback);
    HAL_TIM_RegisterCallback(&htim2, HAL_TIM_ERROR_CB_ID, PWM_DMA_TransmitError_Callback);

    blankAll();
    HAL_Delay(1);  // Delay the whole module for 1ms
}

void ws2812Callback()
{
    if (!rgbStatus.updatedFlag)
        return;
    if (rgbStatus.txFlag)
        return;
    rgbStatus.txFlag = 1;
    unsigned int data;

    /*Pack the RGB Value*/
    for (unsigned int i = 0; i < LED_NUM; i++)
    {
        data = *(unsigned volatile int *)(&rgbStatus.rgbs[i]);
        for (unsigned int j = 0; j < sizeof(RGB) * 8; j++)
            CCRDMABuff[i * sizeof(RGB) * 8 + j] = ((1UL << (23 - j)) & data) ? BIT1_WIDTH : BIT0_WIDTH;
    }
    CCRDMABuff[LED_NUM * sizeof(RGB) * 8] = 0;

    /*Transmit DMA*/
    HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_2, CCRDMABuff, LED_NUM * sizeof(RGB) * 8 + 1);
    rgbStatus.updatedFlag = 0;
}

void blink(int index, unsigned char r, unsigned char g, unsigned char b)
{
    if (!ws2812_isInit)
        return;
    if (index < 0 || index >= LED_NUM)
        return;
    if (rgbStatus.txFlag)  // Could not modify if it is sending message
        return;
    rgbStatus.updatedFlag = 1;
    volatile RGB *pRgb    = &rgbStatus.rgbs[index];
    pRgb->blue            = b;
    pRgb->green           = g;
    pRgb->red             = r;
}

void blank(int index)
{
    if (!ws2812_isInit)
        return;
    blink(index, 0, 0, 0);
}

void blankAll()
{
    for (int i = 0; i < LED_NUM; i++)
        blank(i);
}
