/**
 * @file LPF.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "GeneralConfig.h"

#include "tim.h"


#include "stdint.h"


typedef struct
{
    float lastValue;
    float alpha;
} LPF_t;

/**
 * @brief Initialize the Low Pass Filter with the given alpha value
 * @param lpf Pointer to the LPF_t struct
 * @param alpha Alpha parameter for the filter
 */
void LPF_init(LPF_t *lpf, float alpha);

/**
 * @brief Update the Low Pass Filter with the new value
 * @param lpf Pointer to the LPF_t struct
 * @param value New input value to filter
 * @return Filtered output value
 */
float LPF(LPF_t *lpf, float value);