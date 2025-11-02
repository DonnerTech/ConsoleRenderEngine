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

int ray_aabb(Ray ray, Vector3 min, Vector3 max, double tmax_limit, double *dist_ptr)
{
	//Based on: Robust BVH Ray Traversal by Thiago Ize & Solid Angle

	Vector3 bounds[2];
	bounds[0] = min; // min
	bounds[1] = max; // max

	double txmin = (bounds[ray.sign[0]].x - ray.origin.x) * ray.invdir.x;
	double txmax = (bounds[1 - ray.sign[0]].x - ray.origin.x) * ray.invdir.x;
	double tymin = (bounds[ray.sign[1]].y - ray.origin.y) * ray.invdir.y;
	double tymax = (bounds[1 - ray.sign[1]].y - ray.origin.y) * ray.invdir.y;
	double tzmin = (bounds[ray.sign[2]].z - ray.origin.z) * ray.invdir.z;
	double tzmax = (bounds[1 - ray.sign[2]].z - ray.origin.z) * ray.invdir.z;

	// Combine the slabs
	double tmin = fmax(fmax(txmin, tymin), tzmin);
	double tmax = fmin(fmin(txmax, tymax), tzmax);

	// No hit if range is invalid or intersection is behind the ray
	if (tmax < 0 || tmin > tmax)
		return 0;

	// If the ray starts inside the box
	if (tmin < 0 && tmax > 0) {
		*dist_ptr = 0.0;
		return 1;
	}

	// If the intersection is farther than the allowed limit, skip
	if (tmin > tmax_limit)
		return 0;

	// Return the nearest valid distance
	*dist_ptr = tmin;
	return 1;
}

int raySphereIntersection(Body sphere, Ray ray, double* dist_ptr, Vector3* localHitPoint, Vector3* normal)
{
	double direct_dist = vector3_magnitude(vector3_subtract(sphere.position, ray.origin));
	double angle = vector3_angle(vector3_subtract(sphere.position, ray.origin), ray.direction);

	// if the distance to the point perpendicular to the sphere is less than the radius and it is in front of the camera
	// trig stuff :>

	*dist_ptr = direct_dist - fast_cos(angle) * sphere.sphere.radius;

	double a = fast_sin(angle) * direct_dist;
	if (a <= sphere.sphere.radius && fast_cos(angle) >= 0)
	{
		Vector3 hitPoint = vector3_add(ray.origin, vector3_scale(ray.direction, *dist_ptr));

		*localHitPoint = vector3_subtract(hitPoint, sphere.position);
		*normal = vector3_normalize(*localHitPoint);
		*localHitPoint = quat_rotate_vector(sphere.orientation, *localHitPoint);

		return 1;
	}
	return 0;
}

int rayBoxIntersection(Body box, Ray ray, double* dist_ptr, Vector3* localHitPoint, Vector3* normal)
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
	
	//TODO: normal calculation
	*normal = (Vector3){ 0,0,0 };

	return 1;
}

int rayPlaneIntersection(Body plane, Ray ray, double* dist_ptr, Vector3* localHitPoint, Vector3* normal)
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

	// set the local hit position
	*localHitPoint = vector3_add(ray.origin, vector3_scale(ray.direction, *dist_ptr));
	*normal = plane.plane.normal;

	return 1;
}

RayHit intersectBody(Body body, int id, Ray ray)
{
	RayHit hit = (RayHit){ .dist = 1e30, .hit_id = NO_HIT, .localPosition = (Vector3){0,0,0}, .position = (Vector3){0,0,0}, .normal = (Vector3){0,0,0} };

	int didHit = 0;

	switch (body.type)
	{
		case (SHAPE_SPHERE):
		{
			didHit = (raySphereIntersection(body, ray, &hit.dist, &hit.localPosition, &hit.normal));
			break;
		}
		case (SHAPE_BOX):
		{
			didHit = (rayBoxIntersection(body, ray, &hit.dist, &hit.localPosition, &hit.normal));
			break;
		}
		case (SHAPE_PLANE):
		{
			didHit = (rayPlaneIntersection(body, ray, &hit.dist, &hit.localPosition, &hit.normal));
			break;
		}
		default:
		{
			didHit = 0;
			break;
		}
	}

	if (didHit)
	{
		hit.hit_id = id;
		hit.position = vector3_add(ray.origin, vector3_scale(ray.direction, hit.dist));
	}

	return hit;
}