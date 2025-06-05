/**
 * @file LightEffect.c
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"

void lightEffect_update();

void lightEffect_Rainbow(uint16_t period, uint8_t WS2812_brightness);

void lightEffect_Rainbow_H(uint8_t index, uint16_t* r, uint16_t* g, uint16_t* b, uint32_t tick, uint16_t period, uint8_t WS2812_brightness);

void lightEffect_Angle(uint16_t period, uint8_t WS2812_brightness, uint8_t rk, uint8_t gk, uint8_t bk);

void lightEffect_Angle_H(uint8_t index, uint16_t* r, uint16_t* g, uint16_t* b, uint32_t tick, uint16_t period, uint8_t WS2812_brightness);

void lightEffect_UniColor(uint8_t r, uint8_t g, uint8_t b);