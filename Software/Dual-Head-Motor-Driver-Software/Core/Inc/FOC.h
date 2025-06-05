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

typedef enum 
{
    OPEN_LOOP_ROTATE = 0,
    OPEN_LOOP_POSITION_CONTROL = 3,
} FOCControlMode_t;

typedef struct
{
    float startPostion;
    float displacement;
    float accelerationDistance;
    float uniformVelocityDistance;
} MoveOperation_t;

typedef struct
{
    uint8_t ignoreAllErrors;
    uint8_t enableAutoReset;
    uint8_t triggerReset;
    uint8_t initializing;

    // Valve Control
    uint8_t valveState[2];

    // Micro Switch/GPIO Feedback Status
    uint8_t microSwitchStatus;
    uint8_t GPIOStatus[3];

    // Motor Control
    FOCControlMode_t FOCControlMode;
    uint8_t enableFOCOutput;

    float tourqueLimit[3];
    float velocityLimit[3];     // 轴角速度(rad/s)
    float accelerationLimit[3]; // 轴角加速度(rad/s^2)

    float currentVelocity[3];   // 轴角速度(rad/s)
    float currentPosition[3];   // 轴角度(rad)
    MoveOperation_t movement[3];

    float targetZHeight[2];   // 吸嘴目标 Z 位置, 单位 mm
    float targetZHeightSet;   // 上一个设置的 Z 位置, 单位 mm
    float currentZHeight[2];  // 吸嘴当前 Z 位置, 单位 mm
} ControlStatus_t;

extern ControlStatus_t controlStatus;

extern volatile uint32_t* M1_CCRS[4];
extern volatile uint32_t* M2_CCRS[4];
extern volatile uint32_t* M3_CCRS[4];
void setPhraseVoltage(float phraseA, float phraseB, volatile uint32_t* CCRs[4]);

void controlInit();
void moveMotor(uint8_t index, float displacement);
void setZPosition(uint8_t index, float position);
void armInverseKinematics();
void armForwardKinematics();

extern float measuredIMotor[3];
extern float Vbus;

void FOC_init();
void disableFOC();
void resetAndEnableFOC();