
/**
 * @file IncrementalPID.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 * @note THIS PID IS VERY BUGGY, SOME CALC IS EVEN WRONG, DO NOT USE IT ELSEWHERE !!!!
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "main.h"
#include "GeneralConfig.h"

#include "stdint.h"

typedef struct
{
    float kPOnTarget;
    float kPOnMeasurement;
    float kI;
    float kD;
    float outputLimit;
    float updateFrequency;

    float lastMeasurement;
    float lastLastMeasurement;
    float lastTarget;

    float deltaOutput;
    float output;
} IncrementalPID_t;


/**
 * @brief Set parameters for the Incremental PID Controller
 * @param pid Pointer to the IncrementalPID_t struct
 * @param kPOnTarget Proportional gain on target
 * @param kPOnMeasurement Proportional gain on measurement
 * @param kI Integral gain
 * @param kD Derivative gain
 * @param outputLimit Output limit of the controller
 * @param updateFrequency Update frequency of the controller
 */
void IncrementalPIDsetParameters(IncrementalPID_t *pid, float kPOnTarget, float kPOnMeasurement, float kI, float kD, float outputLimit, float updateFrequency);

/**
 * @brief Update the Incremental PID Controller
 * @return Accumulated output of the controller(Note: Not the intremental output)
 * @param pid Pointer to the IncrementalPID_t struct
 * @param target Target value
 * @param measurement Current measurement value
 * @return Calculated output of the controller
 */
float IncrementalPIDupdate(IncrementalPID_t *pid, float target, float measurement);

/**
 * @brief Reset the state of the Incremental PID Controller
 * @param pid Pointer to the IncrementalPID_t struct
 */
void IncrementalPIDreset(IncrementalPID_t *pid);


/**
 * @brief Set the output of the Incremental PID Controller
 * @param pid Pointer to the IncrementalPID_t struct
 * @param output Output value
 */
void IncrementalPIDSetOutput(IncrementalPID_t *pid, float output);