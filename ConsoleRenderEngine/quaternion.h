#pragma once
#ifndef QUATERNION_H
#define QUATERNION_H

#include <math.h>
#include "vector3.h"
#include "matrix3x3.h"

typedef struct {
    double w; // scalar part (w)
    double x; // x
    double y; // y
    double z; // z
} Quaternion;

// Builders
Quaternion quat_identity(void);
Quaternion quat_from_axis_angle(Vector3 axis, double angle);
Quaternion quat_from_euler(double roll, double pitch, double yaw);

// Operators
Quaternion quat_conjugate(Quaternion q);
double quat_norm(Quaternion q);
Quaternion quat_normalize(Quaternion q);
Quaternion quat_inverse(Quaternion q);
Quaternion quat_multiply(Quaternion q1, Quaternion q2);

// Vector rotation
Vector3 quat_rotate_vector(const Quaternion q, Vector3 vector);

// Integration for physics
Quaternion quat_from_angular_velocity(Vector3 omega, double dt);
Quaternion quat_integrate(Quaternion q, Vector3 omega, double dt);


Matrix3 quat_to_matrix3(Quaternion q);

#endif // QUATERNION_H