/**
 * @file StepperMotor.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "StepperMotor.hpp"

namespace StepperMotor
{
#define FABS(x) ((x) > 0 ? (x) : -(x))

StepperMotor::StepperMotor(uint8_t _CANId) : speedLPF(0.1f)
{
    this->CANId = _CANId;
    this->controlStatus.enableFOCOutput = 1;
    this->controlStatus.RGBControlByMaster = 0;
    this->controlStatus.enableAutoReset = 1;
    this->controlStatus.ignoreAllErrors = 0;
    this->controlStatus.FOCControlMode = CURRENT_TOURQUE_CONTROL;
    this->controlStatus.enableSpeedCloseLoop = 1;
    this->controlStatus.enablePositionCloseLoop = 1;
    this->_triggerReset = 0;

    this->feedbackStatus.status.enableFOCOutput = 0;
    this->feedbackStatus.status.RGBControlByMaster = 0;
    this->feedbackStatus.status.enableAutoReset = 0;
    this->feedbackStatus.status.ignoreAllErrors = 0;
    this->feedbackStatus.status.FOCControlMode = CURRENT_TOURQUE_CONTROL;
    this->feedbackStatus.status.enableSpeedCloseLoop = 0;
    this->feedbackStatus.status.enablePositionCloseLoop = 0;


    this->feedbackStatus.errorStatus.haveError = 0;
    this->feedbackStatus.errorStatus.driverFault = 0;
    this->feedbackStatus.errorStatus.motorStalled = 0;
    this->feedbackStatus.errorStatus.underOverVoltage = 0;
    this->feedbackStatus.errorStatus.overCurrent = 0;
    this->feedbackStatus.errorStatus.overTemperature = 0;
    this->feedbackStatus.errorStatus.overTemperatureWarning = 0;
    this->feedbackStatus.errorStatus.lastControlPacketDecodeError = 0;
    this->feedbackStatus.torque = 0;
    this->feedbackStatus.encoder = 0;
    this->feedbackStatus.operationProgress = 0.0f;
    this->feedbackStatus.accumulatedEncoder = 0;
    this->feedbackStatus.shaftAngularVelocity = 0.0f;

    this->targetPosition = 0;
    this->lastTargetPosition = 0;
    this->lastAccumulatedEncoderWhenTargetPositionSet = 0;
    this->velocityLimit = 200000;
    this->tourqueLimit = 1.2f;
}

void StepperMotor::setTargetPosition(int32_t _targetPosition)
{
    this->lastAccumulatedEncoderWhenTargetPositionSet = this->feedbackStatus.accumulatedEncoder;
    this->targetPosition = _targetPosition;
}

void StepperMotor::setZeroPosition()
{
    this->feedbackStatus.lastEncoder = this->feedbackStatus.encoder;
    this->feedbackStatus.accumulatedEncoder = 0;
}

void StepperMotor::updateDisconnectStatus()
{
    this->disconnectCounter++;
    if(this->disconnectCounter > 100)
    {
        this->connectionStatus = 0;
    }
}

uint8_t* StepperMotor::packTxBuffer()
{
    // 非常粗暴, 一次性把所有状态都修改了
    this->txBuffer[0] = 0x7F;

    uint8_t txBufferByte1 = 0;
    if(controlStatus.enableFOCOutput)
        txBufferByte1 |= CONTROL_PACKET_ENABLE_MOTOR_MASK;
    if(controlStatus.RGBControlByMaster)
        txBufferByte1 |= CONTROL_PACKET_RGB_CONTROL_MASK;
    if(controlStatus.enableAutoReset)
        txBufferByte1 |= CONTROL_PACKET_AUTO_RESET_MASK;
    if(controlStatus.ignoreAllErrors)
        txBufferByte1 |= CONTROL_PACKET_IGNORE_ERROR_MASK;
    if(controlStatus.FOCControlMode == CURRENT_TOURQUE_CONTROL)
        txBufferByte1 |= CONTROL_PACKET_TORQUE_CONTROL_MASK;
    if(controlStatus.enableSpeedCloseLoop)
        txBufferByte1 |= CONTROL_PACKET_SPEED_CONTROL_MASK;
    if(controlStatus.enablePositionCloseLoop)
        txBufferByte1 |= CONTROL_PACKET_POSITION_CONTROL_MASK;
    
    this->txBuffer[1] = txBufferByte1;

    if(this->_triggerReset)
    {
        this->txBuffer[1] |= CONTROL_PACKET_TRIGGER_RESET_MASK;
        this->_triggerReset = 0;
    }
        

    this->txBuffer[2] = ((int16_t)(this->tourqueLimit * 10000.0f)) >> 8;
    this->txBuffer[3] = ((int16_t)(this->tourqueLimit * 10000.0f)) & 0xFF;

    this->txBuffer[4] = ((int16_t)(this->velocityLimit / 16)) >> 8;
    this->txBuffer[5] = ((int16_t)(this->velocityLimit / 16)) & 0xFF;

    // 改发绝对位置
    uint16_t positionSent = this->targetPosition >> 7;
    this->txBuffer[6] = positionSent >> 8;
    this->txBuffer[7] = positionSent & 0xFF;


    this->lastTargetPosition = this->targetPosition;

    return this->txBuffer;
}

void StepperMotor::decodeFeedbackMessage(uint8_t* _rxBuffer)
{
    uint8_t rxBufferByte0 = _rxBuffer[0];

    feedbackStatus.status.enableFOCOutput = (rxBufferByte0 & RESPONSE_PACKET_ENABLE_MOTOR_MASK) >> 0;
    feedbackStatus.status.RGBControlByMaster = (rxBufferByte0 & RESPONSE_PACKET_RGB_CONTROL_MASK) >> 1;
    feedbackStatus.status.enableAutoReset = (rxBufferByte0 & RESPONSE_PACKET_AUTO_RESET_MASK) >> 2;
    feedbackStatus.status.ignoreAllErrors = (rxBufferByte0 & RESPONSE_PACKET_IGNORE_ERROR_MASK) >> 3;
    if(rxBufferByte0 & RESPONSE_PACKET_TORQUE_CONTROL_MASK)
        feedbackStatus.status.FOCControlMode = VOLTAGE_TOURQUE_CONTROL;
    else
        feedbackStatus.status.FOCControlMode = CURRENT_TOURQUE_CONTROL;
    feedbackStatus.status.enableSpeedCloseLoop = (rxBufferByte0 & RESPONSE_PACKET_SPEED_CONTROL_MASK) >> 5;
    feedbackStatus.status.enablePositionCloseLoop = (rxBufferByte0 & RESPONSE_PACKET_POSITION_CONTROL_MASK) >> 6;

    uint8_t rxBufferByte1 = _rxBuffer[1];
    feedbackStatus.errorStatus.haveError = (rxBufferByte1 & RESPONSE_PACKET_ANY_ERROR_MASK) >> 0;
    feedbackStatus.errorStatus.driverFault = (rxBufferByte1 & RESPONSE_PACKET_DRIVER_FAULT_MASK) >> 1;
    feedbackStatus.errorStatus.motorStalled = (rxBufferByte1 & RESPONSE_PACKET_STALL_MASK) >> 2;
    feedbackStatus.errorStatus.underOverVoltage = (rxBufferByte1 & RESPONSE_PACKET_UNDER_OVER_VOLTAGE_MASK) >> 3;
    feedbackStatus.errorStatus.overCurrent = (rxBufferByte1 & RESPONSE_PACKET_OVER_CURRENT_MASK) >> 4;
    feedbackStatus.errorStatus.overTemperature = (rxBufferByte1 & RESPONSE_PACKET_OVER_TEMPERATURE_MASK) >> 5;
    feedbackStatus.errorStatus.overTemperatureWarning = (rxBufferByte1 & RESPONSE_PACKET_OVER_TEMPERATURE_WARNING_MASK) >> 6;
    feedbackStatus.errorStatus.lastControlPacketDecodeError = (rxBufferByte1 & RESPONSE_PACKET_CONTROL_PACKET_ERROR_MASK) >> 7;

    feedbackStatus.torque = ((float)((int16_t)(_rxBuffer[2] << 8 | _rxBuffer[3]))) / 10000.0f;
    feedbackStatus.encoder = _rxBuffer[4] << 8 | _rxBuffer[5];

    int16_t encoderDifference = feedbackStatus.encoder - feedbackStatus.lastEncoder;
    feedbackStatus.accumulatedEncoder += encoderDifference;
    feedbackStatus.lastEncoder = feedbackStatus.encoder;

    feedbackStatus.shaftAngularVelocity = speedLPF.update((float)encoderDifference);

    // 防止初始化的时候出现除零错误
    if(this->targetPosition - this->lastAccumulatedEncoderWhenTargetPositionSet < 2000)
        feedbackStatus.operationProgress = 1.0f;
    else
    {
        feedbackStatus.operationProgress = FABS((float)(feedbackStatus.accumulatedEncoder - this->lastAccumulatedEncoderWhenTargetPositionSet) / (float)(this->targetPosition - this->lastAccumulatedEncoderWhenTargetPositionSet));
        if(feedbackStatus.operationProgress > 1.0f)
            feedbackStatus.operationProgress = 1.0f;
        if(feedbackStatus.operationProgress < 0.0f)
            feedbackStatus.operationProgress = 0.0f;    
    }

    this->disconnectCounter = 0;
    this->connectionStatus = 1; 
}
}