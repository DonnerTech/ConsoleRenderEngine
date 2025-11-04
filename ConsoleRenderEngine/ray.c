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

// solving the quadratic equation for 
// the general ray-sphere equation
int raySphereIntersection(Body sphere, Ray ray, double* dist_ptr, Vector3* localHitPoint, Vector3* normal)
{
	// setup
	Vector3 oc = vector3_subtract(ray.origin, sphere.position);
	double b = 2.0 * vector3_dot(oc, ray.direction);
	double c = vector3_dot(oc, oc) - sphere.sphere.radius * sphere.sphere.radius;
	double disc = b * b - 4.0 * c;
	if (disc < 0.0) return 0;

	// quadratic solving
	double sqrtDisc = sqrt(disc);
	double t = (-b - sqrtDisc) * 0.5;
	if (t < 1e-8) t = (-b + sqrtDisc) * 0.5;
	if (t < 1e-8) return 0;

	// return data
	*dist_ptr = t;
	Vector3 hitPoint = vector3_add(ray.origin, vector3_scale(ray.direction, t));
	*localHitPoint = vector3_subtract(hitPoint, sphere.position);
	*normal = vector3_normalize(*localHitPoint);
	*localHitPoint = quat_rotate_vector(sphere.orientation, *localHitPoint);
	return 1;
}


// Ray and oriented bounding box intersection test
// Credit to Scratchapixel for the Ray-AABB slab intersection tutorial
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html

int rayBoxIntersection(Body box, Ray ray, double* dist_ptr, Vector3* localHitPoint, Vector3* normal)
{
	*dist_ptr = 1e30;

	Ray ba_ray;

	// convert the ray to the box's local space
	const Quaternion inv_brot = quat_conjugate(box.orientation); // if this causes problems revert to quat_inverse(...)
	Vector3 origin = vector3_subtract(ray.origin, box.position);
	origin = quat_rotate_vector(inv_brot, origin);

	// convert direction to local space
	Vector3 direction = quat_rotate_vector(inv_brot, ray.direction);

	create_ray(&ba_ray, origin, direction);

	const Vector3 zero = (Vector3){ 0 };

	Vector3 bounds[2];
	bounds[0] = vector3_subtract(zero, box.box.half_extents); // min
	bounds[1] = box.box.half_extents; // max

	double txmin = (bounds[ba_ray.sign[0]].x - ba_ray.origin.x) * ba_ray.invdir.x;
	double txmax = (bounds[1 - ba_ray.sign[0]].x - ba_ray.origin.x) * ba_ray.invdir.x;
	double tymin = (bounds[ba_ray.sign[1]].y - ba_ray.origin.y) * ba_ray.invdir.y;
	double tymax = (bounds[1 - ba_ray.sign[1]].y - ba_ray.origin.y) * ba_ray.invdir.y;
	double tzmin = (bounds[ba_ray.sign[2]].z - ba_ray.origin.z) * ba_ray.invdir.z;
	double tzmax = (bounds[1 - ba_ray.sign[2]].z - ba_ray.origin.z) * ba_ray.invdir.z;

	// Combine the slabs
	double tmin = fmax(fmax(txmin, tymin), tzmin);
	double tmax = fmin(fmin(txmax, tymax), tzmax);

	// No hit if range is invalid or intersection is behind the ray
	if (tmax < 0 || tmin > tmax)
		return 0;

	// Return the nearest valid distance
	*dist_ptr = tmin;

	*localHitPoint = vector3_add(ba_ray.origin, vector3_scale(ba_ray.direction, *dist_ptr));
	
	*normal = (Vector3){ 0 };

	//normal calculation with slabs
	if (tmin == txmin)
		normal->x = (ba_ray.direction.x > 0) ? -1 : 1;
	else if (tmin == tymin)
		normal->y = (ba_ray.direction.y > 0) ? -1 : 1;
	else
		normal->z = (ba_ray.direction.z > 0) ? -1 : 1;

	*normal = quat_rotate_vector(box.orientation, *normal);

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