/* 
* Author: DonnerTech
* 
* Reference material: 
* https://matthias-research.github.io/pages/tenMinutePhysics/24-morton.pdf
* https://learn.microsoft.com/en-us/cpp/intrinsics/lzcnt16-lzcnt-lzcnt64?view=msvc-170
* https://www.programiz.com/dsa/radix-sort
* https://www.geeksforgeeks.org/dsa/quick-sort-algorithm/
* https://www.geeksforgeeks.org/dsa/tail-recursion/
* https://www.cs.cornell.edu/courses/JavaAndDS/files/sort3Quicksort3.pdf
* 
*/
#pragma once
#ifndef BVH_H
#define BVH_H

#include <intrin.h>
#include <math.h>

#include "vector3.h"
#include "body.h"
#include "ray.h"

// for quantizing spatial positions into virtual cells (10 bit)
#define QUANTIZE(min, center, max) (unsigned int)((center - min) / (max - min) * (1024 - 1))


int nodesVisited;
int leavesVisited;

typedef struct {
	Vector3 min;
	Vector3 max;
} Bounds;

#define IDS_MAX 4

typedef struct BVHNode {
	Bounds bounds; // the nodes bounding volume
	int ids[IDS_MAX]; // the id of the object or -1 if a branch node

	// children of the node, NULL if leaf node
	struct BVHNode* left_ptr;
	struct BVHNode* right_ptr;
} BVHNode;

// SoA of ids and morton codes.
typedef struct MortonIDPairs {
	unsigned int id;
	unsigned int mortonCode;
} MortonIDPairs;

Bounds BVH_calculateBounds(Body body);

inline int BVH_boundsIntersect(Bounds a, Bounds b)
{
	return !(
		b.min.x > a.max.x || b.max.x < a.min.x ||
		b.min.y > a.max.y || b.max.y < a.min.y ||
		b.min.z > a.max.z || b.max.z < a.min.z
		);
}

void BVH_quicksortMortonCodes_L(MortonIDPairs* mortonIDpair_list, int low, int high);

void BVH_quicksortMortonCodes(MortonIDPairs* mortonIDpair_list, int low, int high);

void BVH_mergesortMortonCodes(MortonIDPairs* mortonIDpair_list, int low, int high);

void BVH_insertionsortMortonCodes(MortonIDPairs* mortonIDpair_list, int begin, int end);

void BVH_selectionsortMortonCodes(MortonIDPairs* mortonIDpair_list, int count);

int BVH_getSplitPos(MortonIDPairs* mortonIDpair_list, int begin, int end);

BVHNode* BVH_createTree(Body* body_list, int count);

BVHNode* BVH_createSubTree(MortonIDPairs* mortonIDpair_list, Bounds* bounds_list, int begin, int end);

void BVH_updateTreeBounds(BVHNode* node, Body* body_list);

//BVH_traverse(...) traverses a bvh tree and returns a RayHit
//if no bodies were hit then the value of hit_id is NO_HIT
void BVH_traverse(const BVHNode* node, const Ray ray, const Body* bodies, RayHit* state);

// returns 1 if the tree is valid, 0 if invalid
int BVH_validateTree(BVHNode* node);

void BVH_freeTree(BVHNode* node);

// Spread the lower 10 bits of a so there are 2 zeros between each bit
static inline unsigned int BVH_expandBits(unsigned int a)
{
	//		spread				mask
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

inline Bounds Bounds_union(Bounds a, Bounds b)
{
	return (Bounds) {
		vector3_min(a.min, b.min),
		vector3_max(a.max, b.max)
	};
}

void BVH_DebugPrint(const BVHNode* root);

#endif //BVH_H
