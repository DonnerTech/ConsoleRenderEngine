#pragma once
#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <stdbool.h> // booleans
#include <stdio.h> // I/O
#include <stdlib.h> // memory allocation
#include <math.h>
#include <time.h>
#include <windows.h> // Multithreading

#include <conio.h>   // for _kbhit() and _getch() to pull user input without waiting

#include "quaternion.h"

#include "body.h"

#include "textureLoader.h"

#include "fastTrig.h"

typedef struct {
	COORD position;
	Texture texture;

} Frame;

typedef struct {
	Vector3 origin;
	Vector3 direction;

	Vector3 invdir; // 1.0 / direction

	int sign[3]; // sign for each axis
} Ray;

double deltaTime;

void create_ray(Ray* ray, Vector3 origin, Vector3 direction);

bool raySphereIntersection(Body sphere, Ray ray, double* dist_ptr);

bool rayBoxIntersection(Body box, Ray ray, double* dist_ptr, Vector3* localHitPoint);

bool rayPlaneIntersection(Body plane, Ray ray, double* dist_ptr, Vector3* localHitPoint);

int renderer_raytrace(Body* bodies, int* textureIDs, Texture* textures, int count, Vector3 cameraPos, Quaternion cameraAngle, double fov);

int init(int w, int h);

int userInit();

void resetDeltaTime(void);

void renderFrame(void);

void printfFrameTimes(double targetms);

void end();

#endif /* RENDER_ENGINE */
