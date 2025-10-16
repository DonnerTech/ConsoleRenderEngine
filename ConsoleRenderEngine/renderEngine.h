#pragma once
#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <stdbool.h> // booleans
#include <stdio.h> // I/O
#include <stdlib.h> // memory allocation
#include <math.h>
#include <time.h>
#include <Windows.h> // Multithreading

#include <conio.h>   // for _kbhit() and _getch() to pull user input without waiting

#include "vector2.h"
#include "vector3.h"
#include "quaternion.h"

#include "body.h"

typedef struct {
	Vector3 origin;
	Vector3 direction;

	Vector3 invdir; // 1.0 / direction

	int sign[3]; // sign for each axis
} Ray;

double deltaTime;

void create_ray(Ray* ray, Vector3 origin, Vector3 direction);

// [depricated]
int renderer_raymarch(Vector3* spheres, double* size, int count, double fov, double maxDepth);

int renderer_raytrace(Body* bodies, Vector3 cameraPos, Quaternion cameraAngle, int count, double fov);

void renderer_unit_tests();

int init(int w, int h);

int userInit();

void resetDeltaTime(void);

void blank();

void renderFrame(void);

void printfFrameTimes(double targetms, int tick);

void end();

#endif /* RENDER_ENGINE */
