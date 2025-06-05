/**
 * @file ErrorHandler.c
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
    uint8_t driverFault;
    uint8_t underOverVoltage;
    uint8_t overTemperatureWarning;
    uint8_t overTemperature;

    uint8_t overCurrent;

    uint8_t motorStalled;
    uint8_t lastControlPacketDecodeError;
} ErrorStatus_t;

typedef struct 
{
    uint16_t underOverVoltageCounter;
    uint16_t overTemperatureCounter;
    uint16_t overCurrentCounter;
} ErrorCounter_t;

extern ErrorStatus_t motorErrorStatus;

void ErrorHandlerInit();

uint8_t ErrorHandlerCheckIfAnyError();

void ErrorHandler();

void ErrorHandlerClearErrorStatus();