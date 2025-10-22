#pragma once
#ifndef RAY_H
#define RAY_H

#include "vector3.h"

typedef struct {
	Vector3 origin;
	Vector3 direction;

	Vector3 invdir; // 1.0 / direction

	int sign[3]; // sign for each axis
} Ray;

void create_ray(Ray* ray, Vector3 origin, Vector3 direction);

int ray_aabb(Ray ray, Vector3 pos, Vector3 half_extents);

#endif // RAY_H

