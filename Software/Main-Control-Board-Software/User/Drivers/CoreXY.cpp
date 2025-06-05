/**
 * @file CoreXY.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "CoreXY.hpp"
#include "ZeroTrigger.hpp"
#include "GCodeDecoder.hpp"
#include "Servo.hpp"
#include "RGBEffect.hpp"

#include "stdio.h"

#include "FreeRTOS.h"
#include "task.h"

// CoreXY 和 Z 轴的控制都在这个文件里面
namespace CoreXY
{
const float CoreXYDefaultSpeed = 140.0f;
const float CoreXYFindOriginSpeed = 8.0f;

const float ZAXIS_UPPER_LIMIT_ANGLE = 145.0f;   // 单位: 度
const float ZAXIS_LOWER_LIMIT_ANGLE = 85.0f;    // 单位: 度
const float ZAXIS_UPPER_LIMIT_POSITION = 8.5f;  // 单位: mm
const float ZAXIS_LOWER_LIMIT_POSITION = 0.5f;  // 单位: mm


CoreXYData_t targetPosition;
CoreXYData_t targetSpeed;
CoreXYData_t feedbackPosition;
CoreXYData_t feedbackSpeed;

CoreXYControlStatus_t controlStatus;

float originOffsetX = 0.0f;
float originOffsetY = 0.0f;

// 从 CoreXY 坐标系转换到电机角度, 解码 Feedback
void forwardKinematics(CoreXYData_t* CoreXYData)
{
    CoreXYData->X = (CoreXYData->motorLeft + CoreXYData->motorRight) / 2 * BELT_PITCH * BELT_PULLEY_TEETH / 65536 - originOffsetX;
    CoreXYData->Y = -(CoreXYData->motorLeft - CoreXYData->motorRight) / 2 * BELT_PITCH * BELT_PULLEY_TEETH / 65536 - originOffsetY;
    CoreXYData->Z = (CoreXYData->motorZ - ZAXIS_LOWER_LIMIT_ANGLE) * BELT_PITCH * BELT_PULLEY_TEETH / 360.0f;
    CoreXYData->E = CoreXYData->motorRotate * 360.0f / 65536;
}

// 从电机角度转换到 CoreXY 坐标系, 编码 Transmit
void inverseKinematics(CoreXYData_t* CoreXYData)
{
    CoreXYData->motorLeft = (CoreXYData->X + originOffsetX - (CoreXYData->Y + originOffsetY)) / BELT_PITCH / BELT_PULLEY_TEETH * 65536;
    CoreXYData->motorRight = (CoreXYData->X + originOffsetX + CoreXYData->Y + originOffsetY) / BELT_PITCH / BELT_PULLEY_TEETH * 65536;

    if(CoreXYData->Z > ZAXIS_UPPER_LIMIT_POSITION)
        CoreXYData->Z = ZAXIS_UPPER_LIMIT_POSITION;
    if(CoreXYData->Z < ZAXIS_LOWER_LIMIT_POSITION)
        CoreXYData->Z = ZAXIS_LOWER_LIMIT_POSITION;
    CoreXYData->motorZ = CoreXYData->Z / BELT_PITCH / BELT_PULLEY_TEETH * 360.0f + ZAXIS_LOWER_LIMIT_ANGLE;
    CoreXYData->motorRotate = CoreXYData->E * 65536 / 360.0f;
}



void setTargetPosition(float x, float y, float z, float e)
{
    targetPosition.X = x;
    targetPosition.Y = y;
    targetPosition.Z = z;
    targetPosition.E = e;
    inverseKinematics(&targetPosition);
    MotorManager::motorLeft.setTargetPosition(targetPosition.motorLeft);
    MotorManager::motorRight.setTargetPosition(targetPosition.motorRight);
    MotorManager::motorRotate.setTargetPosition(-targetPosition.motorRotate);
    Servo::SetPositionForced(targetPosition.motorZ);
}

void setXYVelocityLimit(float velocityLimit)
{
    // 不需要进行 CoreXY 转换, 直接设置到电机上
    MotorManager::motorLeft.setVelocityLimit(velocityLimit / BELT_PITCH / BELT_PULLEY_TEETH * 65536);
    MotorManager::motorRight.setVelocityLimit(velocityLimit / BELT_PITCH / BELT_PULLEY_TEETH * 65536);
}

void updateFeedbackPosition()
{
    feedbackPosition.motorLeft = MotorManager::motorLeft.getAccumulatedEncoder();
    feedbackPosition.motorRight = MotorManager::motorRight.getAccumulatedEncoder();
    feedbackPosition.motorRotate = MotorManager::motorRotate.getAccumulatedEncoder();
    feedbackPosition.motorZ = Servo::getEstimatePosition();
    forwardKinematics(&feedbackPosition);
}

void updateFeedbackSpeed()
{
    feedbackSpeed.motorLeft = MotorManager::motorLeft.getshaftAngularVelocity();
    feedbackSpeed.motorRight = MotorManager::motorRight.getshaftAngularVelocity();
    feedbackSpeed.motorRotate = MotorManager::motorRotate.getshaftAngularVelocity();
    forwardKinematics(&feedbackSpeed);
}

void transmitCpltInfo()
{
    if(!controlStatus.needTransmitMovementCpltInfo)
        return;
    if ((MotorManager::motorLeft.getFeedbackStatus()->operationProgress < 0.92f) || (MotorManager::motorRight.getFeedbackStatus()->operationProgress < 0.92f) || (!Servo::Available()))
        return;
        
    controlStatus.needTransmitMovementCpltInfo = 0;
    
    // 这里没有使用 Feedback 的 buffer 是因为 Feedback 的 buffer 可能会在中断里被调用修改, 不安全
    uint8_t feedbackBuffer[128];
    sprintf((char *)feedbackBuffer, "ok Move X:%5d Y:%5d Z:%5d E:%5d\n", int(feedbackPosition.X * 100), int(feedbackPosition.Y * 100), int(feedbackPosition.Z * 100), int(feedbackPosition.E * 100));
    GCodeDecoder::sendFeedback((char *)feedbackBuffer);
}

void updateFeedbacks()
{
    updateFeedbackPosition();
    updateFeedbackSpeed();
    transmitCpltInfo();
}

volatile float approximatedXorigin[3];
volatile float approximatedYorigin[3];
void findOrigin()
{
    /*
    找零点的思路是先撞 X 轴的零点, 然后 X 轴快速回退 5mm, 然后慢速多次撞零点, 记录位置
    然后撞 Y 轴的零点, 然后 Y 轴快速回退 5mm, 然后慢速多次撞零点, 记录位置

    Timeout 的处理办法是在每次移动之前记录当前时间, 然后在 while 里面检查是否超时, 超时则返回运动之前的位置并返回错误
    */
    const float MAX_MOVEMENT_X = 130.0f;
    const float MAX_MOVEMENT_Y = 160.0f;

    // 这个的作用是临时保存新的值, 如果校验通过, 则将其**累加**给 originOffsetX, 因为校准过程中已经有了偏移
    float originOffsetX_temp = 0.0f;
    float originOffsetY_temp = 0.0f;

    float lastMovementLastPositionX = 0.0f;
    float lastMovementLastPositionY = 0.0f;


    MotorManager::stopAllMotors();
    vTaskDelay(10);

    // Step -1: Z 轴抬升
    setTargetPosition(feedbackPosition.X, feedbackPosition.Y, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
    vTaskDelay(10);
    // 阻塞等待到达目标位置
    while(Servo::Available() == false)
    {
        vTaskDelay(1);
    }

    // Y 轴回抽一个电机的距离
    setXYVelocityLimit(CoreXYDefaultSpeed / 2);
    lastMovementLastPositionX = feedbackPosition.X;
    lastMovementLastPositionY = feedbackPosition.Y;
    setTargetPosition(feedbackPosition.X, feedbackPosition.Y - 40, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
    vTaskDelay(10);
    // 阻塞等待到达目标位置
    while((MotorManager::motorLeft.getFeedbackStatus()->operationProgress < 0.92f) || (MotorManager::motorRight.getFeedbackStatus()->operationProgress < 0.92f))
    {
        if(ZeroTrigger::ZeroTriggerStatus.YTriggered == 1)
        {
            // 如果 Y 轴已经触发, 则立即回到原位
            setTargetPosition(lastMovementLastPositionX, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
            vTaskDelay(10);
            while((MotorManager::motorLeft.getFeedbackStatus()->operationProgress < 0.92f) || (MotorManager::motorRight.getFeedbackStatus()->operationProgress < 0.92f))
            {
                vTaskDelay(1);
            }
        }
        vTaskDelay(1);
    }

    // Step 1: X 轴移动到尽可能左边
    lastMovementLastPositionX = feedbackPosition.X;
    lastMovementLastPositionY = feedbackPosition.Y;
    setTargetPosition(lastMovementLastPositionX - MAX_MOVEMENT_X, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);

    // 等待 X 轴触发
    TickType_t startTime = xTaskGetTickCount();
    while(ZeroTrigger::ZeroTriggerStatus.XTriggered == 0)
    {
        if(xTaskGetTickCount() - startTime > 4000)
        {
            setTargetPosition(lastMovementLastPositionX, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
            GCodeDecoder::sendFeedback("er Origin X Axis Timeout 1\n");
            RGBeffect::errorLightEffect();
            return;
        }
        vTaskDelay(1);
    }
    
    approximatedXorigin[0] = feedbackPosition.X;
    
    // Step 2: X 轴快速往回移动 5mm
    lastMovementLastPositionX = feedbackPosition.X;
    lastMovementLastPositionY = feedbackPosition.Y;
    setTargetPosition(lastMovementLastPositionX + 5, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
    startTime = xTaskGetTickCount();
    vTaskDelay(10);
    // 阻塞等待到达目标位置
    while((MotorManager::motorLeft.getFeedbackStatus()->operationProgress < 0.92f) || (MotorManager::motorRight.getFeedbackStatus()->operationProgress < 0.92f))
    {
        if(xTaskGetTickCount() - startTime > 4000)
        {
            setTargetPosition(lastMovementLastPositionX, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
            GCodeDecoder::sendFeedback("er Origin X Axis Timeout 2\n");
            RGBeffect::errorLightEffect();
            return;
        }
        vTaskDelay(1);
    }


    for(int i = 0; i < 2; i++)
    {
        // Step 3: X 轴慢速多次撞零点
        setXYVelocityLimit(CoreXYFindOriginSpeed);
        lastMovementLastPositionX = feedbackPosition.X;
        lastMovementLastPositionY = feedbackPosition.Y;
        setTargetPosition(approximatedXorigin[0] - 5, feedbackPosition.Y, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
        // 等待 X 轴触发
        startTime = xTaskGetTickCount();
        while(ZeroTrigger::ZeroTriggerStatus.XTriggered == 0)
        {
            if(xTaskGetTickCount() - startTime > 3500)
            {
                setTargetPosition(lastMovementLastPositionX, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
                GCodeDecoder::sendFeedback("er Origin X Axis Timeout 3\n");
                RGBeffect::errorLightEffect();
                return;
            }
            vTaskDelay(1);
        }
        // 记录位置
        approximatedXorigin[i + 1] = feedbackPosition.X;

        setXYVelocityLimit(CoreXYDefaultSpeed / 2);
        lastMovementLastPositionX = feedbackPosition.X;
        lastMovementLastPositionY = feedbackPosition.Y;
        setTargetPosition(approximatedXorigin[0] + 5, feedbackPosition.Y, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
        startTime = xTaskGetTickCount();
        vTaskDelay(10);
        // 阻塞等待到达目标位置
        while((MotorManager::motorLeft.getFeedbackStatus()->operationProgress < 0.92f) || (MotorManager::motorRight.getFeedbackStatus()->operationProgress < 0.92f))
        {
            if(xTaskGetTickCount() - startTime > 3500)
            {
                setTargetPosition(lastMovementLastPositionX, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
                GCodeDecoder::sendFeedback("er Origin X Axis Timeout 4\n");
                RGBeffect::errorLightEffect();
                return;
            }
            vTaskDelay(1);
        }
    }

    // 方差校验: 略过
    originOffsetX_temp = (approximatedXorigin[0] + approximatedXorigin[1] + approximatedXorigin[2]) / 3;


    // 校准 Y 轴
    setXYVelocityLimit(CoreXYDefaultSpeed / 2);
    lastMovementLastPositionX = feedbackPosition.X;
    lastMovementLastPositionY = feedbackPosition.Y;
    setTargetPosition(originOffsetX_temp + 5, feedbackPosition.Y - MAX_MOVEMENT_Y, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);

    // 等待 Y 轴触发
    startTime = xTaskGetTickCount();
    while(ZeroTrigger::ZeroTriggerStatus.YTriggered == 0)
    {
        if(xTaskGetTickCount() - startTime > 3500)
        {
            setTargetPosition(lastMovementLastPositionX, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
            GCodeDecoder::sendFeedback("er Origin Y Axis Timeout 1\n");
            RGBeffect::errorLightEffect();
            return;
        }
        vTaskDelay(1);
    }

    approximatedYorigin[0] = feedbackPosition.Y;
    
    lastMovementLastPositionX = feedbackPosition.X;
    lastMovementLastPositionY = feedbackPosition.Y;
    setTargetPosition(originOffsetX_temp + 5, approximatedYorigin[0] + 5, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
    startTime = xTaskGetTickCount();
    vTaskDelay(10);
    // 阻塞等待到达目标位置
    while((MotorManager::motorLeft.getFeedbackStatus()->operationProgress < 0.92f) || (MotorManager::motorRight.getFeedbackStatus()->operationProgress < 0.92f))
    {
        if(xTaskGetTickCount() - startTime > 3500)
        {
            setTargetPosition(lastMovementLastPositionX, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
            GCodeDecoder::sendFeedback("er Origin Y Axis Timeout 2\n");
            RGBeffect::errorLightEffect();
            return;
        }
        vTaskDelay(1);
    }


    for(int i = 0; i < 2; i++)
    {
        // 慢速多次撞零点
        setXYVelocityLimit(CoreXYFindOriginSpeed);
        lastMovementLastPositionX = feedbackPosition.X;
        lastMovementLastPositionY = feedbackPosition.Y;
        setTargetPosition(originOffsetX_temp + 5, approximatedYorigin[0] - 5, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
        // 等待 Y 轴触发
        startTime = xTaskGetTickCount();
        while(ZeroTrigger::ZeroTriggerStatus.YTriggered == 0)
        {
            if(xTaskGetTickCount() - startTime > 3500)
            {
                setTargetPosition(lastMovementLastPositionX, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
                GCodeDecoder::sendFeedback("er Origin Y Axis Timeout 3\n");
                RGBeffect::errorLightEffect();
                return;
            }
            vTaskDelay(1);
        }
        // 记录位置
        approximatedYorigin[i + 1] = feedbackPosition.Y;

        setXYVelocityLimit(CoreXYDefaultSpeed / 2);
        lastMovementLastPositionX = feedbackPosition.X;
        lastMovementLastPositionY = feedbackPosition.Y;
        setTargetPosition(originOffsetX_temp + 5, approximatedYorigin[0] + 5, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
        vTaskDelay(10);
        // 阻塞等待到达目标位置
        while((MotorManager::motorLeft.getFeedbackStatus()->operationProgress < 0.92f) || (MotorManager::motorRight.getFeedbackStatus()->operationProgress < 0.92f))
        {
            if(xTaskGetTickCount() - startTime > 3500)
            {
                setTargetPosition(lastMovementLastPositionX, lastMovementLastPositionY, ZAXIS_UPPER_LIMIT_POSITION, feedbackPosition.E);
                GCodeDecoder::sendFeedback("er Origin Y Axis Timeout 4\n");
                RGBeffect::errorLightEffect();
                return;
            }
            vTaskDelay(1);
        }
    }

    // 方差校验: 略过
    originOffsetY_temp = (approximatedYorigin[0] + approximatedYorigin[1] + approximatedYorigin[2]) / 3;


    // 其他合法性检查, 跳过
    originOffsetX += originOffsetX_temp;
    originOffsetY += originOffsetY_temp;


    // 离开
    setXYVelocityLimit(CoreXYDefaultSpeed);
    setTargetPosition(5, 5, ZAXIS_UPPER_LIMIT_POSITION, 0);


    GCodeDecoder::sendFeedback("ok Origin\n");
    RGBeffect::idleLightEffect();
}

void triggerFindOrigin()
{
    controlStatus.findingOrigin = 1;
}

StackType_t uxCoreXYTaskStack[256];
StaticTask_t xCoreXYTaskTCB;
void CoreXYTask(void *pvPara)
{
    setTargetPosition(0, 0, ZAXIS_UPPER_LIMIT_POSITION, 0);
    setXYVelocityLimit(CoreXYDefaultSpeed);
    MotorManager::motorRotate.setVelocityLimit(20000);

    while (MotorManager::motorLeft.getConnectionStatus() == 0 || MotorManager::motorRight.getConnectionStatus() == 0 || MotorManager::motorRotate.getConnectionStatus() == 0)
    {
        vTaskDelay(10);
    }
    vTaskDelay(500);
    // 重置 accumulatedEncoder, 把 CoreXY 的原点与电机的原点对齐
    MotorManager::motorLeft.setZeroPosition();
    MotorManager::motorRight.setZeroPosition();
    MotorManager::motorRotate.setZeroPosition();
    RGBeffect::idleLightEffect();

    while(1)
    {
        if(controlStatus.findingOrigin)
        {
            findOrigin();
            controlStatus.findingOrigin = 0;
        }
        vTaskDelay(100);
    }
}


void Init()
{
    controlStatus.findingOrigin = 0;
    controlStatus.needTransmitMovementCpltInfo = 0;
    MotorManager::RegisterCallback(updateFeedbacks);
    Servo::Init(ZAXIS_UPPER_LIMIT_ANGLE);

    xTaskCreateStatic(CoreXYTask, "CoreXYTask", 256, NULL, 0, uxCoreXYTaskStack, &xCoreXYTaskTCB);
}
}