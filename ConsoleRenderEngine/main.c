#pragma warning(disable : 4996).

#include <stdbool.h> // booleans
#include <stdio.h> // I/O
#include <stdlib.h> // memory allocation
#include <math.h>
#include <time.h>

#include <conio.h>   // for _kbhit() and _getch() to pull user input without waiting

#include "renderEngine.h" // custom askii renderer
#include "vector.h"

//int* entityIDs;

void draw(int tick)
{
	double speed = 50.0;

	Vector2 vecA;
	Vector2 vecB;
	Vector2 vecC;
	Vector2 vecD;
	Vector2 pos;
	pos.x = (sin(tick / 200.0) + 1) / 8.0;
	pos.y = 0.0;
	double width = PI/4.0, length = 0.25 * pos.x;

	polarToEuler(pos.x, pos.y + (tick / speed), &vecA.x, &vecA.y);
	polarToEuler(pos.x + length, pos.y + (tick / speed) + width, &vecB.x, &vecB.y);

	polarToEuler(pos.x + length, pos.y + (tick / speed), &vecC.x, &vecC.y);
	polarToEuler(pos.x, pos.y + (tick / speed) + width, &vecD.x, &vecD.y);

	//char character = 32 + (int)((double)rand() / RAND_MAX * 32);
	char character = 'X';

	drawTriangleToArray(vecA.x, vecA.y,
		vecB.x, vecB.y,
		vecC.x, vecC.y, character);

	drawTriangleToArray(vecA.x, vecA.y,
		vecD.x, vecD.y,
		vecB.x, vecB.y, character);
}

int main(void)
{
	int code = init();
	if (code != 0)
	{
		return code;
	}

	bool isRunning = true;
	int tick = 0;
	// render loop
	while (isRunning)
	{
		blank(); // clear screen

		//update things
		draw(tick);

		render(15,tick); //16 = 60fps, 32 = 30fps

		tick++;
		// check if a key was pressed to exit the loop (not waiting)
		if (_kbhit())
			isRunning = false;
	}
	end();

	return 0;
}