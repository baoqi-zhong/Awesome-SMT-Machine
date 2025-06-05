/**
 * @file ACM8625S.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "ACM8625S.hpp"

// 硬件配置上, ADR 被 4.7K 上拉, ACM8625S 的地址为 0x58

#include "FreeRTOS.h"
#include "i2c.h"
#include "i2s.h"
#include "main.h"
#include "task.h"

extern const int16_t music[29547];

namespace ACM8625S
{
uint8_t txBuffer[8];
uint8_t rxBuffer[8];

ACM8625SStatus_t status;

const uint8_t defaultRegisterValues[] = {
    0x00, // 00: Register Bank 0
    0x00, // 01: 384 KHz PWM, 双通道独立运行
    0x00, // 02: 0 dB analog gain
    0x00, // 03: 输出异相, 75 KHz 闭环带宽
    0x00, // 04: 正常输出, 不静音, OFF 状态
    0x6F, // 05: 使能自动增益限制, 关闭去混响, 关闭均衡器 & post 均衡器, 关闭音调调节器, 关闭子通道处理, 关闭音效处理
    0xF0, // 06: 高速时钟, 48 kHz 内部运算
    0x00, // 07: 48 kHz, 输入使能, 标准 I2S, 16 位数据
    0x00, // 08: 左通道相位偏移 0
    0x00, // 09: Reserved
    0x00, // 0A: GPIO 1 不输出
    0x00, // 0B: GPIO 2 作为 fault 输出
    0x00, // 0C: GPIO 1 不输出
    0xFF, // 0D: GPIO 2 输出所有类型 fault
    0x00, // 0E: 不使能频率散布
    0xD0, // 0F: 左声道音量
    0xD0, // 10: 右声道音量
    0xA3, // 11: 不自动清除错误, 热保护不自动恢复
};


void init()
{
    HAL_GPIO_WritePin(APM_PDN_GPIO_Port, APM_PDN_Pin, GPIO_PIN_RESET);
    vTaskDelay(5);
    HAL_GPIO_WritePin(APM_PDN_GPIO_Port, APM_PDN_Pin, GPIO_PIN_SET);
    vTaskDelay(2);

    // 发送默认设置
    HAL_I2C_Mem_Write(&hi2c2, ACM8625S_I2C_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, (uint8_t*)defaultRegisterValues, 18, 100);

    setVolume(-20, -20);    // 设置音量为 0dB

    // 进入 play mode
    txBuffer[0] = 0x03;
    HAL_I2C_Mem_Write(&hi2c2, ACM8625S_I2C_ADDRESS, ACM8625S_STATE_CTRL, I2C_MEMADD_SIZE_8BIT, (uint8_t*)txBuffer, 1, 100);

    HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)music, sizeof(music) / sizeof(music[0]));
}

void setVolume(float volumeLeft, float volumeRight)
{
    if(volumeLeft < ACM8625S_VOLUME_MIN)
    {
        volumeLeft = ACM8625S_VOLUME_MIN;
    }
    if(volumeRight < ACM8625S_VOLUME_MIN)
    {
        volumeRight = ACM8625S_VOLUME_MIN;
    }
    if(volumeLeft > ACM8625S_VOLUME_MAX)
    {
        volumeLeft = ACM8625S_VOLUME_MAX;
    }
    if(volumeRight > ACM8625S_VOLUME_MAX)
    {
        volumeRight = ACM8625S_VOLUME_MAX;
    }

    txBuffer[0] = (int8_t(volumeLeft) - ACM8625S_VOLUME_MIN) * 2;
    txBuffer[1] = (int8_t(volumeRight) - ACM8625S_VOLUME_MIN) * 2;
    HAL_I2C_Mem_Write(&hi2c2, ACM8625S_I2C_ADDRESS, ACM8625S_VOLUME_CTRL_L, I2C_MEMADD_SIZE_8BIT, txBuffer, 2, 100);
}

void readStatus()
{
    HAL_I2C_Mem_Read(&hi2c2, ACM8625S_I2C_ADDRESS, ACM8625S_I2S_CLK_FORMAT_RPT1, I2C_MEMADD_SIZE_8BIT, rxBuffer, 7, 100);
    switch (rxBuffer[0] & 0x0F)
    {
        case 0x6:   status.detectedAudioSampleRate = 32.0f; break;
        case 0x8:   status.detectedAudioSampleRate = 44.1f; break;
        case 0x9:   status.detectedAudioSampleRate = 48.0f; break;
        case 0xA:   status.detectedAudioSampleRate = 88.2f; break;
        case 0xB:   status.detectedAudioSampleRate = 96.0f; break;
        case 0xC:   status.detectedAudioSampleRate = 176.4f;break;
        case 0xD:   status.detectedAudioSampleRate = 192.0f;break;
        default:    status.detectedAudioSampleRate = -1.0f; break;
    }

    status.dieID = rxBuffer[3];
    switch (rxBuffer[4] & 0x03)
    {
        case 0x0:   status.state = DigitalOff;  break;
        case 0x1:   status.state = AnalogOff;   break;
        case 0x2:   status.state = DriverOff;   break;
        case 0x3:   status.state = Play;        break;
    }

    status.CH1_OverCurrent  = (rxBuffer[5] & 0x01) >> 0;
    status.CH2_OverCurrent  = (rxBuffer[5] & 0x02) >> 1;
    status.CH1_DCFault      = (rxBuffer[5] & 0x04) >> 2;
    status.CH2_DCFault      = (rxBuffer[5] & 0x08) >> 3;
    status.underVoltage     = (rxBuffer[5] & 0x10) >> 4;
    status.overVoltage      = (rxBuffer[5] & 0x20) >> 5;
    status.overTemperature  = (rxBuffer[5] & 0x40) >> 6;

    status.clockFault       = (rxBuffer[6] & 0x04) >> 2;
}

} // namespace ACM8625S
