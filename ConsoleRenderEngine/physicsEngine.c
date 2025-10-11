#include "physicsEngine.h"

RigidBody rb_create_box(Vector3 pos, Vector3 half_extents, Quaternion orientation, double mass)
{
	RigidBody rigidbody = { 0 };
	Body body = { 0 };

	body.type = SHAPE_BOX;
	body.box.half_extents = half_extents;

	rigidbody.body = body;
	rigidbody.body.position = pos;
	rigidbody.body.orientation = orientation;
	rigidbody.friction = 50;
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
	Body body  = { 0 };

	body.type = SHAPE_SPHERE;
	body.sphere.radius = radius;

	rigidbody.body = body;
	rigidbody.body.position = pos;
	rigidbody.body.orientation = quat_identity();
	rigidbody.friction = 50;
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
	Body body = { 0 };

	body.type = SHAPE_PLANE;
	body.plane.normal = normal;
	body.plane.offset = offset;

	rigidbody.body = body;
	rigidbody.body.position = (Vector3){ 0.0,0.0,0.0 };
	rigidbody.body.orientation = quat_identity();
	rigidbody.friction = 50;
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

	Vector3 r = vector3_subtract(point, body->body.position); // not fully accurate lever vector
	Vector3 torque = vector3_cross(r, force);

	rb_apply_torque(&body, torque);

}

void rb_apply_torque(RigidBody* body, Vector3 torque)
{
	Vector3 angular_accel = vector3_mul_matrix3x3(torque, body->inertiaWorld_inv);

	body->angularVelocity = vector3_add(
		body->angularVelocity,
		angular_accel
	);
}

void update_inertia_tensor(RigidBody* body)
{
	if (body->isStatic) return;

	// Build rotation matrix from orientation quaternion
	Matrix3x3 R = quat_to_matrix3(body->body.orientation);

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
	body->body.position = vector3_add(
		body->body.position,
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
		Quaternion dq = quat_multiply(wq, body->body.orientation);
		dq.w *= 0.5 * dt;
		dq.x *= 0.5 * dt;
		dq.y *= 0.5 * dt;
		dq.z *= 0.5 * dt;

		// q = q + dq
		body->body.orientation.w += dq.w;
		body->body.orientation.x += dq.x;
		body->body.orientation.y += dq.y;
		body->body.orientation.z += dq.z;

		// Normalize to avoid drift
		body->body.orientation = quat_normalize(body->body.orientation);
	}

	// update world inertia tensor
	update_inertia_tensor(&body);
}

double point_plane_distance(Vector3 point, RigidBody* plane)
{
	return vector3_dot(point, plane->body.plane.normal) - plane->body.plane.offset;
}

Contact collide_sphere_plane(RigidBody* sphere, RigidBody* plane)
{
	Contact contact = { 0 };
	contact.collided = false;
	
	// check for intersection. 
	// find the point, the depth, and the normal that points out of A.

	double distance = point_plane_distance(sphere->body.position, plane);

	if(distance < sphere->body.sphere.radius)
	{
		contact.collided = true;
		contact.contact_depth = sphere->body.sphere.radius - distance;
		contact.normal = vector3_scale(plane->body.plane.normal, -1.0); // point out of sphere

		// median point of contact
		contact.contact_point = vector3_subtract(sphere->body.position, vector3_scale(plane->body.plane.normal, 0.5 * distance));
	}
	
	return contact;
}

