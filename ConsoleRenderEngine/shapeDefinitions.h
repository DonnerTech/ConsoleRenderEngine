#pragma once
#ifndef SHAPE_DEFINITIONS_H
#define SHAPE_DEFINITIONS_H

#include "vector3.h"

typedef enum {
	SHAPE_SPHERE,
	SHAPE_BOX,
	SHAPE_PLANE
} ShapeType;

typedef struct {
	double radius;
} SphereShape;

typedef struct {
	Vector3 half_extents; // half sizes (x,y,z)
} BoxShape;

typedef struct {
	Vector3 normal; // unit vector
	double offset;  // distance from origin along normal
} PlaneShape;

typedef struct {
	ShapeType type;
	union {
		SphereShape sphere;
		BoxShape box;
		PlaneShape plane;
	};
} CollisionShape;

#endif