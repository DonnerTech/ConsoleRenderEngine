#pragma once
#ifndef MATRIX3X3_H
#define MATRIX3X3_H

#include "vector3.h"

typedef struct {
    double m[3][3];
} Matrix3; // For inertia tensors & rotations

Vector3 matrix3x3_mul_vector3(Vector3 a, Matrix3 M);

Matrix3 matrix3x3_transpose(Matrix3 M);

Matrix3 matrix3x3_mul(Matrix3 M1, Matrix3 M2);

#endif
