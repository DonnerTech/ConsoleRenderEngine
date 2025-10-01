#include "physicsTest.h"

#define SPHERE_COUNT 10

// hacked together test of physics engine
// missing proper render engine integration
// kinda winging it this is too fun
void physics_test(void)
{
	PhysicsWorld world;
	physicsWorld_Init(&world, (Vector3) { 0.0, 0.981, 0.0 });

	Vector3 sphere_poses[SPHERE_COUNT];
	double sphere_radii[SPHERE_COUNT];

	for (int i = 0; i < SPHERE_COUNT; i++)
	{
		double size = 0.1;
		Vector3 position = vector3_add((Vector3) { 0.0, -1.0 - size * i, 2.0 }, vector3_scale(vector3_random(), 0.05));

		RigidBody sphere = rb_create_sphere(position, size, 1.0);
		sphere.restitution = 0.9;
		physicsWorld_AddBody(&world, sphere);
	}

	RigidBody ground = rb_create_plane((Vector3) { 0.0, -1.0, 0.0 }, -1.0);
	ground.restitution = 0.9;
	physicsWorld_AddBody(&world, ground);

	//RigidBody box = rb_create_box((Vector3) { 0.0, 5.0, 0.0 }, (Vector3) { 0.5, 0.5, 0.5 }, quat_identity(), 1.0);
	//box.restitution = 0.5;
	//physicsWorld_AddBody(&world, box);

	srand(1);

	int init_status = init();
	if (init_status != 0)
	{
		return init_status;
	}

	bool isRunning = true;
	int tick = 0;

	double rot = 0;

	// render loop
	while (isRunning)
	{
		blank(); // clear screen

		// apply forces
		for (int i = 0; i < SPHERE_COUNT; i++)
		{
			Vector3 target = (Vector3){ 0.0, -1.0, 2.0 };
			Vector3 force = vector3_scale(vector3_normalize(vector3_subtract(target, world.bodies[i].position)), 3);
			rb_apply_force(&world.bodies[i], force, world.bodies[i].position);
		}

		// update physics
		for (int i = 0; i < (int)deltaTime/10 + 1; i++)
		{
			physicsWorld_Update(&world, 0.00016); // 1% realtime
		}

		
		// update renderer (jank)
		for (int i = 0; i < SPHERE_COUNT; i++)
		{
			sphere_poses[i] = world.bodies[i].position;
			sphere_radii[i] = world.bodies[i].shape.sphere.radius;
		}

		//rendering
		fsRayTraceMultithreaded(sphere_poses, sphere_radii, SPHERE_COUNT, 90, 20);

		// send frame to console
		renderFrame(); //16 = 60fps, 32 = 30fps

		// debug
		printf("sphere position: ( %lf, %lf, %lf )\n", world.bodies[0].position.x, world.bodies[0].position.y, world.bodies[0].position.z);
		printf("plane position: ( %lf, %lf, %lf )\n", world.bodies[1].position.x, world.bodies[1].position.y, world.bodies[1].position.z);

		// frame timing
		printfFrameTimes(16, tick);

		tick++;


		// check if the esc key was pressed exit the loop
		if (_kbhit())
		{
			if (_getch() == 27)
				isRunning = false;
		}
	}
	end();

	return 0;
}