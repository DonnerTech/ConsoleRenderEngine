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
		RayHit hit = intersectBody(body[i], i, ray);
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
		RayHit hit = intersectBody(body[i], i, ray);
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
		RayHit hit = intersectBody(body[i], i, ray);
	}

	free(body);

	printf("testRayPlane(%d) took: %d ms\n", iter, (clock() - start) * 1000 / CLOCKS_PER_SEC);
}

void testBVHtree(int bodyCount, double scatter, int print)
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

	BVH_validateTree(node);

	start = clock();

	BVH_updateTreeBounds(node, bodies);

	Ray ray;
	Vector3 dir = vector3_normalize((Vector3) { 0, -1, 1 });
	create_ray(&ray, (Vector3) { 0, 0, 0 },dir);

	double out;

	//int i = BVH_traverseTree(node, ray, &out);
	//printf("Check Intersection: %d\n", i);

	//if (i != -1)
	//{
	//	printf("Hit: (%0.2lf, %0.2lf, %0.2lf)\n", bodies[i].position.x, bodies[i].position.y, bodies[i].position.z);
	//}

	printf("%d leaf BVH update time: %d ms\n\n", bodyCount, (clock() - start)*1000 / CLOCKS_PER_SEC);

	if (print)
	{
		BVH_DebugPrint(node);
	}

	BVH_freeTree(node);

	free(bodies);
}