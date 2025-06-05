/**
 * @file GeneralConfig.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "stdint.h"

/* 外设配置 */
// PWM 频率: 170 / 2 / 4000 = 21.25 KHz
#define CURRENT_LOOP_FREQ           21250       // 电流环频率, 单位 Hz
#define CCR                         4000        // Counter Period

#define CAN_ID                      3           // 自己的 CAN ID, 1~8

#define FDCANMANAGER_FILTER_ID      0x100       // FDCAN 接受的 ID
#define FDCANMANAGER_FILTER_ID_MASK 0x7F0       // FDCAN 接受的 ID 掩码

/* 物理参数 */
#define POLE_PAIRS                  50          // 极对数, 1.8° 步进电机是 50 对
#define PHASE_RESISTANCE            2.11453f    // 相电阻, 2.11453Ω

#define KV_M1                       3.0f        // 电机 1 的速度常数, V/(rad/s)
#define KV_M2                       0.1f        // 电机 2 的速度常数, V/(rad/s)

#define ARM_LENGTH                  29.0f       // 摇臂长度, 单位 mm
#define WHEEL_RADIUS                7.5f       // 轮子半径, 单位 mm

/* 校准数据 */
#define VBUS_GAIN                   0.00492074f // 电压采样增益
#define IM1_BIAS                    0           // 电流采样偏置
#define IM1_GAIN                    1           // 电流采样增益
#define IM2_BIAS                    0           // 电流采样偏置
#define IM2_GAIN                    1           // 电流采样增益
#define IM3_BIAS                    0           // 电流采样偏置
#define IM3_GAIN                    1           // 电流采样增益


/* 控制 */
#define DEFAULT_TOURQUE_LIMIT_M1    1.5f        // 默认扭矩限制, 单位 A
#define DEFAULT_ACC_LIMIT_M1        5.0f        // 默认加速度限制, 轴角加速度 rad/s^2
#define DEFAULT_VELOCITY_LIMIT_M1   2.0f        // 默认速度限制, 轴角速度 rad/s
#define POSITIOIN_HOLD_VOLTAGE_M1   7.0f        // 位置保持电压, 单位 V

#define DEFAULT_TOURQUE_LIMIT_M2    1.5f        // 默认扭矩限制, 单位 A
#define DEFAULT_ACC_LIMIT_M2        2000.0f     // 默认加速度限制, 轴角加速度 rad/s^2
#define DEFAULT_VELOCITY_LIMIT_M2   100.0f      // 默认速度限制, 轴角速度 rad/s
#define POSITIOIN_HOLD_VOLTAGE_M2    2.0f       // 位置保持电压, 单位 V

#define OPEN_LOOP_ROTATE_SPEED_M1   1.0f        // 开环转速, 每次调用电流环增加的角度
#define OPEN_LOOP_ROTATE_SPEED_M2   2.0f        // 开环转速, 每次调用电流环增加的角度

#define UNDER_VOLTAGE_THRESHOLD     8.2f        // 电压低于此值会触发 underOverVoltageError
#define OVER_VOLTAGE_THRESHOLD      18.5f       // 电压高于此值会触发 underOverVoltageError
#define OVER_CURRENT_THRESHOLD      2.0f        // 电流大于此值会触发 overCurrentError

#define ERROR_TRIGGER_TRESHOLD_TIME CURRENT_LOOP_FREQ   // 多少个 tick 会触发错误, 不同类型的错误触发 Counter 加速度可以不一样
#define UNDER_OVER_VOLTAGE_TRIGGER_SPEED    20
#define UNDER_OVER_VOLTAGE_UNTRIGGER_SPEED  1
#define OVER_CURRENT_TRIGGER_SPEED          20
#define OVER_CURRENT_UNTRIGGER_SPEED        1


/* 常数和基本函数 */
#define PI 3.1415926f
#define TWO_PI 6.2831853f

#define CLAMP(x, min, max) ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define FABS(x) ((x) > 0 ? (x) : -(x))
#define FMOD(x, y) ((x) - (int32_t)((x) / (y)) * (y))