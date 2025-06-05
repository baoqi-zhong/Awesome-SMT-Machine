/**
 * @file MA732.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "MA732.h"
#include "FOC.h"

#include "LPF.h"
#include "statisticsCalculator.h"
#include "KalmanFilter.h"

SPI_HandleTypeDef *MA732_hspi = NULL;

uint8_t MA732Buffer[8] = {0};
uint8_t emptyBuffer[8] = {0};
uint16_t encoder;
uint16_t lastEncoder = 0;

int32_t accumulatedEncoder = 0;
int16_t electricAngle = 0;
float electricAngleFloat = 0.0f;
int16_t encoderDifference = 0;
float encoderDifferenceLPFFloat = 0.0f;

int32_t shaftAngularVelocity = 0;
float shaftAngularVelocityFloat = 0.0f;
int32_t electricAngularVelocity = 0;
float electricAngularVelocityFloat = 0.0f;

float encoderDelayTime = 0.0f;


LPF_t encoderDifferenceLPF;
#if (MA732_USE_KALMAN_FILTER == 1)
Matrix2x1_t encoderMeasurement;
KalmanFilter rotorKalmanFilter;
#endif
statisticsCalculator_t encoderStatisticsCalculator;

// 数组里存的是偏移的 Encoder 值 / 2, 因为这样能用 8 位表示, 且不会丢失很多精度
#if (CAN_ID == 1)
int8_t ENCODER_BIAS[257] = {
     -2,   0,   4,  10,  10,  13,  14,  18,  22,  23,  25,  28,  29,  33,  31,  32,
     35,  36,  38,  37,  39,  40,  41,  39,  38,  39,  38,  38,  36,  35,  35,  35,
     32,  31,  30,  29,  28,  24,  24,  23,  21,  19,  14,  13,  12,   8,   6,   2,
      1,   0,  -5,  -7, -11, -10, -12, -19, -21, -23, -27, -28, -32, -32, -36, -38,
    -41, -44, -44, -46, -49, -50, -53, -54, -54, -58, -59, -59, -59, -62, -65, -64,
    -62, -64, -64, -66, -64, -63, -64, -65, -65, -65, -65, -65, -63, -64, -63, -60,
    -60, -58, -57, -57, -55, -55, -51, -50, -50, -46, -48, -43, -39, -42, -39, -38,
    -33, -31, -33, -31, -28, -24, -21, -24, -22, -18, -16, -14, -14, -13, -10,  -7,
     -6,  -6,  -5,  -2,  -1,   0,  -1,   0,   0,   0,   2,   3,   1,   1,   0,   1,
      2,   0,   0,  -1,   1,   1,  -2,  -3,  -5,  -5,  -5, -11, -12, -12, -12, -14,
    -19, -22, -23, -24, -26, -30, -33, -35, -37, -39, -45, -48, -50, -52, -55, -60,
    -62, -64, -68, -70, -74, -77, -80, -83, -84, -88, -92, -95, -97, -98, -101, -106,
    -108, -110, -110, -111, -113, -115, -117, -118, -118, -123, -122, -122, -122, -123, -126, -125,
    -126, -125, -124, -126, -124, -119, -119, -120, -121, -117, -114, -114, -112, -111, -109, -105,
    -104, -100, -98, -96, -92, -91, -86, -82, -80, -76, -73, -69, -64, -62, -59, -54,
    -50, -46, -45, -40, -35, -31, -27, -26, -21, -16, -12,  -8,  -7,  -2,   1,   4,
     -2
};
#else
int8_t ENCODER_BIAS[257] = {
    -13, -14, -16, -16, -19, -18, -21, -22, -23, -27, -28, -27, -28, -29, -30, -29,
    -29, -30, -32, -34, -32, -28, -31, -32, -32, -29, -27, -28, -29, -26, -24, -22,
    -25, -21, -18, -16, -15, -15, -14, -12,  -9,  -4,  -4,  -1,   1,   3,   7,  10,
     11,  16,  17,  19,  24,  26,  31,  31,  34,  38,  43,  44,  45,  49,  53,  55,
     58,  59,  63,  68,  68,  70,  71,  75,  78,  81,  82,  83,  84,  88,  90,  93,
     93,  96,  96,  97, 100, 101, 103, 104, 104, 108, 107, 107, 107, 110, 109, 112,
    108, 109, 110, 112, 112, 108, 107, 108, 109, 107, 105, 105, 105, 104, 102, 100,
    100,  97,  95,  96,  92,  92,  90,  87,  86,  84,  82,  78,  77,  77,  75,  71,
     70,  67,  69,  68,  63,  60,  59,  59,  58,  55,  53,  50,  49,  50,  47,  46,
     46,  43,  43,  41,  42,  41,  39,  39,  38,  39,  38,  37,  38,  40,  39,  39,
     36,  39,  41,  39,  39,  37,  38,  42,  42,  44,  43,  46,  49,  49,  49,  47,
     50,  54,  53,  51,  53,  55,  58,  58,  58,  59,  59,  61,  63,  64,  65,  63,
     66,  66,  67,  69,  66,  66,  70,  70,  72,  69,  70,  72,  72,  72,  69,  68,
     72,  72,  71,  69,  68,  70,  70,  69,  66,  65,  67,  66,  66,  64,  61,  61,
     60,  60,  60,  56,  56,  51,  53,  50,  47,  44,  42,  44,  43,  39,  36,  34,
     33,  33,  28,  23,  22,  22,  20,  16,  11,  10,  11,   9,   6,   3,   1,   1,
    -13
};
#endif

/*
作用: 返回还原后的 Encoder 偏移值
*/
int16_t getBias(uint16_t rawAngle)
{
    uint16_t rawAngleLow = rawAngle & 0xFF;
    uint16_t rawAngleHigh = rawAngle >> 8;
    return ENCODER_BIAS[rawAngleHigh] * 2 + (((ENCODER_BIAS[rawAngleHigh + 1] - ENCODER_BIAS[rawAngleHigh]) * rawAngleLow) >> 7);
}

