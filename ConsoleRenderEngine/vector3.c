#include "Vector3.h"
#include <math.h>

Vector3 vector3_add(Vector3 a, Vector3 b) {
    Vector3 result = { a.x + b.x, a.y + b.y , a.z + b.z};
    return result;
}

// Vector subtraction
Vector3 vector3_subtract(Vector3 a, Vector3 b) {
    Vector3 result = { a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

// Magnitude (length) of a vector
double vector3_magnitude(Vector3 v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Scalar multiplication
Vector3 vector3_scale(Vector3 v, double scalar) {
    Vector3 result = { v.x * scalar, v.y * scalar, v.z * scalar };
    return result;
}

// Unit vector (normalized vector)
Vector3 vector3_normalize(Vector3 v) {
    double mag = vector3_magnitude(v);
    if (mag == 0) {
        return v;
    }
    else {
        return vector3_scale(v, 1.0f / mag);
    }
}

// Dot product of two vectors
double vector3_dot(Vector3 a, Vector3 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

Vector3 vector3_reflect(Vector3 ray, Vector3 normal)
{
    Vector3 result = vector3_subtract(ray, vector3_scale(normal,2 *vector3_dot(ray, normal)));
    return result;
}