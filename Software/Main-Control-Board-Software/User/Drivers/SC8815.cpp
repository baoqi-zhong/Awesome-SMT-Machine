/**
 * @file SC8815.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "SC8815.hpp"

namespace SC8815
{
uint8_t txBuffer[1];
uint8_t rxBuffer[27];
FeedbackPowerValue_t feedbackPowerValue;
ErrorStatus_t errorStatus;
ControlStatus_t controlStatus;



void init()
{
    feedbackPowerValue.VBUS = 0.0f;
    feedbackPowerValue.VBAT = 0.0f;
    feedbackPowerValue.IBUS = 0.0f;
    feedbackPowerValue.IBAT = 0.0f;
    feedbackPowerValue.PBUS = 0.0f;
    feedbackPowerValue.PBAT = 0.0f;
    feedbackPowerValue.efficiency = 1.0f;

    errorStatus.anyFault = 0;
    errorStatus.VBUSOverVoltage = 0;
    errorStatus.VBUSOverCurrent = 0;
    errorStatus.VBATOverVoltage = 0;
    errorStatus.VBATOverCurrent = 0;
    errorStatus.lowBuckBoostEfficiency = 0;
    errorStatus.overTemperature = 0;
    errorStatus.VBUSShortCircuit = 0;

    controlStatus.VBUS = 0.0f;
    controlStatus.IBUSLimit = 0.0f;
    controlStatus.IBATLimit = 0.0f;
    controlStatus.powerRatio.VBUSRatio = 5.0f;
    controlStatus.powerRatio.VBATRatio = 12.5f;
    controlStatus.powerRatio.IBUSRatio = 3.0f;
    controlStatus.powerRatio.IBATRatio = 12.0f;

    PowerOff();

    // 放电模式.

    // 无电池内阻补偿, VBAT 电压外部控制(没啥用)
    txBuffer[0] = 0b00100001;
    HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_VBAT_SET, 0x01, txBuffer, 1, 15);


    // VBAT 电压由 VBUSREF_I_SET, VBUSREF_I_SET2, VBUS_RATIO 控制
    setVBUS(DEFAULT_VBUS);
    setIBUSLimit(DEFAULT_IBUS_LIMIT);
    setIBATLimit(DEFAULT_IBAT_LIMIT);
  
    // 放电模式, 150KHz 开关频率, 20ns 死区
    txBuffer[0] = 0b10000000;
    HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_CTRL0_SET, 0x01, txBuffer, 1, 15);


    // 禁用涓流充电, 禁用自动终止, 放电电压内部控制, 使能 VBUS OVP
    txBuffer[0] = 0b01100001;
    HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_CTRL1_SET, 0x01, txBuffer, 1, 15);


    // 使能 ADC, 使关闭路保护(会低压误判), 始终 PWM 模式
    txBuffer[0] = 0b00100100;
    HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_CTRL3_SET, 0x01, txBuffer, 1, 15);

    PowerOn();
}

void setVBUS(float VBUS)
{
    uint8_t ratioRegisterValue = 0;
    HAL_I2C_Mem_Read(&hi2c1, SC8815_ReadAddress, SC8815_RW_RATIO, 0x01, &ratioRegisterValue, 1, 15);

    if (VBUS <= 25.6f)
    {
        controlStatus.VBUS = VBUS;
        // 使用内部 ratio
        if(controlStatus.VBUS <= 10.24f)
        {
            controlStatus.powerRatio.VBUSRatio = 5.0f;
            ratioRegisterValue |= 0x01;
        }
        else
        {
            controlStatus.powerRatio.VBUSRatio = 12.5f;
            ratioRegisterValue &= 0xFE;
        }
        uint16_t VBUSRefRegisterValue = (uint16_t)(controlStatus.VBUS / controlStatus.powerRatio.VBUSRatio / 0.002f);
        uint8_t VBUSRefRegisterValueH = VBUSRefRegisterValue >> 2;
        uint8_t VBUSRefRegisterValueL = (VBUSRefRegisterValue & 0x03) << 6;
        HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_RATIO, 0x01, &ratioRegisterValue, 1, 15);
        HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_VBUSREF_I_SET, 0x01, &VBUSRefRegisterValueH, 1, 15);
        HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_VBUSREF_I_SET2, 0x01, &VBUSRefRegisterValueL, 1, 15);
    }
    else
    {
        // 需要使用外部 ratio
    }
}

void setIBUSLimit(float IBUS)
{
    if(IBUS < 0.3f)
        IBUS = 0.3f;
    

    float IBUSLimDivideValue1 = 3.0f * (0.01f / HWCFG_CS_Resistor);
    float IBUSLimDivideValue2 = 6.0f * (0.01f / HWCFG_CS_Resistor);

    if(IBUS > IBUSLimDivideValue2)
    {
        IBUS = IBUSLimDivideValue2;
    }
    controlStatus.IBUSLimit = IBUS;


    uint8_t ratioRegisterValue = 0;
    HAL_I2C_Mem_Read(&hi2c1, SC8815_ReadAddress, SC8815_RW_RATIO, 0x01, &ratioRegisterValue, 1, 15);
    ratioRegisterValue &= 0b11110011;

    if(controlStatus.IBUSLimit < IBUSLimDivideValue1)
    {
        controlStatus.powerRatio.IBUSRatio = 3.0f;
        ratioRegisterValue |= 0b00001000;
    }
    else
    {
        controlStatus.powerRatio.IBUSRatio = 6.0f;
        ratioRegisterValue |= 0b00000100;
    }
    uint8_t IBUSLimRegisterValue = controlStatus.IBUSLimit * 256.0f / controlStatus.powerRatio.IBUSRatio / 0.01f * HWCFG_CS_Resistor - 1;

    HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_RATIO, 0x01, &ratioRegisterValue, 1, 15);
    HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_IBUS_LIM_SET, 0x01, &IBUSLimRegisterValue, 1, 15);
}


void setIBATLimit(float IBAT)
{
    if(IBAT < 0.3f)
        IBAT = 0.3f;
    

    float IBATLimDivideValue1 = 6.0f * (0.01f / HWCFG_CS_Resistor);
    float IBATLimDivideValue2 = 12.0f * (0.01f / HWCFG_CS_Resistor);

    if(IBAT > IBATLimDivideValue2)
    {
        IBAT = IBATLimDivideValue2;
    }
    controlStatus.IBATLimit = IBAT;


    uint8_t ratioRegisterValue = 0;
    HAL_I2C_Mem_Read(&hi2c1, SC8815_ReadAddress, SC8815_RW_RATIO, 0x01, &ratioRegisterValue, 1, 15);

    if(controlStatus.IBATLimit < IBATLimDivideValue1)
    {
        controlStatus.powerRatio.IBATRatio = 6.0f;
        ratioRegisterValue &= 0b11101111;

    }
    else
    {
        controlStatus.powerRatio.IBATRatio = 12.0f;
        ratioRegisterValue |= 0b00010000;
    }
    uint8_t IBATLimRegisterValue = (controlStatus.IBATLimit * 256.0f / (controlStatus.powerRatio.IBATRatio * (0.01f / HWCFG_CS_Resistor))) - 1;

    HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_RATIO, 0x01, &ratioRegisterValue, 1, 15);
    HAL_I2C_Mem_Write(&hi2c1, SC8815_WriteAddress, SC8815_RW_IBAT_LIM_SET, 0x01, &IBATLimRegisterValue, 1, 15);
}

void PowerOn()
{
    HAL_GPIO_WritePin(PSTOP_GPIO_Port, PSTOP_Pin, GPIO_PIN_RESET);
}

void PowerOff()
{
    HAL_GPIO_WritePin(PSTOP_GPIO_Port, PSTOP_Pin, GPIO_PIN_SET);
}

void readADC()
{
    HAL_I2C_Mem_Read(&hi2c1, SC8815_ReadAddress, SC8815_R_VBUS_FB_VALUE, 0x01, rxBuffer, 8, 100);
    feedbackPowerValue.VBUS = ((rxBuffer[0] << 2) + (rxBuffer[1] >> 6) + 1) * controlStatus.powerRatio.VBUSRatio * 0.002f;
    feedbackPowerValue.VBAT = ((rxBuffer[2] << 2) + (rxBuffer[3] >> 6) + 1) * controlStatus.powerRatio.VBATRatio * 0.002f;
    feedbackPowerValue.IBUS = ((rxBuffer[4] << 2) + (rxBuffer[5] >> 6) + 1) / 600.0f * controlStatus.powerRatio.IBUSRatio * 0.01f / HWCFG_CS_Resistor;
    feedbackPowerValue.IBAT = ((rxBuffer[6] << 2) + (rxBuffer[7] >> 6) + 1) / 600.0f * controlStatus.powerRatio.IBATRatio * 0.01f / HWCFG_CS_Resistor;

    feedbackPowerValue.PBAT = feedbackPowerValue.VBAT * feedbackPowerValue.IBAT;
    feedbackPowerValue.PBUS = feedbackPowerValue.VBUS * feedbackPowerValue.IBUS;

    // assuming Power from VBAT to VBUS (放电模式)
    feedbackPowerValue.efficiency = feedbackPowerValue.PBUS / feedbackPowerValue.PBAT;

    if(feedbackPowerValue.efficiency >1 )
        feedbackPowerValue.efficiency = 1 / feedbackPowerValue.efficiency;

    checkFault();
}

uint8_t checkFault()
{
    HAL_I2C_Mem_Read(&hi2c1, SC8815_ReadAddress, SC8815_R_STATUS, 0x01, rxBuffer, 1, 100);
    errorStatus.overTemperature = (rxBuffer[0] & 0x04) >> 2;
    errorStatus.VBUSShortCircuit = (rxBuffer[0] & 0x08) >> 3;

    if(errorStatus.overTemperature || errorStatus.VBUSShortCircuit)
    {
        errorStatus.anyFault = 1;
    }

    return errorStatus.anyFault;
}

float get_VBUS_Volt()
{
    return feedbackPowerValue.VBUS;
}

float get_VBAT_Volt()
{
    return feedbackPowerValue.VBAT;
}

float get_IBUS_Amp()
{
    return feedbackPowerValue.IBUS;
}

float get_IBAT_Amp()
{
    return feedbackPowerValue.IBAT;
}

};