Contact collide_box_plane(RigidBody* box, RigidBody* plane)
{
	Vector3 corners[8];
	double distances[8];
	int index = 0;

	for(int x = -1; x <= 1; x += 2)
	{
		for (int y = -1; y <= 1; y += 2)
		{
			for (int z = -1; z <= 1; z += 2)
			{
				Vector3 localCorner = (Vector3){ box->body.box.half_extents.x * x, box->body.box.half_extents.y * y, box->body.box.half_extents.z * z };

				// Transform to world space
				Vector3 rotatedCorner = quat_rotate_vector(box->body.orientation, localCorner);
				
				corners[index] = vector3_add(box->body.position, rotatedCorner);

				distances[index] = point_plane_distance(corners[index], plane);

				index++;
			}
		}
	}

	//TODO: Instead calculate the weighted average of the position and depth of the corners that are penetrating the plane

	// Calculates the deepest point
	Contact contact = { 0 };
	contact.collided = false;
	double minDistance = 0;

	for (int i = 0; i < 8; i++)
	{
		if (distances[i] < minDistance)
		{
			minDistance = distances[i];

			contact.collided = true;
			contact.contact_depth = -distances[i];
			contact.contact_point = corners[i];
			contact.normal = vector3_scale(plane->body.plane.normal, -1.0);
		}
	}

	return contact;
}

Contact collide_sphere_sphere(RigidBody* sphereA, RigidBody* sphereB)
{
	Contact contact = { 0 };
	contact.collided = false;

	// vector from A to B
	Vector3 direction = vector3_subtract(sphereB->body.position, sphereA->body.position);
	contact.contact_depth = (sphereA->body.sphere.radius + sphereB->body.sphere.radius) - vector3_magnitude(direction);
	
	//check for collision
	if (contact.contact_depth > 0)
	{
		contact.collided = true;

		// the deepest point on sphere A inside sphere B
		Vector3 normalInto = vector3_normalize(direction);
		contact.normal = vector3_scale(normalInto, -1.0);

		// deepest point
		//contact.contact_point = vector3_add(sphereA->position, vector3_scale(normalInto, sphereA->body.sphere.radius));

		// median point
		contact.contact_point = vector3_add(sphereA->body.position, vector3_scale(direction, 0.5));
	}

	return contact;
}

Contact collide_box_sphere(RigidBody* box, RigidBody* sphere)
{
	// Unimplemented
	Contact contact = { 0 };
	contact.collided = false;

	return contact;
}

