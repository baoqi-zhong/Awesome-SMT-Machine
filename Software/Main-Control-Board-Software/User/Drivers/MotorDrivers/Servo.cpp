/**
 * @file Servo.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "Servo.hpp"

namespace Servo{

uint32_t lastTick = 0;
float targetPosition = 0.0f;
float lastTargetPosition = 0.0f;
float dTargetPosition = 0.0f;

void Init(float initPosition)
{
    HAL_TIM_PWM_Start(SERVO_TIM, SERVO_CHANNEL);
    targetPosition = initPosition;
    lastTargetPosition = initPosition;
    SetPositionForced(initPosition);
    lastTick = 0;
}

void Stop(){
    __HAL_TIM_SET_COMPARE(SERVO_TIM, SERVO_CHANNEL, 0);
    HAL_TIM_PWM_Stop(SERVO_TIM, SERVO_CHANNEL);
}

float getEstimatePosition()
{
    if(Available())
        return targetPosition;
    if(dTargetPosition > 0)
        return targetPosition - dTargetPosition + (vTick - lastTick) * SERVO_SPEED;
    else
        return targetPosition - dTargetPosition - (vTick - lastTick) * SERVO_SPEED;
}

// Available: check if the servo is available for new commend
bool Available()
{
    return float(vTick - lastTick) * SERVO_SPEED > FABS(dTargetPosition);
}

// SetPosition: make sure the servo has moved to the last position
bool SetPosition(float _targetPosition){  
    if(Available()) 
    {
        SetPositionForced(_targetPosition);
        return true;
    }
    return false;       
}

// SetPositionForced: set the position no matter the servo is available for new commend or not
void SetPositionForced(float _targetPosition)
{
    if(_targetPosition > 180)  //limit the position
        _targetPosition = 180;
    if(_targetPosition < 0)
        _targetPosition = 0;
    targetPosition = _targetPosition;

    __HAL_TIM_SET_COMPARE(SERVO_TIM, SERVO_CHANNEL, targetPosition * 100 / 180 + 25);
    dTargetPosition = targetPosition - lastTargetPosition;
    lastTargetPosition = targetPosition;
    lastTick = vTick;
}

}