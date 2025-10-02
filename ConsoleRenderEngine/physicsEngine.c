#include "physicsEngine.h"

RigidBody rb_create_box(Vector3 pos, Vector3 half_extents, Quaternion orientation, double mass)
{
	RigidBody rigidbody = { 0 };
	CollisionShape shape = { 0 };

	shape.type = SHAPE_BOX;
	shape.box.half_extents = half_extents;

	rigidbody.shape = shape;
	rigidbody.position = pos;
	rigidbody.orientation = orientation;
	rigidbody.mass = mass;
	rigidbody.inv_mass = 1 / mass;

	rigidbody.isStatic = false;

	// Inertia tensor for a box: (1/12) * m * diag(h^2 + d^2, w^2 + d^2, w^2 + h^2)
	double w = half_extents.x * 2.0;
	double h = half_extents.y * 2.0;
	double d = half_extents.z * 2.0;
	double coeff = (1.0 / 12.0) * rigidbody.mass;
	rigidbody.inertiaBody.m[0][0] = coeff * (h * h + d * d);
	rigidbody.inertiaBody.m[1][1] = coeff * (w * w + d * d);
	rigidbody.inertiaBody.m[2][2] = coeff * (w * w + h * h);

	// Set to zero
	rigidbody.inertiaBody.m[0][1] = rigidbody.inertiaBody.m[0][2] = 0.0;
	rigidbody.inertiaBody.m[1][0] = rigidbody.inertiaBody.m[1][2] = 0.0;
	rigidbody.inertiaBody.m[2][0] = rigidbody.inertiaBody.m[2][1] = 0.0;

	// Inverse inertia tensor
	rigidbody.inertiaBody_inv.m[0][0] = (rigidbody.inertiaBody.m[0][0] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[0][0] : 0.0;
	rigidbody.inertiaBody_inv.m[1][1] = (rigidbody.inertiaBody.m[1][1] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[1][1] : 0.0;
	rigidbody.inertiaBody_inv.m[2][2] = (rigidbody.inertiaBody.m[2][2] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[2][2] : 0.0;
	rigidbody.inertiaBody_inv.m[0][1] = (rigidbody.inertiaBody_inv.m[0][2] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[0][1] : 0.0;
	rigidbody.inertiaBody_inv.m[1][0] = (rigidbody.inertiaBody_inv.m[1][2] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[1][0] : 0.0;
	rigidbody.inertiaBody_inv.m[2][0] = (rigidbody.inertiaBody_inv.m[2][1] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[2][0] : 0.0;

	update_inertia_tensor(&rigidbody);

	return rigidbody;
}

RigidBody rb_create_sphere(Vector3 pos, double radius, double mass)
{
	RigidBody rigidbody = { 0 };
	CollisionShape shape  = { 0 };

	shape.type = SHAPE_SPHERE;
	shape.sphere.radius = radius;

	rigidbody.shape = shape;
	rigidbody.position = pos;
	rigidbody.orientation = quat_identity();
	rigidbody.mass = mass;
	rigidbody.inv_mass = 1 / mass;

	rigidbody.isStatic = false;

	// Inertia tensor for a solid sphere: (2/5) * m * r^2
	double coeff = (2.0 / 5.0) * rigidbody.mass * radius * radius;
	rigidbody.inertiaBody.m[0][0] = coeff;
	rigidbody.inertiaBody.m[1][1] = coeff;
	rigidbody.inertiaBody.m[2][2] = coeff;

	//Set to zero
	rigidbody.inertiaBody.m[0][1] = rigidbody.inertiaBody.m[0][2] = 0.0;
	rigidbody.inertiaBody.m[1][0] = rigidbody.inertiaBody.m[1][2] = 0.0;
	rigidbody.inertiaBody.m[2][0] = rigidbody.inertiaBody.m[2][1] = 0.0;

	// Inverse inertia tensor
	rigidbody.inertiaBody_inv.m[0][0] = (rigidbody.inertiaBody.m[0][0] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[0][0] : 0.0;
	rigidbody.inertiaBody_inv.m[1][1] = (rigidbody.inertiaBody.m[1][1] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[1][1] : 0.0;
	rigidbody.inertiaBody_inv.m[2][2] = (rigidbody.inertiaBody.m[2][2] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[2][2] : 0.0;
	rigidbody.inertiaBody_inv.m[0][1] = (rigidbody.inertiaBody_inv.m[0][2] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[0][1] : 0.0;
	rigidbody.inertiaBody_inv.m[1][0] = (rigidbody.inertiaBody_inv.m[1][2] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[1][0] : 0.0;
	rigidbody.inertiaBody_inv.m[2][0] = (rigidbody.inertiaBody_inv.m[2][1] != 0.0) ? 1.0 / rigidbody.inertiaBody.m[2][0] : 0.0;

	update_inertia_tensor(&rigidbody);

	return rigidbody;
}

RigidBody rb_create_plane(Vector3 normal, double offset)
{
	RigidBody rigidbody = { 0 };
	CollisionShape shape = { 0 };

	shape.type = SHAPE_PLANE;
	shape.plane.normal = normal;
	shape.plane.offset = offset;

	rigidbody.shape = shape;
	rigidbody.position = (Vector3){ 0.0,0.0,0.0 };
	rigidbody.orientation = quat_identity();
	rigidbody.mass = 0.0;
	rigidbody.inv_mass = 0.0;

	rigidbody.isStatic = true;

	return rigidbody;
}

void rb_apply_force(RigidBody* body, Vector3 force, Vector3 point)
{
	if (body->isStatic)
		return;

	body->linearVelocity = vector3_add(
		body->linearVelocity,
		vector3_scale(force, body->inv_mass)
	);

	Vector3 r = vector3_subtract(point, body->position); // not fully accurate lever vector
	Vector3 torque = vector3_cross(r, force);

	rb_apply_torque(&body, torque);

}

void rb_apply_torque(RigidBody* body, Vector3 torque)
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
	Matrix3x3 R = quat_to_matrix3(body->orientation);

	// I_world_inv = R * I_body_inv * R^T
	Matrix3x3 Rt = matrix3x3_transpose(R);
	Matrix3x3 temp = matrix3x3_mul(R, body->inertiaBody_inv);
	body->inertiaWorld_inv = matrix3x3_mul(temp, Rt);
}


void integrate(RigidBody* body, double dt)
{
	// Integrate (aka apply) both linear and rotational velocities
	// these are kind of the basic methods (eulers methods)

	if (body->isStatic) return;

	// Linear integration
	body->position = vector3_add(
		body->position,
		vector3_scale(body->linearVelocity, dt)
	);

	// Rotational integration
	if (vector3_magnitude(body->angularVelocity) > 1e-8)
	{
		Quaternion wq = { 0.0,
						 body->angularVelocity.x,
						 body->angularVelocity.y,
						 body->angularVelocity.z };

		// dq = 0.5 * wq * q
		Quaternion dq = quat_multiply(wq, body->orientation);
		dq.w *= 0.5 * dt;
		dq.x *= 0.5 * dt;
		dq.y *= 0.5 * dt;
		dq.z *= 0.5 * dt;

		// q = q + dq
		body->orientation.w += dq.w;
		body->orientation.x += dq.x;
		body->orientation.y += dq.y;
		body->orientation.z += dq.z;

		// Normalize to avoid drift
		body->orientation = quat_normalize(body->orientation);
	}

	// update world inertia tensor
	update_inertia_tensor(&body);
}

double point_plane_distance(Vector3 point, RigidBody* plane)
{
	return vector3_dot(point, plane->shape.plane.normal) - plane->shape.plane.offset;
}

Contact collide_sphere_plane(RigidBody* sphere, RigidBody* plane)
{
	Contact contact;
	contact.collided = false;
	
	// check for intersection. 
	// find the point, the depth, and the normal that points out of A.

	double distance = point_plane_distance(sphere->position, plane);

	if(distance < sphere->shape.sphere.radius)
	{
		contact.collided = true;
		contact.contact_depth = sphere->shape.sphere.radius - distance;
		contact.normal = vector3_scale(plane->shape.plane.normal, -1.0); // point out of sphere

		// median point of contact
		contact.contact_point = vector3_subtract(sphere->position, vector3_scale(plane->shape.plane.normal, 0.5 * distance));
	}
	
	return contact;
}

Contact collide_box_plane(RigidBody* box, RigidBody* plane)
{
	Vector3 corners[8];
	double distances[8];
	int index = 0;
	Matrix3x3 mat = quat_to_matrix3(box->orientation);

	for(int x = -1; x <= 1; x += 2)
	{
		for (int y = -1; y <= 1; y += 2)
		{
			for (int z = -1; z <= 1; z += 2)
			{
				corners[index] = (Vector3){ box->position.x + x*box->shape.box.half_extents.x, box->position.y + y*box->shape.box.half_extents.y, box->position.z + z*box->shape.box.half_extents.z };
				corners[index] = matrix3x3_mul_vector3(corners[index], mat);

				distances[index] = point_plane_distance(corners[index], plane);

				index++;
			}
		}
	}

	//TODO: Instead calculate the weighted average of the position and depth of the corners that are penetrating the plane

	// Calculates the deepest point
	Contact contact;
	contact.collided = false;
	contact.contact_depth = RAND_MAX;
	for (int i = 0; i < 8; i++)
	{
		if(distances[i] < 0)
		{
			if (distances[i] <= -contact.contact_depth)
			{
				contact.collided = true;
				contact.contact_depth = -distances[i];
				contact.contact_point = corners[i];
				contact.normal = vector3_scale(plane->shape.plane.normal, -1.0); // point out of box
			}
		}
	}

	return contact;
}

Contact collide_sphere_sphere(RigidBody* sphereA, RigidBody* sphereB)
{
	Contact contact;
	contact.collided = false;

	// vector from A to B
	Vector3 direction = vector3_subtract(sphereB->position, sphereA->position);
	contact.contact_depth = (sphereA->shape.sphere.radius + sphereB->shape.sphere.radius) - vector3_magnitude(direction);
	
	//check for collision
	if (contact.contact_depth > 0)
	{
		contact.collided = true;

		// the deepest point on sphere A inside sphere B
		Vector3 normalInto = vector3_normalize(direction);
		contact.normal = vector3_scale(normalInto, -1.0);

		// deepest point
		//contact.contact_point = vector3_add(sphereA->position, vector3_scale(normalInto, sphereA->shape.sphere.radius));

		// median point
		contact.contact_point = vector3_add(sphereA->position, vector3_scale(direction, 0.5));
	}

	return contact;
}

Contact collide_box_sphere(RigidBody* box, RigidBody* sphere)
{
	// Unimplemented
	Contact contact;
	contact.collided = false;

	return contact;
}

Contact collide_box_box(RigidBody* boxA, RigidBody* boxB)
{
	// Unimplemented
	Contact contact;
	contact.collided = false;

	return contact;
}

void resolve_contact(RigidBody* a, RigidBody* b, Contact contact, double restitution)
{
	if (!contact.collided) return;
	// Separate the bodies to avoid sinking
	if (!a->isStatic)
	{
		a->position = vector3_add(a->position, vector3_scale(contact.normal, contact.contact_depth * 0.5));
	}
	if (!b->isStatic)
	{
		b->position = vector3_subtract(b->position, vector3_scale(contact.normal, contact.contact_depth * 0.5));
	}
	// Relative velocity at contact point
	Vector3 ra = vector3_subtract(contact.contact_point, a->position);
	Vector3 rb = vector3_subtract(contact.contact_point, b->position);
	Vector3 va = vector3_add(a->linearVelocity, vector3_cross(a->angularVelocity, ra));
	Vector3 vb = vector3_add(b->linearVelocity, vector3_cross(b->angularVelocity, rb));
	Vector3 relative_velocity = vector3_subtract(va, vb);
	double vel_along_normal = vector3_dot(relative_velocity, contact.normal);
	// Do not resolve if velocities are separating
	if (vel_along_normal > 0)
		return;
	// Calculate impulse scalar
	double inv_mass_a = a->isStatic ? 0.0 : a->inv_mass;
	double inv_mass_b = b->isStatic ? 0.0 : b->inv_mass;
	Vector3 ra_cross_n = vector3_cross(ra, contact.normal);
	Vector3 rb_cross_n = vector3_cross(rb, contact.normal);
	Vector3 term_a = matrix3x3_mul_vector3(ra_cross_n, a->inertiaWorld_inv);
	Vector3 term_b = matrix3x3_mul_vector3(rb_cross_n, b->inertiaWorld_inv);
	double angular_factor = vector3_dot(vector3_cross(term_a, ra), contact.normal) +
							vector3_dot(vector3_cross(term_b, rb), contact.normal);
	double j = -(1 + restitution) * vel_along_normal; // bounciness factor
	j /= inv_mass_a + inv_mass_b + angular_factor + 1e-8; // avoid div by zero
	Vector3 impulse = vector3_scale(contact.normal, j);
	// Apply impulse
	if (!a->isStatic)
	{
		a->linearVelocity = vector3_add(a->linearVelocity, vector3_scale(impulse, inv_mass_a));
		a->angularVelocity = vector3_add(a->angularVelocity, matrix3x3_mul_vector3(vector3_cross(ra, impulse), a->inertiaWorld_inv));
	}
	if (!b->isStatic)
	{
		b->linearVelocity = vector3_subtract(b->linearVelocity, vector3_scale(impulse, inv_mass_b));
		b->angularVelocity = vector3_subtract(b->angularVelocity, matrix3x3_mul_vector3(vector3_cross(rb, impulse), b->inertiaWorld_inv));
	}
}

void physicsWorld_Init(PhysicsWorld* physicsWorld, Vector3 gravity)
{
	physicsWorld->body_count = 0;
	physicsWorld->gravity = gravity;
}

void physicsWorld_AddBody(PhysicsWorld* physicsWorld, RigidBody rigidbody)
{
	if (physicsWorld->body_count >= MAX_BODIES)
	{
		return -1;
	}

	physicsWorld->bodies[physicsWorld->body_count++] = rigidbody; // Add to world
}

void physicsWorld_Update(PhysicsWorld* physicsWorld, float dt)
{
	// Already recieved user applied forces

	for (int i = 0; i < physicsWorld->body_count; i++)
	{
		// apply gravity
		Vector3 gravity_force = vector3_scale(physicsWorld->gravity, physicsWorld->bodies[i].mass);
		rb_apply_force(&physicsWorld->bodies[i], gravity_force, physicsWorld->bodies[i].position);

		// integrate
		integrate(&physicsWorld->bodies[i], dt);
	}

	// Collision detection and resolution
	for (int i = 0; i < physicsWorld->body_count; i++)
	{
		for (int j = i + 1; j < physicsWorld->body_count; j++)
		{
			Contact contact;
			contact.collided = false;
			switch (physicsWorld->bodies[i].shape.type)
			{
				case SHAPE_SPHERE:
				{
					switch (physicsWorld->bodies[j].shape.type)
					{
						case SHAPE_SPHERE:
							contact = collide_sphere_sphere(&physicsWorld->bodies[i], &physicsWorld->bodies[j]);
							break;
						case SHAPE_BOX:
							contact = collide_box_sphere(&physicsWorld->bodies[j], &physicsWorld->bodies[i]);
							contact.normal = vector3_scale(contact.normal, -1.0); // invert normal to point out of A
							break;
						case SHAPE_PLANE:
							contact = collide_sphere_plane(&physicsWorld->bodies[i], &physicsWorld->bodies[j]);
							contact.normal = vector3_scale(contact.normal, -1.0);
							break;
					}
					break;
				}
				case SHAPE_BOX:
				{
					switch (physicsWorld->bodies[j].shape.type)
					{
						case SHAPE_SPHERE:
							contact = collide_box_sphere(&physicsWorld->bodies[i], &physicsWorld->bodies[j]);
							break;
						case SHAPE_BOX:
							contact = collide_box_box(&physicsWorld->bodies[i], &physicsWorld->bodies[j]);
							break;
						case SHAPE_PLANE:
							contact = collide_box_plane(&physicsWorld->bodies[i], &physicsWorld->bodies[j]);
							break;
					}
					break;
				}
				case SHAPE_PLANE:
				{
					switch (physicsWorld->bodies[j].shape.type)
					{
						case SHAPE_SPHERE:
							contact = collide_sphere_plane(&physicsWorld->bodies[j], &physicsWorld->bodies[i]);
							contact.normal = vector3_scale(contact.normal, -1.0); // invert normal to point out of A
							break;
						case SHAPE_BOX:
							contact = collide_box_plane(&physicsWorld->bodies[j], &physicsWorld->bodies[i]);
							contact.normal = vector3_scale(contact.normal, -1.0); // invert normal to point out of A
							break;
						case SHAPE_PLANE:
							// Plane-plane collision is not defined
							break;
					}
					break;
				}
			}

			if (contact.collided)
			{
				double average_restitution = (physicsWorld->bodies[i].restitution = physicsWorld->bodies[j].restitution ) / 2.0;
				resolve_contact(&physicsWorld->bodies[i], &physicsWorld->bodies[j], contact, average_restitution);
			}
		}
	}
}
