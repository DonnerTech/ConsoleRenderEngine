#pragma once
#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <stdio.h> // I/O
#include <stdlib.h> // memory allocation
#include <math.h>
#include <time.h>
#include <windows.h> // Multithreading

#include <conio.h>   // for _kbhit() and _getch() to pull user input without waiting

#include "quaternion.h"
#include "body.h"
#include "ray.h"
#include "textureLoader.h"
#include "bvh.h"

#include "fastTrig.h"

typedef struct {
	COORD position;
	Texture texture;

} Frame;

double deltaTime;

int renderer_raytrace(Body* bodies, int* textureIDs, Texture* textures, int count, Vector3 cameraPos, Quaternion cameraAngle, double fov);

int renderer_raytrace_b(Body* bodies, int* textureIDs, Texture* textures, int count, Vector3 cameraPos, Quaternion cameraAngle, double fov);

int init(int w, int h);

int userInit();

void resetDeltaTime(void);

void renderFrame(void);

void printfFrameTimes(double targetms);

void end();

#endif /* RENDER_ENGINE */
