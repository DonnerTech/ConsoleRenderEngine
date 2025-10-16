#pragma once
#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "quaternion.h"
#include "vector3.h"
#include "matrix3x3.h"
#include "body.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct {
	Body body;

	Vector3 linearVelocity;
	Vector3 angularVelocity;

	double mass;
	double inv_mass;
	double restitution; // bounciness [0,1]
	double friction;    // [0,infinity)

	Matrix3x3 inertiaBody;     // local-space inertia matrix3
	Matrix3x3 inertiaBody_inv; // inverse inertia matrix3 in local space 
	Matrix3x3 inertiaWorld_inv;// inverse inertia matrix3 in world space

	bool isStatic; // 1 = static, 0 = dynamic
} RigidBody;

// Initialization
RigidBody rb_create_box(Vector3 pos, Vector3 half_extents, Quaternion orientation, double mass);
RigidBody rb_create_sphere(Vector3 pos, double radius, double mass);
RigidBody rb_create_plane(Vector3 normal, double offset);

// Dynamics
void rb_apply_force(RigidBody* body, Vector3 force, Vector3 point);
void rb_apply_torque(RigidBody* body, Vector3 torque);

// Integration of forces
void integrate(RigidBody* body, double dt);
void update_inertia_tensor(RigidBody* body);

typedef struct {
	int collided;
	Vector3 normal;  // collision normal (pointing out of body A)
	double contact_depth;
	Vector3 contact_point;
} Contact;

// Narrowphase collision tests
Contact collide_sphere_plane(RigidBody* sphere, RigidBody* plane);
Contact collide_box_plane(RigidBody* box, RigidBody* plane);
Contact collide_sphere_sphere(RigidBody* sphereA, RigidBody* sphereB);
Contact collide_box_sphere(RigidBody* box, RigidBody* sphere);
Contact collide_box_box(RigidBody* boxA, RigidBody* boxB);

// Collision Resolution
void resolve_contact(RigidBody* a, RigidBody* b, Contact contact, double restitution, double friction);

#define MAX_BODIES 1024

typedef struct {
	RigidBody rigidbodies[MAX_BODIES];
	Body bodies[MAX_BODIES]; // for rendering
	int body_count;

	Vector3 gravity;
} PhysicsWorld;


// Physics World Simulation
void physicsWorld_Init(PhysicsWorld* physicsWorld, Vector3 gravity);

void physicsWorld_AddBody(PhysicsWorld* physicsWorld, RigidBody rigidbody);

void physicsWorld_Update(PhysicsWorld* physicsWorld, float dt);

#endif