/**
 * @file Calibrator.c
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
    float IalphaADCBias;
    float IbetaADCBias;

    float IalphaADCOnCurrent;
    float IbetaADCOnCurrent;

    float VBusADCValue;
} ADCCalibrationResult_t;


void calibrateEncoder();
void calibrateCurrentSensor();