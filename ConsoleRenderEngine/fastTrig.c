#include "fastTrig.h"

void init_trig_tables(void) {
	for (int i = 0; i < TABLE_SIZE; i++) {
		float angle = i * PI / HALF_TABLE_SIZE; // Convert degrees to radians
		sin_table[i] = sin(angle);
		cos_table[i] = cos(angle);
	}
}

#if _DEBUG

void test_trig_tables(void)
{
	init_trig_tables();

	clock_t trig_start = clock();
	double num = 0;
	for (int i = 0; i < 1000000000; i++)
	{
		num += cos(i);
	}
	printf("1000000000 cos time: %d\n", clock() - trig_start);
	printf("num: %0.2lf\n", num);

	trig_start = clock();
	num = 0;
	for (int i = 0; i < 1000000000; i++)
	{
		num += fast_cos(i);
	}
	printf("1000000000 fast_cos time: %d\n", clock() - trig_start);
	printf("num: %0.2lf\n", num);
	system("pause");
}


#endif // _DEBUG
