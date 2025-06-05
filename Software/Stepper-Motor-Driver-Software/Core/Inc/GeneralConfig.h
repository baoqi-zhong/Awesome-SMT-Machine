#ifndef __GENERAL_CONFIG_H
#define __GENERAL_CONFIG_H

#include "stdint.h"

/* 外设配置 */
#define CURRENT_LOOP_FREQ           42500       // 电流环频率, 单位 Hz
#define CCR                         1000        // Counter Period

#define CAN_ID                      3           // 自己的 CAN ID, 1~8

#define FDCANMANAGER_FILTER_ID      0x100       // FDCAN 接受的 ID
#define FDCANMANAGER_FILTER_ID_MASK 0x7F0       // FDCAN 接受的 ID 掩码

/* 物理参数 */
#define AB_PHASE_SWAP               0           // 1 为交叉接法, 0 为正常接法
#define POLE_PAIRS                  50          // 极对数, 1.8° 步进电机是 50 对
#define PHASE_RESISTANCE            2.11453f    // 相电阻, 2.11453Ω
#define PHASE_INDUCTANCE            2.00f       // 相电感, 2.00mH
#define KV                          800.0f      // 37.5 RPM/V

/* 校准数据 */
#if (CAN_ID == 1)
#define VBUS_GAIN                   0.008946261f    // 电压采样增益
#define IALPHA_BIAS                 2037.52173f     // 电流采样偏置
#define IALPHA_GAIN                 -0.002982521f   // 电流采样增益
#define IBETA_BIAS                  2031.25757f     // 电流采样偏置
#define IBETA_GAIN                  0.003155451f    // 电流采样增益
#elif (CAN_ID == 2)
#define VBUS_GAIN                   0.008946261f    // 电压采样增益
#define IALPHA_BIAS                 2039.27039f     // 电流采样偏置
#define IALPHA_GAIN                 -0.002781669f   // 电流采样增益
#define IBETA_BIAS                  2020.38947f     // 电流采样偏置
#define IBETA_GAIN                  0.0027568069f   // 电流采样增益
#elif (CAN_ID == 3)
// 未校准
#define VBUS_GAIN                   0.008946261f    // 电压采样增益
#define IALPHA_BIAS                 2039.27039f     // 电流采样偏置
#define IALPHA_GAIN                 -0.002781669f   // 电流采样增益
#define IBETA_BIAS                  2020.38947f     // 电流采样偏置
#define IBETA_GAIN                  0.0027568069f   // 电流采样增益
#endif

/* 控制 */
#define CALIBRATE_ENCODER           0           // 是否校准编码器

#define DEFAULT_TOURQUE_LIMIT       1.5f        // 默认扭矩限制, 单位 A

#if (CAN_ID == 1 || CAN_ID == 2)
#define DEFAULT_VELOCITY_LIMIT      200000      // 默认速度限制, 轴角速度定点数 / s
#else
#define DEFAULT_VELOCITY_LIMIT      10000       // 默认速度限制, 轴角速度定点数 / s
#endif

#define OPEN_LOOP_ROTATE_SPEED      64          // 开环转速, 每次调用电流环增加的角度, 定点数

#if (CAN_ID == 1 || CAN_ID == 2)
#define OPENLOOP_DRAG_VOLTAGE       3.0f        // 开环拖动电压, 单位 V
#else
#define OPENLOOP_DRAG_VOLTAGE       2.0f        // 开环拖动电压, 单位 V
#endif

#define ALINE_VOLTAGE               8.0f       // 对齐电角度的电压, 单位 V

#define UNDER_VOLTAGE_THRESHOLD     16.0f       // 电压低于此值会触发 underOverVoltageError
#define OVER_VOLTAGE_THRESHOLD      30.0f       // 电压高于此值会触发 underOverVoltageError
#define OVER_TEMPERATURE_THRESHOLD  80.0f       // 温度高于此值会触发 overTemperatureError
#define OVER_CURRENT_THRESHOLD      8.0f        // 电流大于此值会触发 overCurrentError

#define ERROR_TRIGGER_TRESHOLD_TIME CURRENT_LOOP_FREQ   // 多少个 tick 会触发错误, 不同类型的错误触发 Counter 加速度可以不一样
#define UNDER_OVER_VOLTAGE_TRIGGER_SPEED    20
#define UNDER_OVER_VOLTAGE_UNTRIGGER_SPEED  1
#define OVER_TEMPERATURE_TRIGGER_SPEED      1
#define OVER_TEMPERATURE_UNTRIGGER_SPEED    1
#define OVER_CURRENT_TRIGGER_SPEED          20
#define OVER_CURRENT_UNTRIGGER_SPEED        1

#define MA732_USE_KALMAN_FILTER     0           // 是否使用卡尔曼滤波器 0 使用 LPF, 1 使用卡尔曼滤波器

#define MUSIC_STRENGTH              OPENLOOP_DRAG_VOLTAGE  // 音乐模式的电压


/* 常数和基本函数 */
#define PI 3.1415926f
#define TWO_PI 6.2831853f

#define CLAMP(x, min, max) ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))

#define FABS(x) ((x) > 0 ? (x) : -(x))
#define FMOD(x, y) ((x) - (int32_t)((x) / (y)) * (y))
#endif