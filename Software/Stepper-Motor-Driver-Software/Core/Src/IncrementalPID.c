/**
 * @file IncrementalPID.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 * @note THIS PID IS VERY BUGGY, SOME CALC IS EVEN WRONG, DO NOT USE IT ELSEWHERE !!!!
 *
 * @copyright Copyright (c) 2025
 */

#include "IncrementalPID.h"

void IncrementalPIDsetParameters(IncrementalPID_t *pid, float kPOnTarget, float kPOnMeasurement, float kI, float kD, float outputLimit, float updateFrequency)
{
    assert_param(outputLimit >= 0.0f);
    assert_param(updateFrequency > 0.0f);

    
    pid->kPOnTarget = kPOnTarget;
    pid->kPOnMeasurement = kPOnMeasurement;
    pid->kI = kI;
    pid->kD = kD;
    pid->outputLimit = outputLimit;
    pid->updateFrequency = updateFrequency;

    IncrementalPIDreset(pid);
}

float IncrementalPIDupdate(IncrementalPID_t *pid, float target, float measurement)
{
    pid->deltaOutput =  pid->kPOnTarget * (target - pid->lastTarget) - pid->kPOnMeasurement * (measurement - pid->lastMeasurement) +
                        pid->kI * (target - measurement) + 
                        pid->kD * (measurement - 2.0f * pid->lastMeasurement + pid->lastLastMeasurement);
    pid->lastTarget          = target;
    pid->lastLastMeasurement = pid->lastMeasurement;
    pid->lastMeasurement     = measurement;

    pid->output += pid->deltaOutput / pid->updateFrequency;
    pid->output = CLAMP(pid->output, -pid->outputLimit, pid->outputLimit);
    return pid->output;
}

void IncrementalPIDreset(IncrementalPID_t *pid)
{
    pid->lastTarget      = 0.0f;
    pid->lastMeasurement = 0.0f;
    pid->lastLastMeasurement = 0.0f;
    pid->deltaOutput     = 0.0f;
    pid->output          = 0.0f;
}

void IncrementalPIDSetOutput(IncrementalPID_t *pid, float output)
{
    pid->output = output;
}