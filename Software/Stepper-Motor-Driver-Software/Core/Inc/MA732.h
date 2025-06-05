/**
 * @file MA732.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "GeneralConfig.h"
#include "stdint.h"
#include "spi.h"

#define fmod(x, y) ((x) - (int)((x) / (y)) * (y))

#define MA732_READ_REG      0x40
#define MA732_WRITE_REG     0x80

#define MA732_ROTATION_DIRECTION_CW   0x00
#define MA732_ROTATION_DIRECTION_CCW  0x80

#define MA732_FILTER_CUTOFF_FREQ_6000 51
#define MA732_FILTER_CUTOFF_FREQ_3000 68
#define MA732_FILTER_CUTOFF_FREQ_1500 85
#define MA732_FILTER_CUTOFF_FREQ_740  102
#define MA732_FILTER_CUTOFF_FREQ_370  119
#define MA732_FILTER_CUTOFF_FREQ_185  136
#define MA732_FILTER_CUTOFF_FREQ_93   153
#define MA732_FILTER_CUTOFF_FREQ_46   170
#define MA732_FILTER_CUTOFF_FREQ_23   187

extern uint16_t encoder;
extern int32_t accumulatedEncoder;

extern int16_t electricAngle;
extern float electricAngleFloat;

extern int16_t encoderDifference;
extern float encoderDifferenceLPFFloat;

extern int32_t shaftAngularVelocity;
extern float shaftAngularVelocityFloat;

extern int32_t electricAngularVelocity;
extern float electricAngularVelocityFloat;

extern float encoderDelayTime;

/**
 * @brief Normalizes an angle to be within the range [0, 2*pi).
 * 
 * @param _angle The angle to be normalized.
 * @return The normalized angle.
 */
float normalizeAngleZeroToTwoPi(float _angle);

/**
 * @brief Normalizes an angle to be within the range [-pi, pi).
 * 
 * @param _angle The angle to be normalized.
 * @return The normalized angle.
 */
float normalizeAngleNegPiToPi(float _angle);

/**
 * @brief Writes data to a register in the MA732 sensor.
 * 
 * @param regesterAddress The address of the register to write to.
 * @param data The data to write to the register.
 */
void MA732_WriteReg(uint8_t regesterAddress, uint8_t data);

/**
 * @brief Reads data from a register in the MA732 sensor.
 * 
 * @param regesterAddress The address of the register to read from.
 * @return The data read from the register.
 */
uint16_t MA732_ReadReg(uint8_t regesterAddress);

/**
 * @brief Resets the MA732 sensor.
 */
void MA732_Reset();

/**
 * @brief Sets the zero position for the MA732 sensor.
 */
void MA732_setZero();

/**
 * @brief Resets the last encoder value of the sensor driver.
 */
void MA732_resetLastEncoder();

/**
 * @brief Reset everything in the sensor driver.
 */
void MA732_setZeroSoftware();

/**
 * @brief Initializes the MA732 sensor.
 * 
 * @param hspi Pointer to the SPI handle.
 */
void MA732_Init(SPI_HandleTypeDef *hspi);

/**
 * @brief Reads data from the MA732 sensor in a blocking manner.
 */
void MA732_ReadBlocking();

/**
 * @brief Gets the raw encoder value from the MA732 sensor.
 * 
 * @return The raw encoder value.
 */
uint16_t MA732_GetRawEncoder();