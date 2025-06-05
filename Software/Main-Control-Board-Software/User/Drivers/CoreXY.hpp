
/**
 * @file CoreXY.hpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "MotorManager.hpp"

namespace CoreXY
{
#define BELT_PITCH 2.0f             // 皮带的齿距 mm
#define BELT_PULLEY_TEETH 20.0f     // 同步轮的齿数


typedef struct
{
    float X;                    // mm
    float Y;                    // mm
    float Z;                    // mm
    float E;                    // mm
    int32_t motorLeft;          // X 轴的电机, 定点数
    int32_t motorRight;         // Y 轴的电机, 定点数
    int32_t motorRotate;        // E 轴的电机, 定点数
    float motorZ;              // Z 轴的电机, 单位: 度
} CoreXYData_t;

typedef struct
{
    uint8_t findingOrigin;
    uint8_t needTransmitMovementCpltInfo;
} CoreXYControlStatus_t;

extern CoreXYControlStatus_t controlStatus;

extern CoreXYData_t targetPosition;
extern CoreXYData_t targetSpeed;
extern CoreXYData_t feedbackPosition;
extern CoreXYData_t feedbackSpeed;

// 单位：mm
void setTargetPosition(float x, float y, float z, float e);

void Init();

void triggerFindOrigin();
}