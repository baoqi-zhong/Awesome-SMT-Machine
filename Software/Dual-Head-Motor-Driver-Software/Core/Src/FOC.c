/**
 * @file FOC.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "FOC.h"

#include "tim.h"
#include "CordicHelper.h"

#include "stdint.h"

#include "LPF.h"
#include "ErrorHandler.h"


// 控制信号
ControlStatus_t controlStatus;
uint32_t control1KHzCounter = MOTOR_CONTROL_1KHZ_PERIOD / 3;

void controlInit()
{
    controlStatus.ignoreAllErrors = 0;
    controlStatus.enableAutoReset = 1;
    controlStatus.triggerReset = 0;

    // Valve Control
    controlStatus.valveState[0] = 0;
    controlStatus.valveState[1] = 0;

    // Micro Switch/GPIO Feedback Status
    controlStatus.microSwitchStatus = 0;
    for(int index = 0; index < 3; index++)
    {
        controlStatus.GPIOStatus[index] = 0;
    }
    
    // Motor Control
    for(int index = 0; index < 3; index++)
    {
        controlStatus.currentPosition[index] = 0;
        controlStatus.currentVelocity[index] = 0;
        controlStatus.movement[index].startPostion = 0;
        controlStatus.movement[index].displacement = 0;
        controlStatus.movement[index].accelerationDistance = 0;
        controlStatus.movement[index].uniformVelocityDistance = 0;
    }

    controlStatus.tourqueLimit[0] = DEFAULT_TOURQUE_LIMIT_M1;
    controlStatus.velocityLimit[0] = DEFAULT_VELOCITY_LIMIT_M1;
    controlStatus.accelerationLimit[0] = DEFAULT_ACC_LIMIT_M1;

    controlStatus.tourqueLimit[1] = DEFAULT_TOURQUE_LIMIT_M2;
    controlStatus.velocityLimit[1] = DEFAULT_VELOCITY_LIMIT_M2;
    controlStatus.accelerationLimit[1] = DEFAULT_ACC_LIMIT_M2;

    controlStatus.tourqueLimit[2] = DEFAULT_TOURQUE_LIMIT_M2;
    controlStatus.velocityLimit[2] = DEFAULT_VELOCITY_LIMIT_M2;
    controlStatus.accelerationLimit[2] = DEFAULT_ACC_LIMIT_M2;

    controlStatus.targetZHeight[0] = 0;
    controlStatus.targetZHeight[1] = 0;
    controlStatus.targetZHeightSet = 0;

    controlStatus.FOCControlMode = OPEN_LOOP_POSITION_CONTROL;
    FOC_init();
}

void moveMotor(uint8_t index, float displacement)
{
    if(index >= 3 || displacement == 0 || controlStatus.movement[index].displacement)
    {
        return;
    }

    controlStatus.movement[index].startPostion = controlStatus.currentPosition[index];
    controlStatus.movement[index].displacement = displacement;

    float absDisplacement = FABS(displacement);
    if(controlStatus.velocityLimit[index] * controlStatus.velocityLimit[index] / (2 * controlStatus.accelerationLimit[index]) > absDisplacement)
    {
        controlStatus.movement[index].accelerationDistance = absDisplacement / 2;
        controlStatus.movement[index].uniformVelocityDistance = 0;
    }
    else
    {
        controlStatus.movement[index].accelerationDistance = controlStatus.velocityLimit[index] * controlStatus.velocityLimit[index] / (2 * controlStatus.accelerationLimit[index]);
        controlStatus.movement[index].uniformVelocityDistance = absDisplacement - 2 * controlStatus.movement[index].accelerationDistance;
    }
}

float arcsinf(float x)
{
    // Poly: x * (1 + a2*x^2 + a4*x^4 + a6*x^6) / (1 + b2*x^2 + b4*x^4 + b6*x^6)
    // a2 = -1.628248f; a4 = 0.698611666f; a6 = -0.0529363263f;
    // b2 = -2.819450f; b4 = 1.449471946f; b6 = -0.1833973450f;
    register float xPow2 = x * x;
    register float xPow4 = xPow2 * xPow2;
    register float xPow6 = xPow4 * xPow2;

    // 扩大 10 倍, 避免除法误差
    float numerator = (10 - 16.28248f * xPow2 + 06.98611666f * xPow4 - 00.529363263f * xPow6) * x;
    float denominator = 10 - 28.19450f * xPow2 + 14.49471946f * xPow4 - 01.833973450f * xPow6;
    return numerator / denominator;
}

void armInverseKinematics()
{
    float minZPosition = MAX(controlStatus.targetZHeight[0], controlStatus.targetZHeight[1]);
    // float ==, dangerous
    if(FABS(minZPosition - controlStatus.targetZHeightSet) < 1e-6)
    {
        return;
    }
    controlStatus.targetZHeightSet = minZPosition;

    float theta = arcsinf(minZPosition / ARM_LENGTH);

    if(controlStatus.targetZHeight[0] > controlStatus.targetZHeight[1])
        moveMotor(0, theta - controlStatus.currentPosition[0]);
    else
        moveMotor(0, controlStatus.currentPosition[0] - theta);
}

void armForwardKinematics()
{
    float outputSin, outputCos;
    hcordic.Instance->WDATA = (singleFloatToCordic15(0.999999f) << 16) | singleFloatToCordic15(controlStatus.currentPosition[0]);
    cordic15ToDualFloat((int32_t)(hcordic.Instance->RDATA), &outputSin, &outputCos);

    if(outputSin > 0)
    {
        controlStatus.targetZHeight[0] = ARM_LENGTH * outputSin;
        controlStatus.targetZHeight[1] = 0;
    }
    else
    {
        controlStatus.targetZHeight[0] = 0;
        controlStatus.targetZHeight[1] = ARM_LENGTH * outputSin;
    }
}

void setZPosition(uint8_t index, float position)
{
    if(index != 2 && index != 3)
    {
        return;
    }

    controlStatus.currentPosition[index - 1] = position;
}

void control1KHz()
{
    if(!controlStatus.enableFOCOutput)
    {
        return;
    }

    HAL_GPIO_WritePin(VALVE_1_GPIO_Port, VALVE_1_Pin, controlStatus.valveState[0]);
    HAL_GPIO_WritePin(VALVE_2_GPIO_Port, VALVE_2_Pin, controlStatus.valveState[1]);
    
    controlStatus.microSwitchStatus = 1 - HAL_GPIO_ReadPin(MICRO_SW_GPIO_Port, MICRO_SW_Pin);
    controlStatus.GPIOStatus[0] = 1 - HAL_GPIO_ReadPin(GPIO1_GPIO_Port, GPIO1_Pin);
    controlStatus.GPIOStatus[1] = 1 - HAL_GPIO_ReadPin(GPIO2_GPIO_Port, GPIO2_Pin);
    controlStatus.GPIOStatus[2] = 1 - HAL_GPIO_ReadPin(GPIO1_GPIO_Port, GPIO3_Pin);
}

// FOC
/*
硬件连接:
htim1 CN1: A+
htim1 CN2: A-
htim1 CN3: B+
htim1 CN4: B-

规定以 A 为 alpha 轴, B 为 beta 轴
*/
volatile uint32_t* M1_CCRS[4]; 
volatile uint32_t* M2_CCRS[4];
volatile uint32_t* M3_CCRS[4];

