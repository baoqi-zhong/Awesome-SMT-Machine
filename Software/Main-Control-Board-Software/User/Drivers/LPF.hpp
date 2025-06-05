/**
 * @file LPF.hpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

 #pragma once

#include "main.h"
#include "stdint.h"

class LPF
{
public:
    LPF(float alpha);

    float update(float value);

private:
    float alpha;
    float lastValue;
};