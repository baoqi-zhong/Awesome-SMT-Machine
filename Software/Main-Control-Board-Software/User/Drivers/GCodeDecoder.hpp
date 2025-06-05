
/**
 * @file GCodeDecoder.hpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "tick.hpp"
#include "stdint.h"

#define FIRMWARE_VERSION "1.0"
#define FIRMWARE_NAME "ELEC3300_SMT"
/*
M115: Get Firmware Version and Capabilities
*/

namespace GCodeDecoder
{
typedef struct
{
    uint8_t busyDecoding;

    uint32_t commandCounter;
    uint32_t commandErrorCounter;
    uint32_t unknownCommandCounter;
} GCodeDecoderStatus_t;

extern GCodeDecoderStatus_t GCodeDecoderStatus;

extern char feedbackBuffer[128];

void init();

void sendFeedback(const char* pBuffer);

void decodeMessage(uint8_t *rxData, uint8_t size);
}