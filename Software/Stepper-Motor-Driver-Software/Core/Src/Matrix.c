/**
 * @file Matrix.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "Matrix.h"

Matrix2x2_t Matrix2x2Zero = {
    .array = {
        {0.0f, 0.0f},
        {0.0f, 0.0f}
    }
};

Matrix2x2_t Matrix2x2Identity = {
    .array = {
        {1.0f, 0.0f},
        {0.0f, 1.0f}
    }
};

void Matrix2x2Copy(Matrix2x2_t *result, Matrix2x2_t *a)
{
    result->element.a11 = a->element.a11;
    result->element.a12 = a->element.a12;
    result->element.a21 = a->element.a21;
    result->element.a22 = a->element.a22;
}

void Matrix2x2Add(Matrix2x2_t *result, Matrix2x2_t *a, Matrix2x2_t *b)
{
    result->element.a11 = a->element.a11 + b->element.a11;
    result->element.a12 = a->element.a12 + b->element.a12;
    result->element.a21 = a->element.a21 + b->element.a21;
    result->element.a22 = a->element.a22 + b->element.a22;
}

void Matrix2x1Add(Matrix2x1_t *result, Matrix2x1_t *a, Matrix2x1_t *b)
{
    result->element.a11 = a->element.a11 + b->element.a11;
    result->element.a21 = a->element.a21 + b->element.a21;
}

void Matrix2x2Sub(Matrix2x2_t *result, Matrix2x2_t *a, Matrix2x2_t *b)
{
    result->element.a11 = a->element.a11 - b->element.a11;
    result->element.a12 = a->element.a12 - b->element.a12;
    result->element.a21 = a->element.a21 - b->element.a21;
    result->element.a22 = a->element.a22 - b->element.a22;
}

void Matrix2x1Sub(Matrix2x1_t *result, Matrix2x1_t *a, Matrix2x1_t *b)
{
    result->element.a11 = a->element.a11 - b->element.a11;
    result->element.a21 = a->element.a21 - b->element.a21;
}

void Matrix2x2Scale(Matrix2x2_t *result, Matrix2x2_t *a, float scale)
{
    result->element.a11 = a->element.a11 * scale;
    result->element.a12 = a->element.a12 * scale;
    result->element.a21 = a->element.a21 * scale;
    result->element.a22 = a->element.a22 * scale;
}

void Matrix2x1Scale(Matrix2x1_t *result, Matrix2x1_t *a, float scale)
{
    result->element.a11 = a->element.a11 * scale;
    result->element.a21 = a->element.a21 * scale;
}

void Matrix2x2MulMatrix2x1(Matrix2x1_t *result, Matrix2x2_t *a, Matrix2x1_t *b)
{
    // 需要处理 result 和 b 指向同一地址的情况
    float temp = b->element.a11;
    result->element.a11 = a->element.a11 * b->element.a11 + a->element.a12 * b->element.a21;
    result->element.a21 = a->element.a21 * temp           + a->element.a22 * b->element.a21;
}

void Matrix2x2MulMatrix2x2(Matrix2x2_t *result, Matrix2x2_t *a, Matrix2x2_t *b)
{
    // 需要处理 result 和 a 或者 b 指向同一地址的情况
    float temp_a_a11 = a->element.a11;
    float temp_b_a11 = b->element.a11;
    float temp_b_a12 = b->element.a12;
    float temp_a_a21 = a->element.a21;

    result->element.a11 = a->element.a11 * b->element.a11 + a->element.a12 * b->element.a21;
    result->element.a12 = temp_a_a11     * b->element.a12 + a->element.a12 * b->element.a22;
    result->element.a21 = a->element.a21 * temp_b_a11     + a->element.a22 * b->element.a21;
    result->element.a22 = temp_a_a21     * temp_b_a12     + a->element.a22 * b->element.a22;
}

float Matrix2x2Determinant(Matrix2x2_t *a)
{
    return a->element.a11 * a->element.a22 - a->element.a12 * a->element.a21;
}

void Matrix2x2Transpose(Matrix2x2_t *result, Matrix2x2_t *a)
{
    // 需要处理 result 和 a 指向同一地址的情况
    float temp = a->element.a12;
    result->element.a11 = a->element.a11;
    result->element.a12 = a->element.a21;
    result->element.a21 = temp;
    result->element.a22 = a->element.a22;
}

void Matrix2x2Inverse(Matrix2x2_t *result, Matrix2x2_t *a)
{
    float det = Matrix2x2Determinant(a);
    // if(det == 0)
    // {
    //     result->element.a11 = 0;
    //     result->element.a12 = 0;
    //     result->element.a21 = 0;
    //     result->element.a22 = 0;
    //     return;
    // }

    // 需要处理 result 和 a 指向同一地址的情况
    float temp = a->element.a11;
    result->element.a11 = a->element.a22 / det;
    result->element.a12 = -a->element.a12 / det;
    result->element.a21 = -a->element.a21 / det;
    result->element.a22 = temp / det;
}

// void MatrixTester()
// {
//     static volatile Matrix2x2_t a = {1.1f, 2.2f, 3.3f, 4.4f};
//     static volatile Matrix2x2_t b = {1.4f, 2.3f, 3.2f, 4.1f};
//     static volatile Matrix2x2_t result = {0};
//     static volatile Matrix2x1_t result21 = {0};
//     static volatile float det = 0;

//     Matrix2x2Add(&result, &a, &b);
//     Matrix2x2Sub(&result, &a, &b);
//     Matrix2x2Scale(&result, &a, 2.0f);
//     Matrix2x2MulMatrix2x1(&result21, &a, (Matrix2x1_t *)&b);
//     Matrix2x2MulMatrix2x2(&result, &a, &b);
//     det = Matrix2x2Determinant(&a);
//     Matrix2x2Transpose(&result, &a);
//     Matrix2x2Inverse(&result, &a);

//     Matrix2x2MulMatrix2x2(&result, &a, &b);
//     Matrix2x2Copy(&result, &a);
//     Matrix2x2MulMatrix2x2(&result, &result, &b);
//     Matrix2x2Copy(&result, &b);
//     Matrix2x2MulMatrix2x2(&result, &a, &result);
// }