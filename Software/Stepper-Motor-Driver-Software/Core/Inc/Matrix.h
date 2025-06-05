/**
 * @file Matrix.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "main.h"
#include "GeneralConfig.h"

#include "stdint.h"

typedef union {
    float array[2][2];

    struct {
        float a11;
        float a12;
        float a21;
        float a22;
    } element;
} Matrix2x2_t;

typedef union {
    float array[2];

    struct {
        float a11;
        float a21;
    } element;
} Matrix2x1_t;

extern Matrix2x2_t Matrix2x2Zero;
extern Matrix2x2_t Matrix2x2Identity;

/**
 * @brief Copies a 2x2 matrix.
 * 
 * @param result Pointer to the result matrix where the copy will be stored.
 * @param a Pointer to the input matrix to be copied.
 */
void Matrix2x2Copy(Matrix2x2_t *result, Matrix2x2_t *a);

/**
 * @brief Adds two 2x2 matrices.
 * 
 * @param result Pointer to the result matrix where the sum will be stored.
 * @param a Pointer to the first input matrix.
 * @param b Pointer to the second input matrix.
 */
void Matrix2x2Add(Matrix2x2_t *result, Matrix2x2_t *a, Matrix2x2_t *b);


/**
 * @brief Adds two 2x1 matrices.
 * 
 * @param result Pointer to the result matrix where the sum will be stored.
 * @param a Pointer to the first input matrix.
 * @param b Pointer to the second input matrix.
 */
void Matrix2x1Add(Matrix2x1_t *result, Matrix2x1_t *a, Matrix2x1_t *b);

/**
 * @brief Subtracts two 2x2 matrices.
 * 
 * @param result Pointer to the result matrix where the difference will be stored.
 * @param a Pointer to the first input matrix.
 * @param b Pointer to the second input matrix.
 */
void Matrix2x2Sub(Matrix2x2_t *result, Matrix2x2_t *a, Matrix2x2_t *b);

/**
 * @brief Subtracts two 2x1 matrices.
 * 
 * @param result Pointer to the result matrix where the difference will be stored.
 * @param a Pointer to the first input matrix.
 * @param b Pointer to the second input matrix.
 */
void Matrix2x1Sub(Matrix2x1_t *result, Matrix2x1_t *a, Matrix2x1_t *b);

/**
 * @brief Scales a 2x2 matrix by a scalar value.
 * 
 * @param result Pointer to the result matrix where the scaled matrix will be stored.
 * @param a Pointer to the input matrix to be scaled.
 * @param scale The scalar value by which to scale the matrix.
 */
void Matrix2x2Scale(Matrix2x2_t *result, Matrix2x2_t *a, float scale);


/**
 * @brief Scales a 2x1 matrix by a scalar value.
 * 
 * @param result Pointer to the result matrix where the scaled matrix will be stored.
 * @param a Pointer to the input matrix to be scaled.
 * @param scale The scalar value by which to scale the matrix.
 */
void Matrix2x1Scale(Matrix2x1_t *result, Matrix2x1_t *a, float scale);

/**
 * @brief Multiplies a 2x2 matrix by a 2x1 matrix.
 * 
 * @param result Pointer to the result matrix where the product will be stored.
 * @param a Pointer to the 2x2 matrix.
 * @param b Pointer to the 2x1 matrix.
 */
void Matrix2x2MulMatrix2x1(Matrix2x1_t *result, Matrix2x2_t *a, Matrix2x1_t *b);

/**
 * @brief Multiplies two 2x2 matrices.
 * 
 * @param result Pointer to the result matrix where the product will be stored.
 * @param a Pointer to the first input matrix.
 * @param b Pointer to the second input matrix.
 */
void Matrix2x2MulMatrix2x2(Matrix2x2_t *result, Matrix2x2_t *a, Matrix2x2_t *b);

/**
 * @brief Calculates the determinant of a 2x2 matrix.
 * 
 * @param a Pointer to the input matrix.
 * @return The determinant of the input matrix.
 */
float Matrix2x2Determinant(Matrix2x2_t *a);

/**
 * @brief Transposes a 2x2 matrix.
 * 
 * @param result Pointer to the result matrix where the transpose will be stored.
 * @param a Pointer to the input matrix to be transposed.
 */
void Matrix2x2Transpose(Matrix2x2_t *result, Matrix2x2_t *a);

/**
 * @brief Calculates the inverse of a 2x2 matrix.
 * 
 * @param result Pointer to the result matrix where the inverse will be stored.
 * @param a Pointer to the input matrix.
 */
void Matrix2x2Inverse(Matrix2x2_t *result, Matrix2x2_t *a);


void MatrixTester();