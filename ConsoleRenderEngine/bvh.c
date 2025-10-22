
#include "bvh.h"

// may want to change this to pass by value
// returns the bounds of a body
Bounds BVH_calculateBounds(Body body)
{
	switch (body.type)
	{
		case (SHAPE_SPHERE):
		{
			Vector3 extents = (Vector3){body.sphere.radius, body.sphere.radius, body.sphere.radius };
			return (Bounds) {
				vector3_subtract(body.position, extents),
				vector3_add(body.position, extents)
			};
		}
		case(SHAPE_BOX):
		{
			// creates a bounding box that is the worst case senerio
			// could be optimized
			Vector3 one = (Vector3){ 1,1,1 };

			return (Bounds) {
				vector3_subtract(body.position, vector3_scale(one, vector3_magnitude(body.box.half_extents))),
				vector3_add(body.position, vector3_scale(one, vector3_magnitude(body.box.half_extents)))
			};
		}
		case(SHAPE_PLANE):
		{
			return (Bounds) { 0 };
		}
	}
}

int BVH_getSplitPos(MortonIDPairs* list, int begin, int end)
{
	unsigned int firstCode = list[begin].mortonCode;
	unsigned int lastCode = list[end].mortonCode;

	// If all codes are identical, split in the middle
	if (firstCode == lastCode)
		return (begin + end) >> 1;

	// Determine the number of highest-order bits that are the same
	int commonPrefix = __lzcnt(firstCode ^ lastCode); 

	// Binary search to find where the prefix length changes
	int split = begin;
	int step = end - begin;

	do {
		step = (step + 1) >> 1;
		int newSplit = split + step;
		if (newSplit < end) {
			unsigned int splitCode = list[newSplit].mortonCode;
			int prefix = __lzcnt(firstCode ^ splitCode);
			if (prefix > commonPrefix)
				split = newSplit;
		}
	} while (step > 1);

	return split;
}

static inline void swap(MortonIDPairs* mortonIDpair_list, int indexA, int indexB)
{
	MortonIDPairs temp = mortonIDpair_list[indexA];
	mortonIDpair_list[indexA] = mortonIDpair_list[indexB];
	mortonIDpair_list[indexB] = temp;
}

static void medianThreeSwap(MortonIDPairs* pairs, unsigned int a, unsigned int b, unsigned int c, int i0, int i1, int i2)
{
	if ((a > b) ^ (a > c))
		swap(pairs, i0, i2);
	else if ((b < a) ^ (b < c))
		swap(pairs, i1, i2);
}

// partition for quick sort
static int partition(MortonIDPairs* pairs_ptr, int low, int high)
{
	medianThreeSwap(pairs_ptr, pairs_ptr[low].mortonCode, pairs_ptr[(high + low) / 2].mortonCode, pairs_ptr[high].mortonCode, low, (high + low) / 2, high);
	unsigned int pivot = pairs_ptr[high].mortonCode;

	int i = low -1;

	for (int j = low; j <= high - 1; j++)
	{
		if (pairs_ptr[j].mortonCode < pivot)
		{
			i++;
			swap(pairs_ptr, i, j);
		}
	}

	swap(pairs_ptr, i+1, high);
	return i + 1;
}

// quick sort
// this version it sorts the largest partition in a loop
void BVH_quicksortMortonCodes_L(MortonIDPairs* mortonIDpair_list, int low, int high)
{
	// Use a while loop to handle the larger partition iteratively
	// use insertion sort on the last few iterations is faster
	while (high - low > 50 )
	{
		int pivotPosition = partition(mortonIDpair_list, low, high);

		if (pivotPosition - low < high - pivotPosition) {
			// Left part is smaller: Recurse on Left, loop (tail-call) on Right
			BVH_quicksortMortonCodes_L(mortonIDpair_list, low, pivotPosition - 1);
			low = pivotPosition + 1; // Update 'low' for the next iteration (Right side)
		}
		else {
			// Right part is smaller or equal: Recurse on Right, loop (tail-call) on Left
			BVH_quicksortMortonCodes_L(mortonIDpair_list, pivotPosition + 1, high);
			high = pivotPosition - 1; // Update 'high' for the next iteration (Left side)
		}
	}

	BVH_insertionsortMortonCodes(mortonIDpair_list, low, high);

}

void BVH_quicksortMortonCodes(MortonIDPairs* mortonIDpair_list, int low, int high)
{
	// Use a while loop to handle the larger partition iteratively
	if (low < high)
	{
		// opt for insertion sort over recursive quick sorting
		if (high - low < 50)
		{
			BVH_insertionsortMortonCodes(mortonIDpair_list, low, high);
			return;
		}

		int pivotPosition = partition(mortonIDpair_list, low, high);

		BVH_quicksortMortonCodes(mortonIDpair_list, low, pivotPosition - 1);

		BVH_quicksortMortonCodes(mortonIDpair_list, pivotPosition + 1, high);

	}
}

