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


#include "MA732.h"
#include "IncrementalPID.h"
#include "PositionalPID.h"
#include "LPF.h"
#include "MusicManager.h"
#include "ErrorHandler.h"

// 控制信号
MotorControlStatus_t motorControlStatus;

PositionalPID_t positionToCurrentPID;
PositionalPID_t positionToVelocityPID;
PositionalPID_t velocityPID;

// 为了错开 4KHz 和 1KHz 的相位
uint32_t MotorControl4KHzCounter = MOTOR_CONTROL_4KHZ_PERIOD / 3;
uint32_t MotorControl1KHzCounter = MOTOR_CONTROL_1KHZ_PERIOD * 2 / 3;

void MotorControlInit()
{
    // 估计值
    PositionalPIDsetParameters(&positionToCurrentPID, 
                        3.4e-4f,    // kPOnError
                        1.4e-4f,    // kIonError
                        1e-5f,      // kDonMeasurement
                        0.0f,       // kPonMeasurement
                        5e-6f,      // kDonTarget
                        0.1f,       // alpha
                        DEFAULT_TOURQUE_LIMIT,     // outputLimit
                        1000.0f     // updateFrequency
                        );

    // 2024.1.21 按照新协议:速度位置都是 Encoder 值调好
    PositionalPIDsetParameters(&positionToVelocityPID, 
                        100.0f,       // kPOnError
                        10.0f,       // kIonError
                        0.5f,      // kDonMeasurement
                        0.0f,       // kPonMeasurement
                        0.0f,       // kDonTarget
                        0.2f,       // alpha
                        DEFAULT_VELOCITY_LIMIT,     // outputLimit
                        1000.0f     // updateFrequency
                        );

    // 2024.1.21 按照新协议:速度位置都是 Encoder 值调好
    PositionalPIDsetParameters(&velocityPID, 
                        6e-6f,      // kPOnError
                        9e-5f,      // kIonError
                        1e-5f,       // kDonMeasurement
                        0.0f,       // kPonMeasurement
                        0.0f,       // kDonTarget
                        0.0f,       // alpha
                        DEFAULT_TOURQUE_LIMIT,     // outputLimit
                        4000.0f     // updateFrequency
                        );

    motorControlStatus.targetPosition = 0;
    motorControlStatus.targetVelocity = 0;
    motorControlStatus.tourqueLimit = DEFAULT_TOURQUE_LIMIT;
    motorControlStatus.velocityLimit = DEFAULT_VELOCITY_LIMIT;
#if ((CAN_ID == 1 || CAN_ID == 2) && CALIBRATE_ENCODER == 0)
    motorControlStatus.enablePositionCloseLoop = 1;
    motorControlStatus.enableSpeedCloseLoop = 1;
#else
    motorControlStatus.enablePositionCloseLoop = 0;
    motorControlStatus.enableSpeedCloseLoop = 0;
#endif

    motorControlStatus.ignoreAllErrors = 0;
    motorControlStatus.enableAutoReset = 1;
    motorControlStatus.triggerReset = 0;
    motorControlStatus.RGBControlByMaster = 0;
    FOC_init();

    MusicManagerInit();
}


void MotorControl4KHz()
{
    if(!motorControlStatus.enableFOCOutput)
    {
        return;
    }

    if(motorControlStatus.enableSpeedCloseLoop)
    {
        velocityPID.outputLimit = FABS(motorControlStatus.tourqueLimit);
        motorControlStatus.targetIq = PositionalPIDupdate(&velocityPID, (float)(motorControlStatus.targetVelocity), shaftAngularVelocity);
    }
    else
    {
        if(motorControlStatus.enablePositionCloseLoop == 0)
            motorControlStatus.targetIq = motorControlStatus.tourqueLimit;
    }
}

extern uint8_t signal1KHz;
void MotorControl1KHz()
{
    signal1KHz = 1;
    if(!motorControlStatus.enableFOCOutput)
    {
        return;
    }

    if(motorControlStatus.enablePositionCloseLoop)
    {
        if(motorControlStatus.enableSpeedCloseLoop)
        {
            positionToVelocityPID.outputLimit = FABS(motorControlStatus.velocityLimit);
            motorControlStatus.targetVelocity = PositionalPIDupdate(&positionToVelocityPID, (float)(motorControlStatus.targetPosition), (float)(accumulatedEncoder));
        }
        else
        {
            positionToCurrentPID.outputLimit = FABS(motorControlStatus.tourqueLimit);
            motorControlStatus.targetIq = PositionalPIDupdate(&positionToCurrentPID, (float)(motorControlStatus.targetPosition), (float)(accumulatedEncoder));
        }
    }
    else
    {
        motorControlStatus.targetVelocity = motorControlStatus.velocityLimit;
    }
}

