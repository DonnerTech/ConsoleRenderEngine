#pragma once
#ifndef MATRIX3X3_H
#define MATRIX3X3_H

#include "vector3.h"

typedef struct {
    double m[3][3];
} Matrix3x3; // For inertia tensors & rotations


Vector3 vector3_mul_matrix3x3(Vector3 a, Matrix3x3 M);

Matrix3x3 matrix3x3_transpose(Matrix3x3 M);

Matrix3x3 matrix3x3_mul(Matrix3x3 M1, Matrix3x3 M2);

#endif