// IOC 里所有通道进行了反向
void setPhraseVoltage(float phraseA, float phraseB, volatile uint32_t* CCRs[4])
{
    if(phraseA > 0)
    {
        *CCRs[0] = 0;
        *CCRs[1] = phraseA * CCR;
    }
    else
    {
        *CCRs[0] = - phraseA * CCR;
        *CCRs[1] = 0;
    }
    if(phraseB > 0)
    {
        *CCRs[2] = 0;
        *CCRs[3] = phraseB * CCR;
    }
    else
    {
        *CCRs[2] = - phraseB * CCR;
        *CCRs[3] = 0;
    }
}

uint32_t adcBuffer[2] = {0};
float Vbus = 18.0f;

float measuredIMotor[3] = {0};

float outputUalpha[3];
float outputUbeta[3];
float outputUd[3];

int16_t outputAngle[3];

void FOC_init()
{
    cordic16_init();
    M1_CCRS[0] = &htim3.Instance->CCR1;
    M1_CCRS[1] = &htim1.Instance->CCR1;
    M1_CCRS[2] = &htim1.Instance->CCR2;
    M1_CCRS[3] = &htim1.Instance->CCR3;

    M2_CCRS[0] = &htim2.Instance->CCR1;
    M2_CCRS[1] = &htim8.Instance->CCR1;
    M2_CCRS[2] = &htim8.Instance->CCR2;
    M2_CCRS[3] = &htim2.Instance->CCR2;

    M3_CCRS[0] = &htim4.Instance->CCR1;
    M3_CCRS[1] = &htim4.Instance->CCR2;
    M3_CCRS[2] = &htim4.Instance->CCR4;
    M3_CCRS[3] = &htim4.Instance->CCR3;
}

void disableFOC()
{
    controlStatus.enableFOCOutput = 0;

    // 对地短路, 使电机停转
    setPhraseVoltage(0, 0, M1_CCRS);
    setPhraseVoltage(0, 0, M2_CCRS);
    setPhraseVoltage(0, 0, M3_CCRS);
}

void resetAndEnableFOC()
{
    // 可能会在中断里执行, 不能用 HAL_Delay
    // HAL_Delay(10);

    controlStatus.enableFOCOutput = 1;
}

// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// {
//     if(htim == &htim1)
//     {
//     }
// }

