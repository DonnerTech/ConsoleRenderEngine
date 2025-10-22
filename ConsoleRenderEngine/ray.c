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

int ray_aabb(Ray ray, Vector3 min, Vector3 max)
{
	Vector3 pos = vector3_scale(vector3_add(min, max), 0.5);

	ray.origin = vector3_subtract(ray.origin, pos);

	const Vector3 zero = (Vector3){ 0,0,0 };

	Vector3 bounds[2];
	bounds[0] = min; // min
	bounds[1] = max; // max

	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (bounds[ray.sign[0]].x - ray.origin.x) * ray.invdir.x;
	tmax = (bounds[1 - ray.sign[0]].x - ray.origin.x) * ray.invdir.x;
	tymin = (bounds[ray.sign[1]].y - ray.origin.y) * ray.invdir.y;
	tymax = (bounds[1 - ray.sign[1]].y - ray.origin.y) * ray.invdir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return 0;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	tzmin = (bounds[ray.sign[2]].z - ray.origin.z) * ray.invdir.z;
	tzmax = (bounds[1 - ray.sign[2]].z - ray.origin.z) * ray.invdir.z;

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
		return 1;
}