/*
硬件连接:
htim1 CN1: A+
htim1 CN2: A-
htim1 CN3: B+
htim1 CN4: B-

规定以 A 为 alpha 轴, B 为 beta 轴
*/

#if (AB_PHASE_SWAP == 0)
// A+ A- B+ B- (正常版本)
void setPhraseVoltage(float phraseA, float phraseB)
{
    if(phraseA > 0)
    {
        htim1.Instance->CCR1 = phraseA * CCR;
        htim1.Instance->CCR2 = 0;
    }
    else
    {
        htim1.Instance->CCR1 = 0;
        htim1.Instance->CCR2 = - phraseA * CCR;
    }
    if(phraseB > 0)
    {
        htim1.Instance->CCR3 = phraseB * CCR;
        htim1.Instance->CCR4 = 0;
    }
    else
    {
        htim1.Instance->CCR3 = 0;
        htim1.Instance->CCR4 = - phraseB * CCR;
    }
}
#else
// A+ B+ A- B- (互换版本)
void setPhraseVoltage(float phraseA, float phraseB)
{
    if(phraseA > 0)
    {
        htim1.Instance->CCR1 = phraseA * CCR;
        htim1.Instance->CCR3 = 0;
    }
    else
    {
        htim1.Instance->CCR1 = 0;
        htim1.Instance->CCR3 = - phraseA * CCR;
    }
    if(phraseB > 0)
    {
        htim1.Instance->CCR2 = phraseB * CCR;
        htim1.Instance->CCR4 = 0;
    }
    else
    {
        htim1.Instance->CCR2 = 0;
        htim1.Instance->CCR4 = - phraseB * CCR;
    }
}
#endif


void alineMotor()
{
    for(int i = 1; i <= 100; i++)
    {
        setPhraseVoltage(ALINE_VOLTAGE / 24.0f / 100 * i, 0.0f);
        HAL_Delay(2);
    }

    HAL_Delay(300);
    MA732_setZeroSoftware();
    HAL_Delay(10);
    for(int i = 9; i >= 0; i--)
    {
        setPhraseVoltage(ALINE_VOLTAGE / 24.0f / 10 * i, 0.0f);
        HAL_Delay(1);
    }
    setPhraseVoltage(0.0f, 0.0f);
    HAL_Delay(50);
}


// 开环电角度(不考虑 Pole Pairs)
int16_t openLoopRotateSpeed = OPEN_LOOP_ROTATE_SPEED;
int32_t openLoopTheta = 0;
float openLoopThetaFloat = 0; // 这个的单位和其他 float 的不同,这个是定点数的浮点表示
float manaticEncoderError = 0;

// 电流环
uint32_t adcBuffer[2] = {0};
float Vbus = 24.0f, NTCTemperature = 0;
uint16_t adcVbusBuffer = 0;
uint16_t adcNTCBuffer = 0;

float measuredIalpha = 0.0f;
float measuredIbeta = 0.0f;
float measuredIq = 0.0f;
float measuredId = 0.0f;
float outputUq = 0.0f;
float outputUd = 0.0f;
float outputUqWithFeedForward = 0;
float outputUdWithFeedForward = 0;
float outputUalpha = 0;
float outputUbeta = 0;
int16_t outputAngle = 0;

float backwardEMF = 0;
float outputUqFeedForward = 0.0f;
float outputUdFeedForward = 0.0f;

IncrementalPID_t IqPID, IdPID;


void FOC_init()
{
    IncrementalPIDsetParameters(&IqPID,
                                80000.0f,    // kPOnTarget
                                80000.0f,    // kPOnMeasurement
                                1000.0f,      // kI
                                0.0f,       // kD
                                24.0f,     // outputLimit
                                CURRENT_LOOP_FREQ    // updateFrequency
                                );

    IncrementalPIDsetParameters(&IdPID,
                                80000.0f,    // kPOnTarget
                                80000.0f,    // kPOnMeasurement
                                1000.0f,      // kI
                                0.0f,       // kD
                                24.0f,     // outputLimit
                                CURRENT_LOOP_FREQ    // updateFrequency
                                );

    cordic16_init();

    #if (CAN_ID == 1 || CAN_ID == 2)
    motorControlStatus.FOCControlMode = CURRENT_TOURQUE_CONTROL;
    #else
    motorControlStatus.FOCControlMode = OPEN_LOOP_POSITION_CONTROL;
    #endif
    motorControlStatus.targetIq = 0.0f;
    motorControlStatus.targetId = 0.0f;
}

