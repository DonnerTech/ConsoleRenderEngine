#include "physicsTest.h"

#define SPHERE_COUNT 100

// hacked together test of physics engine
// missing proper render engine integration
// kinda winging it this is too fun
void sphere_physics_test(void)
{
	//PhysicsWorld *world = malloc(sizeof(PhysicsWorld));
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

	int renderer_quality = LOW_QUALITY_RT, rq_cooldown = 0;

	bool isRunning = true;
	int tick = 0;
	double sim_frequency = 0;
	resetDeltaTime();
	// render loop
	while (isRunning)
	{
		blank(); // clear screen

		// update physics
		sim_frequency = 0;
		for (int i = 0; i < (int)deltaTime / 10 + 1; i++)
		{
			// apply forces
			for (int i = 0; i < SPHERE_COUNT; i++)
			{

				Vector3 target = (Vector3){ 0.0, 1.0, 2.0 };
				Vector3 force = vector3_scale(vector3_normalize(vector3_subtract(target, world.bodies[i].position)), 0.5);
				rb_apply_force(&world.bodies[i], force, world.bodies[i].position);
			}

			// update
			physicsWorld_Update(&world, 0.00016); // 1% realtime
			sim_frequency++;
		}

		
		// update renderer (jank)
		for (int i = 0; i < SPHERE_COUNT; i++)
		{
			sphere_poses[i] = world.bodies[i].position;
			sphere_radii[i] = world.bodies[i].shape.sphere.radius;
		}

		//rendering
		fsrayTraceMultithreaded(sphere_poses, sphere_radii, SPHERE_COUNT, 90, 20, renderer_quality);

		// send frame to console
		renderFrame(); //16 = 60fps, 32 = 30fps

		// debug
		printf("sphere position: ( %lf, %lf, %lf )\n", world.bodies[0].position.x, world.bodies[0].position.y, world.bodies[0].position.z);
		printf("plane position: ( %lf, %lf, %lf )\n", world.bodies[SPHERE_COUNT].position.x, world.bodies[SPHERE_COUNT].position.y, world.bodies[SPHERE_COUNT].position.z);
		sim_frequency /= (deltaTime / 1000.0);
		printf("simulation frequency: %lf TPS \n", sim_frequency);

		// frame timing
		printfFrameTimes(32, tick);

		tick++;
		rq_cooldown -= deltaTime;

		// check if the esc key was pressed exit the loop
		if (_kbhit())
		{
			int input = _getch();
			if (input == 27)// 'esc' key
				isRunning = false;
			if (input == 'r') // 'r' key
			{
				if (rq_cooldown <= 0)
				{
					renderer_quality = renderer_quality ? LOW_QUALITY_RT : HIGH_QUALITY_RT;
					rq_cooldown = 1000; // miliseconds
				}
			}
		}
	}
	end();

	return 0;
}