/**
 * @file GCodeDecoder.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "GCodeDecoder.hpp"
#include "CoreXY.hpp"
#include "UIManager.hpp"
#include "RGBeffect.hpp"

#include "stdio.h"
#include "string.h"

#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_SYSVIEW.h"

namespace GCodeDecoder
{
GCodeDecoderStatus_t GCodeDecoderStatus;

char feedbackBuffer[128];

// void packPositionFeedback(char* pBuffer)
// {
//     sprintf(pBuffer, "X:%.2f Y:%.2f Z:%.2f E:%.2f", 0.0, 0.0, 0.0, 0.0);
// }

void sendFeedback(const char* pBuffer)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)pBuffer, strlen((char *)pBuffer), 100);
}

void init()
{
    GCodeDecoderStatus.busyDecoding = 0;
    GCodeDecoderStatus.commandCounter = 0;
    GCodeDecoderStatus.commandErrorCounter = 0;
    GCodeDecoderStatus.unknownCommandCounter = 0;
}

int16_t scanNumber(uint8_t *pBuffer, uint8_t maxLength)
{
    if(pBuffer == NULL)
    {
        return 32767;
    }

    // Skip leading spaces
    while (pBuffer[0] == ' ')
    {
        pBuffer++;
    }
    
    if((pBuffer[0] < '0' || pBuffer[0] > '9') && pBuffer[0] != '-')
    {
        return 32767;
    }

    // Skip - sign
    uint8_t negative = 0;
    if(pBuffer[0] == '-')
    {
        pBuffer++;
        negative = 1;
    }

    int16_t number = 0;
    uint8_t i = 0;

    while (i < maxLength)
    {
        if (pBuffer[i] >= '0' && pBuffer[i] <= '9')
        {
            number = number * 10 + pBuffer[i] - '0';
        }
        else
        {
            break;
        }
        i++;
    }
    if(negative)
    {
        number = -number;
    }
    return number;
}

void decodeMessageErrorHandler()
{
    GCodeDecoderStatus.commandErrorCounter += 1;
    GCodeDecoderStatus.busyDecoding = 0;
}

void decodeMessage(uint8_t *rxData, uint8_t size)
{
    if (GCodeDecoderStatus.busyDecoding)
    {
        /* Too Fast, use queue or slow down. */
        __BKPT(0);
    }
    
    GCodeDecoderStatus.busyDecoding = 1;
    GCodeDecoderStatus.commandCounter += 1;

    // 消息合法性检查
    if(rxData[size - 1] != '\n')
    {
        decodeMessageErrorHandler();
        return;
    }
    if(rxData[0] != 'G' && rxData[0] != 'M')
    {
        decodeMessageErrorHandler();
        return;
    }

    // 提取消息中的控制数字
    int16_t num = scanNumber(rxData + 1, size - 2);
    if(num == 32767)
    {
        decodeMessageErrorHandler();
        return;
    }

    float x = 0;
    float y = 0;
    float z = 0;
    float e = 0;
    char componentName[4] = {0};

    switch (rxData[0])
    {
    case 'G':
        switch (num)
        {
        case 0:
        case 1:
            if(num == 0)
                HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_RESET);
            else
                HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_SET);
            // G0 / G1: Rapid Move
            if(rxData[3] != 'X' || rxData[10] != 'Y' || rxData[17] != 'Z' || rxData[24] != 'E')
            {
                decodeMessageErrorHandler();
                return;
            }
            x = (float)(scanNumber(rxData + 4, 5)) / 100.0f;
            y = (float)(scanNumber(rxData + 11, 5)) / 100.0f;
            z = (float)(scanNumber(rxData + 18, 5)) / 100.0f;
            e = (float)(scanNumber(rxData + 25, 5)) / 100.0f;
            if(x == 32767 || y == 32767 || z == 32767 || e == 32767)
            {
                decodeMessageErrorHandler();
                return;
            }
            
            CoreXY::controlStatus.needTransmitMovementCpltInfo = 1;
            CoreXY::setTargetPosition(x, y, z, e);
            break;

        case 28:
            // G28: Home
            RGBeffect::calibrationLightEffect();
            CoreXY::triggerFindOrigin();
            break;

        default:
            GCodeDecoderStatus.unknownCommandCounter += 1;
            break;
        }
        break;

    case 'M':
        switch (num)
        {
        case 112:
            // M112: Emergency Stop
            MotorManager::motorLeft.disableMotor();
            MotorManager::motorRight.disableMotor();
            MotorManager::motorRotate.disableMotor();
            RGBeffect::errorLightEffect();
            sendFeedback("ok Emergency Stopped\n");
            break;

        case 115:
            // M115: Get Firmware Version and Capabilities
            sprintf(feedbackBuffer, "ok Conn FIRMWARE_NAME:%s FIRMWARE_VERSION:%s\n", FIRMWARE_NAME, FIRMWARE_VERSION);
            sendFeedback(feedbackBuffer);
            break;

        case 997:
            // M998: 用户定义: 事件, 用来刷 WS2812
            if(rxData[5] == 'S') // SMT 开始
                RGBeffect::SMTLightEffect();
            else if (rxData[5] == 'I') // Idle
                RGBeffect::idleLightEffect();
            else if (rxData[5] == 'F') // SMT Finish
                RGBeffect::finishSMTLightEffect();
            
            break;
        case 998:
            // M998: 用户定义: 接受下一个要贴片的元件数据, 用来刷 UI
            componentName[0] = rxData[5];
            componentName[1] = rxData[6];
            componentName[2] = rxData[7];
            componentName[3] = 0;

            UIManager::newComponent(componentName, rxData[9] - '0');
            break;

        case 999:
            // M999: Restart
            RGBeffect::idleLightEffect();

            MotorManager::motorLeft.triggerReset();
            MotorManager::motorRight.triggerReset();
            MotorManager::motorRotate.triggerReset();
            MotorManager::motorRotate.enableMotor();
            MotorManager::motorLeft.enableMotor();
            MotorManager::motorRight.enableMotor();
            // 防止别电机
            CoreXY::setTargetPosition(CoreXY::feedbackPosition.X, CoreXY::feedbackPosition.Y, CoreXY::feedbackPosition.Z, CoreXY::feedbackPosition.E);
            MotorManager::motorLeft.setAutoResetStatus(1);
            MotorManager::motorRight.setAutoResetStatus(1);
            MotorManager::motorRotate.setAutoResetStatus(1);
            sendFeedback("ok Emergency Restarted\n");
            break;

        default:
            GCodeDecoderStatus.unknownCommandCounter += 1;
            break;
        }
        break;

    default:
        // Should not reach here
        decodeMessageErrorHandler();
        return;
    }


    GCodeDecoderStatus.busyDecoding = 0;
    // taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
}

}