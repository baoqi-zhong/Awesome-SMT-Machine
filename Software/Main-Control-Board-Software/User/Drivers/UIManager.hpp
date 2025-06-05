/**
 * @file UIManager.hpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "stdint.h"

namespace UIManager
{
typedef struct
{
    uint8_t newComponent;
    char name[4];
    uint8_t index;
} UIManagerStatus_t;

void init();

void newComponent(const char* name, uint8_t index);

void update();
} // namespace UIManager
