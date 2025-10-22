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

	printf("\033[38;2;55;255;0m");
	printf("===BENCHMARKING ENABLED===\n\n");
	printf("\033[0m");

	testRaySphere(1000000);

	testRayBox(1000000);

	testRayPlane(1000000);

	test_trig_tables();

	testBVHtree(10, 25, 1);

	
	for (int i = 0; i < 5; i++)
	{

		testBVHtree(100, 10, 0);

		//testBVHtree(1000, 10, 0);

		testBVHtree(10000, pow(10, i),0);

		//testBVHtree(100000, pow(10, i),0);

		// selection sort took: 51 seconds
		// merge sort took: 0.655 seconds
		// quick sort took: 0.045 seconds
		//testBVHtree(1000000, pow(10, i),0);
	}

	printf("\033[38;2;55;255;0m");
	printf("===BENCHMARKS COMPLETE====\n\n");
	printf("\033[0m");

#endif // _BENCHMARK

	system("pause");

	physics_test();

	return 0;
}