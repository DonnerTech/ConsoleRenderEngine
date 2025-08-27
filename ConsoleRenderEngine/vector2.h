#include <math.h>
#pragma warning(disable : 4996).
#ifndef VECTOR_2
#define VECTOR_2

typedef struct {
    double x;
    double y;

} Vector2;

// Vector addition
Vector2 vector_add(Vector2 a, Vector2 b) {
    Vector2 result = { a.x + b.x, a.y + b.y };
    return result;
}

// Vector subtraction
Vector2 vector_subtract(Vector2 a, Vector2 b) {
    Vector2 result = { a.x - b.x, a.y - b.y };
    return result;
}

// Scalar multiplication
Vector2 vector_scale(Vector2 v, double scalar) {
    Vector2 result = { v.x * scalar, v.y * scalar };
    return result;
}

// Magnitude (length) of a vector
double vector_magnitude(Vector2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

// Unit vector (normalized vector)
Vector2 vector_normalize(Vector2 v) {
    double mag = vector_magnitude(v);
    if (mag == 0) {
        return v;
    }
    else {
        return vector_scale(v, 1.0f / mag);
    }
}

// Dot product of two vectors
double vector_dot(Vector2 a, Vector2 b) {
    return (a.x * b.x) + (a.y * b.y);
}

// Cross product of two 2D vectors (results in a scalar)
double vector_cross(Vector2 a, Vector2 b) {
    return (a.x * b.y) - (a.y * b.x);
}

// (returns angle in radians between -PI and PI)
double vector_angle(Vector2 a, Vector2 b) {
    return atan2(vector_cross(a,b), vector_dot(a,b));
}


#endif /* VECTOR_2 */