// 必须在下一次 ADC 采样触发前跑完电流环
// 暂时不考虑 ADC 的 latency 问题
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    control1KHzCounter++;
    if(control1KHzCounter >= MOTOR_CONTROL_1KHZ_PERIOD)
    {
        control1KHzCounter = 0;
        control1KHz();
    }

    // ADC1: M1, M3
    // ADC2: M2, Vbus
    Vbus =              ((float)(adcBuffer[1] >> 16)                 * VBUS_GAIN) * 0.1f + Vbus * 0.9f;
    measuredIMotor[0] = (((float)(adcBuffer[0] & 0xFFFF) - IM1_BIAS) * IM1_GAIN) * 0.5f + measuredIMotor[0] * 0.5f;
    measuredIMotor[1] = (((float)(adcBuffer[0] >> 16)    - IM2_BIAS) * IM2_GAIN) * 0.5f + measuredIMotor[1] * 0.5f;
    measuredIMotor[2] = (((float)(adcBuffer[1] & 0xFFFF) - IM3_BIAS) * IM3_GAIN) * 0.5f + measuredIMotor[2] * 0.5f;

    ErrorHandler();
    
    if(controlStatus.enableFOCOutput == 0)
    {
        // 有待斟酌 到底是 set 个 0 的电压还是切换到高阻态.
        // setPhraseVoltage(0, 0);
        return;
    }
    
    if(controlStatus.FOCControlMode == OPEN_LOOP_ROTATE)
    {
        controlStatus.currentVelocity[0] = OPEN_LOOP_ROTATE_SPEED_M1;
        controlStatus.currentVelocity[1] = OPEN_LOOP_ROTATE_SPEED_M2;
        controlStatus.currentVelocity[2] = OPEN_LOOP_ROTATE_SPEED_M2;
    }
    
    else if(controlStatus.FOCControlMode == OPEN_LOOP_POSITION_CONTROL)
    {
        for(int index = 0; index < 3; index++)
        {
            if(controlStatus.movement[index].displacement > 0)
            {
                float currentDisplacement = controlStatus.currentPosition[index] - controlStatus.movement[index].startPostion;

                if(currentDisplacement < controlStatus.movement[index].accelerationDistance)
                {
                    controlStatus.currentVelocity[index] += controlStatus.accelerationLimit[index] / CURRENT_LOOP_FREQ;
                }
                else if(currentDisplacement > controlStatus.movement[index].accelerationDistance + controlStatus.movement[index].accelerationDistance)
                {
                    if(controlStatus.currentVelocity[index] > 0)
                        controlStatus.currentVelocity[index] -= controlStatus.accelerationLimit[index] / CURRENT_LOOP_FREQ;
                    else
                        controlStatus.movement[index].displacement = 0;
                }
            }
            else if(controlStatus.movement[index].displacement < 0)
            {
                float currentDisplacement = controlStatus.movement[index].startPostion - controlStatus.currentPosition[index];

                if(currentDisplacement < controlStatus.movement[index].accelerationDistance)
                {
                    controlStatus.currentVelocity[index] -= controlStatus.accelerationLimit[index] / CURRENT_LOOP_FREQ;
                }
                else if(currentDisplacement > controlStatus.movement[index].accelerationDistance + controlStatus.movement[index].accelerationDistance)
                {
                    if(controlStatus.currentVelocity[index] < 0)
                        controlStatus.currentVelocity[index] += controlStatus.accelerationLimit[index] / CURRENT_LOOP_FREQ;
                    else
                        controlStatus.movement[index].displacement = 0;
                }
            }
            else
            {
                controlStatus.currentVelocity[index] = 0;
            }
        }
    }

    outputUd[0] = POSITIOIN_HOLD_VOLTAGE_M1 + KV_M1 * FABS(controlStatus.currentVelocity[0]);
    outputUd[1] = POSITIOIN_HOLD_VOLTAGE_M2 + KV_M2 * FABS(controlStatus.currentVelocity[1]);
    outputUd[2] = POSITIOIN_HOLD_VOLTAGE_M2 + KV_M2 * FABS(controlStatus.currentVelocity[2]);

    outputUd[0] = CLAMP(outputUd[0], -Vbus, Vbus);
    outputUd[1] = CLAMP(outputUd[1], -Vbus, Vbus);
    outputUd[2] = CLAMP(outputUd[2], -Vbus, Vbus);
    for (int index = 0; index < 3; index++)
    {
        controlStatus.currentPosition[index] += (float)(controlStatus.currentVelocity[index]) / CURRENT_LOOP_FREQ;
        outputAngle[index] = (int16_t)(controlStatus.currentPosition[index] * POLE_PAIRS / TWO_PI * 65536);

        hcordic.Instance->WDATA = (singleFloatToCordic15(outputUd[index] / Vbus) << 16) | (outputAngle[index] & 0xFFFF);
        cordic15ToDualFloat((int32_t)(hcordic.Instance->RDATA), &outputUbeta[index], &outputUalpha[index]);

    } 
    setPhraseVoltage(outputUalpha[0], outputUbeta[0], M1_CCRS);
    setPhraseVoltage(outputUalpha[1], outputUbeta[1], M2_CCRS);
    setPhraseVoltage(outputUalpha[2], outputUbeta[2], M3_CCRS);
}