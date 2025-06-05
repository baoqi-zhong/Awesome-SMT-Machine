/**
 * @file statisticsCalculator.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "statisticsCalculator.h"
#include "math.h"

void statisticsCalculatorInit(statisticsCalculator_t *calculator)
{
    statisticsCalculatorReset(calculator);
}

void statisticsCalculatorReset(statisticsCalculator_t *calculator)
{
    calculator->n = 0;
    calculator->correctedSumOfSquares = 0;
    calculator->mean = 0;
    calculator->standardDeviation = 0;
    calculator->dropPeriod = 0;
    calculator->dropCounter = 0;
}

void statisticsCalculatorSetDropRate(statisticsCalculator_t *calculator, float dropRate)
{
    if(dropRate == 0)
    {
        calculator->dropPeriod = 0;
    }
    else
    {
        calculator->dropPeriod = 1.0f / (1.0f - dropRate);    
    }

}

void statisticsCalculatorAddData(statisticsCalculator_t *calculator, float data)
{
    if(calculator->dropPeriod)
        calculator->dropCounter++;
    
    if(calculator->dropPeriod == 0 || calculator->dropCounter > calculator->dropPeriod)
    {
        float oldMean = calculator->mean;
        calculator->mean += (data - oldMean) / (calculator->n + 1);
        calculator->correctedSumOfSquares += (data - oldMean) * (data - calculator->mean);
        calculator->n++;    

        calculator->dropCounter = 0;
        statisticsCalculatorGetStandardDeviation(calculator);
    }
}

float statisticsCalculatorGetMean(statisticsCalculator_t *calculator)
{
    return calculator->mean;
}

float statisticsCalculatorGetStandardDeviation(statisticsCalculator_t *calculator)
{
    if(calculator->n == 0)
        return 0;

    calculator->standardDeviation = sqrt(calculator->correctedSumOfSquares / calculator->n);
    return calculator->standardDeviation;
}