Contact collide_box_box(RigidBody* boxA, RigidBody* boxB /*, Contact* contacts*/)
{
	// Unimplemented
	Contact contact = { 0 };
	contact.collided = true;
	contact.contact_point = (Vector3){ 0.0,0.0,0.0 };

	// Get local axes
	Vector3 Aaxis[3] = {
		quat_rotate_vector(boxA->body.orientation, (Vector3) { 1,0,0 }),
		quat_rotate_vector(boxA->body.orientation, (Vector3) { 0,1,0 }),
		quat_rotate_vector(boxA->body.orientation, (Vector3) { 0,0,1 })
	};
	Vector3 Baxis[3] = {
		quat_rotate_vector(boxB->body.orientation, (Vector3) { 1,0,0 }),
		quat_rotate_vector(boxB->body.orientation, (Vector3) { 0,1,0 }),
		quat_rotate_vector(boxB->body.orientation, (Vector3) { 0,0,1 })
	};

	Matrix3x3 R;
	Matrix3x3 AbsR;

	// Rotation matrix from A to B
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			R.m[i][j] = vector3_dot(Aaxis[i], Baxis[j]);

	// Compute translation of B relative to A (in A’s local coordinates)
	Vector3 t_world = vector3_subtract(boxB->body.position, boxA->body.position);
	double tA[3] = {
		vector3_dot(t_world, Aaxis[0]),
		vector3_dot(t_world, Aaxis[1]),
		vector3_dot(t_world, Aaxis[2])
	};

	// Compute AbsR with small epsilon to counter floating-point errors
	const double EPS = 1e-8;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			AbsR.m[i][j] = fabs(R.m[i][j]) + EPS;

	double A[3] = {
	boxA->body.box.half_extents.x,
	boxA->body.box.half_extents.y,
	boxA->body.box.half_extents.z
	};


	double B[3] = {
	boxA->body.box.half_extents.x,
	boxA->body.box.half_extents.y,
	boxA->body.box.half_extents.z
	};

	double minPenetration = INFINITY;
	Vector3 bestAxis = { 0 };  // world-space normal of the minimum penetration axis

	double ra, rb, proj, overlap;

	// Test axes L = A0, A1, A2
	for (int i = 0; i < 3; i++) {
		ra = A[i];
		rb = boxB->body.box.half_extents.x * AbsR.m[i][0] + boxB->body.box.half_extents.y * AbsR.m[i][1] + boxB->body.box.half_extents.z * AbsR.m[i][2];
		proj = fabs(tA[i]);
		if (proj > ra + rb) return contact;

		overlap = (ra + rb) - fabs(proj);
		if (overlap < minPenetration) {
			minPenetration = overlap;
			// store axis direction (normalized)
			bestAxis = Aaxis[i];

			// Ensure the axis points from A to B
			if (vector3_dot(bestAxis, t_world) > 0)
				bestAxis = vector3_scale(bestAxis, -1.0);

			double planeOffset = vector3_dot(bestAxis, boxA->body.position);
			Vector3 sum = { 0.0, 0.0, 0.0 };
			int count = 1;

			for (int x = -1; x <= 1; x += 2)
			{
				for (int y = -1; y <= 1; y += 2)
				{
					for (int z = -1; z <= 1; z += 2)
					{
						Vector3 corner = (Vector3){ A[0] * x, A[1] * y, A[2] * z };
						corner = quat_rotate_vector(boxA->body.orientation, corner);

						double dist = vector3_dot(bestAxis, corner) - planeOffset;

						if (dist < 0)
						{
							//contacts[count] = (Contact){ true, bestAxis, minPenetration, vector3_add(boxA->body.position, corner) };
							sum = vector3_add(sum, corner);
							count++; 
						}
					}
				}
			}

			contact.contact_point = vector3_add(boxA->body.position, vector3_scale(sum, 1.0 / count));
			
		}
	}

	// Test axes L = B0, B1, B2
	for (int j = 0; j < 3; j++) {
		ra = boxA->body.box.half_extents.x * AbsR.m[0][j] + boxA->body.box.half_extents.y * AbsR.m[1][j] + boxA->body.box.half_extents.z * AbsR.m[2][j];
		rb = B[j];
		proj = fabs(tA[0] * R.m[0][j] + tA[1] * R.m[1][j] + tA[2] * R.m[2][j]);
		if (proj > ra + rb) return contact;

		overlap = (ra + rb) - fabs(proj);
		if (overlap < minPenetration) {
			minPenetration = overlap;
			// store axis direction (normalized)
			bestAxis = Baxis[j];


			// Ensure the axis points from A to B
			if (vector3_dot(bestAxis, t_world) > 0)
				bestAxis = vector3_scale(bestAxis, -1.0);

			double planeOffset = vector3_dot(bestAxis, boxB->body.position);
			Vector3 sum = { 0.0, 0.0, 0.0 };
			int count = 1;

			for (int x = -1; x <= 1; x += 2)
			{
				for (int y = -1; y <= 1; y += 2)
				{
					for (int z = -1; z <= 1; z += 2)
					{
						Vector3 corner = (Vector3){ B[0] * x, B[1] * y, B[2] * z };
						corner = quat_rotate_vector(boxB->body.orientation, corner);

						double dist = vector3_dot(bestAxis, corner) - planeOffset;

						if (dist < 0)
						{
							sum = vector3_add(sum, corner);
							count++;
						}
					}
				}
			}

			contact.contact_point = vector3_add(boxB->body.position, vector3_scale(sum, 1.0 / count));
		}
	}

	Vector3 a = boxA->body.box.half_extents;
	Vector3 b = boxB->body.box.half_extents;

	bool isEdgeContact = false;
	int bestAxisAIndex = 0;
	int bestAxisBIndex = 0;

	// Axis L = A0 x B0
	ra = a.y * AbsR.m[2][0] + a.z * AbsR.m[1][0];
	rb = b.y * AbsR.m[0][2] + b.z * AbsR.m[0][1];
	proj = fabs(tA[2] * R.m[1][0] - tA[1] * R.m[2][0]);
	if (proj > ra + rb) return contact;
	overlap = (ra + rb) - fabs(proj);
	if (overlap < minPenetration) {
		minPenetration = overlap;
		bestAxis = vector3_normalize(vector3_cross(Aaxis[0], Baxis[0]));  // store axis direction (normalized)
		bestAxisAIndex = 0;
		bestAxisBIndex = 0;
		isEdgeContact = true;
	}

	// L = A0 x B1
	ra = a.y * AbsR.m[2][1] + a.z * AbsR.m[1][1];
	rb = b.x * AbsR.m[0][2] + b.z * AbsR.m[0][0];
	if (fabs(tA[2] * R.m[1][1] - tA[1] * R.m[2][1]) > ra + rb) return contact;
	overlap = (ra + rb) - fabs(proj);
	if (overlap < minPenetration) {
		minPenetration = overlap;
		bestAxis = vector3_normalize(vector3_cross(Aaxis[0], Baxis[1]));  // store axis direction (normalized)
		bestAxisAIndex = 0;
		bestAxisBIndex = 1;
		isEdgeContact = true;
	}

	// L = A0 x B2
	ra = a.y * AbsR.m[2][2] + a.z * AbsR.m[1][2];
	rb = b.x * AbsR.m[0][1] + b.y * AbsR.m[0][0];
	if (fabs(tA[2] * R.m[1][2] - tA[1] * R.m[2][2]) > ra + rb) return contact;
	if (overlap < minPenetration) {
		minPenetration = overlap;
		bestAxis = vector3_normalize(vector3_cross(Aaxis[0], Baxis[2]));  // store axis direction (normalized)
		bestAxisAIndex = 0;
		bestAxisBIndex = 2;
		isEdgeContact = true;
	}

	// L = A1 x B0
	ra = a.x * AbsR.m[2][0] + a.z * AbsR.m[0][0];
	rb = b.y * AbsR.m[1][2] + b.z * AbsR.m[1][1];
	if (fabs(tA[0] * R.m[2][0] - tA[2] * R.m[0][0]) > ra + rb) return contact;
	if (overlap < minPenetration) {
		minPenetration = overlap;
		bestAxis = vector3_normalize(vector3_cross(Aaxis[1], Baxis[0]));  // store axis direction (normalized)
		bestAxisAIndex = 1;
		bestAxisBIndex = 0;
		isEdgeContact = true;
	}

	// L = A1 x B1
	ra = a.x * AbsR.m[2][1] + a.z * AbsR.m[0][1];
	rb = b.x * AbsR.m[1][2] + b.z * AbsR.m[1][0];
	if (fabs(tA[0] * R.m[2][1] - tA[2] * R.m[0][1]) > ra + rb) return contact;
	if (overlap < minPenetration) {
		minPenetration = overlap;
		bestAxis = vector3_normalize(vector3_cross(Aaxis[1], Baxis[1]));  // store axis direction (normalized)
		bestAxisAIndex = 1;
		bestAxisBIndex = 1;
		isEdgeContact = true;
	}

	// L = A1 x B2
	ra = a.x * AbsR.m[2][2] + a.z * AbsR.m[0][2];
	rb = b.x * AbsR.m[1][1] + b.y * AbsR.m[1][0];
	if (fabs(tA[0] * R.m[2][2] - tA[2] * R.m[0][2]) > ra + rb) return contact;
	if (overlap < minPenetration) {
		minPenetration = overlap;
		bestAxis = vector3_normalize(vector3_cross(Aaxis[1], Baxis[2]));  // store axis direction (normalized)
		bestAxisAIndex = 1;
		bestAxisBIndex = 2;
		isEdgeContact = true;
	}

	// L = A2 x B0
	ra = a.x * AbsR.m[1][0] + a.y * AbsR.m[0][0];
	rb = b.y * AbsR.m[2][2] + b.z * AbsR.m[2][1];
	if (fabs(tA[1] * R.m[0][0] - tA[0] * R.m[1][0]) > ra + rb) return contact;
	if (overlap < minPenetration) {
		minPenetration = overlap;
		bestAxis = vector3_normalize(vector3_cross(Aaxis[2], Baxis[0]));  // store axis direction (normalized)
		bestAxisAIndex = 2;
		bestAxisBIndex = 0;
		isEdgeContact = true;
	}

	// L = A2 x B1
	ra = a.x * AbsR.m[1][1] + a.y * AbsR.m[0][1];
	rb = b.x * AbsR.m[2][2] + b.z * AbsR.m[2][0];
	if (fabs(tA[1] * R.m[0][1] - tA[0] * R.m[1][1]) > ra + rb) return contact;
	if (overlap < minPenetration) {
		minPenetration = overlap;
		bestAxis = vector3_normalize(vector3_cross(Aaxis[2], Baxis[1]));  // store axis direction (normalized)
		bestAxisAIndex = 2;
		bestAxisBIndex = 1;
		isEdgeContact = true;
	}

	// L = A2 x B2
	ra = a.x * AbsR.m[1][2] + a.y * AbsR.m[0][2];
	rb = b.x * AbsR.m[2][1] + b.y * AbsR.m[2][0];
	if (fabs(tA[1] * R.m[0][2] - tA[0] * R.m[1][2]) > ra + rb) return contact;
	if (overlap < minPenetration) {
		minPenetration = overlap;
		bestAxis = vector3_normalize(vector3_cross(Aaxis[2], Baxis[2]));  // store axis direction (normalized)
		bestAxisAIndex = 2;
		bestAxisBIndex = 2;
		isEdgeContact = true;
	}

	if (isEdgeContact)
	{
		Vector3 aDir = Aaxis[bestAxisAIndex];
		Vector3 bDir = Baxis[bestAxisBIndex];


		Vector3 aCenter = boxA->body.position;
		for (int k = 0; k < 3; ++k) {
			double sign = (vector3_dot(Aaxis[k], bestAxis) > 0) ? 1.0 : -1.0;
			if (k != bestAxisAIndex)
				aCenter = vector3_add(aCenter, vector3_scale(Aaxis[k], sign * A[k]));
		}

		Vector3 bCenter = boxA->body.position;
		for (int k = 0; k < 3; ++k) {
			double sign = (vector3_dot(Baxis[k], bestAxis) > 0) ? 1.0 : -1.0;
			if (k != bestAxisBIndex)
				bCenter = vector3_add(bCenter, vector3_scale(Baxis[k], sign * B[k]));
		}

		Vector3 r = vector3_subtract(aCenter, bCenter);
		double aDotA = vector3_dot(aDir, aDir);
		double bDotB = vector3_dot(bDir, bDir);
		double aDotB = vector3_dot(aDir, bDir);
		double aDotR = vector3_dot(aDir, r);
		double bDotR = vector3_dot(bDir, r);

		double denom = aDotA * bDotB - aDotB * aDotB;
		double s = (aDotB * bDotR - bDotB * aDotR) / denom;
		double t = (aDotA * bDotR - aDotB * aDotR) / denom;

		Vector3 pA = vector3_add(aCenter, vector3_scale(aDir, s));
		Vector3 pB = vector3_add(bCenter, vector3_scale(bDir, t));
		contact.contact_point = vector3_scale(vector3_add(pA, pB), 0.5);
	}

	if (vector3_dot(bestAxis, t_world) > 0)
		bestAxis = vector3_scale(bestAxis, -1.0);

	contact.normal = bestAxis;
	contact.contact_depth = minPenetration;
	
	// this is a rough estimate of the contact point
	if(vector3_magnitude(contact.contact_point) < 1e-16)
		contact.contact_point = vector3_scale(vector3_add(boxA->body.position, boxB->body.position), 0.5);

	// If we got this far — no separating axis found
	return contact;
}