// 归一化角度到 [0,2PI]
float normalizeAngleZeroToTwoPi(float _angle)
{
  _angle = fmod(_angle, TWO_PI);
  return _angle >= 0 ? _angle : (_angle + TWO_PI);  
}

// 归一化角度到 [-PI, PI]
float normalizeAngleNegPiToPi(float _angle)
{
    _angle = fmod(_angle, TWO_PI);
    if(_angle > PI)
        return _angle - TWO_PI;
    else if(_angle < -PI)
        return _angle + TWO_PI;
    return _angle;  
}

void MA732_WriteReg(uint8_t regesterAddress, uint8_t data)
{
    MA732Buffer[0] = data;
    MA732Buffer[1] = MA732_WRITE_REG | regesterAddress;
    HAL_SPI_TransmitReceive(MA732_hspi, MA732Buffer, (uint8_t *)(&MA732Buffer[2]), 1, 100);  // 必须 Transmit 0x00, 否则会意外修改寄存器
    HAL_Delay(25);
    HAL_SPI_TransmitReceive(MA732_hspi, emptyBuffer, (uint8_t *)(&MA732Buffer[2]), 1, 100);  // 必须 Transmit 0x00, 否则会意外修改寄存器
    // if(MA732Buffer[3] != data)
    //     __BKPT(0);
    HAL_Delay(5);
}

uint16_t MA732_ReadReg(uint8_t regesterAddress)
{
    MA732Buffer[0] = 0;
    MA732Buffer[1] = MA732_READ_REG | regesterAddress;
    HAL_SPI_TransmitReceive(MA732_hspi, MA732Buffer, (uint8_t *)(&MA732Buffer[2]), 1, 100);  // 必须 Transmit 0x00, 否则会意外修改寄存器
    HAL_Delay(1);
    HAL_SPI_TransmitReceive(MA732_hspi, emptyBuffer, (uint8_t *)(&MA732Buffer[2]), 1, 100);  // 必须 Transmit 0x00, 否则会意外修改寄存器
    return MA732Buffer[3];
}

/*
作用: 把储存在 MA732 内部 Flash 的校准数据恢复出厂设置 
*/
void MA732_Reset()
{
    MA732_WriteReg(0x00, 0x00);
    MA732_WriteReg(0x01, 0x00);
    MA732_WriteReg(0x02, 0x00);
    MA732_WriteReg(0x03, 0x00);
    MA732_WriteReg(0x04, 0xC0);
    MA732_WriteReg(0x05, 0xFF);
    MA732_WriteReg(0x06, 0x1C);
    MA732_WriteReg(0x09, MA732_ROTATION_DIRECTION_CCW);
    MA732_WriteReg(0x0E, MA732_FILTER_CUTOFF_FREQ_370);
    MA732_WriteReg(0x10, 0x9C);
}

