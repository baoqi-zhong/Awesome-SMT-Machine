/**
 * @file statisticsCalculator.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "main.h"
#include "GeneralConfig.h"

#include "stdint.h"

typedef struct
{
    float n;
    float correctedSumOfSquares;
    float mean;
    float standardDeviation;
    uint32_t dropPeriod;
    uint32_t dropCounter;
} statisticsCalculator_t;


/**
 * @brief Initialize the statistics calculator with default values
 * @param calculator Pointer to the statisticsCalculator_t struct
 */
void statisticsCalculatorInit(statisticsCalculator_t *calculator);

/**
 * @brief Reset the statistics calculator to the initial state
 * @param calculator Pointer to the statisticsCalculator_t struct
 */
void statisticsCalculatorReset(statisticsCalculator_t *calculator);

/**
 * @brief Set the drop rate of the statistics calculator
 * @param calculator Pointer to the statisticsCalculator_t struct
 * @param dropRate Drop rate of the statistics calculator
 */
void statisticsCalculatorSetDropRate(statisticsCalculator_t *calculator, float dropRate);

/**
 * @brief Add new data to the statistics calculator for calculations
 * @param calculator Pointer to the statisticsCalculator_t struct
 * @param data New data point to add
 */
void statisticsCalculatorAddData(statisticsCalculator_t *calculator, float data);

/**
 * @brief Get the mean value of the data added to the statistics calculator
 * @param calculator Pointer to the statisticsCalculator_t struct
 * @return Mean value of the data
 */
float statisticsCalculatorGetMean(statisticsCalculator_t *calculator);

/**
 * @brief Get the standard deviation value of the data added to the statistics calculator
 * @param calculator Pointer to the statisticsCalculator_t struct
 * @return Standard deviation value of the data
 */
float statisticsCalculatorGetStandardDeviation(statisticsCalculator_t *calculator);