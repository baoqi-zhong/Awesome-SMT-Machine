/**
 * @file ZeroTrigger.hpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "stdint.h"


namespace ZeroTrigger
{
typedef struct
{
    uint8_t XTriggered;
    uint8_t YTriggered;
} ZeroTriggerStatus_t;

extern ZeroTriggerStatus_t ZeroTriggerStatus;

void update();

void Init();
}