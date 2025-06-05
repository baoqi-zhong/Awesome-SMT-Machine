/**
 * @file ErrorHandler.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "ErrorHandler.h"
#include "FOC.h"

ErrorStatus_t errorStatus;
ErrorCounter_t errorCounter;


void ErrorHandlerInit()
{
    errorStatus.haveError = 0;
    errorStatus.underOverVoltage = 0;
    errorStatus.lastControlPacketDecodeError = 0;

    errorCounter.underOverVoltageCounter = 0;

    for(int i = 0; i < 3; i++)
    {
        errorStatus.driverFault[i] = 0;
        errorStatus.overCurrent[i] = 0;
        errorCounter.overCurrentCounter[i] = 0;
    }
}

uint8_t ErrorHandlerCheckIfAnyError()
{
    return errorStatus.driverFault[0] || \
        errorStatus.driverFault[1] || \
        errorStatus.driverFault[2] || \
        errorStatus.overCurrent[0] || \
        errorStatus.overCurrent[1] || \
        errorStatus.overCurrent[2] || \
        errorStatus.underOverVoltage || \
        errorStatus.lastControlPacketDecodeError;
}

void ErrorHandler()
{
    errorStatus.driverFault[0] = 1 - HAL_GPIO_ReadPin(M1_nFAULT_GPIO_Port, M1_nFAULT_Pin);
    errorStatus.driverFault[1] = 1 - HAL_GPIO_ReadPin(M2_nFAULT_GPIO_Port, M2_nFAULT_Pin);
    errorStatus.driverFault[2] = 1 - HAL_GPIO_ReadPin(M3_nFAULT_GPIO_Port, M3_nFAULT_Pin);

    // 处理造成了 Fault 的严重 Error
    if(errorStatus.driverFault[0] == 1 || errorStatus.driverFault[1] == 1 || errorStatus.driverFault[2] == 1)
    {
        if(controlStatus.enableFOCOutput)
        {
            disableFOC();
            errorStatus.haveError = 1;
            errorCounter.underOverVoltageCounter = ERROR_TRIGGER_TRESHOLD_TIME - 1;
            for(int i = 0; i < 3; i++)
            {
                errorCounter.overCurrentCounter[i] = ERROR_TRIGGER_TRESHOLD_TIME - 1;
            }
        }
    }
    
    if(controlStatus.ignoreAllErrors || controlStatus.initializing)
        return;


    // 检查其他状态, 按需修改 errorStatus
    if(Vbus < UNDER_VOLTAGE_THRESHOLD || Vbus > OVER_VOLTAGE_THRESHOLD)
    {
        if(errorCounter.underOverVoltageCounter < ERROR_TRIGGER_TRESHOLD_TIME)
        {
            errorCounter.underOverVoltageCounter += UNDER_OVER_VOLTAGE_TRIGGER_SPEED;
        }
    }
    else if(errorCounter.underOverVoltageCounter)
    {
        errorCounter.underOverVoltageCounter -= UNDER_OVER_VOLTAGE_UNTRIGGER_SPEED;
    }

    
    for(int i = 0; i < 3; i++)
    {
        if(measuredIMotor[i]> OVER_CURRENT_THRESHOLD)
        {
            if(errorCounter.overCurrentCounter[i] < ERROR_TRIGGER_TRESHOLD_TIME)
            {
                errorCounter.overCurrentCounter[i] += OVER_CURRENT_TRIGGER_SPEED;
            }
        }
        else if(errorCounter.overCurrentCounter[i])
        {
            errorCounter.overCurrentCounter[i] -= OVER_CURRENT_UNTRIGGER_SPEED;
        }
    }

    if(errorCounter.underOverVoltageCounter >= ERROR_TRIGGER_TRESHOLD_TIME)
    {
        disableFOC();
        errorStatus.haveError = 1;
        errorStatus.underOverVoltage = 1;
    }
    else if(errorCounter.underOverVoltageCounter == 0 && errorStatus.underOverVoltage)
    {
        errorStatus.underOverVoltage = 0;
    }

    for(int i = 0; i < 3; i++)
    {
        if(errorCounter.overCurrentCounter[i] >= ERROR_TRIGGER_TRESHOLD_TIME)
        {
            disableFOC();
            errorStatus.haveError = 1;
            errorStatus.overCurrent[i] = 1;
        }
        else if(errorCounter.overCurrentCounter[i] == 0 && errorStatus.overCurrent[i])
        {
            errorStatus.overCurrent[i] = 0;
        }
    }


    if(controlStatus.enableFOCOutput == 0 && controlStatus.triggerReset == 0 && ErrorHandlerCheckIfAnyError() == 0)
    {
        // 进行 auto Recovery
        if(controlStatus.enableAutoReset)
            controlStatus.triggerReset = 1;
    }
}

void ErrorHandlerClearErrorStatus()
{
    
}
