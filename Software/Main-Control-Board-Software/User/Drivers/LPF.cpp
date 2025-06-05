
/**
 * @file LPF.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

 #include "LPF.hpp"

LPF::LPF(float _alpha)
{
    this->alpha = _alpha;
    this->lastValue = 0;
}

float LPF::update(float value)
{
    this->lastValue = this->alpha * value + (1 - alpha) * this->lastValue;
    return this->lastValue;
}