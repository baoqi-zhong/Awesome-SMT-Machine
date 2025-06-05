/**
 * @file tick.hpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "tim.h"
#include "stdint.h"

extern uint32_t vTick;

void tickCallback(TIM_HandleTypeDef *htim);

/**
* @brief This function provides minimum delay (in milliseconds).
* @param Delay specifies the delay time length, in milliseconds.
*/
void HAL_Delay(uint32_t Delay);

void Tick_Init();