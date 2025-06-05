/**
 * @file LPF.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

// 低通滤波器
#include "LPF.h"

void LPF_init(LPF_t *lpf, float alpha)
{
    lpf->alpha = alpha;
    lpf->lastValue = 0.0f;
}

float LPF(LPF_t *lpf, float value)
{
    lpf->lastValue = value * lpf->alpha + lpf->lastValue * (1 - lpf->alpha);
    return lpf->lastValue;
}