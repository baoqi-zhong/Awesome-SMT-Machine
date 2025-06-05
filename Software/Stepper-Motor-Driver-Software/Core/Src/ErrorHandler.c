/**
 * @file ErrorHandler.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "ErrorHandler.h"

#include "FOC.h"
#include "IncrementalPID.h"
#include "PositionalPID.h"

ErrorStatus_t motorErrorStatus;
ErrorCounter_t motorErrorCounter;

// 控制信号

extern float measuredIalpha, measuredIbeta, Vbus;

void ErrorHandlerInit()
{
    motorErrorStatus.haveError = 0;
    motorErrorStatus.driverFault = 0;
    motorErrorStatus.underOverVoltage = 0;
    motorErrorStatus.overTemperatureWarning = 0;
    motorErrorStatus.overTemperature = 0;
    motorErrorStatus.overCurrent = 0;
    motorErrorStatus.motorStalled = 0;
    motorErrorStatus.lastControlPacketDecodeError = 0;

    motorErrorCounter.underOverVoltageCounter = 0;
    motorErrorCounter.overTemperatureCounter = 0;
    motorErrorCounter.overCurrentCounter = 0;
}

uint8_t ErrorHandlerCheckIfAnyError()
{
    return motorErrorStatus.driverFault || motorErrorStatus.underOverVoltage || motorErrorStatus.overTemperature || motorErrorStatus.overTemperatureWarning || motorErrorStatus.overCurrent;
}

/// @brief 
void ErrorHandler()
{
    // 处理造成了 Fault 的严重 Error
    if(motorErrorStatus.driverFault == 1)
    {
        if(motorControlStatus.enableFOCOutput)
        {
            disableFOC();
            motorErrorStatus.haveError = 1;
            motorErrorCounter.underOverVoltageCounter = ERROR_TRIGGER_TRESHOLD_TIME - 1;
            motorErrorCounter.overCurrentCounter = ERROR_TRIGGER_TRESHOLD_TIME - 1;        
        }

        if(motorErrorStatus.overTemperatureWarning)
        {
            motorErrorStatus.overTemperature = 1;
        }
    }
    
    if(motorControlStatus.ignoreAllErrors || motorControlStatus.initializing)
        return;


    // 检查其他状态, 按需修改 motorErrorStatus
    if(Vbus < UNDER_VOLTAGE_THRESHOLD || Vbus > OVER_VOLTAGE_THRESHOLD)
    {
        if(motorErrorCounter.underOverVoltageCounter < ERROR_TRIGGER_TRESHOLD_TIME)
        {
            motorErrorCounter.underOverVoltageCounter += UNDER_OVER_VOLTAGE_TRIGGER_SPEED;
        }
    }
    else if(motorErrorCounter.underOverVoltageCounter)
    {
        motorErrorCounter.underOverVoltageCounter -= UNDER_OVER_VOLTAGE_UNTRIGGER_SPEED;
    }

    if((measuredIalpha > OVER_CURRENT_THRESHOLD || measuredIbeta > OVER_CURRENT_THRESHOLD))
    {
        if(motorErrorCounter.overCurrentCounter < ERROR_TRIGGER_TRESHOLD_TIME)
        {
            motorErrorCounter.overCurrentCounter += OVER_CURRENT_TRIGGER_SPEED;
        }
    }
    else if(motorErrorCounter.overCurrentCounter)
    {
        motorErrorCounter.overCurrentCounter -= OVER_CURRENT_UNTRIGGER_SPEED;
    }
    

    if(motorErrorCounter.underOverVoltageCounter >= ERROR_TRIGGER_TRESHOLD_TIME)
    {
        disableFOC();
        motorErrorStatus.haveError = 1;
        motorErrorStatus.underOverVoltage = 1;
    }
    else if(motorErrorCounter.underOverVoltageCounter == 0 && motorErrorStatus.underOverVoltage)
    {
        motorErrorStatus.underOverVoltage = 0;
    }


    if(motorErrorCounter.overCurrentCounter >= ERROR_TRIGGER_TRESHOLD_TIME)
    {
        disableFOC();
        motorErrorStatus.haveError = 1;
        motorErrorStatus.overCurrent = 1;
    }
    else if(motorErrorCounter.overCurrentCounter == 0 && motorErrorStatus.overCurrent)
    {
        motorErrorStatus.overCurrent = 0;
    }

    if(motorControlStatus.enableFOCOutput == 0 && motorControlStatus.triggerReset == 0 && ErrorHandlerCheckIfAnyError() == 0)
    {
        // 进行 auto Recovery
        if(motorControlStatus.enableAutoReset)
            motorControlStatus.triggerReset = 1;
    }
}

void ErrorHandlerClearErrorStatus()
{
    
}
