#include "physicsTest.h"


#define BODY_COUNT 20
#define TWO_PI 6.28318530718

void physics_test(void)
{
	PhysicsWorld world;

	// initialize the world with earth's gravity
	physicsWorld_Init(&world, (Vector3) { 0.0, 0.981, 0.0 });
	
	for (int i = 0; i < BODY_COUNT; i++)
	{
		// creates a sphere
		//double size = 0.1;
		//Vector3 position = vector3_add((Vector3) { 0.0, -1.0 - size * i, 2.0 }, vector3_scale(vector3_random(), 0.05));

		//RigidBody sphere = rb_create_sphere(position, size, 1.0);
		//sphere.restitution = 0.999;
		//physicsWorld_AddBody(&world, sphere);

		//creates a box
		Vector3 half_extents = (Vector3){0.3, 0.4, 0.5};
		Vector3 position = vector3_add((Vector3) { 0.0, -1.0 - 2 * half_extents.y * (i*2 + 1), 2.0 }, vector3_scale(vector3_random(), 0.05));

		Quaternion orientation = quat_from_euler(vector3_random().x * TWO_PI, vector3_random().y * TWO_PI, vector3_random().z * TWO_PI);
		orientation = quat_normalize(orientation);
		RigidBody box = rb_create_box(position, half_extents, orientation, 1.0);
		box.restitution = 0.2;
		physicsWorld_AddBody(&world, box);
	}
	

	// creates the ground plane
	RigidBody ground = rb_create_plane((Vector3) { 0.0, -1.0, 0.0 }, -1.5);
	ground.restitution = 0.999;
	physicsWorld_AddBody(&world, ground);

	// inits the renderer
	int init_status = userInit();
	if (init_status != 0)
	{
		return init_status;
	}

	bool isRunning = true;
	int tick = 0;
	double sim_frequency = 0;
	resetDeltaTime();

	// update loop
	while (isRunning)
	{
		blank(); // clear screen

		// update physics
		sim_frequency = 0;
		for (int i = 0; i < (int)deltaTime / 10 + 1; i++)
		{
			// apply forces
			for (int i = 0; i < world.body_count; i++)
			{

				Vector3 target = (Vector3){ 0.0, 1.0, 2.0 };
				Vector3 force = vector3_scale(vector3_normalize(vector3_subtract(target, world.rigidbodies[i].body.position)), 0.5);
				rb_apply_force(&world.rigidbodies[i], force, world.rigidbodies[i].body.position);
			}

			// update
			physicsWorld_Update(&world, 0.00016); // 1% realtime
			sim_frequency++;
		}

		//rendering
		renderer_raytrace(world.bodies, world.body_count, 90.0);

		// send frame to console
		renderFrame(); //16 = 60fps, 32 = 30fps

		// DEBUG INFO
		
		// print the object count
		printf("object count: %d \n", world.body_count);

		// print object positions
		//for (int i = 0; i < world.body_count; i++)
		//{
		//	printf("obj %d pos: { %lf, %lf, %lf } \n", i, world.rigidbodies[i].body.position.x, world.rigidbodies[i].body.position.y, world.rigidbodies[i].body.position.z);
		//}

		Contact contact;
		contact = collide_box_plane(&world.rigidbodies[0], &world.rigidbodies[world.body_count - 1]);

		printf("collided: %d depth: %lf \n", contact.collided, contact.contact_depth);

		// print the simulation frequency
		sim_frequency /= (deltaTime / 1000.0);
		printf("simulation frequency: %lf TPS \n", sim_frequency);

		// frame timing
		printfFrameTimes(32, tick);

		tick++;

		// check if the esc key was pressed exit the loop
		if (_kbhit())
		{
			int input = _getch();
			if (input == 27)// 'esc' key
				isRunning = false;
			if (input == 'r') // 'r' key
			{

			}
		}
	}

	end();

	return 0;
}