#pragma once
#ifndef VECTOR_3
#define VECTOR_3

#include <math.h>
#include <stdlib.h>

#define HALF_RAND_MAX (RAND_MAX / 2.0)

typedef struct {
    double x;
    double y;
    double z;

} Vector3;

Vector3 vector3_add(Vector3 a, Vector3 b);

Vector3 vector3_subtract(Vector3 a, Vector3 b);

double vector3_magnitude(Vector3 v);

Vector3 vector3_scale(Vector3 v, double scalar);

Vector3 vector3_normalize(Vector3 v);

double vector3_dot(Vector3 a, Vector3 b);

double vector3_angle(Vector3 a, Vector3 b);

Vector3 vector3_reflect(Vector3 ray, Vector3 normal);

Vector3 vector3_random();

#endif /* VECTOR_3 */