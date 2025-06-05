/**
 * @file ZeroTrigger.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "ZeroTrigger.hpp"
#include "MotorManager.hpp"


namespace ZeroTrigger
{
ZeroTriggerStatus_t ZeroTriggerStatus;

void update()
{
    ZeroTriggerStatus.XTriggered = MotorManager::motorRotate.getFeedbackStatus()->errorStatus.motorStalled;
    ZeroTriggerStatus.YTriggered = 1 - HAL_GPIO_ReadPin(TriggerY_GPIO_Port, TriggerY_Pin);
}

void Init()
{
    ZeroTriggerStatus.XTriggered = 0;
    ZeroTriggerStatus.YTriggered = 0;
}
}