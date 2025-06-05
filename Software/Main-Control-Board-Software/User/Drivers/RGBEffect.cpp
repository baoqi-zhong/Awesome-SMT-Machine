/**
 * @file RGBEffect.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "RGBEffect.hpp"
#include "ws2812.hpp"
#include "tick.hpp"
#include "math.h"


extern int16_t setSMTLightEffectRainbowAfterDelay;

namespace RGBeffect
{

enum RGBState
{
    RS_Empty,
    RS_Rainbow,
    RS_UniColorSin,
    RS_UniColor,
    RS_UniColorSin_to_Rainbow,
};
RGBState currentState = RS_Empty;
RGBState targetState = RS_Empty;
bool switching = false;
uint8_t SwitchTime = 200;


void Init()
{
    WS2812_Init();
    Tick_Init();
}

typedef struct
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
} RGBValue;

RGBValue rgbValueBuffer[LED_NUM];


// Input type 1:    r g b
typedef struct
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
} RGBInputType1;

RGBInputType1 currentType1;
RGBInputType1 targetType1;


void uniColor(uint8_t r, uint8_t g, uint8_t b, uint8_t switchtime = 200)
{
    targetType1.r = r;
    targetType1.g = g;
    targetType1.b = b;
    SwitchTime = switchtime;
    targetState = RS_UniColor;
    switching = true;
}

void uniColor_Update(uint8_t r, uint8_t g, uint8_t b, uint8_t multiplier = 1)
{
    for(int i = 0; i < LED_NUM; i++)
    {
        rgbValueBuffer[i].r += r * multiplier;
        rgbValueBuffer[i].g += g * multiplier;
        rgbValueBuffer[i].b += b * multiplier;
    }
}

// Input type 2:    period brightness
typedef struct
{
    uint32_t period = 0;
    uint8_t brightness = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
} RGBInputType2;

RGBInputType2 currentType2;
RGBInputType2 targetType2;

void rainbow(uint32_t period, uint8_t brightness, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t switchtime = 200)
{
    targetType2.period = period;
    targetType2.brightness = brightness;
    targetType2.r = r;
    targetType2.g = g;
    targetType2.b = b;
    SwitchTime = switchtime;
    targetState = RS_Rainbow;
    switching = true;
}

void rainbow_Update(uint32_t tick, uint32_t period, uint8_t brightness, uint8_t multiplier = 1, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255)
{
    brightness =brightness / 2;
    for(int i = 0; i < LED_NUM; i++)
    {
        float t = ((float)tick / (float)period * 6.28318f + i * 6.28318f / LED_NUM);
        uint8_t rv = (sin(t) * brightness + brightness) * r /255;
        uint8_t gv = (sin(t + 6.28318f/3.0f) * brightness + brightness) * g /255;
        uint8_t bv = (sin(t + 6.28318f/1.5f) * brightness + brightness) * b /255;
        rgbValueBuffer[i].r += rv * multiplier;
        rgbValueBuffer[i].g += gv * multiplier;
        rgbValueBuffer[i].b += bv * multiplier;
    }
}

void uniColorSin(uint32_t period, uint8_t brightness, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255,  uint8_t switchtime = 200)
{
    targetType2.period = period;
    targetType2.brightness = brightness;
    targetType2.r = r;
    targetType2.g = g;
    targetType2.b = b;
    SwitchTime = switchtime;
    targetState = RS_UniColorSin;
    switching = true;
}

void uniColorSin_Update(uint32_t tick, uint32_t period, uint8_t brightness, uint8_t multiplier = 1, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255)
{
    brightness = brightness / 2;
    float t = ((float)tick / (float)period * 6.28318f);
    uint8_t rv = (sin(t) * brightness + brightness) * r /255;
    uint8_t gv = (sin(t + 6.28318f/3.0f) * brightness + brightness) * g /255;
    uint8_t bv = (sin(t + 6.28318f/1.5f) * brightness + brightness) * b /255;
    for(int i = 0; i < LED_NUM; i++)
    {
        rgbValueBuffer[i].r += rv * multiplier;
        rgbValueBuffer[i].g += gv * multiplier;
        rgbValueBuffer[i].b += bv * multiplier;
    }
}

uint8_t switchCounter = 0;

void updateEffect()
{
    //uniColor(50, 50, 50);
    for(int i = 0; i < LED_NUM; i++)
    {
        rgbValueBuffer[i].r = 0;
        rgbValueBuffer[i].g = 0;
        rgbValueBuffer[i].b = 0;
    }

    if(!switching)
    {
        if(currentState == RS_Empty)
            uniColor(0, 0, 0);
        if(currentState == RS_Rainbow)
            rainbow_Update(vTick, currentType2.period, currentType2.brightness,1 , currentType2.r, currentType2.g, currentType2.b);
        if(currentState == RS_UniColorSin)
            uniColorSin_Update(vTick, currentType2.period, currentType2.brightness,1, currentType2.r, currentType2.g, currentType2.b);
        if(currentState == RS_UniColor)
            uniColor_Update(currentType1.r, currentType1.g, currentType1.b);
        for(int i = 0; i < LED_NUM; i++)
        {
            blink(i, rgbValueBuffer[i].r, rgbValueBuffer[i].g, rgbValueBuffer[i].b);
        }
    }
    
    else
    {
        if(switchCounter == SwitchTime)
        {
            currentState = targetState;
            switchCounter = 0;
            currentType2 = targetType2;
            currentType1 = targetType1;
            switching = false;
        }
            
        
        if(targetState == RS_Rainbow)
            rainbow_Update(vTick, targetType2.period, targetType2.brightness, switchCounter, targetType2.r, targetType2.g, targetType2.b);
        if(targetState == RS_UniColorSin)
            uniColorSin_Update(vTick, targetType2.period, targetType2.brightness, switchCounter, targetType2.r, targetType2.g, targetType2.b);
        if(targetState == RS_UniColor)
            uniColor_Update(targetType1.r, targetType1.g, targetType1.b, switchCounter);
        
        if(currentState == RS_Rainbow)
            rainbow_Update(vTick, currentType2.period, currentType2.brightness, SwitchTime - switchCounter, currentType2.r, currentType2.g, currentType2.b);
        if(currentState == RS_UniColorSin)
            uniColorSin_Update(vTick, currentType2.period, currentType2.brightness, SwitchTime - switchCounter, currentType2.r, currentType2.g, currentType2.b);
        if(currentState == RS_UniColor)
            uniColor_Update(currentType1.r, currentType1.g, currentType1.b, SwitchTime - switchCounter);
        switchCounter++;
        for(int i = 0; i < LED_NUM; i++)
        {
            blink(i, rgbValueBuffer[i].r / SwitchTime, rgbValueBuffer[i].g / SwitchTime, rgbValueBuffer[i].b / SwitchTime);
        }
    }   
}

void startLightEffect()
{
    // 蓝色
    rainbow(800, 255, 0, 0, 127, 100);
}

void idleLightEffect() 
{
    rainbow(3000, 255, 255, 255, 255, 100);
}

void calibrationLightEffect()
{
    // 黄色
    uniColor(200, 180, 0, 80);
}

void errorLightEffect()
{
    // 红色
    uniColor(255, 25, 25, 50);
}

void SMTLightEffect()
{
    uniColor(20, 200, 20, 20);
    setSMTLightEffectRainbowAfterDelay = 2000;
}

void finishSMTLightEffect()
{
    // 绿色
    uniColorSin(1000, 255);
}
}