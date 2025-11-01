#pragma once
#ifndef VECTOR3_H
#define VECTOR3_H

#include <math.h>
#include "fastTrig.h"
#include <stdlib.h>

#define HALF_RAND_MAX (RAND_MAX / 2.0)

typedef struct Vector3 {
    double x;
    double y;
    double z;

} Vector3;

inline Vector3 vector3_add(Vector3 a, Vector3 b) {
    return (Vector3) { a.x + b.x, a.y + b.y, a.z + b.z };
}

// Vector subtraction
inline Vector3 vector3_subtract(Vector3 a, Vector3 b) {
    return (Vector3) { a.x - b.x, a.y - b.y, a.z - b.z };
}

// Magnitude (length) of a vector
inline double vector3_magnitude(Vector3 v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Scalar multiplication
inline Vector3 vector3_scale(Vector3 v, double scalar) {
    return (Vector3){ v.x * scalar, v.y * scalar, v.z * scalar };

}

Vector3 vector3_normalize(Vector3 v);

double vector3_dot(Vector3 a, Vector3 b);

Vector3 vector3_cross(Vector3 a, Vector3 b);

double vector3_angle(Vector3 a, Vector3 b);

Vector3 vector3_reflect(Vector3 ray_dir, Vector3 normal);

Vector3 vector3_random(void);

inline Vector3 vector3_min(Vector3 a, Vector3 b)
{
    return (Vector3) { min(a.x, b.x), min(a.y, b.y), min(a.z, b.z) };
}

inline Vector3 vector3_max(Vector3 a, Vector3 b)
{
    return (Vector3) { max(a.x, b.x), max(a.y, b.y), max(a.z, b.z) };
}


#endif /* VECTOR_3 */