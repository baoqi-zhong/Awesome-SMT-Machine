/**
 * @file InterBoard.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "main.h"
#include "GeneralConfig.h"

#include "stdint.h"
#include "FDCANManager.h"

/*
* 通信协议

* 所有数据都是 int16_t 类型.

* 主控板发送控制包:
* CAN ID: 0x100 + ID = 0x101~0x108
*         | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
* Byte 0: | 0 |      Control MASK         |
* Byte 1: |          Control Data         |
* Byte 2: |             Data              |
* Byte 3: |             Data              |
* Byte 4: |             Data              |
* Byte 5: |             Data              |
* Byte 6: |             Data              |
* Byte 7: |             Data              |


1. 当 Byte 0 的 Bit 7 == 0
    * Byte 0:   Control MASK
        * Bit 7: 0
        * Bit 6: 位置设置 MASK                          0: 不修改, 1: 修改
        * Bit 5: 速度设置 MASK                          0: 不修改, 1: 修改
        * Bit 4: 扭矩设置 MASK                          0: 不修改, 1: 修改
        * Bit 3: 忽略错误 MASK                          0: 不修改, 1: 修改
        * Bit 2: 自动复位 MASK                          0: 不修改, 1: 修改
        * Bit 1: RGB 控制 MASK                          0: 不修改, 1: 修改
        * Bit 0: 使能输出 MASK                          0: 不修改, 1: 修改

    * Byte 1:   Control Data
        * Bit 7: 手动触发复位?                          0: 无效, 1: 复位    推荐: 0
        * Bit 6: 位置 Byte 是否有效(是否闭环位置)?      0: 无效, 1: 有效    推荐: 1
        * Bit 5: 速度 Byte 是否有效(是否闭环速度)?      0: 无效, 1: 有效    推荐: 1
        * Bit 4: 扭矩 Byte 使用电流闭环还是电压控制?    0: 电压, 1: 电流    推荐: 1
        * Bit 3: 强制忽略所有错误?                      0: 否,   1: 是      推荐: 0
        * Bit 2: 自动复位?                              0: 否,   1: 自动复位推荐: 0
        * Bit 1: 谁控制管理 RGB?                        0: 电机, 1: 主控板  推荐: 0
        * Bit 0: 是否使能输出?                          0: 禁用, 1: 启用    推荐: 1

    * Byte 2: 最大扭矩电流(加速度) 高 8 位      数值 = Iq / 10000.0f        E.g. 最大扭矩约为 3.2A
    * Byte 3: 最大扭矩电流(加速度) 低 8 位
    * Byte 4: 目标速度 高 8 位                  数值 = 轴角速度定点数 / 16  E.g. 最大速度约为 8 圈/s
    * Byte 5: 目标速度 低 8 位
    * Byte 6: 目标位置(增量) 高 8 位            数值 = 轴角度定点数 / 32    E.g. 最小移动单位约为轴角度 0.18°
    * Byte 7: 目标位置(增量) 低 8 位

2. 当 Byte 0 的 Bit 7 == 1
    * Byte 0:
        * Bit 7: 1
        * 其他位: 保留
    * 其他 Byte: 保留

    * Byte 5: LED 颜色 R
    * Byte 6: LED 颜色 G
    * Byte 7: LED 颜色 B


* 电机返回状态包:
* CAN ID: 0x200 + ID = 0x201~0x208

* Byte 0: 电机运行状态
    * Bit 7: 0
    * Bit 6: 位置闭环?                              0: 电压, 1: 电流
    * Bit 5: 速度闭环?                              0: 否,   1: 是
    * Bit 4: 电压/电流控制?                         0: 电压, 1: 电流
    * Bit 3: 强制忽略所有错误?                      0: 否,   1: 是
    * Bit 2: 是否会自动复位?                        0: 否,   1: 自动复位
    * Bit 1: RGB 谁控制?                            0: 电机, 1: 主控板
    * Bit 0: 电机是否使能                           0: Dis   1: Enable

* Byte 1: 错误状态
    * Bit 7: 控制包解码错误?                        0: 无,   1: 有
    * Bit 6: 过温 Warning?                          0: 无,   1: 有
    * Bit 5: 过温错误?                              0: 无,   1: 过温
    * Bit 4: 过流错误?                              0: 无,   1: 过流
    * Bit 3: 欠压 过压错误 ?                        0: 正常, 1: 欠压或过压
    * Bit 2: 是否堵转?                              0: 无,   1: 有
    * Bit 1: 驱动器 FAULT?                          0: 无,   1: 有
    * Bit 0: 电机是否有 Error?                      0: 无,   1: 有

* Byte 2: 实际转矩电流 高 8 位                  数值 = Iq * 10000.0f        E.g. 最大实际转矩约为 3.2A
* Byte 3: 实际转矩电流 低 8 位
* Byte 4: 编码器(输出轴单圈位置) 高 8 位        数值 = 轴角度定点数 / 32    E.g. 最小反馈角度约为轴角度 0.18°
* Byte 5: 编码器(输出轴单圈位置) 低 8 位
* Byte 6: 到目标位置的执行进度(0~100%)[lagecy]  事实上,这个计算应该在主控板完成, 所以这里永远是 0

*/

// 返回包
#define RESPONSE_PACKET_ENABLE_MOTOR_MASK       0x01
#define RESPONSE_PACKET_RGB_CONTROL_MASK        0x02
#define RESPONSE_PACKET_AUTO_RESET_MASK         0x04
#define RESPONSE_PACKET_IGNORE_ERROR_MASK       0x08
#define RESPONSE_PACKET_TORQUE_CONTROL_MASK     0x10
#define RESPONSE_PACKET_SPEED_CONTROL_MASK      0x20
#define RESPONSE_PACKET_POSITION_CONTROL_MASK   0x40

#define RESPONSE_PACKET_ANY_ERROR_MASK          0x01
#define RESPONSE_PACKET_DRIVER_FAULT_MASK       0x02
#define RESPONSE_PACKET_STALL_MASK              0x04
#define RESPONSE_PACKET_UNDER_OVER_VOLTAGE_MASK 0x08
#define RESPONSE_PACKET_OVER_CURRENT_MASK       0x10
#define RESPONSE_PACKET_OVER_TEMPERATURE_MASK   0x20
#define RESPONSE_PACKET_OVER_TEMPERATURE_WARNING_MASK 0x40
#define RESPONSE_PACKET_ERROR_MASK              0x80

// 控制包
#define CONTROL_PACKET_ENABLE_MOTOR_MASK        0x01
#define CONTROL_PACKET_RGB_CONTROL_MASK         0x02
#define CONTROL_PACKET_AUTO_RESET_MASK          0x04
#define CONTROL_PACKET_IGNORE_ERROR_MASK        0x08
#define CONTROL_PACKET_TORQUE_CONTROL_MASK      0x10
#define CONTROL_PACKET_SPEED_CONTROL_MASK       0x20
#define CONTROL_PACKET_POSITION_CONTROL_MASK    0x40
#define CONTROL_PACKET_TRIGGER_RESET_MASK       0x80

void InterBoardInit();

void InterBoardTransmitFeedback();