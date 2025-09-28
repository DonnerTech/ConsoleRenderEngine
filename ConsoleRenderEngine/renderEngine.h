#pragma once
#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <stdbool.h> // booleans
#include <stdio.h> // I/O
#include <stdlib.h> // memory allocation
#include <math.h>
#include <time.h>
#include <Windows.h>

#include <conio.h>   // for _kbhit() and _getch() to pull user input without waiting

#include "vector2.h"
#include "vector3.h"

double deltaTime;

void polarToEuler(double r, double theta, double* x, double* y);

void rotatingTriangleDemo(int tick);

void drawTriangleToArray(double x1, double y1, double x2, double y2, double x3, double y3, char c);

void fsRayTrace(Vector3* spheres, double* size, int count, double fov, double maxDepth);

int fsRayTraceMultithreaded(Vector3* spheres, double* size, int count, double fov, double maxDepth);

void renderer_unit_tests();

int init();

void blank();

void render(double targetms, int tick);

void end();

#endif /* RENDER_ENGINE */
