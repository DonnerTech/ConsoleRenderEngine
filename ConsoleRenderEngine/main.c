#define CRT_SECURE_NO_WARNINGS

//#include "physicsTest.h"

#include "craps.h"

int main(void)
{
	// seed the random number generator
	srand((unsigned int)time(NULL));

	// run the craps game
	craps_game();

	double bankBalance = 0;


}