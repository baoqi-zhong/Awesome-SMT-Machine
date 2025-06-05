/**
 * @file fakeSin.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "fakeSin.h"
 
float fakeSin(float rad)
{
    // aaa = fakeCos(angle - PI / 2);
    rad = rad * 4.0f / 6.28318f;
    while(rad < -1)
        rad += 4;
    while(rad > 3)
        rad -= 4;
    if(rad <=1)
        return 0.5f - 0.5f * rad * rad;
    else
        return -0.5f + 0.5f * (rad - 2) * (rad - 2);
    //return fakeCos(angle - PI / 2);
}

float fakeCos(float rad)
{
    return fakeSin(rad + 1.5708f);
}