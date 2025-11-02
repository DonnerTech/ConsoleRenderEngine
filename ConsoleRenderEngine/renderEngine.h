#pragma once
#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <stdio.h> // I/O
#include <stdlib.h> // memory allocation
#include <math.h>
#include <time.h>

#if _WIN32
#include <windows.h> // WIN32 API Features
#endif

#include <omp.h> // Multithreading cross platform

#include <conio.h>   // for _kbhit() and _getch() to pull user input without waiting

#include "quaternion.h"
#include "body.h"
#include "ray.h"
#include "textureLoader.h"
#include "bvh.h"
#include "material.h"

#include "fastTrig.h"

#ifndef _WIN32
typedef unsigned char BYTE;
typedef struct COORD
{
	short X;
	short Y;
} COORD;
#endif // !_WIN32


typedef struct {
	COORD position;
	Texture texture;

} Frame;

double deltaTime;

static void ray_bvh(BYTE RGBAout[4], BVHNode* node, Ray ray, int depth);

int renderer_raytrace(Body* bodies, short* matIDs, Material* mats, int count, Vector3 cameraPos, Quaternion cameraAngle, double fov);

int init(int w, int h);

int userInit();

void resetDeltaTime(void);

void renderFrame(void);

void printfFrameTimes(double targetms);

//end program (cleanup)
void end();

#endif /* RENDER_ENGINE */
