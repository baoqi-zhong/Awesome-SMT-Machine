/**
 * @file InterBoard.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "InterBoard.h"
#include "ErrorHandler.h"
#include "FOC.h"
#include "PositionalPID.h"
#include "IncrementalPID.h"
#include "MA732.h"

extern PositionalPID_t positionToCurrentPID, positionToVelocityPID, velocityPID;
extern IncrementalPID_t IqPID, IdPID;

uint8_t InterBoardTxBuffer[8] = {0};
void InterBoardDecode(uint32_t CANId, uint8_t* rxBuffer)
{
    if(CANId != 0x100 + CAN_ID)
        return;
    if((rxBuffer[0] & 0x80) == 0)
    {
        if(rxBuffer[0] & CONTROL_PACKET_ENABLE_MOTOR_MASK)
        {
            if(rxBuffer[1] & 0x01 && !motorControlStatus.enableFOCOutput)
            {
                    resetAndEnableFOC();
            }
            else if((rxBuffer[1] & 0x01) == 0 && motorControlStatus.enableFOCOutput)
            {
                disableFOC();
            }
                
        }
            
        if(rxBuffer[0] & CONTROL_PACKET_RGB_CONTROL_MASK)
            motorControlStatus.RGBControlByMaster = (rxBuffer[1] & CONTROL_PACKET_RGB_CONTROL_MASK) >> 1;

        if(rxBuffer[0] & CONTROL_PACKET_AUTO_RESET_MASK)
            motorControlStatus.enableAutoReset = (rxBuffer[1] & CONTROL_PACKET_AUTO_RESET_MASK) >> 2;
  
        if(rxBuffer[0] & CONTROL_PACKET_IGNORE_ERROR_MASK)
            motorControlStatus.ignoreAllErrors = (rxBuffer[1] & CONTROL_PACKET_IGNORE_ERROR_MASK) >> 3;

// HACK: 如果是 3 号纯开环的电机, 则不需要这些控制, 防止模式被主控板修改
#if ((CAN_ID == 1 || CAN_ID == 2))
        if(rxBuffer[0] & CONTROL_PACKET_TORQUE_CONTROL_MASK)
        {
            if((rxBuffer[1] & CONTROL_PACKET_TORQUE_CONTROL_MASK) && motorControlStatus.FOCControlMode != CURRENT_TOURQUE_CONTROL)
            {
                IncrementalPIDreset(&IqPID);
                IncrementalPIDreset(&IdPID);
                motorControlStatus.FOCControlMode = CURRENT_TOURQUE_CONTROL;
            }
            else if ((rxBuffer[1] & CONTROL_PACKET_TORQUE_CONTROL_MASK) == 0 && motorControlStatus.FOCControlMode != VOLTAGE_TOURQUE_CONTROL)
            {
                motorControlStatus.FOCControlMode = VOLTAGE_TOURQUE_CONTROL;     
            }
        }

        if(rxBuffer[0] & CONTROL_PACKET_SPEED_CONTROL_MASK)
        {
            if((rxBuffer[1] & CONTROL_PACKET_SPEED_CONTROL_MASK) && !motorControlStatus.enableSpeedCloseLoop)
            {
                PositionalPIDreset(&velocityPID);
                PositionalPIDreset(&positionToVelocityPID);
                motorControlStatus.enableSpeedCloseLoop = 1;            
            }
            else if((rxBuffer[1] & CONTROL_PACKET_SPEED_CONTROL_MASK) == 0 && motorControlStatus.enableSpeedCloseLoop)
            {
                PositionalPIDreset(&positionToCurrentPID);
                motorControlStatus.enableSpeedCloseLoop = 0;
            }
        }

        if(rxBuffer[0] & CONTROL_PACKET_POSITION_CONTROL_MASK)
        {
            if(rxBuffer[1] & CONTROL_PACKET_POSITION_CONTROL_MASK && !motorControlStatus.enablePositionCloseLoop)
            {
                if(motorControlStatus.enableSpeedCloseLoop)
                    PositionalPIDreset(&positionToVelocityPID);
                else
                    PositionalPIDreset(&positionToCurrentPID);
                motorControlStatus.enablePositionCloseLoop = 1;
            }
            else if((rxBuffer[1] & CONTROL_PACKET_POSITION_CONTROL_MASK) == 0 && motorControlStatus.enablePositionCloseLoop)
            {
                motorControlStatus.enablePositionCloseLoop = 0;
            }
        }
#endif

        motorControlStatus.tourqueLimit = (float)((int16_t)(rxBuffer[2] << 8 | rxBuffer[3])) / 10000.0f;
        #if (CAN_ID == 1 || CAN_ID == 2)
        motorControlStatus.velocityLimit = ((int32_t)((int16_t)(rxBuffer[4] << 8 | rxBuffer[5]))) * 16;
        #else
        motorControlStatus.velocityLimit = ((int32_t)((int16_t)(rxBuffer[4] << 8 | rxBuffer[5]))) * 64;
        #endif
        // motorControlStatus.targetPosition += ((int32_t)((int16_t)(rxBuffer[6] << 8 | rxBuffer[7]))) * 32;
        motorControlStatus.targetPosition = ((int32_t)((int16_t)(rxBuffer[6] << 8 | rxBuffer[7]))) * 128;
        // 最后触发复位
        if(rxBuffer[1] & CONTROL_PACKET_TRIGGER_RESET_MASK)
        {
            motorControlStatus.triggerReset = 1;
        }
    }
}

void InterBoardInit()
{
    FDCANManagerRegisterCallback(InterBoardDecode);
    FDCANManagerInit(&hfdcan1);
}

extern float measuredIq;
void InterBoardTransmitFeedback()
{
     for(uint8_t i = 0; i < 8; i++)
        InterBoardTxBuffer[i] = 0;

    uint8_t temp = 0;
    if(motorControlStatus.enablePositionCloseLoop)
        temp |= RESPONSE_PACKET_POSITION_CONTROL_MASK;
    if(motorControlStatus.enableSpeedCloseLoop)
        temp |= RESPONSE_PACKET_SPEED_CONTROL_MASK;
    if(motorControlStatus.FOCControlMode == VOLTAGE_TOURQUE_CONTROL)
        temp |= RESPONSE_PACKET_TORQUE_CONTROL_MASK;
    if(motorControlStatus.ignoreAllErrors)
        temp |= RESPONSE_PACKET_IGNORE_ERROR_MASK;
    if(motorControlStatus.enableAutoReset)
        temp |= RESPONSE_PACKET_AUTO_RESET_MASK;
    if(motorControlStatus.RGBControlByMaster)
        temp |= RESPONSE_PACKET_RGB_CONTROL_MASK;
    if(motorControlStatus.enableFOCOutput)
        temp |= RESPONSE_PACKET_ENABLE_MOTOR_MASK;
    InterBoardTxBuffer[0] = temp;

    temp = 0;
    if(motorErrorStatus.lastControlPacketDecodeError)
        temp |= RESPONSE_PACKET_ERROR_MASK;
    if(motorErrorStatus.overTemperatureWarning)
        temp |= RESPONSE_PACKET_OVER_TEMPERATURE_WARNING_MASK;
    if(motorErrorStatus.overTemperature)
        temp |= RESPONSE_PACKET_OVER_TEMPERATURE_MASK;
    if(motorErrorStatus.overCurrent)
        temp |= RESPONSE_PACKET_OVER_CURRENT_MASK;
    if(motorErrorStatus.underOverVoltage)
        temp |= RESPONSE_PACKET_UNDER_OVER_VOLTAGE_MASK;
    if(motorErrorStatus.motorStalled)
        temp |= RESPONSE_PACKET_STALL_MASK;
    if(motorErrorStatus.driverFault)
        temp |= RESPONSE_PACKET_DRIVER_FAULT_MASK;
    if(motorErrorStatus.haveError)
        temp |= RESPONSE_PACKET_ANY_ERROR_MASK;
    InterBoardTxBuffer[1] = temp;

    InterBoardTxBuffer[2] = (int16_t)(measuredIq*10000) >> 8;
    InterBoardTxBuffer[3] = (int16_t)(measuredIq*10000) & 0xFF;
    InterBoardTxBuffer[4] = encoder >> 8;
    InterBoardTxBuffer[5] = encoder & 0xFF;

    InterBoardTxBuffer[6] = (motorControlStatus.targetPosition - accumulatedEncoder) / 32;

    FDCANManagerTransmit(0x200 + CAN_ID, InterBoardTxBuffer);
}
