
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

int BVH_getSplitPos(MortonIDPair* list, int begin, int end)
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

// partition for quick sort
static int partition(MortonIDPair* mortonIDpair_list, int low, int high)
{
	unsigned int pivot = mortonIDpair_list[high].mortonCode;
	int left = low, right = high - 1;

	while (left < right)
	{
		while (mortonIDpair_list[left].mortonCode <= pivot && left < right) {
			left++;
		}
		while (mortonIDpair_list[right].mortonCode >= pivot && left < right) {
			right--;
		}

		if (left >= right) {
			break;
		}
		MortonIDPair temp = mortonIDpair_list[left];
		mortonIDpair_list[left] = mortonIDpair_list[right];
		mortonIDpair_list[right] = temp;
	}

	MortonIDPair temp = mortonIDpair_list[left];
	mortonIDpair_list[left] = mortonIDpair_list[high];
	mortonIDpair_list[high] = temp;
	return left;
}

// quick sort
// this is a (tail recursion) ver
// it sorts the largest partition in a loop
void BVH_quicksortMortonCodes(MortonIDPair* mortonIDpair_list, int low, int high)
{
	// Use a while loop to handle the larger partition iteratively
	while (low < high)
	{
		int pivotPosition = partition(mortonIDpair_list, low, high);

		if (pivotPosition - low < high - pivotPosition) {
			// Left part is smaller: Recurse on Left, loop (tail-call) on Right
			BVH_quicksortMortonCodes(mortonIDpair_list, low, pivotPosition - 1);
			low = pivotPosition + 1; // Update 'low' for the next iteration (Right side)
		}
		else {
			// Right part is smaller or equal: Recurse on Right, loop (tail-call) on Left
			BVH_quicksortMortonCodes(mortonIDpair_list, pivotPosition + 1, high);
			high = pivotPosition - 1; // Update 'high' for the next iteration (Left side)
		}
	}
}

// selection sort for fun
void BVH_sortMortonCodes(MortonIDPair* mortonIDpair_list, int count)
{
	for (int i = 0; i < count; i++)
	{
		int min = i;
		for (int j = i + 1; j < count; j++)
		{
			if (mortonIDpair_list[i].mortonCode > mortonIDpair_list[j].mortonCode)
			{
				MortonIDPair temp = mortonIDpair_list[i];
				mortonIDpair_list[i] = mortonIDpair_list[j];
				mortonIDpair_list[j] = temp;
			}
		}

		//swap
		//MortonIDPair temp = mortonIDpair_list[i];
		//mortonIDpair_list[i] = mortonIDpair_list[min];
		//mortonIDpair_list[min] = temp;
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
	MortonIDPair* mortonCode_list = malloc(sizeof(MortonIDPair) * count);

	if (mortonCode_list == NULL)
		return NULL;

	for (int i = 0; i < count; i++) {
		Vector3 center = body_list[i].position;

		unsigned int x = QUANTIZE(sceneBounds.min.x, center.x, sceneBounds.max.x);
		unsigned int y = QUANTIZE(sceneBounds.min.y, center.y, sceneBounds.max.y);
		unsigned int z = QUANTIZE(sceneBounds.min.z, center.z, sceneBounds.max.z);

		mortonCode_list[i].id = i;
		mortonCode_list[i].mortonCode = BVH_mortonCodeGen(x, y, z);
	}

#if _DEBUG || _BENCHMARK

	printf("Morton Codes: \n");
	for (int n = 0; n < count; n++)
	{
		printf("code %u: ", mortonCode_list[n].id);
		for (int i = sizeof(mortonCode_list[n].mortonCode) * 8 - 1; i >= 0; i--)
		{
			printf("%d", (mortonCode_list[n].mortonCode >> i) & 1);
		}
		printf("\n");
	}

	clock_t startsort = clock();
#endif // _DEBUG || _BENCHMARK


	// sort the codes
	//BVH_quicksortMortonCodes(mortonCode_list, 0, count - 1);
	BVH_sortMortonCodes(mortonCode_list, count);


#if _DEBUG || _BENCHMARK
	printf("%d leaf BVH sort time: %d ms\n", count, (clock() - startsort) * 1000 / CLOCKS_PER_SEC);

	printf("Morton Codes: \n");
	for (int n = 0; n < count; n++)
	{
		printf("code %u: ", mortonCode_list[n].id);
		for (int i = sizeof(mortonCode_list[n].mortonCode) * 8 - 1; i >= 0; i--)
		{
			printf("%d", (mortonCode_list[n].mortonCode >> i) & 1);
		}
		printf("\n");
	}

#endif // _DEBUG || _BENCHMARK

	BVHNode* root = BVH_createSubTree(mortonCode_list, bodyBounds_list, 0, count-1);

	free(mortonCode_list);
	free(bodyBounds_list);
	return root;
}

BVHNode* BVH_createSubTree(MortonIDPair* mortonIDpair_list, Bounds* bounds_list, int begin, int end)
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

// convenience wrapper
void BVH_DebugPrint(const BVHNode* root)
{
	printf("======== BVH Tree ========\n");
	BVH_PrintNode(root, 0);
	printf("==========================\n");
}
