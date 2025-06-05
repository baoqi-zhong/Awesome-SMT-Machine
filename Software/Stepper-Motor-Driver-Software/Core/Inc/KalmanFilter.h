/**
 * @file KalmanFilter.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "GeneralConfig.h"

#include "stdint.h"

#if (MA732_USE_KALMAN_FILTER == 1)

#include "Matrix.h"


typedef struct {
    Matrix2x1_t observer; // 观测值
    Matrix2x2_t F;      // 状态转移矩阵
    Matrix2x1_t B;      // 控制输入矩阵

    Matrix2x2_t P;      // 状态协方差矩阵
    Matrix2x2_t Q;      // 状态噪声协方差矩阵

    // Matrix2x2_t H;   // 观测矩阵: Identity Matrix
    Matrix2x2_t K;      // 卡尔曼增益矩阵
    Matrix2x2_t R;      // 观测噪声协方差矩阵
    
    Matrix2x2_t F_transpose;
    
    

    // Matrix2x1_t 
    float dt;
} KalmanFilter;

/**
 * @brief Initializes a Kalman filter.
 * 
 * @param kalmanFilter Pointer to the Kalman filter to be initialized.
 * @param dt The time step of the Kalman filter.
 */
void KalmanFilterInit(KalmanFilter *kalmanFilter, float dt);

/**
 * @brief Updates the Kalman filter with a new control input.
 * @param kalmanFilter Pointer to the Kalman filter to be updated.
 * @param control The control output.
 * @param measurement The measurement input from the sensor.
 */
void KalmanFilterRun(KalmanFilter *kalmanFilter, float control, Matrix2x1_t* measurement);

#endif