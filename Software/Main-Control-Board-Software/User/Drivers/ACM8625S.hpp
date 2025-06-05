/**
 * @file ACM8625S.hpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "stdint.h"

//Device address
#define ACM8625S_I2C_ADDRESS        0x58

//Register Map
// #define ACM8625S_REG_PAGE           0x00
#define ACM8625S_AMP_CTRL1          0x01
#define ACM8625S_AMP_CTRL2          0x02
#define ACM8625S_AMP_CTRL3          0x03
#define ACM8625S_STATE_CTRL         0x04
#define ACM8625S_PROCESSING_CTRL1   0x05
#define ACM8625S_PROCESSING_CTRL2   0x06
#define ACM8625S_I2S_DATA_FORMAT1   0x07
#define ACM8625S_I2S_DATA_FORMAT2   0x08
#define ACM8625S_I2S_DATA_FORMAT3   0x09
#define ACM8625S_GPIO2_CTRL         0x0A
#define ACM8625S_GPIO1_CTRL         0x0B
#define ACM8625S_GPIO1_FAULT_SEL    0x0C
#define ACM8625S_GPIO2_FAULT_SEL    0x0D
#define ACM8625S_SS_CTRL            0x0E
#define ACM8625S_VOLUME_CTRL_L      0x0F
#define ACM8625S_VOLUME_CTRL_R      0x10
#define ACM8625S_MSIC_CTRL          0x11
#define ACM8625S_I2S_CLK_FORMAT_RPT1    0x12
#define ACM8625S_I2S_CLK_FORMAT_RPT2    0x13

#define ACM8625S_DIEID_RPT          0x15
#define ACM8625S_STATE_RPT          0x16
#define ACM8625S_FAULT_RPT1         0x17
#define ACM8625S_FAULT_RPT2         0x18
#define ACM8625S_FAULT_RPT3         0x19

#define ACM8625S_GPIO_PP_OD_CTRL    0x27
#define ACM8625S_DIG_DSP_CTRL       0x28

#define ACM8625S_XOR_CHECKSUM       0x7E
#define ACM8625S_CRC_CHECKSUM       0x7F


#define ACM8625S_VOLUME_MIN         -104
#define ACM8625S_VOLUME_MAX         24

#define ACM8625S_AUDIO_BUFFER_SIZE 1024

namespace ACM8625S
{
typedef enum
{
    DigitalOff,
    AnalogOff,
    DriverOff,
    Play
} ACM8625SState;

typedef struct
{
    float detectedAudioSampleRate;
    uint8_t dieID;
    ACM8625SState state;
    uint8_t overTemperature;
    uint8_t overVoltage;
    uint8_t underVoltage;
    uint8_t CH1_DCFault;
    uint8_t CH2_DCFault;
    uint8_t CH1_OverCurrent;
    uint8_t CH2_OverCurrent;
    uint8_t clockFault;
} ACM8625SStatus_t;

void init();

/*
    * @brief 设置音量, 步进 0.5 dB
    * @param volumeLeft 左声道音量, 单位 dB, 范围 -110dB ~ 24dB
    * @param volumeRight 右声道音量, 单位 dB, 范围 -110dB ~ 24dB
    */
void setVolume(float volumeLeft, float volumeRight);


/*
    * @brief 读取状态
    */
void readStatus();
   
} // namespace ACM8625S
