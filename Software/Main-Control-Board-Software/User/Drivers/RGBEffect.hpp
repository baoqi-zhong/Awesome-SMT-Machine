/**
 * @file RGBEffect.hpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "tick.hpp"

namespace RGBeffect
{
void rainbow(uint32_t period, uint8_t brightness, uint8_t r, uint8_t g, uint8_t b, uint8_t switchtime);
void rainbow_Update(uint32_t tick, uint32_t period, uint8_t brightness, uint8_t multiplier, uint8_t r, uint8_t g, uint8_t b);

void uniColorSin(uint32_t period, uint8_t brightness, uint8_t r, uint8_t g, uint8_t b, uint8_t switchtime);
void uniColorSin_Update(uint32_t tick, uint32_t period, uint8_t brightness, uint8_t multiplier, uint8_t r, uint8_t g, uint8_t b);

void uniColor(uint8_t r, uint8_t g, uint8_t b, uint8_t switchtime);
void uniColor_Update(uint8_t r, uint8_t g, uint8_t b, uint8_t multiplier);

void updateEffect();

void startLightEffect();

void idleLightEffect();

void calibrationLightEffect();

void errorLightEffect();

void SMTLightEffect();

void finishSMTLightEffect();

}