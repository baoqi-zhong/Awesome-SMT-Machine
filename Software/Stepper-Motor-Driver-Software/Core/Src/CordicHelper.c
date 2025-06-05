/**
 * @file CordicHelper.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "CordicHelper.h"

void cordic16_init()
{
    CORDIC_ConfigTypeDef cordicConfig;
    cordicConfig.Function = CORDIC_FUNCTION_SINE;
    cordicConfig.Scale = CORDIC_SCALE_0;
    cordicConfig.InSize = CORDIC_INSIZE_16BITS;
    cordicConfig.OutSize = CORDIC_OUTSIZE_16BITS;
    cordicConfig.NbWrite = CORDIC_NBWRITE_1;
    cordicConfig.NbRead = CORDIC_NBREAD_1;
    cordicConfig.Precision = CORDIC_PRECISION_8CYCLES;
    HAL_CORDIC_Configure(&hcordic, &cordicConfig);
}

void cordic32_init()
{
    CORDIC_ConfigTypeDef cordicConfig;
    cordicConfig.Function = CORDIC_FUNCTION_SINE;
    cordicConfig.Scale = CORDIC_SCALE_0;
    cordicConfig.InSize = CORDIC_INSIZE_32BITS;
    cordicConfig.OutSize = CORDIC_OUTSIZE_32BITS;
    cordicConfig.NbWrite = CORDIC_NBWRITE_2;
    cordicConfig.NbRead = CORDIC_NBREAD_2;
    cordicConfig.Precision = CORDIC_PRECISION_8CYCLES;
    HAL_CORDIC_Configure(&hcordic, &cordicConfig);
}

int32_t floatToCordic31(float floatingValue)
{
    if (floatingValue > 1.0f)
    {
        floatingValue = 1.0f;
    }
    else if (floatingValue < -1.0f)
    {
        floatingValue = -1.0f;
    }
	return (int32_t)(floatingValue * 0x80000000);;
}

float cordic31ToFloat(int cordic31)
{
	if(cordic31&0x80000000)			//为负数
	{
		cordic31 = cordic31&0x7fffffff;
		return ((float)(cordic31)-0x80000000)/0x80000000;
	}
	else							//为正数
	{
		return (float)(cordic31) / 0x80000000;
	}
}

int32_t dualFloatToCordic15(float valueA, float valueB)
{
    int32_t CORDIC15;
    CORDIC15 = (int32_t)(valueB * 0x8000) << 16;
    CORDIC15 = CORDIC15 | (int32_t)(valueA * 0x8000);
    return CORDIC15;
}

int32_t singleFloatToCordic15(float valueA)
{
    return valueA * 0x8000;
}

void cordic15ToDualFloat(int32_t CORDIC15, float *valueA, float *valueB)
{
    // 处理高16位
    if (CORDIC15 & 0x80000000) // 为负数
    {
        *valueB = ((float)((CORDIC15 >> 16) & 0x7FFF) - 0x8000) / 0x8000;
    }
    else // 为正数
    {
        *valueB = (float)((CORDIC15 >> 16) & 0xFFFF) / 0x8000;
    }
    
    // 处理低16位
    if (CORDIC15&0x8000) // 为负数
    {
        *valueA = ((float)(CORDIC15 & 0x7FFF) - 0x8000) / 0x8000;
    }
    else // 为正数
    {
        *valueA = (float)(CORDIC15 & 0xFFFF) / 0x8000;
    }
}

float cordic15ToSingleFloat(int32_t CORDIC15)
{
    if (CORDIC15&0x8000) // 为负数
    {
        return ((float)(CORDIC15 & 0x7FFF) - 0x8000) / 0x8000;
    }
    else // 为正数
    {
        return (float)(CORDIC15 & 0xFFFF) / 0x8000;
    }
}