void BVH_insertionsortMortonCodes(MortonIDPairs* mortonIDpair_list, int begin, int end)
{
	for (int i = begin + 1; i <= end; i++)
	{
		MortonIDPairs key = mortonIDpair_list[i];

		int j = i - 1;

		while (j >= 0 && mortonIDpair_list[j].mortonCode > key.mortonCode)
		{
			mortonIDpair_list[j + 1] = mortonIDpair_list[j];
			j--;
		}
		mortonIDpair_list[j + 1] = key;
	}
}

static void merge(MortonIDPairs* pairs_ptr, int begin, int middle, int end)
{
	int i, j, k;
	int n1 = middle - begin + 1;
	int n2 = end - middle;

	//allocate arrays
	MortonIDPairs* left_ptr = malloc(sizeof(MortonIDPairs) * n1);
	if (left_ptr == NULL) return;

	MortonIDPairs* right_ptr = malloc(sizeof(MortonIDPairs) * n2);
	if (right_ptr == NULL) return;

	//initalize arrays
	for (i = 0; i < n1; i++)
	{
		left_ptr[i] = pairs_ptr[begin + i];
	}

	for (j = 0; j < n2; j++)
	{
		right_ptr[j] = pairs_ptr[middle + 1 + j];
	}

	// Merge them arrays
	i = 0;
	j = 0;
	k = begin;

	while (i < n1 && j < n2)
	{
		if (left_ptr[i].mortonCode <= right_ptr[j].mortonCode)
		{
			pairs_ptr[k] = left_ptr[i];
			i++;
		}
		else
		{
			pairs_ptr[k] = right_ptr[j];
			j++;
		}
		k++;
	}

	// Copy remaining
	while (i < n1)
	{
		pairs_ptr[k] = left_ptr[i];
		i++;
		k++;
	}

	while (j < n2)
	{
		pairs_ptr[k] = right_ptr[j];
		j++;
		k++;
	}
}

void BVH_mergesortMortonCodes(MortonIDPairs* mortonIDpair_list, int begin, int end)
{
	// single element
	if (begin < end)
	{
		int middle = begin + (end - begin) / 2;

		// sort
		BVH_mergesortMortonCodes(mortonIDpair_list, begin, middle);
		BVH_mergesortMortonCodes(mortonIDpair_list, middle + 1, end);
		
		merge(mortonIDpair_list, begin, middle, end);
	}
}

// selection sort for fun: O(n^2) time
void BVH_selectionsortMortonCodes(MortonIDPairs* mortonIDpair_list, int count)
{
	for (int i = 0; i < count; i++)
	{
		int min = i;
		for (int j = i + 1; j < count; j++)
		{
			if (mortonIDpair_list[i].mortonCode > mortonIDpair_list[j].mortonCode)
			{
				swap(mortonIDpair_list, i, j);
			}
		}
	}
}

BVHNode* BVH_createTree(Body* body_list, int count)
{
	// Calculate the bounds of all bodies an compute scene bounds from the bodies
	Bounds* bodyBounds_list = malloc(sizeof(Bounds) * count);

	if (bodyBounds_list == NULL)
		return NULL;

	bodyBounds_list[0] = BVH_calculateBounds(body_list[0]);
	Bounds sceneBounds = bodyBounds_list[0];

	for (int i = 1; i < count; i++) {
		bodyBounds_list[i] = BVH_calculateBounds(body_list[i]);
		sceneBounds.min = vector3_min(sceneBounds.min, bodyBounds_list[i].min);
		sceneBounds.max = vector3_max(sceneBounds.max, bodyBounds_list[i].max);
	}

	// compute the morton code list
	// I quantize the positions of each body
	// then generate the code by inverleaving the 10 bit quantized positions
	// currently hardcoded to 10 bit or 1024 different positions
	// the interlieved result is 30 bits long. this fits into a uint
	MortonIDPairs* mortonIDpair_list = malloc(sizeof(MortonIDPairs) * count);

	if (mortonIDpair_list == NULL)
		return NULL;

	for (int i = 0; i < count; i++) {
		Vector3 center = body_list[i].position;

		unsigned int x = QUANTIZE(sceneBounds.min.x, center.x, sceneBounds.max.x);
		unsigned int y = QUANTIZE(sceneBounds.min.y, center.y, sceneBounds.max.y);
		unsigned int z = QUANTIZE(sceneBounds.min.z, center.z, sceneBounds.max.z);

		mortonIDpair_list[i].id = i;
		mortonIDpair_list[i].mortonCode = BVH_mortonCodeGen(x, y, z);
	}

#if _DEBUG || _BENCHMARK

	if (count <= 10)
	{
		printf("Morton Codes: \n");
		for (int n = 0; n < count; n++)
		{
			printf("code %u: ", mortonIDpair_list[n].mortonCode);
			for (int i = sizeof(mortonIDpair_list[n].mortonCode) * 8 - 1; i >= 0; i--)
			{
				printf("%d", (mortonIDpair_list[n].mortonCode >> i) & 1);
			}
			printf("\n");
		}
	}
	clock_t startsort = clock();
#endif // _DEBUG || _BENCHMARK


	// sort the codes
	//BVH_sortMortonCodes(mortonIDpair_list, count);
	//BVH_mergesortMortonCodes(mortonIDpair_list, 0, count - 1);
	BVH_quicksortMortonCodes_L(mortonIDpair_list, 0, count - 1);
	//BVH_quicksortMortonCodes(mortonIDpair_list, 0, count - 1);


#if _DEBUG || _BENCHMARK
	printf("%d leaf BVH sort time: %d ms\n", count, (clock() - startsort) * 1000 / CLOCKS_PER_SEC);

	if (count <= 10)
	{
		printf("Morton Codes: \n");
		for (int n = 0; n < count; n++)
		{
			printf("code %u: ", mortonIDpair_list[n].id);
			for (int i = sizeof(mortonIDpair_list[n].mortonCode) * 8 - 1; i >= 0; i--)
			{
				printf("%d", (mortonIDpair_list[n].mortonCode >> i) & 1);
			}
			printf("\n");
		}
	}

#endif // _DEBUG || _BENCHMARK

	BVHNode* root = BVH_createSubTree(mortonIDpair_list, bodyBounds_list, 0, count-1);

	free(mortonIDpair_list);

	free(bodyBounds_list);
	return root;
}

