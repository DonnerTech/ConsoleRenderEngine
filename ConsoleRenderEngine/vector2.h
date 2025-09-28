#pragma once
#ifndef VECTOR2_H
#define VECTOR2_H

typedef struct {
    double x;
    double y;

} Vector2;

Vector2 vector2_add(Vector2 a, Vector2 b);

Vector2 vector2_subtract(Vector2 a, Vector2 b);

Vector2 vector2_scale(Vector2 v, double scalar);

double vector2_magnitude(Vector2 v);

Vector2 vector2_normalize(Vector2 v);

double vector2_dot(Vector2 a, Vector2 b);

double vector2_cross(Vector2 a, Vector2 b);

double vector2_angle(Vector2 a, Vector2 b);

#endif /* VECTOR_2 */