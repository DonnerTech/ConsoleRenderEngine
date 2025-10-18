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

	printf("testRaySphere(%d) took: %d ms\n", iter, clock() - start);
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

	printf("testRayBox(%d) took: %d ms\n", iter, clock() - start);
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

	printf("testRayPlane(%d) took: %d ms\n", iter, clock() - start);
}