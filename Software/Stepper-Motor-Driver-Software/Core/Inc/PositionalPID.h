/**
 * @file PositionalPID.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "GeneralConfig.h"
#include "tim.h"
#include "stdint.h"

/**
 * @param kPonError         Kp
 * @param kIonError         Ki
 * @param kDonMeasurement   Kd
 * @param kPonMeasurement   Kp on measurement, used to reduce or cancel the unavoidable overshoot caused by integral term
 * @param kDonTarget        Kd on target, make the derivative term more sensitive to the change of the target value
 * @param alpha             alpha for derivative term low pass filter
 * @param outputLimit       abs output upper limit
 */
typedef struct
{
    float kPonError;
    float kIonError;
    float kDonMeasurement;
    float kPonMeasurement;
    float kDonTarget;
    float alpha;
    float outputLimit;
    float updateFrequency;

    float lastTarget;
    float lastMeasurement;
    float pOut;
    float iOut;
    float dOut;

    float output;
} PositionalPID_t;

/**
 * @brief Set parameters for the Positional PID Controller
 * @param pid Pointer to the PositionalPID_t struct
 * @param kPonError Proportional gain on error
 * @param kIonError Integral gain on error
 * @param kDonMeasurement Derivative gain on measurement
 * @param kPonMeasurement Proportional gain on measurement
 * @param kDonTarget Derivative gain on target
 * @param alpha Alpha for derivative term low pass filter
 * @param outputLimit Output limit of the controller
 * @param updateFrequency Update frequency of the controller
 */
void PositionalPIDsetParameters(PositionalPID_t *pid, float kPonError, float kIonError, float kDonMeasurement, float kPonMeasurement, float kDonTarget, float alpha, float outputLimit, float updateFrequency);

/**
 * @brief Update the Positional PID Controller with the new target and measurement
 * @param pid Pointer to the PositionalPID_t struct
 * @param target Target value
 * @param measurement Current measurement value
 * @return Calculated output of the controller
 */
float PositionalPIDupdate(PositionalPID_t *pid, float target, float measurement);

/**
 * @brief Reset the state of the Positional PID Controller
 * @param pid Pointer to the PositionalPID_t struct
 */
void PositionalPIDreset(PositionalPID_t *pid);