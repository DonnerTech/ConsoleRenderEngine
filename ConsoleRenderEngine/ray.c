#include "ray.h"

// creates a ray from an origin and direction
void create_ray(Ray* ray, Vector3 origin, Vector3 direction)
{
	ray->origin = origin;
	ray->direction = direction;
	ray->invdir.x = 1.0 / direction.x;
	ray->invdir.y = 1.0 / direction.y;
	ray->invdir.z = 1.0 / direction.z;
	ray->sign[0] = (ray->invdir.x < 0);
	ray->sign[1] = (ray->invdir.y < 0);
	ray->sign[2] = (ray->invdir.z < 0);
}

int ray_aabb(Ray ray, Vector3 min, Vector3 max, double tmax, double *dist_ptr)
{
	//Based on: Robust BVH Ray Traversal by Thiago Ize & Solid Angle

	Vector3 bounds[2];
	bounds[0] = min; // min
	bounds[1] = max; // max

	double tmin = 0, txmin, txmax, tymin, tymax, tzmin, tzmax;
	txmin = (bounds[ray.sign[0]].x - ray.origin.x) * ray.invdir.x;
	//txmax = (bounds[1 - ray.sign[0]].x - ray.origin.x) * ray.invdir.x;
	tymin = (bounds[ray.sign[1]].y - ray.origin.y) * ray.invdir.y;
	//tymax = (bounds[1 - ray.sign[1]].y - ray.origin.y) * ray.invdir.y;
	tzmin = (bounds[ray.sign[2]].z - ray.origin.z) * ray.invdir.z;
	//tzmax = (bounds[1 - ray.sign[2]].z - ray.origin.z) * ray.invdir.z;
	tmin = max(tzmin, max(tymin, max(txmin, tmin)));
	//tmax = min(tzmax, min(tymax, min(txmax, tmax)));

	*dist_ptr = min(max(tmin, 0), max(tmax, 0));

	return *dist_ptr < tmax;
}

int raySphereIntersection(Body sphere, Ray ray, double* dist_ptr)
{
	double direct_dist = vector3_magnitude(vector3_subtract(sphere.position, ray.origin));
	double angle = vector3_angle(vector3_subtract(sphere.position, ray.origin), ray.direction);

	// if the distance to the point perpendicular to the sphere is less than the radius and it is in front of the camera
	// trig stuff :>

	*dist_ptr = direct_dist - fast_cos(angle) * sphere.sphere.radius;

	double a = fast_sin(angle) * direct_dist;
	if (a <= sphere.sphere.radius && fast_cos(angle) >= 0)
	{

		return 1;
	}

	return 0;
}

int rayBoxIntersection(Body box, Ray ray, double* dist_ptr, Vector3* localHitPoint)
{
	*dist_ptr = 1e30;

	Ray ba_ray;

	// convert the ray to the box's local space
	Vector3 origin = vector3_subtract(ray.origin, box.position);
	origin = quat_rotate_vector(quat_conjugate(box.orientation), origin);

	// convert direction to local space
	Vector3 direction = quat_rotate_vector(quat_conjugate(box.orientation), ray.direction);

	create_ray(&ba_ray, origin, direction);

	// do ray intersection with axis alligned bounding box
	// Credit to Scratchapixel for the AABB-Ray optimized intersection algorithm
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html

	const Vector3 zero = (Vector3){ 0,0,0 };

	Vector3 bounds[2];
	bounds[0] = vector3_subtract(zero, box.box.half_extents); // min
	bounds[1] = vector3_add(zero, box.box.half_extents); // max

	double tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (bounds[ba_ray.sign[0]].x - ba_ray.origin.x) * ba_ray.invdir.x;
	tmax = (bounds[1 - ba_ray.sign[0]].x - ba_ray.origin.x) * ba_ray.invdir.x;
	tymin = (bounds[ba_ray.sign[1]].y - ba_ray.origin.y) * ba_ray.invdir.y;
	tymax = (bounds[1 - ba_ray.sign[1]].y - ba_ray.origin.y) * ba_ray.invdir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return 0;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	tzmin = (bounds[ba_ray.sign[2]].z - ba_ray.origin.z) * ba_ray.invdir.z;
	tzmax = (bounds[1 - ba_ray.sign[2]].z - ba_ray.origin.z) * ba_ray.invdir.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return 0;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	// Compute intersection distance
	if (tmax < 0)
		return 0; // box is behind ray

	if (tmin < 0)
		*dist_ptr = tmax; // inside box
	else
		*dist_ptr = tmin; // first hit

	*localHitPoint = vector3_add(ba_ray.origin, vector3_scale(ba_ray.direction, *dist_ptr));

	return 1;
}

int rayPlaneIntersection(Body plane, Ray ray, double* dist_ptr, Vector3* localHitPoint)
{
	double denom = vector3_dot(plane.plane.normal, ray.direction);

	// If the ray is parallel to the plane, no intersection
	if (fabs(denom) < 1e-16)
		return 0;

	// Compute dist for the intersection point
	*dist_ptr = -(vector3_dot(plane.plane.normal, ray.origin) - plane.plane.offset) / denom;

	// Intersection must be in front of the ray
	if (*dist_ptr < 0.0)
		return 0;

	// set the hit position (currently global)
	*localHitPoint = vector3_add(ray.origin, vector3_scale(ray.direction, *dist_ptr));

	return 1;
}

int intersectBody(Body body, Ray ray, double* dist_ptr)
{

	if (body.type == SHAPE_SPHERE)
	{
		if (raySphereIntersection(body, ray, dist_ptr))
		{
			return 1;
		}
	}
	else if (body.type == SHAPE_BOX)
	{
		Vector3 localHitPoint = { 0 };

		if (rayBoxIntersection(body, ray, dist_ptr, &localHitPoint))
		{
			return 1;
		}
	}
	else if (body.type == SHAPE_PLANE)
	{
		// not  in a bvh
		return 0;
	}
	else
	{
		// unknown shape
		return 0;
	}

	return 0;
}