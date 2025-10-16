#pragma warning(disable : 4996).

#include "physicsTest.h"

int main(void)
{
	// seed the random number generator
	srand((unsigned int)time(NULL));

	printf("Welcome to RenPhyGame. The Console based game engine.\n \n");

	physics_test();
}