/*
作用: 将当前的 Encoder 值设为 0, 断电后能保存, 只需要设置一次
*/
void MA732_setZero()
{
    MA732_WriteReg(0x00, 0x00);
    MA732_WriteReg(0x01, 0x00);
    setPhraseVoltage(0.2f, 0.0f);
    HAL_Delay(200);

    HAL_SPI_TransmitReceive(&hspi2, emptyBuffer, MA732Buffer, 1, 100);  // 必须 Transmit 0x00, 否则会意外修修改.
    encoder = (MA732Buffer[1] << 8) + MA732Buffer[0];

    MA732_WriteReg(0x00, (65535 - encoder) & 0xFF);
    MA732_WriteReg(0x01, (65535 - encoder) >> 8);
    setPhraseVoltage(0.0f, 0.0f);
}

void MA732_resetLastEncoder()
{
    // 这里不用 MA732_ReadBlocking 是为了避免调用 decodeBuffer
    HAL_SPI_TransmitReceive(&hspi2, emptyBuffer, MA732Buffer, 1, 100);  // 必须 Transmit 0x00, 否则会意外修修改.
    encoder = *(uint16_t *)(MA732Buffer);
    lastEncoder = encoder;
}
/*
作用: 将当前的 electricAngle 值设为 0, 断电后不能保存, 是软件虚拟位置回零
*/
void MA732_setZeroSoftware()
{
    MA732_resetLastEncoder();
    accumulatedEncoder = 0;
    electricAngle = 0;
    electricAngleFloat = 0.0f;
    encoderDifference = 0;
    shaftAngularVelocity = 0;
    shaftAngularVelocityFloat = 0.0f;
    electricAngularVelocity = 0;
    electricAngularVelocityFloat = 0.0f;
}

void MA732_Init(SPI_HandleTypeDef *hspi)
{
    MA732_hspi = hspi;

    LPF_init(&encoderDifferenceLPF, 0.005f);
    // statisticsCalculatorInit(&encoderStatisticsCalculator);
    // statisticsCalculatorSetDropRate(&encoderStatisticsCalculator, 0.999f);

    #if (MA732_USE_KALMAN_FILTER == 1)
    Matrix2x2Copy(&rotorKalmanFilter.F, &Matrix2x2Identity);
    rotorKalmanFilter.F.element.a12 = rotorKalmanFilter.dt;

    rotorKalmanFilter.B.element.a11 = 0.5f * rotorKalmanFilter.dt * rotorKalmanFilter.dt;
    rotorKalmanFilter.B.element.a21 = 0.0f;

    Matrix2x2Copy(&rotorKalmanFilter.Q, &Matrix2x2Zero);
    rotorKalmanFilter.Q.element.a11 = 0.2f; // 角度的过程噪声方差
    rotorKalmanFilter.Q.element.a22 = 0.05f; // 角速度的过程噪声方差

    Matrix2x2Copy(&rotorKalmanFilter.R, &Matrix2x2Zero);
    rotorKalmanFilter.R.element.a11 = 42.25f;   // 角度的观测噪声方差, 由 statisticsCalculator 统计得到
    rotorKalmanFilter.R.element.a22 = 7.075f;   // 角速度的观测噪声方差, 由 statisticsCalculator 统计得到
    KalmanFilterInit(&rotorKalmanFilter, 1.0f / CURRENT_LOOP_FREQ);
    #endif

    __HAL_SPI_ENABLE(hspi);
    MA732_WriteReg(0x0E, MA732_FILTER_CUTOFF_FREQ_1500);
    encoderDelayTime = 10.0f / CURRENT_LOOP_FREQ;
    HAL_Delay(1);
}


