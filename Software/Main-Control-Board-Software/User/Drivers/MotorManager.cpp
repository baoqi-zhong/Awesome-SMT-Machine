/**
 * @file MotorManager.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "MotorManager.hpp"
#include "FDCANManager.hpp"
#include "ZeroTrigger.hpp"

#include "math.h"

#include "FreeRTOS.h"
#include "task.h"


namespace MotorManager
{
StepperMotor::StepperMotor motorLeft(1);
StepperMotor::StepperMotor motorRight(2);
StepperMotor::StepperMotor motorRotate(3);

uint8_t* stepperMotorTxBuffer;

void (*motorStatusUpdatedNotificationCallback)() = NULL;


StackType_t uxMotorControlTaskStack[256];
StaticTask_t xMotorControlTaskTCB;


void motorControlTask(void *pvPara)
{
    while (true)
    {
        motorLeft.updateDisconnectStatus();
        motorRight.updateDisconnectStatus();
        motorRotate.updateDisconnectStatus();

        stepperMotorTxBuffer = motorLeft.packTxBuffer();
        FDCANManager::Transmit(0x101, stepperMotorTxBuffer);

        stepperMotorTxBuffer = motorRight.packTxBuffer();
        FDCANManager::Transmit(0x102, stepperMotorTxBuffer);

        stepperMotorTxBuffer = motorRotate.packTxBuffer();
        FDCANManager::Transmit(0x103, stepperMotorTxBuffer);

        ZeroTrigger::update();
        vTaskDelay(2);
    }
}

void stepperMotorReceiveCallback(uint32_t CANId, uint8_t* rxBuffer)
{
    if(CANId == 0x201)
    {
        motorLeft.decodeFeedbackMessage(rxBuffer);
    }
    else if(CANId == 0x202)
    {
        motorRight.decodeFeedbackMessage(rxBuffer);
    }
    else if(CANId == 0x203)
    {
        motorRotate.decodeFeedbackMessage(rxBuffer);
    }

    if(motorStatusUpdatedNotificationCallback != NULL)
    {
        motorStatusUpdatedNotificationCallback();
    }
}

void RegisterCallback(void (*callback)())
{
    motorStatusUpdatedNotificationCallback = callback;
}

void stopAllMotors()
{
    // motorLeft.triggerReset = 1;
    // motorRight.triggerReset = 1;
}

void Init()
{
    FDCANManager::RegisterCallback(stepperMotorReceiveCallback);
    xTaskCreateStatic(motorControlTask, "motorControlTask", 256, NULL, 0, uxMotorControlTaskStack, &xMotorControlTaskTCB);
}
}