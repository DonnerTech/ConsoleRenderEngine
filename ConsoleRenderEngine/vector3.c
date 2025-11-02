#include "vector3.h"

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
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

Vector3 vector3_cross(Vector3 a, Vector3 b) {
    Vector3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}


double vector3_angle(Vector3 a, Vector3 b)
{
    double dot = vector3_dot(a, b);
    double mag_a = vector3_dot(a, a);
    double mag_b = vector3_dot(b, b);

    //Compute the cosine of the angle
    double cos_theta = dot / sqrt(mag_a * mag_b);

    //Clamp value to avoid domain error in acos
    cos_theta = fmax(fmin(cos_theta, 1.0), -1.0);

    return acos(cos_theta);
}

Vector3 vector3_reflect(Vector3 ray_dir, Vector3 normal)
{
    Vector3 result = vector3_subtract(ray_dir, vector3_scale(normal,2 *vector3_dot(ray_dir, normal)));
    return result;
}

Vector3 vector3_random()
{
    Vector3 result = { (double)rand() / HALF_RAND_MAX -1, (double)rand() / HALF_RAND_MAX -1, (double)rand() / HALF_RAND_MAX -1};
    return result;
}
