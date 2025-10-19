
#include "bvh.h"

Bounds BVH_calculateBounds(Body* body_ptr)
{
	switch (body_ptr->type)
	{
		case (SHAPE_SPHERE):
		{
			Vector3 extents = (Vector3){body_ptr->sphere.radius, body_ptr->sphere.radius, body_ptr->sphere.radius };
			return (Bounds) {
				vector3_subtract(body_ptr->position, extents),
				vector3_add(body_ptr->position, extents)
			};
		}
		case(SHAPE_BOX):
		{
			// creates a bounding box that is the worst case senerio
			// could be optimized
			Vector3 one = (Vector3){ 1,1,1 };

			return (Bounds) {
				vector3_subtract(body_ptr->position, vector3_scale(one, vector3_magnitude(body_ptr->box.half_extents))),
				vector3_add(body_ptr->position, vector3_scale(one, vector3_magnitude(body_ptr->box.half_extents)))
			};
		}
		case(SHAPE_PLANE):
		{
			return (Bounds) { 0 };
		}
	}
}