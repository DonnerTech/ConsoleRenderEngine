#pragma once
#ifndef BVH_H
#define BVH_H

#include <math.h>
#include "vector3.h"
#include "body.h"


// for quantizing spatial positions into virtual cells
#define QUANTIZE(min, center, max) (unsigned int)((center - min) / (max - min) * (1024 - 1))

typedef struct {
	Vector3 min;
	Vector3 max;
} Bounds;

typedef struct BVHNode {
	Bounds bounds; // the nodes bounding volume
	unsigned int id; // the id of the object or -1 if a branch node

	// children of the node
	struct BVHNode* left_ptr;
	struct BVHNode* right_ptr;
} BVHNode;

typedef struct {
	unsigned int id;
	unsigned int mortonCode;
} MortonIDPair;

Bounds BVH_calculateBounds(Body* body_ptr);

void BVH_sortByMortonCodes(MortonIDPair* mortonIDpair_list, int count);

int BVH_getSplitPos(MortonIDPair* mortonIDpair_list, int begin, int end);

BVHNode* BVH_createTree(Body* body_list, int count);

BVHNode* BVH_createSubTree(MortonIDPair* mortonIDpair_list, int begin, int end);

// Spread the lower 10 bits of a so there are 2 zeros between each bit
static inline unsigned int BVH_expandBits(unsigned int a)
{
	a = (a * 0x00010001u) & 0xFF0000FFu;
	a = (a * 0x00000101u) & 0x0F00F00Fu;
	a = (a * 0x00000011u) & 0xC30C30C3u;
	a = (a * 0x00000005u) & 0x49249249u;
	return a;
}

// Combine expanded bits of x, y, z into a single Morton code
static inline unsigned int BVH_mortonCodeGen(unsigned int x, unsigned int y, unsigned int z)
{
	// interleaving the bits for sorting
	return (BVH_expandBits(x) << 2) | (BVH_expandBits(y) << 1) | BVH_expandBits(z);
}

static inline Bounds Bounds_union(Bounds a, Bounds b)
{
	return (Bounds) {
		vector3_min(a.min, b.min),
		vector3_max(a.max, b.max)
	};
}

#endif //BVH_H