void resolve_contact(RigidBody* a, RigidBody* b, Contact contact, double restitution, double friction)
{
	if (!contact.collided) return;
	// Separate the rigidbodies to avoid sinking
	if (!a->isStatic)
	{
		a->body.position = vector3_add(a->body.position, vector3_scale(contact.normal, contact.contact_depth * 0.5));
	}
	if (!b->isStatic)
	{
		b->body.position = vector3_subtract(b->body.position, vector3_scale(contact.normal, contact.contact_depth * 0.5));
	}
	// Relative velocity at contact point
	Vector3 ra = vector3_subtract(contact.contact_point, a->body.position);
	Vector3 rb = vector3_subtract(contact.contact_point, b->body.position);
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
	Vector3 term_a = vector3_mul_matrix3x3(ra_cross_n, a->inertiaWorld_inv);
	Vector3 term_b = vector3_mul_matrix3x3(rb_cross_n, b->inertiaWorld_inv);
	double angular_factor = vector3_dot(vector3_cross(term_a, ra), contact.normal) +
							vector3_dot(vector3_cross(term_b, rb), contact.normal);
	double j = -(1 + restitution) * vel_along_normal; // bounciness factor
	j /= inv_mass_a + inv_mass_b + angular_factor + 1e-8; // avoid div by zero
	Vector3 impulse = vector3_scale(contact.normal, j);
	// Apply impulse
	if (!a->isStatic)
	{
		a->linearVelocity = vector3_add(a->linearVelocity, vector3_scale(impulse, inv_mass_a));
		a->angularVelocity = vector3_add(a->angularVelocity, vector3_mul_matrix3x3(vector3_cross(ra, impulse), a->inertiaWorld_inv));
	}
	if (!b->isStatic)
	{
		b->linearVelocity = vector3_subtract(b->linearVelocity, vector3_scale(impulse, inv_mass_b));
		b->angularVelocity = vector3_subtract(b->angularVelocity, vector3_mul_matrix3x3(vector3_cross(rb, impulse), b->inertiaWorld_inv));
	}

	// Friction impulse
	Vector3 tangent = vector3_subtract(relative_velocity, vector3_scale(contact.normal, vel_along_normal));
	if (vector3_magnitude(tangent) > 1e-8)
	{
		tangent = vector3_normalize(tangent);
		double jt = -vector3_dot(relative_velocity, tangent);
		jt /= inv_mass_a + inv_mass_b + 
			  vector3_dot(vector3_cross(vector3_mul_matrix3x3(vector3_cross(ra, tangent), a->inertiaWorld_inv), ra), tangent) +
			  vector3_dot(vector3_cross(vector3_mul_matrix3x3(vector3_cross(rb, tangent), b->inertiaWorld_inv), rb), tangent) + 1e-8; // avoid div by zero
		// Coulomb's law
		double mu = friction;
		Vector3 frictionImpulse;
		if (fabs(jt) < j * mu)
			frictionImpulse = vector3_scale(tangent, jt);
		else
			frictionImpulse = vector3_scale(tangent, -j * mu);
		// Apply friction impulse
		if (!a->isStatic)
		{
			a->linearVelocity = vector3_add(a->linearVelocity, vector3_scale(frictionImpulse, inv_mass_a));
			a->angularVelocity = vector3_add(a->angularVelocity, vector3_mul_matrix3x3(vector3_cross(ra, frictionImpulse), a->inertiaWorld_inv));
		}
		if (!b->isStatic)
		{
			b->linearVelocity = vector3_subtract(b->linearVelocity, vector3_scale(frictionImpulse, inv_mass_b));
			b->angularVelocity = vector3_subtract(b->angularVelocity, vector3_mul_matrix3x3(vector3_cross(rb, frictionImpulse), b->inertiaWorld_inv));
		}
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
		return;
	}

	physicsWorld->rigidbodies[physicsWorld->body_count++] = rigidbody; // Add to world
}

