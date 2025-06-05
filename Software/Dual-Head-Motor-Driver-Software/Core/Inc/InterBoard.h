/**
 * @file InterBoard.h
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
* Byte 0: |         Control Data          |
* Byte 1: |             Data              |
* Byte 2: |             Data              |
* Byte 3: |             Data              |
* Byte 4: |             Data              |
* Byte 5: |             Data              |
* Byte 6: |             Data              |
* Byte 7: |             Data              |


1. 当 Byte 0 的 Bit 7 == 0
    * Byte 0:   Control Data
        * Bit 0: 是否使能输出?                  0: 禁用, 1: 启用    推荐: 1
        * Bit 1: 手动触发复位?                  0: 无效, 1: 复位    推荐: 0
        * Bit 2: 自动复位?                      0: 否,   1: 自动复位推荐: 0
        * Bit 3: 强制忽略所有错误?              0: 否,   1: 是      推荐: 0
        * Bit 4: 左阀门状态                     0: 关闭, 1: 打开
        * Bit 5: 右阀门状态                     0: 关闭, 1: 打开
        * Bit 6: 0
        * Bit 7: 0

    * Byte 2: 左吸嘴下移位移量(unsign8 绝对)    数值 = 下移 mm 数 * 10    E.g. 最小移动单位为 0.1mm, 最大移动范围为 25.5mm
    * Byte 3: 右吸嘴下移位移量(unsign8 绝对)    数值 = 下移 mm 数 * 10    E.g. 最小移动单位为 0.1mm, 最大移动范围为 25.5mm
    * Byte 4: M2 目标位置(相对) 高 8 位         数值 = 轴角度弧度 * 10000 E.g. 最小移动单位约为轴角度 0.018°
    * Byte 5: M2 目标位置(相对) 低 8 位         
    * Byte 6: M3 目标位置(相对) 高 8 位         数值 = 轴角度弧度 * 10000 E.g. 最小移动单位约为轴角度 0.018°
    * Byte 7: M3 目标位置(相对) 低 8 位         


* 电机返回状态包:
* CAN ID: 0x200 + ID = 0x201~0x208

* Byte 0: 电机运行状态
    * Bit 0: 电机是否使能                       0: Dis   1: Enable
    * Bit 1: 0
    * Bit 2: 是否会自动复位?                    0: 否,   1: 自动复位
    * Bit 3: 强制忽略所有错误?                  0: 否,   1: 是
    * Bit 4: 微动开关状态                       0: 上拉, 1: 触发
    * Bit 5: GPIO 1 状态                        0: 上拉, 1: 触发
    * Bit 6: GPIO 2 状态                        0: 上拉, 1: 触发
    * Bit 7: GPIO 3 状态                        0: 上拉, 1: 触发

* Byte 1: 错误状态
    * Bit 0: 电机是否有 Error?                  0: 无,   1: 有
    * Bit 1: 欠压 过压错误 ?                    0: 正常, 1: 欠压或过压
    * Bit 2: M1 驱动器 FAULT?                   0: 无,   1: 是
    * Bit 3: M2/M3 驱动器 FAULT?                0: 无,   1: 是
    * Bit 4: M1 过流错误?                       0: 无,   1: 过流
    * Bit 5: M2/M3 过流错误?                    0: 无,   1: 过流
    * Bit 6: 0
    * Bit 7: 0

* Byte 2: 左吸嘴当前下移位移量(unsign8 绝对)    数值 = 下移 mm 数 * 10    E.g. 最小移动单位为 0.1mm, 最大移动范围为 25.5mm
* Byte 3: 右吸嘴当前下移位移量(unsign8 绝对)    数值 = 下移 mm 数 * 10    E.g. 最小移动单位为 0.1mm, 最大移动范围为 25.5mm
* Byte 4: M2 当前位置(相对) 高 8 位             数值 = 轴角度弧度 * 10000 E.g. 最小移动单位约为轴角度 0.018°
* Byte 5: M2 当前位置(相对) 低 8 位         
* Byte 6: M2 当前位置(相对) 高 8 位             数值 = 轴角度弧度 * 10000 E.g. 最小移动单位约为轴角度 0.018°
* Byte 7: M2 当前位置(相对) 低 8 位     
*/

// 返回包
#define RESPONSE_PACKET_ENABLE_MOTOR_MASK       0x01

#define RESPONSE_PACKET_AUTO_RESET_MASK         0x04
#define RESPONSE_PACKET_IGNORE_ERROR_MASK       0x08
#define RESPONSE_PACKET_MICRO_SWITCH_MASK       0x10
#define RESPONSE_PACKET_GPIO1_MASK              0x20
#define RESPONSE_PACKET_GPIO2_MASK              0x40
#define RESPONSE_PACKET_GPIO3_MASK              0x80

#define RESPONSE_PACKET_ANY_ERROR_MASK          0x01
#define RESPONSE_PACKET_UNDER_OVER_VOLTAGE_MASK 0x02
#define RESPONSE_PACKET_M1_DRIVER_FAULT_MASK    0x04
#define RESPONSE_PACKET_M2_M3_DRIVER_FAULT_MASK 0x08
#define RESPONSE_PACKET_M1_OVER_CURRENT_MASK    0x10
#define RESPONSE_PACKET_M2_M3_OVER_CURRENT_MASK 0x20

// 控制包
#define CONTROL_PACKET_ENABLE_MOTOR_MASK        0x01
#define CONTROL_PACKET_TRIGGER_RESET_MASK       0x02
#define CONTROL_PACKET_AUTO_RESET_MASK          0x04
#define CONTROL_PACKET_IGNORE_ERROR_MASK        0x08
#define CONTROL_PACKET_VALVE_1_MASK             0x10
#define CONTROL_PACKET_VALVE_2_MASK             0x20

void InterBoardInit();

void InterBoardTransmitFeedback();