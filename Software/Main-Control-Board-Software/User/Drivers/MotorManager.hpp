/**
 * @file MotorManager.hpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "tick.hpp"

#include "StepperMotor.hpp"



namespace MotorManager
{
extern StepperMotor::StepperMotor motorLeft;
extern StepperMotor::StepperMotor motorRight;
extern StepperMotor::StepperMotor motorRotate;

void RegisterCallback(void (*callback)());

void stopAllMotors();

void Init();
}