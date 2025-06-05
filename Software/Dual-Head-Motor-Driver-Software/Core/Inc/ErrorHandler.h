/**
 * @file ErrorHandler.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "GeneralConfig.h"

#include "stdint.h"

typedef struct 
{
    uint8_t haveError;
    uint8_t driverFault[3];
    uint8_t underOverVoltage;
    uint8_t overCurrent[3];
    uint8_t lastControlPacketDecodeError;
} ErrorStatus_t;

typedef struct 
{
    uint16_t underOverVoltageCounter;
    uint16_t overCurrentCounter[3];
} ErrorCounter_t;

extern ErrorStatus_t errorStatus;

void ErrorHandlerInit();

uint8_t ErrorHandlerCheckIfAnyError();

void ErrorHandler();

void ErrorHandlerClearErrorStatus();