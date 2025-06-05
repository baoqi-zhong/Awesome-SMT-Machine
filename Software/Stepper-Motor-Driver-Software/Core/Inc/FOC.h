/**
 * @file FOC.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

 #pragma once

#include "main.h"
#include "GeneralConfig.h"

#define MOTOR_CONTROL_1KHZ_PERIOD CURRENT_LOOP_FREQ / 1000
#define MOTOR_CONTROL_4KHZ_PERIOD CURRENT_LOOP_FREQ / 4000

typedef enum 
{
    OPEN_LOOP_ROTATE = 0,
    CURRENT_TOURQUE_CONTROL = 1,
    VOLTAGE_TOURQUE_CONTROL = 2,
    OPEN_LOOP_POSITION_CONTROL = 3,
    MUSIC = 4
} FOCControlMode_t;

typedef struct
{
    FOCControlMode_t FOCControlMode;
    uint8_t enableSpeedCloseLoop;
    uint8_t enablePositionCloseLoop;
    uint8_t enableFOCOutput;
    uint8_t RGBControlByMaster;
    uint8_t ignoreAllErrors;
    uint8_t enableAutoReset;
    uint8_t triggerReset;
    uint8_t initializing;

    float tourqueLimit;
    int32_t velocityLimit;      // 轴角速度定点数

    float targetIq;
    float targetId;
    int32_t targetVelocity;     // 轴角速度定点数
    int32_t targetPosition;     // 轴角度定点数
} MotorControlStatus_t;

extern MotorControlStatus_t motorControlStatus;

void setPhraseVoltage(float phraseA, float phraseB);

void alineMotor();

void MotorControlInit();

void FOC_init();
void disableFOC();
void resetAndEnableFOC();