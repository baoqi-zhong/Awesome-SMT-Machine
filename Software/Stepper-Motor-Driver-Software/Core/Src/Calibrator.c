/**
 * @file Calibrator.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "Calibrator.h"

#include "math.h"
#include "FOC.h"
#include "statisticsCalculator.h"
#include "MA732.h"

extern uint32_t adcBuffer[2];

// 开环电角度(不考虑 Pole Pairs)
extern int32_t openLoopTheta;
extern float openLoopThetaFloat;
extern float manaticEncoderError;

extern float outputUalpha;
extern float outputUbeta;

extern float Vbus;

void openLoopMoveTo(int32_t openLoopTheta_)
{
    openLoopTheta = openLoopTheta_;
    openLoopThetaFloat = (float)(openLoopTheta_) / 65536 * TWO_PI;
    outputUalpha = OPENLOOP_DRAG_VOLTAGE * cosf(openLoopThetaFloat);
    outputUbeta = OPENLOOP_DRAG_VOLTAGE * sinf(openLoopThetaFloat);
    setPhraseVoltage(outputUalpha / Vbus, outputUbeta / Vbus);
}

// 用于校准编码器 必须使用定点数
int32_t encoderCalibrationResult[256] = {0};

float calibrationMonitor = 0;
void calibrateBackAndForth(int16_t step)
{

    // 先移动到起始位置往回的位置然后转回来, 以使结果稳定
    openLoopTheta = 0;
    for(int i = 0; i < 16 * 4; i++)
    {
        openLoopTheta -= step;
        openLoopMoveTo(openLoopTheta);
        HAL_Delay(5);
        MA732_ReadBlocking();
    }
    HAL_Delay(20);
    for(int i = 0; i < 15 * 4 + 2; i++)
    {
        openLoopTheta += step;
        openLoopMoveTo(openLoopTheta);
        HAL_Delay(5);
        MA732_ReadBlocking();
    }
    // 当前位置: -2 * step


    for(int i = 0; i < 256; i++)
    {
        int32_t encoderSum = 0;
        for(int j = 0; j < 4; j++)
        {
            openLoopTheta += step;
            openLoopMoveTo(openLoopTheta);
            HAL_Delay(5);
            MA732_ReadBlocking();
            encoderSum += accumulatedEncoder - openLoopTheta / POLE_PAIRS;
        }
        
        calibrationMonitor = encoderSum;
        if(step > 0)
            encoderCalibrationResult[i] += encoderSum;
        else
            encoderCalibrationResult[255 - i] += encoderSum;
    }
    // 当前位置: 1022 * step = 1 圈 - 2 * step

    for(int i = 0; i < 16 * 4; i++)
    {
        openLoopTheta += step;
        openLoopMoveTo(openLoopTheta);
        HAL_Delay(5);
        MA732_ReadBlocking();
    }
    HAL_Delay(20);
    for(int i = 0; i < 16 * 4; i++)
    {
        openLoopTheta -= step;
        openLoopMoveTo(openLoopTheta);
        HAL_Delay(5);
        MA732_ReadBlocking();
    }
    // 当前位置: 1 圈 - 2 * step
        
    // 不需要移动半个 step, 因为之前已经有了半个 step 的偏移
    for(int i = 0; i < 256; i++)
    {
        int32_t encoderSum = 0;
        for(int j = 0; j < 4; j++)
        {
            openLoopTheta -= step;
            openLoopMoveTo(openLoopTheta);
            HAL_Delay(5);
            MA732_ReadBlocking();
            encoderSum += accumulatedEncoder - openLoopTheta / POLE_PAIRS;
        }
        
        calibrationMonitor = encoderSum;
        if(step > 0)
            encoderCalibrationResult[255 - i] += encoderSum;
        else
            encoderCalibrationResult[i] += encoderSum;
    }
    // 当前位置: - 2 * step
    HAL_Delay(20);
    for(int i = 0; i < 2; i++)
    {
        openLoopTheta += step;
        openLoopMoveTo(openLoopTheta);
        HAL_Delay(5);
        MA732_ReadBlocking();
    }
    // 当前位置: 0
}

void calibrateEncoder()
{
    HAL_GPIO_WritePin(NRST_AB_GPIO_Port, NRST_AB_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NRST_CD_GPIO_Port, NRST_CD_Pin, GPIO_PIN_SET);  
    alineMotor();
    MA732_Reset();
    for(int i = 1; i <= 10; i++)
    {
        setPhraseVoltage(OPENLOOP_DRAG_VOLTAGE / Vbus / 10 * i, 0.0f);
        HAL_Delay(1);
    }

    HAL_Delay(200);
    MA732_setZero();
    MA732_setZeroSoftware();
    for(int i = 9; i >= 0; i--)
    {
        setPhraseVoltage(OPENLOOP_DRAG_VOLTAGE / Vbus / 10 * i, 0.0f);
        HAL_Delay(1);
    }
    
    for(int i = 0; i < 256; i++)
    {
        encoderCalibrationResult[i] = 0;
    }

    // 一圈一共得到 256 个点, 每个点是周围 4 个点的平均值, 一共 256 * 4 = 1024 个点
    // 正反转一次各 1024 个点
    // step 是 1/1024 圈对应的电角度的增量值
    int16_t step = 65536 * POLE_PAIRS / 1024; // 正向来回
    calibrateBackAndForth(step);
    HAL_Delay(100);
    step = - 65536 * POLE_PAIRS / 1024; // 反向来回
    calibrateBackAndForth(step);
    
    for(int i = 0; i < 256; i++)
    {
        HAL_Delay(5);
        encoderCalibrationResult[i] = encoderCalibrationResult[i] / 32;
        calibrationMonitor = encoderCalibrationResult[i];
    }
    
    alineMotor(); // 回到原点
    while(1);
}

volatile ADCCalibrationResult_t ADCCalibrationResult;
void calibrateCurrentSensor()
{
    setPhraseVoltage(0.0f, 0.0f);
    HAL_Delay(100);

    statisticsCalculator_t VBusStatisticsCalculator;
    statisticsCalculatorInit(&VBusStatisticsCalculator);
    statisticsCalculator_t IalphaStatisticsCalculator;
    statisticsCalculatorInit(&IalphaStatisticsCalculator);
    statisticsCalculator_t IbetaStatisticsCalculator;
    statisticsCalculatorInit(&IbetaStatisticsCalculator);

    for(uint16_t i = 0; i < 5000; i++)
    {
        uint16_t VBusADC        = adcBuffer[1] & 0xFFFF;
        uint16_t IalphaADC      = adcBuffer[0] & 0xFFFF;
        uint16_t IbetaADC       = adcBuffer[0] >> 16;
        statisticsCalculatorAddData(&VBusStatisticsCalculator, VBusADC);
        statisticsCalculatorAddData(&IalphaStatisticsCalculator, IalphaADC);
        statisticsCalculatorAddData(&IbetaStatisticsCalculator, IbetaADC);

        HAL_Delay(1);
    }
    ADCCalibrationResult.VBusADCValue = statisticsCalculatorGetMean(&VBusStatisticsCalculator);
    ADCCalibrationResult.IalphaADCBias = statisticsCalculatorGetMean(&IalphaStatisticsCalculator);
    ADCCalibrationResult.IbetaADCBias = statisticsCalculatorGetMean(&IbetaStatisticsCalculator);

    statisticsCalculatorReset(&IalphaStatisticsCalculator);
    statisticsCalculatorReset(&IbetaStatisticsCalculator);

    setPhraseVoltage(0.1f, 0.1f);
    HAL_Delay(100);

    for(uint16_t i = 0; i < 5000; i++)
    {
        uint16_t IalphaADC      = adcBuffer[0] & 0xFFFF;
        uint16_t IbetaADC       = adcBuffer[0] >> 16;
        statisticsCalculatorAddData(&IalphaStatisticsCalculator, IalphaADC);
        statisticsCalculatorAddData(&IbetaStatisticsCalculator, IbetaADC);

        HAL_Delay(1);
    }
    ADCCalibrationResult.IalphaADCOnCurrent = statisticsCalculatorGetMean(&IalphaStatisticsCalculator);
    ADCCalibrationResult.IbetaADCOnCurrent = statisticsCalculatorGetMean(&IbetaStatisticsCalculator);

    setPhraseVoltage(0.0f, 0.0f);
    while(1);
}