BVHNode* BVH_createSubTree(MortonIDPairs* mortonIDpair_list, Bounds* bounds_list, int begin, int end)
{
	if (begin > end)
	{
		printf("BVH Error: Begin > End\n");
		return NULL;
	}

	BVHNode* node = malloc(sizeof(BVHNode));

	if (node == NULL)
	{
		printf("BVH Error: Out of Memory\n");
		return NULL;
	}

	// leaf node
	if (begin == end)
	{
		node->bounds = bounds_list[begin];
		node->id = mortonIDpair_list[begin].id;
		node->left_ptr = NULL;
		node->right_ptr = NULL;
	}
	// branch node
	else
	{
		unsigned int split = BVH_getSplitPos(mortonIDpair_list, begin, end);
		node->left_ptr = BVH_createSubTree(mortonIDpair_list, bounds_list, begin, split);
		node->right_ptr = BVH_createSubTree(mortonIDpair_list, bounds_list, split+1, end);
		node->id = -1; // this is an internal node
		node->bounds = Bounds_union(node->left_ptr->bounds, node->right_ptr->bounds);
	}

	return node;
}

void BVH_updateTreeBounds(BVHNode* node, Body* body_list)
{
	if (node == NULL)
	{
		printf("Invalid node in updateTreeBounds\n");
		return NULL;
	}

	// branch node
	if (node->id == -1)
	{
		BVH_updateTreeBounds(node->left_ptr, body_list);
		BVH_updateTreeBounds(node->right_ptr, body_list);

		node->bounds = Bounds_union(node->left_ptr->bounds, node->right_ptr->bounds);
	}
	// leaf node
	else
	{
		node->bounds = BVH_calculateBounds(body_list[node->id]);
	}
}

int BVH_traverseTree(BVHNode* root, Ray ray)
{
	if (root == NULL)
		return -1;

	if (ray_aabb(ray, root->bounds.min, root->bounds.max))
	{
		int id_l = BVH_traverseTree(root->left_ptr, ray);

		if (id_l != -1)
			return id_l;

		int id_r = BVH_traverseTree(root->right_ptr, ray);

		if (id_r != -1)
			return id_r;
	}
	
	return root->id;
}

void BVH_freeTree(BVHNode* node)
{
	if (node == NULL) return;

	BVH_freeTree(node->left_ptr);
	BVH_freeTree(node->right_ptr);
	free(node);
}

// DEBUG

static void BVH_PrintNode(const BVHNode* node, int depth)
{
	if (node == NULL)
		return;

	// indentation for hierarchy
	for (int i = 0; i < depth-1; ++i)
		printf("  ");

	if(depth > 0)
		printf("%c%c", 192, 196);

	// print node info
	if (node->id == -1)
		printf("Branch Node\n");
	else
		printf("Leaf Node (id=%d)\n", node->id);

	// print bounds
	for (int i = 0; i < depth; ++i)
		printf("  ");
	printf("Bounds: min(%.2lf, %.2lf, %.2lf), max(%.2lf, %.2lf, %.2lf)\n",
		node->bounds.min.x, node->bounds.min.y, node->bounds.min.z,
		node->bounds.max.x, node->bounds.max.y, node->bounds.max.z);

	// recurse into children
	if (node->left_ptr) {
		for (int i = 0; i < depth; ++i)
			printf("  ");
		printf("Left:\n");
		BVH_PrintNode(node->left_ptr, depth + 1);
	}

	if (node->right_ptr) {
		for (int i = 0; i < depth; ++i)
			printf("  ");
		printf("Right:\n");
		BVH_PrintNode(node->right_ptr, depth + 1);
	}
}

void BVH_DebugPrint(const BVHNode* root)
{
	printf("======== BVH Tree ========\n");
	BVH_PrintNode(root, 0);
	printf("==========================\n");
}
