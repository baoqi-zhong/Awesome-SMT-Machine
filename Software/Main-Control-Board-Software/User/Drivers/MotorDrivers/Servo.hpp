/**
 * @file Servo.hpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "tick.hpp"

namespace Servo
{
#define SERVO_TIM &htim15
#define SERVO_CHANNEL TIM_CHANNEL_1

#define SERVO_SPEED 0.2f //degree per ms

#define FABS(x) ((x) > 0 ? (x) : -(x))

// Init: initialize the servo
void Init(float initPosition);

// Stop: stop the servo
void Stop();

// Available: check if the servo is available for new commend
bool Available();

// SetPosition: make sure the servo has moved to the last position
bool SetPosition(float position);

// getEstimatePosition: get the estimated position of the servo
float getEstimatePosition();

// SetPositionForced: set the position no matter the servo is available for new commend or not
void SetPositionForced(float position);

}