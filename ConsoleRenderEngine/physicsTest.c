#include "physicsTest.h"

#define BODY_COUNT 50
#define TWO_PI 6.28318530718

void playerController(RigidBody *player, Quaternion rotation);

void cameraController(Vector3 target, Vector3* camera_pos, Quaternion* camera_angle, double dt);

void physics_test(void)
{
	PhysicsWorld world;

	// initialize the world with earth's gravity
	physicsWorld_Init(&world, (Vector3) { 0.0, 9.81, 0.0 });
	
	for (int i = 0; i < BODY_COUNT; i++)
	{
		// creates a sphere
		double size = 0.2;
		Vector3 position = vector3_add((Vector3) { 0.0, -1.0 - size * i, 2.0 }, vector3_scale(vector3_random(), 0.05));

		RigidBody sphere = rb_create_sphere(position, size, 1.0);
		sphere.restitution = 1;
		sphere.friction = 1;
		physicsWorld_AddBody(&world, sphere);

		//creates a box
		//Vector3 half_extents = (Vector3){0.5, 0.5, 0.5};
		//Vector3 position = vector3_add((Vector3) { 0.0, -1.0 - 2 * half_extents.y * (i*2 + 1), 4.0 }, vector3_scale(vector3_random(), 0.05));

		//Quaternion orientation = quat_from_euler(vector3_random().x * TWO_PI, vector3_random().y * TWO_PI, vector3_random().z * TWO_PI);
		//orientation = quat_normalize(orientation);
		//RigidBody box = rb_create_box(position, half_extents, orientation, 1.0);
		//box.restitution = 0.2;
		//physicsWorld_AddBody(&world, box);
	}
	

	// creates the ground plane
	RigidBody ground = rb_create_plane((Vector3) { 0.0, -1.0, 0.0 }, -1.5);
	ground.restitution = 0.5;
	ground.friction = 1;
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
	system("cls");

	Quaternion camera_angle = quat_from_axis_angle((Vector3) { 1, 0, 0 }, -3.14 / 8.0);
	Vector3 camera_pos = { 0 };

	Texture *texture = (Texture*)malloc(sizeof(Texture));
	if (texLoader_LoadTexture(texture, L"textures\\texture_test.png") == 0)
	{
		printf("Texture Load Fail");
		return;
	}

	int* textureIDs = calloc(world.body_count, sizeof(int));

	// update loop
	while (isRunning)
	{
		// update physics
		sim_frequency = 0;
		for (int i = 0; i < 3; i++)
		{
			playerController(&world.rigidbodies[0], camera_angle);

			// update
			physicsWorld_Update(&world, 0.00001 * deltaTime); // 16% realtime
			sim_frequency++;
		}

		cameraController(world.rigidbodies[0].body.position, &camera_pos, &camera_angle, deltaTime);


		//rendering
		renderer_raytrace(world.bodies, textureIDs, texture, world.body_count, camera_pos, camera_angle, 90.0);

		// send frame to console
		renderFrame();

		// DEBUG INFO
		
		// print the object count
		printf("object count: %d \n", world.body_count);

		// print object - object data
		//for (int i = 0; i < world.body_count; i++)
		//{
		//	printf("obj %d pos: { %lf, %lf, %lf } \n", i, world.rigidbodies[i].body.position.x, world.rigidbodies[i].body.position.y, world.rigidbodies[i].body.position.z);
		//
		//	for (int j = i + 1; j < world.body_count; j++)
		//	{
		//		double average_restitution = (world.rigidbodies[i].restitution * world.rigidbodies[j].restitution) / 2.0;
		//		double average_friction = (world.rigidbodies[i].friction * world.rigidbodies[j].friction) / 2.0;

		//		printf("obj %d and obj %d: ", i, j);
		//		printf("{ average_restitution: %lf ", average_restitution);
		//		printf("average_friction: %lf }\n", average_friction);
		//	}
		//}

		//Contact contact;
		//contact = collide_box_plane(&world.rigidbodies[0], &world.rigidbodies[world.body_count - 1]);

		//printf("collided: %d depth: %lf \n", contact.collided, contact.contact_depth);

		printf("FPS: %.2lf\n", 1000 / deltaTime);

		// print the simulation frequency
		sim_frequency *= (1000 / deltaTime);
		printf("TPS: %.2lf\n", sim_frequency);
		// frame timing
		printfFrameTimes(16);

		tick++;

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			isRunning = false;
		}
	}

	free(textureIDs);
	texLoader_FreeTexture(texture);

	end();

	return 0;
}

void cameraController(Vector3 target, Vector3* camera_pos, Quaternion* camera_angle, double dt)
{
	const double speed = 350;
	double input = 0;

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		input--;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		input++;
	}

	*camera_angle = quat_integrate(*camera_angle, (Vector3) { 0, input * speed * dt / 16, 0 }, 0.00016);


	Vector3 camera_forward = quat_rotate_vector(*camera_angle, (Vector3) { 0, 0, 2 });

	*camera_pos = vector3_add(vector3_subtract(target, camera_forward), (Vector3) {0, -1, 0});
}

void playerController(RigidBody *player, Quaternion rotation)
{
	const double power = 10;
	const double counterMoveMult = 0.02;
	
	Vector2 input = { 0 };

	if (GetAsyncKeyState('W') & 0x8000) {
		input.y++;
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		input.y--;
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		input.x--;
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		input.x++;
	}

	//input = vector2_normalize(input);


	rb_apply_force(player, quat_rotate_vector(rotation, (Vector3){ input.x* power, 0, input.y* power }), player->body.position);

	// correction force

	Vector3 forward = { 0,0,1 };
	Vector3 right = { -1,0,0 };

	forward = quat_rotate_vector(rotation, forward);
	right = quat_rotate_vector(rotation, right);

	Vector2 mag = (Vector2){ vector3_dot(right, player->linearVelocity), vector3_dot(forward, player->linearVelocity) };

	rb_apply_force(player, vector3_scale(right , -mag.x * (1.0 - fabs(input.x))  * counterMoveMult), player->body.position);

	rb_apply_force(player, vector3_scale(forward, -mag.y * (1.0 - fabs(input.y)) * counterMoveMult), player->body.position);

	if (vector2_magnitude(input) == 0)
	{
		player->angularVelocity = (Vector3){ 0,0,0 };
	}
}