void MA732_DecodeBuffer()
{
    lastEncoder = encoder;
    encoder = *(uint16_t *)(MA732Buffer);
    // statisticsCalculatorAddData(&encoderStatisticsCalculator, (float)encoder);
    #if (CALIBRATE_ENCODER == 0)
    encoder-= getBias(encoder);
    #endif

    encoderDifference = encoder - lastEncoder;

    if(encoderDifference > 20000 || encoderDifference < -20000)
    {
        // 电角度突变, 说明 SPI 通信被干扰.
        encoder = lastEncoder;
        return;
    }

    accumulatedEncoder += encoderDifference;
    #if (MA732_USE_KALMAN_FILTER == 1)
    encoderMeasurement.array[0] = (float)accumulatedEncoder;
    encoderMeasurement.array[1] = (float)encoderDifference;
    KalmanFilterRun(&rotorKalmanFilter, 0.0f, &encoderMeasurement);
    #endif
    // statisticsCalculatorAddData(&encoderStatisticsCalculator, (float)encoderDifference);

    // encoderDifferenceLPFFloat 是经过了 LPF 的
    #if (MA732_USE_KALMAN_FILTER == 1)
    encoderDifferenceLPFFloat = rotorKalmanFilter.observer.array[1];
    int32_t estimateAccumulatedEncoder = rotorKalmanFilter.observer.array[0] + (int32_t)(encoderDifferenceLPFFloat * encoderDelayTime); // * CURRENT_LOOP_FREQ
    #else
    encoderDifferenceLPFFloat = LPF(&encoderDifferenceLPF, (float)(encoderDifference));
    int32_t estimateAccumulatedEncoder = accumulatedEncoder + (int32_t)(encoderDifferenceLPFFloat * encoderDelayTime); // * CURRENT_LOOP_FREQ
    #endif
    
    shaftAngularVelocity = encoderDifferenceLPFFloat * CURRENT_LOOP_FREQ;
    shaftAngularVelocityFloat = shaftAngularVelocity / 65536.0f * TWO_PI;
    electricAngularVelocityFloat = shaftAngularVelocityFloat * POLE_PAIRS;

    uint32_t electricAngle_ = (((estimateAccumulatedEncoder % 65536) * POLE_PAIRS) % 65536);
    if(electricAngle_ > 32767)
        electricAngle_ -= 65536;
    electricAngle = electricAngle_;
    electricAngleFloat = ((float)(estimateAccumulatedEncoder * POLE_PAIRS) / 65536) * TWO_PI;
}

void MA732_ReadBlocking()
{
    #if (CAN_ID == 3)
    // 如果是无磁编码器, 则直接使用 openLoopTheta
    extern int32_t openLoopTheta;
    accumulatedEncoder = openLoopTheta / POLE_PAIRS;
    uint32_t electricAngle_ = (((accumulatedEncoder % 65536) * POLE_PAIRS) % 65536);
    if(electricAngle_ > 32767)
        electricAngle_ -= 65536;
    electricAngle = electricAngle_;
    electricAngleFloat = ((float)(accumulatedEncoder * POLE_PAIRS) / 65536) * TWO_PI;
    return;
    #endif

    // 提前于电流环调用, 在 Update Event 触发了 ADC 之后就可以调用.

    // MA732 本身有 9 us 的 Latency, 但是需要加上从读取到 setPhaseVoltage 的时间 19 us
    // encoderDelayTime = 0.000009f + 0.000019;

    if(MA732_hspi->Lock == HAL_LOCKED)
        return;
    MA732_hspi->Lock = HAL_LOCKED;

    /* Set fiforxthreshold according the reception data length: 16bit */
    CLEAR_BIT(MA732_hspi->Instance->CR2, SPI_RXFIFO_THRESHOLD);
    while(!__HAL_SPI_GET_FLAG(MA732_hspi, SPI_FLAG_TXE)); // 阻塞等待直到 Tx 空闲
    MA732_hspi->Instance->DR = *((uint16_t *)emptyBuffer);

    while(!__HAL_SPI_GET_FLAG(MA732_hspi, SPI_FLAG_RXNE)); // 阻塞等待直到 Rx 有数据
    *((uint16_t *)MA732Buffer) = (uint16_t)MA732_hspi->Instance->DR;

    // End transection
    while ((MA732_hspi->Instance->SR & SPI_FLAG_FTLVL) != SPI_FTLVL_EMPTY);         // Control if the TX fifo is empty
    while ((__HAL_SPI_GET_FLAG(MA732_hspi, SPI_FLAG_BSY) ? SET : RESET) != RESET);  // Control the BSY flag
    while ((MA732_hspi->Instance->SR & SPI_FLAG_FRLVL) != SPI_FRLVL_EMPTY);         // Control if the RX fifo is empty
    MA732_hspi->Lock = HAL_UNLOCKED;

    MA732_DecodeBuffer();
}

uint16_t MA732_GetRawEncoder()
{
    return *(uint16_t *)(MA732Buffer);
}