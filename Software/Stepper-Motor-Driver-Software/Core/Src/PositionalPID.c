/**
 * @file PositionalPID.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "PositionalPID.h"

void PositionalPIDsetParameters(PositionalPID_t *pid, float kPonError, float kIonError, float kDonMeasurement, float kPonMeasurement, float kDonTarget, float alpha, float outputLimit, float updateFrequency)
{
    assert_param(outputLimit >= 0.0f);
    assert_param(updateFrequency > 0.0f);
    assert_param(alpha >= 0.0f && alpha <= 1.0f);
    
    
    pid->kPonError = kPonError;
    pid->kIonError = kIonError;
    pid->kDonMeasurement = kDonMeasurement;
    pid->kPonMeasurement = kPonMeasurement;
    pid->kDonTarget = kDonTarget;
    pid->alpha = alpha;
    pid->outputLimit = outputLimit;
    pid->updateFrequency = updateFrequency;

    PositionalPIDreset(pid);
}


float PositionalPIDupdate(PositionalPID_t *pid, float target, float measurement)
{
    float error             = target - measurement;
    float deltaTarget       = target - pid->lastTarget;
    float deltaMeasurement  = measurement - pid->lastMeasurement;
    pid->lastTarget         = target;
    pid->lastMeasurement    = measurement;


    float pOut = pid->kPonError * error;
    float iOut = pid->iOut + pid->kIonError * error / pid->updateFrequency;
    float dOut = (-pid->kDonMeasurement * deltaMeasurement + pid->kDonTarget * deltaTarget) * pid->updateFrequency;
    pid->dOut =  dOut * pid->alpha + pid->dOut * (1 - pid->alpha);

    // apply p on measurement
    iOut -= pid->kPonMeasurement * deltaMeasurement;

    // clamp p on error output
    float output     = CLAMP(pOut + pid->dOut, - pid->outputLimit, pid->outputLimit);
    pid->pOut = output - pid->dOut;

    // clamp integral output
    pid->iOut = CLAMP(CLAMP(iOut, - pid->outputLimit - output, pid->outputLimit - output), - pid->outputLimit, pid->outputLimit);

    // calculate output
    pid->output = CLAMP(output + iOut, - pid->outputLimit, pid->outputLimit);

    return pid->output;
}


void PositionalPIDreset(PositionalPID_t *pid)
{
    pid->lastMeasurement = 0.0f;
    pid->lastTarget = 0.0f;
    pid->pOut   = 0.0f;
    pid->iOut    = 0.0f;
    pid->dOut   = 0.0f;
    pid->output = 0.0f;
}
