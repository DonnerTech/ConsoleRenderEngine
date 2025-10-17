#pragma warning(disable : 4996).

#include "physicsTest.h"

#include "textureLoader.h"

int main(void)
{
	// seed the random number generator
	srand((unsigned int)time(NULL));

	printf("Welcome to CoralEngine. The Console based game engine.\n\n");

	system("pause");

	physics_test();

	return 0;
}