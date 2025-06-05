/**
 * @file CordicHelper.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "GeneralConfig.h"
#include "cordic.h"
#include "stdint.h"

/**
 * @brief Initialize the CORDIC algorithm for 16-bit operations
 */
void cordic16_init();

/**
 * @brief Initialize the CORDIC algorithm for 32-bit operations
 */
void cordic32_init();

/**
 * @brief Convert a floating-point value to a 31-bit CORDIC fixed-point representation
 * @param floatingValue Input floating-point value to convert
 * @return Converted 31-bit CORDIC fixed-point representation
 */
int32_t floatToCordic31(float floatingValue);

/**
 * @brief Convert a 31-bit CORDIC fixed-point representation to a floating-point value
 * @param cordic31 Input 31-bit CORDIC fixed-point value to convert
 * @return Converted floating-point value
 */
float cordic31ToFloat(int cordic31);

/**
 * @brief Convert two floating-point values to a 15-bit CORDIC fixed-point representation
 * @param valueA First input floating-point value (High 16 bits)
 * @param valueB Second input floating-point value (Low 16 bits)
 * @return Combined 15-bit CORDIC fixed-point representation
 */
int32_t dualFloatToCordic15(float valueA, float valueB);

/**
 * @brief Convert a floating-point value to a 15-bit CORDIC fixed-point representation
 * @param valueA Input floating-point value
 * @return Converted 15-bit CORDIC fixed-point representation
 */
int32_t singleFloatToCordic15(float valueA);

/**
 * @brief Convert a 15-bit CORDIC fixed-point representation to two floating-point values
 * @param CORDIC15 Input 15-bit CORDIC fixed-point value to convert
 * @param valueA Pointer to store the first output floating-point value (High 16 bits)
 * @param valueB Pointer to store the second output floating-point value (Low 16 bits)
 */
void cordic15ToDualFloat(int32_t CORDIC15, float *valueA, float *valueB);

/**
 * @brief Convert a 15-bit CORDIC fixed-point representation to a floating-point value
 * @param CORDIC15 Input 15-bit CORDIC fixed-point value to convert
 * @return Converted floating-point value
 */
float cordic15ToSingleFloat(int32_t CORDIC15);
