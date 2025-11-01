#pragma once
#ifndef RE_PTESTS_H
#define RE_PTESTS_H

#include "renderEngine.h"
#include "bvh.h"
#include "ray.h"

void testRaySphere(int iter);

void testRayBox(int iter);

void testRayPlane(int iter);

void testBVHtree(int bodyCount, double scatter, int print);

#endif // RE_PTESTS_H
