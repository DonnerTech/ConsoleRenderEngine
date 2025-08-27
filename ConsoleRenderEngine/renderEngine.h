#pragma warning(disable : 4996).

#include <stdbool.h> // booleans
#include <stdio.h> // I/O
#include <stdlib.h> // memory allocation
#include <math.h>
#include <time.h>

#include <conio.h>   // for _kbhit() and _getch() to pull user input without waiting

#include "vector2.h"
#include "vector3.h"

#define PI 3.14159
#define TWO_PI 6.28318

const double TRIANGLE_SIZE = 0.45;
const double TRIANGLE_POS_X = 0;
const double TRIANGLE_POS_Y = 0;

int width, height;
char* renderArray;

clock_t executiontimeStart;

//create the new frame in a buffer and push it to the console in one call.
void printArray()
{
	// Each cell is (2 chars) + newline per row + null terminator
	size_t bufferSize = (width * 2 + 1) * height + 1;
	char* buffer = (char*)malloc(bufferSize);
	if (!buffer) return;

	char* ptr = buffer;

	for (int i = 0; i < width * height; i++)
	{
		ptr += sprintf(ptr, "%c ", renderArray[i]);

		if (i % width == width - 1)
			ptr += sprintf(ptr, "\n");
	}

	*ptr = '\0'; // Null terminate

	system("cls");
	printf("%s", buffer);

	free(buffer);
}

double area(double x1, double y1, double x2, double y2, double x3, double y3) {
	return fabs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0f);
}

// The point is inside the triangle if the sum of areas of sub-triangles (PAB, PBC, PCA) equals the area of ABC.
bool pointInTriangle(double px, double py,
	double x1, double y1,
	double x2, double y2,
	double x3, double y3)
{
	double A = area(x1, y1, x2, y2, x3, y3);
	double A1 = area(px, py, x2, y2, x3, y3);
	double A2 = area(x1, y1, px, py, x3, y3);
	double A3 = area(x1, y1, x2, y2, px, py);

	return fabs(A - (A1 + A2 + A3)) < 1e-4; // allow for some error
}

void polarToEuler(double r, double theta, double* x, double* y) {
	*x = r * cos(theta);
	*y = r * sin(theta);
}

void rotatingTriangleDemo(int tick)
{
	double x1, y1;
	double x2, y2;
	double x3, y3;

	double rot = (double)tick / 100;
	polarToEuler(TRIANGLE_SIZE, rot, &x1, &y1);
	rot += (TWO_PI / 3.0);
	polarToEuler(TRIANGLE_SIZE, rot, &x2, &y2);
	rot += (TWO_PI / 3.0);
	polarToEuler(TRIANGLE_SIZE, rot, &x3, &y3);

	x1 += TRIANGLE_POS_X;
	x2 += TRIANGLE_POS_X;
	x3 += TRIANGLE_POS_X;

	y1 += TRIANGLE_POS_Y;
	y2 += TRIANGLE_POS_Y;
	y3 += TRIANGLE_POS_Y;

	for (int i = 0; i < width * height; i++)
	{

		if (pointInTriangle(i % width, i / width,
			x1 * width + width / 2, y1 * height + height / 2,
			x2 * width + width / 2, y2 * height + height / 2,
			x3 * width + width / 2, y3 * height + height / 2))
		{
			renderArray[i] = '#';
		}

	}
}

void drawTriangleToArray(double x1, double y1, double x2, double y2, double x3, double y3, char c)
{

	x1 += TRIANGLE_POS_X;
	x2 += TRIANGLE_POS_X;
	x3 += TRIANGLE_POS_X;

	y1 += TRIANGLE_POS_Y;
	y2 += TRIANGLE_POS_Y;
	y3 += TRIANGLE_POS_Y;

	for (int i = 0; i < width * height; i++)
	{


		if (pointInTriangle(i % width, i / width,
			x1 * width + width / 2, y1 * height + height / 2,
			x2 * width + width / 2, y2 * height + height / 2,
			x3 * width + width / 2, y3 * height + height / 2))
		{
			renderArray[i] = c;
		}

	}
}

char traceRay(int* pos, int* size, Vector3 position, Vector3 direction, double maxDistance)
{

}

void fsRayTrace(int *pos, int *size)
{
	for (int i = 0; i < width * height; i++)
	{
		Vector3 rayDir;
		Vector3 rayPos;
		renderArray[i] = traceRay(pos, size, rayPos, rayDir, 100);
	}
}

int init()
{

	// get the users input
	printf("Hello. \nEnter the canvas width: ");
	scanf("%d", &width);
	printf("Enter the canvas height: ");
	scanf("%d", &height);

	if (width <= 0 || height <= 0)
	{
		printf("Invalid Dimensions\n");
		return 1;
	}

	printf("Width: %d \n", width);
	printf("Height: %d \n", height);

	printf("Array memory size: %lld bytes \n", sizeof(renderArray) * width * height);

	printf("\nPress enter to begin rendering: ");


	// wait for user input to contine
	_getch();

	// dynamically allocates the memory for an array of boolians the size of the output display.
	// calloc initalizes all the references to zero.
	renderArray = (char*)calloc(width * height, sizeof(char));

	if (renderArray == NULL)
	{
		printf("Memory Allocation Error\n");
		return 2;
	}

	renderArray[width / 2] = '#'; // half width point
	renderArray[height / 2 * width] = '#'; // half height point
	renderArray[height / 2 * width + width / 2] = '#'; // center point

	return 0;
}

void blank()
{
	executiontimeStart = clock(); // mesure frame times

	for (int i = 0; i < width * height; i++)
	{
		renderArray[i] = '.';
	}
}

void render(double targetms,int tick)
{
	printArray();

	clock_t executiontimeEnd = clock();   // End timing
	double time_elapsed = (double)(executiontimeEnd - executiontimeStart); // Calculate elapsed frametime
	printf("Execution time: %f milliseconds\n", time_elapsed);

	double frameTime = targetms < time_elapsed ? time_elapsed : targetms;
	printf("Frame time: %f milliseconds\n", frameTime);

	if (time_elapsed < targetms)
		_sleep(targetms - (int)time_elapsed); // 16ms per frame = 60 fps
}

void end()
{
	//end program (cleanup)
	free(renderArray);
}
