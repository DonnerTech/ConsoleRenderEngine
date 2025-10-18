#pragma once
#ifndef FAST_TRIG_H
#define FAST_TRIG_H

#if _DEBUG

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void test_trig_tables(void);

#endif // _DEBUG

#include <math.h>

#define PI 3.14159
#define TWO_PI 6.28318

#define TABLE_SIZE 360*4
#define HALF_TABLE_SIZE (TABLE_SIZE / 2)

float sin_table[TABLE_SIZE];
float cos_table[TABLE_SIZE];

void init_trig_tables(void);

// Lookup table for sin
inline float fast_sin(double rad) {
	return sin_table[(int)((rad < 0) + fmod(rad * HALF_TABLE_SIZE / PI, TABLE_SIZE))];
}

// Lookup table for cos
inline float fast_cos(double rad) {
	return cos_table[(int)((rad < 0) + fmod(rad * HALF_TABLE_SIZE / PI, TABLE_SIZE))];
}

#endif // FAST_TRIG_H
