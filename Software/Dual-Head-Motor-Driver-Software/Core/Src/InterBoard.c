/**
 * @file InterBoard.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "InterBoard.h"
#include "ErrorHandler.h"
#include "FOC.h"


uint8_t InterBoardTxBuffer[8] = {0};
void InterBoardDecode(uint32_t CANId, uint8_t* rxBuffer)
{
    if(CANId != 0x100 + CAN_ID)
        return;

    controlStatus.enableAutoReset = (rxBuffer[1] & CONTROL_PACKET_AUTO_RESET_MASK) >> 2;
    controlStatus.ignoreAllErrors = (rxBuffer[1] & CONTROL_PACKET_IGNORE_ERROR_MASK) >> 3;
    controlStatus.valveState[0] = (rxBuffer[0] & CONTROL_PACKET_VALVE_1_MASK) >> 4;
    controlStatus.valveState[1] = (rxBuffer[0] & CONTROL_PACKET_VALVE_2_MASK) >> 5;

    setZPosition(1, ((int32_t)((int16_t)(rxBuffer[2])) * 10));
    setZPosition(2, ((int32_t)((int16_t)(rxBuffer[3])) * 10));
    // updateArmMotor();
    // move(1, ((int32_t)((int16_t)(rxBuffer[4] << 8 | rxBuffer[5]))) * 32 / 65536 * TWO_PI);
    // move(2, ((int32_t)((int16_t)(rxBuffer[6] << 8 | rxBuffer[7]))) * 32 / 65536 * TWO_PI);

    // 最后调整 Enable 状态
    if(rxBuffer[1] & CONTROL_PACKET_ENABLE_MOTOR_MASK && !controlStatus.enableFOCOutput)
    {
        resetAndEnableFOC();
    }
    else if((rxBuffer[1] & CONTROL_PACKET_ENABLE_MOTOR_MASK) == 0 && controlStatus.enableFOCOutput)
    {
        disableFOC();
    }

    // 最后触发复位
    if(rxBuffer[1] & CONTROL_PACKET_TRIGGER_RESET_MASK)
    {
        controlStatus.triggerReset = 1;
    }
    
}

void InterBoardInit()
{
    FDCANManagerRegisterCallback(InterBoardDecode);
    FDCANManagerInit(&hfdcan1);
}

void InterBoardTransmitFeedback()
{
    // for(uint8_t i = 0; i < 8; i++)
    //     InterBoardTxBuffer[i] = 0;

    // Byte 0
    register uint8_t temp = 0;
    if(controlStatus.enableFOCOutput)
        temp |= RESPONSE_PACKET_ENABLE_MOTOR_MASK;
    if(controlStatus.enableAutoReset)
        temp |= RESPONSE_PACKET_AUTO_RESET_MASK;
    if(controlStatus.ignoreAllErrors)
        temp |= RESPONSE_PACKET_IGNORE_ERROR_MASK;
    if(controlStatus.microSwitchStatus)
        temp |= RESPONSE_PACKET_MICRO_SWITCH_MASK;
    if(controlStatus.GPIOStatus[0])
        temp |= RESPONSE_PACKET_GPIO1_MASK;
    if(controlStatus.GPIOStatus[1])
        temp |= RESPONSE_PACKET_GPIO2_MASK;
    if(controlStatus.GPIOStatus[2])
        temp |= RESPONSE_PACKET_GPIO3_MASK;


    InterBoardTxBuffer[0] = temp;

    // Byte 1
    temp = 0;
    if(errorStatus.haveError)
        temp |= RESPONSE_PACKET_ANY_ERROR_MASK;
    if(errorStatus.underOverVoltage)
        temp |= RESPONSE_PACKET_UNDER_OVER_VOLTAGE_MASK;
    if(errorStatus.driverFault[0])
        temp |= RESPONSE_PACKET_M1_DRIVER_FAULT_MASK;
    if(errorStatus.driverFault[1] || errorStatus.driverFault[2])
        temp |= RESPONSE_PACKET_M2_M3_DRIVER_FAULT_MASK;
    if(errorStatus.overCurrent[0])
        temp |= RESPONSE_PACKET_M1_OVER_CURRENT_MASK;
    if(errorStatus.overCurrent[1] || errorStatus.overCurrent[2])
        temp |= RESPONSE_PACKET_M2_M3_OVER_CURRENT_MASK;
    InterBoardTxBuffer[1] = temp;

    armForwardKinematics();
    // Byte 2-7
    InterBoardTxBuffer[2] = controlStatus.currentZHeight[0] * 10;
    InterBoardTxBuffer[3] = controlStatus.currentZHeight[1] * 10;
    InterBoardTxBuffer[4] = (int16_t)(controlStatus.currentPosition[1] * 10000) >> 8;
    InterBoardTxBuffer[5] = (int16_t)(controlStatus.currentPosition[1] * 10000);
    InterBoardTxBuffer[6] = (int16_t)(controlStatus.currentPosition[2] * 10000) >> 8;
    InterBoardTxBuffer[7] = (int16_t)(controlStatus.currentPosition[2] * 10000);
    FDCANManagerTransmit(0x200 + CAN_ID, InterBoardTxBuffer);
}
