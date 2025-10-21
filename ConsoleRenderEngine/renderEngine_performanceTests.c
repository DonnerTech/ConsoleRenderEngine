#include "renderEngine_performanceTests.h"


void testRaySphere(int iter)
{
	Body* body = (Body*)malloc(sizeof(Body) * iter);

	if (body == NULL)
		return;

	for (int i = 0; i < iter; i++)
	{
		body[i].type = SHAPE_SPHERE;
		body[i].sphere.radius = 1;

		body[i].position = vector3_scale(vector3_random(), 100);
		body[i].orientation = quat_from_axis_angle(vector3_normalize(vector3_random()), 0);
	}

	Ray ray;
	create_ray(&ray, (Vector3) { 0, 0, 0 }, (Vector3) { 0, 0, 1 });

	clock_t start = clock();
	for (int i = 0; i < iter; i++)
	{
		double dist;
		raySphereIntersection(body[i], ray, &dist);
	}

	free(body);

	printf("testRaySphere(%d) took: %d ms\n", iter, (clock() - start) * 1000 / CLOCKS_PER_SEC);
}

void testRayBox(int iter)
{
	Body* body = (Body*)malloc(sizeof(Body) * iter);

	if (body == NULL)
		return;

	for (int i = 0; i < iter; i++)
	{

		body[i].type = SHAPE_BOX;
		body[i].box.half_extents = (Vector3){ 1,1,1 };

		body[i].position = vector3_scale(vector3_random(), 100);
		body[i].orientation = quat_from_axis_angle(vector3_normalize(vector3_random()), 0);
	}

	Ray ray;
	create_ray(&ray, (Vector3) { 0, 0, 0 }, (Vector3) { 0, 0, 1 });

	clock_t start = clock();
	for (int i = 0; i < iter; i++)
	{
		double dist;
		Vector3 hit;
		rayBoxIntersection(body[i], ray, &dist, &hit);
	}

	free(body);

	printf("testRayBox(%d) took: %d ms\n", iter, (clock() - start) * 1000 / CLOCKS_PER_SEC);
}

void testRayPlane(int iter)
{
	Body* body = (Body*)malloc(sizeof(Body) * iter);

	if (body == NULL)
		return;

	for (int i = 0; i < iter; i++)
	{
		body[i].type = SHAPE_PLANE;
		body[i].plane.normal = vector3_normalize(vector3_random());
		body[i].plane.offset = rand() % 100;

		//body[i].position = vector3_scale(vector3_random(), 100);
		//body[i].orientation = quat_from_axis_angle(vector3_normalize(vector3_random()), 0);
	}

	Ray ray;
	create_ray(&ray, (Vector3) { 0, 0, 0 }, (Vector3) { 0, 0, 1 });

	clock_t start = clock();
	for (int i = 0; i < iter; i++)
	{
		double dist;
		raySphereIntersection(body[i], ray, &dist);
	}

	free(body);

	printf("testRayPlane(%d) took: %d ms\n", iter, (clock() - start) * 1000 / CLOCKS_PER_SEC);
}

int validateBVHTree(BVHNode* node)
{
	if (node == NULL)
	{
		printf("Invalid BVH Tree! (found dead node in branch)\n");

		return 0;
	}

	// if branch node continue
	if (node->id == -1)
	{
		if (!validateBVHTree(node->left_ptr))
		{

			printf("(left node parent) data: %d\n", node->id);
		}
		if (!validateBVHTree(node->right_ptr))
		{
			printf("(right node parent) data: %d\n", node->id);
		}
	}

	return 1;
}

void testBVHtree(int bodyCount, double scatter, bool print)
{
	Body* bodies = malloc(sizeof(Body) * bodyCount);

	if (bodies == NULL)
		return;

	for (int i = 0; i < bodyCount; i++)
	{
		bodies[i].type = SHAPE_BOX;
		bodies[i].box.half_extents = (Vector3){ 1,1,1 };
		//bodies[i].type = SHAPE_SPHERE;
		//bodies[i].sphere.radius = rand() % 10 + 1;

		bodies[i].position = vector3_scale(vector3_random(), scatter);
		bodies[i].orientation = quat_identity();
	}
	
	clock_t start = clock();

	BVHNode* node = BVH_createTree(bodies, bodyCount);

	printf("%d leaf BVH creation time: %d ms\n", bodyCount, (clock() - start)*1000 / CLOCKS_PER_SEC);

	validateBVHTree(node);

	start = clock();

	BVH_updateTreeBounds(node, bodies);

	printf("%d leaf BVH update time: %d ms\n\n", bodyCount, (clock() - start)*1000 / CLOCKS_PER_SEC);

	if (print)
	{
		BVH_DebugPrint(node);
	}

	BVH_freeTree(node);

	free(bodies);
}