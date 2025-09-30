#pragma once
#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include "quaternion.h"
#include "vector3.h"
#include "matrix3x3.h"
#include <stdbool.h>

typedef enum {
    SHAPE_SPHERE,
    SHAPE_BOX,
    SHAPE_PLANE
} ShapeType;

typedef struct {
    double radius;
} SphereShape;

typedef struct {
    Vector3 half_extents; // half sizes (x,y,z)
} BoxShape;

typedef struct {
    Vector3 normal; // unit vector
    double offset;  // distance from origin along normal
} PlaneShape;

typedef struct {
    ShapeType type;
    union {
        SphereShape sphere;
        BoxShape box;
        PlaneShape plane;
    };
} CollisionShape;

typedef struct {
    Vector3 position;
    Quaternion orientation;

    Vector3 linearVelocity;
    Vector3 angularVelocity;

    double mass;
    double inv_mass;

    Matrix3x3 inertiaBody;     // local-space inertia matrix3
    Matrix3x3 inertiaBody_inv; // inverse inertia matrix3 in local space 
    Matrix3x3 inertiaWorld_inv;// inverse inertia matrix3 in world space

    CollisionShape shape;

    bool isStatic; // 1 = static, 0 = dynamic
} RigidBody;

// Initialization
RigidBody create_box(Vector3 pos, Vector3 half_extents, Quaternion orientation, double mass);
RigidBody create_sphere(Vector3 pos, double radius, double mass);
RigidBody create_plane(Vector3 normal, double offset);

// Dynamics
void apply_force(RigidBody* body, Vector3 force, Vector3 point);
void apply_torque(RigidBody* body, Vector3 torque);

// Integration
void integrate(RigidBody* body, double dt);

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
void resolve_contact(RigidBody* a, RigidBody* b, Contact contact, double restitution);

#define MAX_BODIES 128

typedef struct {
    RigidBody bodies[MAX_BODIES];
    int body_count;

    Vector3 gravity;
} PhysicsWorld;


// Physics World Simulation
void physicsWorld_Init(PhysicsWorld* physicsWorld, Vector3 gravity);

void physicsWorld_AddBody(PhysicsWorld *physicsWorld, ShapeType type, Vector3 position, Quaternion rotation, Vector3 dimension);

void physicsWorld_Update(float dt);

#endif