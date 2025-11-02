#pragma once
#ifndef RAY_H
#define RAY_H

#include "vector3.h"
#include "body.h"

typedef struct {
	Vector3 origin;
	Vector3 direction;

	Vector3 invdir; // 1.0 / direction

	int sign[3]; // sign for each axis
} Ray;


#define NO_HIT -1

//fields:
//	double dist
//	int hit_id
//	Vector3 position
//	Vector3 localPosition
//	Vector3 normal
typedef struct RayHit {
	double dist;
	int hit_id;
	Vector3 position;
	Vector3 localPosition;
	Vector3 normal;
} RayHit;

void create_ray(Ray* ray, Vector3 origin, Vector3 direction);

int ray_aabb(Ray ray, Vector3 min, Vector3 max, double tmax_limit, double* dist_ptr);

RayHit intersectBody(Body body, int id, Ray ray);

#endif // RAY_H

