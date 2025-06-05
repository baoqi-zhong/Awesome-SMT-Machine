/**
 * @file SC8815.hpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "stdint.h"
#include "i2c.h"

namespace SC8815
{
//Device address
#define SC8815_WriteAddress         0xE8
#define SC8815_ReadAddress          0xE9

//Register Map
#define SC8815_BASE_REG             0x00
#define SC8815_RW_VBAT_SET          0x00
#define SC8815_RW_VBUSREF_I_SET     0x01
#define SC8815_RW_VBUSREF_I_SET2    0x02
#define SC8815_RW_VBUSREF_E_SET     0x03
#define SC8815_RW_VBUSREF_E_SET2    0x04
#define SC8815_RW_IBUS_LIM_SET      0x05
#define SC8815_RW_IBAT_LIM_SET      0x06
#define SC8815_RW_VINREG_SET        0x07
#define SC8815_RW_RATIO             0x08
#define SC8815_RW_CTRL0_SET         0x09
#define SC8815_RW_CTRL1_SET         0x0A
#define SC8815_RW_CTRL2_SET         0x0B
#define SC8815_RW_CTRL3_SET         0x0C

#define SC8815_R_VBUS_FB_VALUE      0x0D
#define SC8815_R_VBUS_FB_VALUE2     0x0E
#define SC8815_R_VBAT_FB_VALUE      0x0F
#define SC8815_R_VBAT_FB_VALUE2     0x10
#define SC8815_R_IBUS_VALUE         0x11
#define SC8815_R_IBUS_VALUE2        0x12
#define SC8815_R_IBAT_VALUE         0x13
#define SC8815_R_IBAT_VALUE2        0x14
#define SC8815_R_ADIN_VALUE         0x15
#define SC8815_R_ADIN_VALUE_2       0x16
#define SC8815_R_STATUS             0x17
#define SC8815_RW_MASK              0x19

#define DEFAULT_VBUS                2.0f
#define DEFAULT_IBUS_LIMIT          12.0f
#define DEFAULT_IBAT_LIMIT          6.0f


//Hardware CONFIG
#define HWCFG_CS_Resistor           0.005f


typedef struct
{
    float VBUSRatio;
    float VBATRatio;
    float IBUSRatio;
    float IBATRatio;
} Ratio;

typedef struct
{
    float VBUS;
    float IBUSLimit;
    float IBATLimit;
    Ratio powerRatio;
} ControlStatus_t;

typedef struct
{
    float VBUS;
    float VBAT;
    float IBUS;
    float IBAT;
    float PBUS;
    float PBAT;
    float efficiency;
} FeedbackPowerValue_t;

typedef struct
{
    uint8_t anyFault;

    uint8_t VBUSOverVoltage;
    uint8_t VBUSOverCurrent;
    uint8_t VBATOverVoltage;
    uint8_t VBATOverCurrent;

    uint8_t lowBuckBoostEfficiency;
    uint8_t overTemperature;
    uint8_t VBUSShortCircuit;
} ErrorStatus_t;


// SC8815_INIT_Default: initialize the SC8815
void init();

void setVBUS(float VBUS);
void setIBUSLimit(float IBUS);
void setIBATLimit(float IBAT);

// SC8815_PowerOn: power on the SC8815
// If the SC8815 is initialized and no fault is detected, return true. Otherwise, return false.
void PowerOn();

// SC8815_PowerOff: power off the SC8815
void PowerOff();

// readADC: update the data from SC8815
void readADC();

// checkFault: check if there is any fault by examine register data in SC8815
uint8_t checkFault();

// get_VBUS_Volt: get the VBUS value from SC8815
float get_VBUS_Volt();

// get_VBAT_Volt: get the VBAT value from SC8815
float get_VBAT_Volt();

// get_IBUS_Amp: get the IBUS value from SC8815
float get_IBUS_Amp();

// get_IBAT_Amp: get the IBAT value from SC8815
float get_IBAT_Amp();

}