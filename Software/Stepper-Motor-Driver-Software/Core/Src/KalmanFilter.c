/**
 * @file KalmanFilter.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "KalmanFilter.h"
#if (MA732_USE_KALMAN_FILTER == 1)

void KalmanFilterInit(KalmanFilter *kalmanFilter, float dt)
{
    Matrix2x2Transpose(&kalmanFilter->F_transpose, &kalmanFilter->F);
    kalmanFilter->dt = dt;

    Matrix2x2Copy(&kalmanFilter->P, &Matrix2x2Identity);
    Matrix2x2Copy(&kalmanFilter->K, &Matrix2x2Identity);
}

void KalmanFilterReset(KalmanFilter *kalmanFilter)
{
    Matrix2x2Copy(&kalmanFilter->P, &Matrix2x2Identity);
    Matrix2x2Copy(&kalmanFilter->K, &Matrix2x2Identity);
}

void KalmanFilterPredict(KalmanFilter *kalmanFilter, float u)
{
    // x = Fx + Bu
    Matrix2x1_t temp;
    Matrix2x2MulMatrix2x1(&kalmanFilter->observer, &kalmanFilter->F, &kalmanFilter->observer);
    Matrix2x1Scale(&temp, &kalmanFilter->B, u);
    Matrix2x1Add(&kalmanFilter->observer, &kalmanFilter->observer, &temp);

    // P = FPF' + Q
    Matrix2x2MulMatrix2x2(&kalmanFilter->P, &kalmanFilter->F, &kalmanFilter->P);
    Matrix2x2MulMatrix2x2(&kalmanFilter->P, &kalmanFilter->P, &kalmanFilter->F_transpose);
    Matrix2x2Add(&kalmanFilter->P, &kalmanFilter->P, &kalmanFilter->Q); 
}   

void KalmanFilterUpdate(KalmanFilter *kalmanFilter, Matrix2x1_t* measurement)
{
    // K = PH^T(HPH^T + R)^-1
    // 我们的传感器有位置和速度, 所以H = I
    // K = PH^T(HPH^T + R)^-1 = P(P + R)^-1
    Matrix2x2_t temp;
    Matrix2x2Add(&temp, &kalmanFilter->P, &kalmanFilter->R);
    Matrix2x2Inverse(&temp, &temp);
    Matrix2x2MulMatrix2x2(&kalmanFilter->K, &kalmanFilter->P, &temp);

    // x = x + K(measurement - Hx) = x + K(measurement - x)
    Matrix2x1_t temp2;
    Matrix2x1Sub(&temp2, measurement, &kalmanFilter->observer);
    Matrix2x2MulMatrix2x1(&temp2, &kalmanFilter->K, &temp2);
    Matrix2x1Add(&kalmanFilter->observer, &kalmanFilter->observer, &temp2);

    // P = (I - KH)P = (I - K)P
    Matrix2x2Sub(&temp, &Matrix2x2Identity, &kalmanFilter->K);
    Matrix2x2MulMatrix2x2(&kalmanFilter->P, &temp, &kalmanFilter->P);
}

void KalmanFilterRun(KalmanFilter *kalmanFilter, float control, Matrix2x1_t* measurement)
{
    KalmanFilterPredict(kalmanFilter, control);
    KalmanFilterUpdate(kalmanFilter, measurement);
}

#endif