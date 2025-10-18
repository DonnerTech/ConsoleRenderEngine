#define _CRT_SECURE_NO_WARNINGS

#include "physicsTest.h"

int main(void)
{
	// seed the random number generator
	srand((unsigned int)time(NULL));

	printf("Welcome to CoralEngine. The Console based game engine.\n\n");

	system("pause");

	physics_test();

	return 0;
}