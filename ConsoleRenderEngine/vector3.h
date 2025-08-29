#pragma once
#ifndef VECTOR_3
#define VECTOR_3

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

#endif /* VECTOR_3 */