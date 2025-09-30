#include "physicsEngine.h"

RigidBody create_box(Vector3 pos, Vector3 half_extents, Quaternion orientation, double mass)
{
	RigidBody rigidbody;
	CollisionShape shape;

	shape.type = SHAPE_BOX;
	shape.box.half_extents = half_extents;

	rigidbody.shape = shape;
	rigidbody.position = pos;
	rigidbody.orientation = orientation;
	rigidbody.mass = mass;
	rigidbody.inv_mass = 1 / mass;

	rigidbody.isStatic = false;
}

RigidBody create_sphere(Vector3 pos, double radius, double mass)
{
	RigidBody rigidbody;
	CollisionShape shape;

	shape.type = SHAPE_SPHERE;
	shape.sphere.radius = radius;

	rigidbody.shape = shape;
	rigidbody.position = pos;
	rigidbody.orientation = quat_identity();
	rigidbody.mass = mass;
	rigidbody.inv_mass = 1 / mass;

	rigidbody.isStatic = false;
}

RigidBody create_plane(Vector3 normal, double offset)
{
	RigidBody rigidbody;
	CollisionShape shape;

	shape.type = SHAPE_PLANE;
	shape.plane.normal = normal;
	shape.plane.offset = offset;

	rigidbody.shape = shape;
	rigidbody.position = (Vector3){ 0.0,0.0,0.0 };
	rigidbody.orientation = quat_identity();
	rigidbody.mass = 0.0;
	rigidbody.inv_mass = 0.0;

	rigidbody.isStatic = true;
}

void apply_force(RigidBody* body, Vector3 force, Vector3 point)
{
	if (body->isStatic)
		return;

	body->linearVelocity = vector3_add(
		body->linearVelocity,
		vector3_scale(force, body->inv_mass)
	);

	Vector3 r = vector3_subtract(point, body->position);	// lever arm
	Vector3 torque = vector3_cross(r, force);				// ? = r   F

	apply_torque(&body, torque);

}

void apply_torque(RigidBody* body, Vector3 torque)
{
	Vector3 angular_accel = matrix3x3_mul_vector3(torque, body->inertiaWorld_inv);

	body->angularVelocity = vector3_add(
		body->angularVelocity,
		angular_accel
	);
}

// TODO: Add matrix functions

void update_inertia_tensor(RigidBody* body)
{
	if (body->isStatic) return;

	// Build rotation matrix from orientation quaternion
	Matrix3 R = quat_to_matrix3(body->orientation);

	// I_world_inv = R * I_body_inv * R^T
	Matrix3 Rt = matrix3x3_transpose(R);
	Matrix3 temp = matrix3x3_mul(R, body->inertiaBody_inv);
	body->inertiaWorld_inv = matrix3x3_mul(temp, Rt);
}


void integrate(RigidBody* body, double dt)
{

}