void disableFOC()
{
    motorControlStatus.enableFOCOutput = 0;

    setPhraseVoltage(0, 0);
    HAL_GPIO_WritePin(NRST_AB_GPIO_Port, NRST_AB_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NRST_CD_GPIO_Port, NRST_CD_Pin, GPIO_PIN_RESET);  
}

void resetAndEnableFOC()
{
    HAL_GPIO_WritePin(NRST_AB_GPIO_Port, NRST_AB_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NRST_CD_GPIO_Port, NRST_CD_Pin, GPIO_PIN_RESET);

    motorControlStatus.targetIq = 0;
    motorControlStatus.targetId = 0;

    PositionalPIDreset(&positionToCurrentPID);
    PositionalPIDreset(&positionToVelocityPID);
    PositionalPIDreset(&velocityPID);
    IncrementalPIDreset(&IqPID);
    IncrementalPIDreset(&IdPID);
    

    // 可能会在中断里执行, 不能用 HAL_Delay
    // HAL_Delay(10);
    HAL_GPIO_WritePin(NRST_AB_GPIO_Port, NRST_AB_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NRST_CD_GPIO_Port, NRST_CD_Pin, GPIO_PIN_SET);

    motorControlStatus.enableFOCOutput = 1;
}


// 基准电流, 在计算 Park 变换的时候调用 cordic 前的缩放值. 最大电流不应超过此数
#define I_BASE 8.0f
float a = 0;
int32_t targetPositionEncoder = 0;
// 必须在下一次 ADC 采样触发前跑完电流环
// 暂时不考虑 ADC 的 latency 问题
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    MotorControl1KHzCounter++;
    MotorControl4KHzCounter++;
    if(MotorControl1KHzCounter >= MOTOR_CONTROL_1KHZ_PERIOD)
    {
        MotorControl1KHzCounter = 0;
        MotorControl1KHz();
    }
    if(MotorControl4KHzCounter >= MOTOR_CONTROL_4KHZ_PERIOD)
    {
        MotorControl4KHzCounter = 0;
        MotorControl4KHz();
    }


    Vbus =              (float)(adcBuffer[1] & 0xFFFF)                 * VBUS_GAIN;
    measuredIalpha =    ((float)(adcBuffer[0] & 0xFFFF) - IALPHA_BIAS) * IALPHA_GAIN;
    measuredIbeta =     ((float)(adcBuffer[0] >> 16) - IBETA_BIAS)     * IBETA_GAIN;

    // adcNTCBuffer = adcBuffer[1] >> 16;
    motorErrorStatus.driverFault = 1 - HAL_GPIO_ReadPin(nFAULT_GPIO_Port, nFAULT_Pin);
    motorErrorStatus.overTemperatureWarning = 1 - HAL_GPIO_ReadPin(nOTW_GPIO_Port, nOTW_Pin);
    ErrorHandler();
    
    // GPIOC->BSRR = GPIO_PIN_10;
    // GPIOC->BRR = GPIO_PIN_10;

    // Park 变换
    // measuredId = measuredIalpha * cosf(realAngle) + measuredIbeta * sinf(realAngle);
    // measuredIq = measuredIalpha * sinf(realAngle) - measuredIbeta * cosf(realAngle);
    
    float cordicOutputSinMulIalpha;
    float cordicOutputCosMulIalpha;

    hcordic.Instance->WDATA = (singleFloatToCordic15(measuredIalpha / I_BASE) << 16) | (electricAngle & 0xFFFF);
    cordic15ToDualFloat((int32_t)(hcordic.Instance->RDATA), &cordicOutputSinMulIalpha, &cordicOutputCosMulIalpha);

    float cordicOutputSinMulIbeta;
    float cordicOutputCosMulIbeta;
    hcordic.Instance->WDATA = (singleFloatToCordic15(measuredIbeta / I_BASE) << 16) | (electricAngle & 0xFFFF);
    cordic15ToDualFloat((int32_t)(hcordic.Instance->RDATA), &cordicOutputSinMulIbeta, &cordicOutputCosMulIbeta);

    measuredId = (cordicOutputCosMulIalpha + cordicOutputSinMulIbeta) * I_BASE;
    measuredIq = (-cordicOutputSinMulIalpha + cordicOutputCosMulIbeta) * I_BASE;

    if(motorControlStatus.enableFOCOutput == 0)
    {
        // 有待斟酌 到底是 set 个 0 的电压还是切换到高阻态.
        // setPhraseVoltage(0, 0);
        return;
    }

    float outputLimitVoltage = Vbus;
    if(motorControlStatus.enablePositionCloseLoop == 0 && motorControlStatus.enableSpeedCloseLoop == 0)
    {
        motorControlStatus.targetIq = motorControlStatus.tourqueLimit;
    }
    if(motorControlStatus.FOCControlMode == CURRENT_TOURQUE_CONTROL)
    {
        // 电流 PI
        outputUq = IncrementalPIDupdate(&IqPID, motorControlStatus.targetIq, measuredIq);
        outputUd = IncrementalPIDupdate(&IdPID, motorControlStatus.targetId, measuredId);

        /*
        float v_d_ff = (1.0f * controller->i_d_ref * R_PHASE - controller->dtheta_elec * L_Q * controller->i_q); // feed-forward voltages
        float v_q_ff = (1.0f * controller->i_q_ref * R_PHASE + controller->dtheta_elec * (L_D * controller->i_d + 1.0f * WB));
        */
        // 解耦 + 前馈
        // 反电动势前馈
        // backwardEMF = shaftAngularVelocityFloat * 60.0f / KV;
        // outputUqFeedForward = (PHASE_RESISTANCE * motorControlStatus.targetIq + measuredId * electricAngularVelocityFloat * PHASE_INDUCTANCE + backwardEMF);
        // outputUdFeedForward = (PHASE_RESISTANCE * motorControlStatus.targetId - measuredIq * electricAngularVelocityFloat * PHASE_INDUCTANCE);

        if(outputUqFeedForward > outputLimitVoltage)
            outputUqFeedForward = outputLimitVoltage;
        else if(outputUqFeedForward < -outputLimitVoltage)
            outputUqFeedForward = -outputLimitVoltage;
        if(outputUdFeedForward > outputLimitVoltage)
            outputUdFeedForward = outputLimitVoltage;
        else if(outputUdFeedForward < -outputLimitVoltage)
            outputUdFeedForward = -outputLimitVoltage;


        outputUqWithFeedForward = outputUq + outputUqFeedForward;
        outputUdWithFeedForward = outputUd + outputUdFeedForward;

        if(outputUqWithFeedForward > outputLimitVoltage)
        {
            IncrementalPIDSetOutput(&IqPID, outputUq - outputUqWithFeedForward - outputLimitVoltage);
            outputUqWithFeedForward = outputLimitVoltage;
        }
        else if(outputUqWithFeedForward < -outputLimitVoltage)
        {
            IncrementalPIDSetOutput(&IqPID, outputUq - outputUqWithFeedForward + outputLimitVoltage);
            outputUqWithFeedForward = -outputLimitVoltage;
        }
        
        if(outputUdWithFeedForward > outputLimitVoltage)
        {
            IncrementalPIDSetOutput(&IdPID, outputUd - outputUdWithFeedForward - outputLimitVoltage);
            outputUdWithFeedForward = outputLimitVoltage;
        }
        else if(outputUdWithFeedForward < -outputLimitVoltage)
        {
            IncrementalPIDSetOutput(&IdPID, outputUd - outputUdWithFeedForward + outputLimitVoltage);
            outputUdWithFeedForward = -outputLimitVoltage;
        }
        outputAngle = electricAngle;
    }
    
    else if(motorControlStatus.FOCControlMode == OPEN_LOOP_ROTATE)
    {
        manaticEncoderError = accumulatedEncoder - openLoopTheta / POLE_PAIRS;
        outputUqWithFeedForward = 0;
        outputUdWithFeedForward = OPENLOOP_DRAG_VOLTAGE;

        openLoopTheta += openLoopRotateSpeed;
        outputAngle = (int16_t)(openLoopTheta % 65536);
    }
    
    else if(motorControlStatus.FOCControlMode == OPEN_LOOP_POSITION_CONTROL)
    {
        outputUqWithFeedForward = 0;
        outputUdWithFeedForward = OPENLOOP_DRAG_VOLTAGE;

        if(motorControlStatus.targetPosition * POLE_PAIRS - openLoopThetaFloat > (float)(motorControlStatus.velocityLimit * POLE_PAIRS) / CURRENT_LOOP_FREQ)
        {
            openLoopThetaFloat += (float)(motorControlStatus.velocityLimit * POLE_PAIRS) / CURRENT_LOOP_FREQ;
        }
        else if(motorControlStatus.targetPosition * POLE_PAIRS - openLoopThetaFloat < -(float)(motorControlStatus.velocityLimit * POLE_PAIRS) / CURRENT_LOOP_FREQ)
        {
            openLoopThetaFloat -= (float)(motorControlStatus.velocityLimit * POLE_PAIRS) / CURRENT_LOOP_FREQ;
        }
        else
        {
            openLoopThetaFloat = motorControlStatus.targetPosition * POLE_PAIRS;
        }
            
        outputAngle = (int16_t)((int32_t)(openLoopThetaFloat) % 65536);
    }

    else if(motorControlStatus.FOCControlMode == VOLTAGE_TOURQUE_CONTROL)
    {
        // 不使用电流采样, 直接使用前馈电压控制

        // 解耦 + 前馈
        backwardEMF = shaftAngularVelocityFloat * 60.0f / KV / 2;
        outputUqWithFeedForward = (PHASE_RESISTANCE * motorControlStatus.targetIq  + backwardEMF);
        outputUdWithFeedForward = (PHASE_RESISTANCE * motorControlStatus.targetId - motorControlStatus.targetIq * electricAngularVelocityFloat * PHASE_INDUCTANCE);

        outputUqWithFeedForward = CLAMP(outputUqWithFeedForward, -outputLimitVoltage, outputLimitVoltage);
        outputUdWithFeedForward = CLAMP(outputUdWithFeedForward, -outputLimitVoltage, outputLimitVoltage);
        outputAngle = electricAngle;
    }
    else if (motorControlStatus.FOCControlMode == MUSIC)
    {
        MusicManagerUpdate(&outputAngle, &outputUqWithFeedForward, &outputUdWithFeedForward);
    }


    // Inverse Park Transform
    // outputUalpha = outputUd * cosf(realAngle) - outputUq * sinf(realAngle);
    // outputUbeta = outputUd * sinf(realAngle) + outputUq * cosf(realAngle);
    float cordicOutputSinMulUq;
    float cordicOutputCosMulUq;
    hcordic.Instance->WDATA = (singleFloatToCordic15(outputUqWithFeedForward / Vbus) << 16) | (outputAngle & 0xFFFF);
    cordic15ToDualFloat((int32_t)(hcordic.Instance->RDATA), &cordicOutputSinMulUq, &cordicOutputCosMulUq);

    float cordicOutputSinMulUd;
    float cordicOutputCosMulUd;
    hcordic.Instance->WDATA = (singleFloatToCordic15(outputUdWithFeedForward / Vbus) << 16) | (outputAngle & 0xFFFF);
    cordic15ToDualFloat((int32_t)(hcordic.Instance->RDATA), &cordicOutputSinMulUd, &cordicOutputCosMulUd);

    outputUalpha = (cordicOutputCosMulUd - cordicOutputSinMulUq);
    outputUbeta = (cordicOutputSinMulUd + cordicOutputCosMulUq);

    // targetPositionEncoder = (int32_t)(motorControlStatus.targetPosition * 65536 / TWO_PI / POLE_PAIRS);
    // if(FABS(accumulatedEncoder - targetPositionEncoder) < 16384 / POLE_PAIRS)
    // {
    //     hcordic.Instance->WDATA = (singleFloatToCordic15(4.0f / Vbus) << 16) | (targetPositionEncoder & 0xFFFF);
    //     cordic15ToDualFloat((int32_t)(hcordic.Instance->RDATA), &cordicOutputSinMulUd, &cordicOutputCosMulUd);

    //     //不能直接加上, 电流环会抵消这个效果, 而且需要限幅.
    //     outputUalpha += cordicOutputCosMulUd;
    //     outputUbeta += cordicOutputSinMulUd;
    // }
    setPhraseVoltage(outputUalpha, outputUbeta);    
}