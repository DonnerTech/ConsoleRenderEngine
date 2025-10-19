#define _CRT_SECURE_NO_WARNINGS

#include "physicsTest.h"

#if _BENCHMARK || _DEBUG
#include "renderEngine_performanceTests.h"
#endif

int main(void)
{
	// seed the random number generator
	srand((unsigned int)time(NULL));

	//init trig tables
	init_trig_tables();

	printf("Welcome to CoralEngine. The Console based game engine.\n\n");

#if _DEBUG
	printf("\033[38;2;255;55;0m");
	printf("WARNING! YOU ARE RUNNING IN DEBUG MODE!\nRUN IN RELEASE MODE FOR COMPILER OPTIMIZATIONS!\n\n");
	printf("\033[0m");
#endif // _DEBUG

#if _BENCHMARK || _DEBUG
	testRaySphere(1000000);

	testRayBox(1000000);

	testRayPlane(1000000);

	test_trig_tables();
#endif // _BENCHMARK

	system("pause");

	physics_test();

	return 0;
}