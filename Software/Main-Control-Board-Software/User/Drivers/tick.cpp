/**
 * @file tick.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "tick.hpp"
#include "stdint.h"
#include "tim.h"
#include "RGBEffect.hpp"

uint32_t vTick = 0;

void tickCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6)
    {
        vTick++;
        //extern void buzzerCallback();
        extern void ws2812Callback();
        //buzzerCallback();
        if (vTick % 10 == 0)
        {
            RGBeffect::updateEffect();
            ws2812Callback();
        }
        
    }
}

/*This is a re-implementation of the HAL libarary delay function*/
/*To gurarantee an accurate delay function*/

void HAL_Delay(uint32_t Delay)
{
    uint32_t tickstart = HAL_GetTick();
    uint32_t wait      = Delay;

    while ((HAL_GetTick() - tickstart) < wait)
    {
        // Do nothing
    };
}

void Tick_Init()
{
    HAL_TIM_RegisterCallback(&htim6, HAL_TIM_PERIOD_ELAPSED_CB_ID, tickCallback);
    HAL_TIM_Base_Start_IT(&htim6);
}
