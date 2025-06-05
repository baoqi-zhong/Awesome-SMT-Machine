/**
 * @file LightEffect.c
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "LightEffect.h"
#include "ws2812.h"
#include "tick.h"
#include "MA732.h"
#include "fakeSin.h"

enum lightEffectState {LS_Rainbow, LS_Angle, LS_UniColor, LS_RtA, LS_AtR};

enum lightEffectState ledCurrentState = LS_UniColor;


typedef struct
{
    uint8_t rk;
    uint8_t gk;
    uint8_t bk;
    uint16_t period;
    uint8_t WS2812_brightness;
} RGBparam;

RGBparam ledUniPara;
RGBparam ledRainbowPara;
RGBparam ledAnglePara;

#define idleTime 3000
#define SwithTime 255
uint16_t idleCounter = 0;
uint8_t mixer = 0;

float aaa = 0;

void lightEffect_update()
{
    if(shaftAngularVelocityFloat < 1.5f && shaftAngularVelocityFloat > -1.5f)
    {
        if(idleCounter <= idleTime - 1)
            idleCounter += 1;
    }
    else
    {
        if(idleCounter >= 10)
            idleCounter -= 10;
    }

    
    static uint16_t r = 0, g = 0, b = 0;
    for(uint8_t i=0; i<8; i++)
    {
        r = 0;
        g = 0;
        b = 0;


        if(idleCounter > 1980)
            {mixer =  (idleCounter - 1980)/4;}
        else
            {mixer = 0;}
        
        lightEffect_Rainbow_H(i, &r, &g, &b, vTick, 2000, mixer / 7);
        lightEffect_Angle_H(i, &r, &g, &b, vTick, 1000, 255-mixer);

        if (ledCurrentState == LS_UniColor)
        {
            r = ledUniPara.rk;
            g = ledUniPara.gk;
            b = ledUniPara.bk;
        }

        blink(i, r, g, b);
    }

}

void lightEffect_UniColor(uint8_t r, uint8_t g, uint8_t b)
{
    
    ledCurrentState = LS_UniColor;
    ledUniPara.rk = r;
    ledUniPara.gk = g;
    ledUniPara.bk = b;
}


void lightEffect_Rainbow(uint16_t period, uint8_t WS2812_brightness)
{

    ledCurrentState = LS_RtA;

    ledRainbowPara.period = period;
    ledRainbowPara.WS2812_brightness = WS2812_brightness;
    ledRainbowPara.rk = 40;
    ledRainbowPara.gk = 40;
    ledRainbowPara.bk = 40;
}

void lightEffect_Rainbow_H(uint8_t index, uint16_t* r, uint16_t* g, uint16_t* b, uint32_t tick, uint16_t period, uint8_t brightness)
{
    //brightness = brightness / 2;
    float f = 0;
    if (CAN_ID == 1)
        f = ((float)tick / (float)period * 6.28318f + index * 6.28318f / LED_NUM);
    else if (CAN_ID == 2)
        f = ((float)tick / (float)period * 6.28318f - index * 6.28318f / LED_NUM);
    else
        f = ((float)tick / (float)period * 6.28318f);
    *r += (fakeSin(f) * brightness + brightness/2);
    *g += (fakeSin(f + 6.28318f/3.0f) * brightness + brightness/2);
    *b += (fakeSin(f + 6.28318f/1.5f) * brightness + brightness/2);
    
}

void lightEffect_Angle(uint16_t period, uint8_t WS2812_brightness, uint8_t rk, uint8_t gk, uint8_t bk)
{
    ledCurrentState = LS_Angle;
    ledAnglePara.period = period;
    ledAnglePara.WS2812_brightness = WS2812_brightness;
    ledAnglePara.rk = rk;
    ledAnglePara.gk = gk;
    ledAnglePara.bk = bk;
}

void lightEffect_Angle_H(uint8_t index, uint16_t* r, uint16_t* g, uint16_t* b, uint32_t tick, uint16_t period, uint8_t brightness)
{
    brightness = brightness;
    float angle = encoder /65535.0f * 6.28318f;
    int16_t l = (fakeCos(angle - index * 3.14159f/4.0f) * brightness);
    
    if(l < 0)
        l = 0;
    *r += l * ledAnglePara.rk / 255 *1.5f;
    *g += l * ledAnglePara.gk / 255 *1.5f;
    *b += l * ledAnglePara.bk / 255 *1.5f;
} 