void physicsWorld_Update(PhysicsWorld* physicsWorld, double dt)
{
	// Already recieved user applied forces

	for (int i = 0; i < physicsWorld->body_count; i++)
	{
		physicsWorld->bodies[i] = physicsWorld->rigidbodies[i].body; // update body pointers for rendering
	}

	for (int i = 0; i < physicsWorld->body_count; i++)
	{
		// apply gravity
		Vector3 gravity_force = vector3_scale(physicsWorld->gravity, physicsWorld->rigidbodies[i].mass);
		rb_apply_force(&physicsWorld->rigidbodies[i], gravity_force, physicsWorld->rigidbodies[i].body.position);

		// integrate
		integrate(&physicsWorld->rigidbodies[i], dt);
	}

	// Collision detection and resolution
	for (int i = 0; i < physicsWorld->body_count; i++)
	{
		for (int j = i + 1; j < physicsWorld->body_count; j++)
		{
			Contact contact;
			contact.collided = false;
			switch (physicsWorld->rigidbodies[i].body.type)
			{
				case SHAPE_SPHERE:
				{
					switch (physicsWorld->rigidbodies[j].body.type)
					{
						case SHAPE_SPHERE:
							contact = collide_sphere_sphere(&physicsWorld->rigidbodies[i], &physicsWorld->rigidbodies[j]);
							break;
						case SHAPE_BOX:
							contact = collide_box_sphere(&physicsWorld->rigidbodies[j], &physicsWorld->rigidbodies[i]);
							contact.normal = vector3_scale(contact.normal, -1.0); // invert normal to point out of A
							break;
						case SHAPE_PLANE:
							contact = collide_sphere_plane(&physicsWorld->rigidbodies[i], &physicsWorld->rigidbodies[j]);
							contact.normal = vector3_scale(contact.normal, -1.0);
							break;
					}
					break;
				}
				case SHAPE_BOX:
				{
					switch (physicsWorld->rigidbodies[j].body.type)
					{
						case SHAPE_SPHERE:
							contact = collide_box_sphere(&physicsWorld->rigidbodies[i], &physicsWorld->rigidbodies[j]);
							break;
						case SHAPE_BOX:
							contact = collide_box_box(&physicsWorld->rigidbodies[i], &physicsWorld->rigidbodies[j]);
							break;
						case SHAPE_PLANE:
							contact = collide_box_plane(&physicsWorld->rigidbodies[i], &physicsWorld->rigidbodies[j]);
							contact.normal = vector3_scale(contact.normal, -1.0); // invert normal to point out of A
							break;
					}
					break;
				}
				case SHAPE_PLANE:
				{
					switch (physicsWorld->rigidbodies[j].body.type)
					{
						case SHAPE_SPHERE:
							contact = collide_sphere_plane(&physicsWorld->rigidbodies[j], &physicsWorld->rigidbodies[i]);
							contact.normal = vector3_scale(contact.normal, -1.0); // invert normal to point out of A
							break;
						case SHAPE_BOX:
							contact = collide_box_plane(&physicsWorld->rigidbodies[j], &physicsWorld->rigidbodies[i]);
							//contact.normal = vector3_scale(contact.normal, -1.0); // invert normal to point out of A
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
				double average_restitution = (physicsWorld->rigidbodies[i].restitution * physicsWorld->rigidbodies[j].restitution ) / 2.0;
				double average_friction = (physicsWorld->rigidbodies[i].friction * physicsWorld->rigidbodies[j].friction) / 2.0;
				resolve_contact(&physicsWorld->rigidbodies[i], &physicsWorld->rigidbodies[j], contact, average_restitution, average_friction);
			}
		}
	}
}
