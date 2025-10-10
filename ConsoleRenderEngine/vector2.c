#include "vector2.h"

// Vector addition
Vector2 vector2_add(Vector2 a, Vector2 b) {
    Vector2 result = { a.x + b.x, a.y + b.y };
    return result;
}

// Vector subtraction
Vector2 vector2_subtract(Vector2 a, Vector2 b) {
    Vector2 result = { a.x - b.x, a.y - b.y };
    return result;
}

// Scalar multiplication
Vector2 vector2_scale(Vector2 v, double scalar) {
    Vector2 result = { v.x * scalar, v.y * scalar };
    return result;
}

// Magnitude (length) of a vector
double vector2_magnitude(Vector2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

// Unit vector (normalized vector)
Vector2 vector2_normalize(Vector2 v) {
    double mag = vector2_magnitude(v);
    if (mag == 0) {
        return v;
    }
    else {
        return vector2_scale(v, 1.0f / mag);
    }
}

// Dot product of two vectors
double vector2_dot(Vector2 a, Vector2 b) {
    return (a.x * b.x) + (a.y * b.y);
}

// Cross product of two 2D vectors (results in a scalar)
double vector2_cross(Vector2 a, Vector2 b) {
    return (a.x * b.y) - (a.y * b.x);
}

// (returns angle in radians between -PI and PI)
double vector2_angle(Vector2 a, Vector2 b) {
    return atan2(vector2_cross(a, b), vector2_dot(a, b));
}