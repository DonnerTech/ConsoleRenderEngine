#include "physicsTest.h"

#define BODY_COUNT 100
#define TWO_PI 6.28318530718

void playerController(RigidBody *player, Quaternion rotation);

void cameraController(Vector3 target, Vector3* camera_pos, Quaternion* camera_angle, double dt);

void freeCam(Vector3* camera_pos, Quaternion* camera_angle, double dt);

void randomizePositions(Body* bodies, int count, double distance);

void physics_test(void)
{
	// INITIALIZE MATERIALS AND TEXTURES
	const int mat_count = 4;

	Material* mat_list = (Material*)malloc(sizeof(Material) * mat_count);

	if (mat_list == NULL)
	{
		printf("Material List Allocation Fail");
		return;
	}

	create_material(&mat_list[0], PROJECT_PLANER, (BYTE[4]) { 64, 64, 64, 255 }, 0);
	const short texture_path_0[60] = L"textures\\kenney_prototype-textures\\PNG\\Dark\\texture_05.png";
	if (mat_list[0].baseTexture == NULL || !texLoader_LoadTexture(mat_list[0].baseTexture, texture_path_0))
	{printf("Texture Load Fail"); return;}
	mat_list[0].baseTexture->uvScale = 0.0125f;


	create_material(&mat_list[1], PROJECT_PLANER, (BYTE[4]) { 100, 100, 100, 255 }, 1);
	texLoader_generateTexture(mat_list[1].baseTexture, 4, 2, 2);
	texLoader_fillTexture(mat_list[1].baseTexture, (BYTE[4]) { 200, 150, 10, 255 });

	create_material(&mat_list[2], PROJECT_PLANER, (BYTE[4]) { 20, 20, 20, 255 }, 1);
	texLoader_generateTexture(mat_list[2].baseTexture, 4, 2, 2);
	texLoader_fillTexture(mat_list[2].baseTexture, (BYTE[4]) { 20, 150, 250, 255 });

	create_material(&mat_list[3], PROJECT_LOCAL_SPHERICAL, (BYTE[4]) { 0, 0, 0, 0 }, 0);
	const short texture_path_1[60] = L"textures\\kenney_prototype-textures\\PNG\\Light\\texture_06.png";
	if (mat_list[3].baseTexture == NULL || !texLoader_LoadTexture(mat_list[3].baseTexture, texture_path_1))
	{printf("Texture Load Fail"); return;}
	mat_list[3].baseTexture->uvScale = 0.25f;

	short* matIDs = calloc(BODY_COUNT+1, sizeof(short));

	if(matIDs == NULL)
	{
		printf("Material ID Initialization Failed");
		return;
	}

	//INITIALIZE PHYSICS WORLD

	PhysicsWorld world;

	// initialize the world with earth's gravity
	physicsWorld_Init(&world, (Vector3) { 0.0, 9.81, 0.0 }); // 9.81

	for (int i = 0; i < BODY_COUNT; i++)
	{
		// creates a sphere
		matIDs[world.body_count] = i % 3 + 1; // alternate between texture ids 1 through 3

		double size = 0.5 + (double)(rand() % 1000) / 500;
		Vector3 position = vector3_add((Vector3) { 0.0, -2.5 - size * i, 2.0 }, vector3_scale(vector3_random(), 2));
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
	matIDs[world.body_count] = 0;
	RigidBody ground = rb_create_plane((Vector3) { 0.0, -1.0, 0.0 }, -1.5);
	ground.restitution = 0.5;
	ground.friction = 1;
	physicsWorld_AddBody(&world, ground);


	// INITIALIZE THE RENDERER
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

	// update loop
	while (isRunning)
	{

		// update physics
		sim_frequency = 0;
		for (int i = 0; i < 3; i++)
		{
			//playerController(&world.rigidbodies[0], camera_angle);

			// update
			physicsWorld_Update(&world, 0.00001 * deltaTime); // 16% realtime
			sim_frequency++;
		}

		//cameraController(world.rigidbodies[0].body.position, &camera_pos, &camera_angle, deltaTime);
		freeCam(&camera_pos, &camera_angle, deltaTime);

		//rendering
		if (!renderer_raytrace(world.bodies, matIDs, mat_list, world.body_count, camera_pos, camera_angle, 90.0))
		{
			printf("RT Error!");
			system("pause");
		}

		// send frame to console
		renderFrame();

		// DEBUG INFO
		
		// print the object count
		printf("object count: %d \n", world.body_count);

		//print object - object data
		//for (int i = 0; i < world.body_count; i++)
		//{
		//	printf("obj %d pos: { %lf, %lf, %lf } \n", i, world.rigidbodies[i].body.position.x, world.rigidbodies[i].body.position.y, world.rigidbodies[i].body.position.z);
		//	printf("obj type: %d\n", world.rigidbodies[i].body.type);
		//	//for (int j = i + 1; j < world.body_count; j++)
		//	//{
		//	//	double average_restitution = (world.rigidbodies[i].restitution * world.rigidbodies[j].restitution) / 2.0;
		//	//	double average_friction = (world.rigidbodies[i].friction * world.rigidbodies[j].friction) / 2.0;
		//	//	printf("obj %d and obj %d: ", i, j);
		//	//	printf("{ average_restitution: %lf ", average_restitution);
		//	//	printf("average_friction: %lf }\n", average_friction);
		//	//}
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

	free(matIDs);
	free_material_list(mat_list, mat_count);

	end();

	return 0;
}

void bvh_test(void)
{
	// INITIALIZE MATERIALS AND TEXTURES
	const int mat_count = 2;

	Material* mat_list = (Material*)malloc(sizeof(Material) * mat_count);

	if (mat_list == NULL)
	{
		printf("Material List Allocation Fail");
		return;
	}

	create_material(&mat_list[0], PROJECT_PLANER, (BYTE[4]) { 200, 200, 200, 255 }, 1);

	if (mat_list[0].baseTexture == NULL || texLoader_LoadTexture(mat_list[0].baseTexture, L"textures\\kenney_prototype-textures\\PNG\\Dark\\texture_05.png") == 0)
	{
		printf("Texture Load Fail");
		return;
	}
	mat_list[0].baseTexture->uvScale = 0.0125f;


	create_material(&mat_list[1], PROJECT_TRIPLANER, (BYTE[4]) { 200, 50, 500, 255 }, 1);
	texLoader_generateTexture(mat_list[1].baseTexture, 4, 2, 2);

	short* matIDs = calloc(BODY_COUNT+1, sizeof(short));

	// INITIALIZE BODIES

	Body bodies[BODY_COUNT] = { 0 };

	for (int i = 0; i < BODY_COUNT; i++)
	{
		matIDs[i] = 1;

		bodies[i].type = SHAPE_SPHERE;
		bodies[i].sphere.radius = 0.2;
		bodies[i].orientation = quat_identity();
	}

	randomizePositions(bodies, BODY_COUNT, 40);

	// INITIALIZE THE RENDERER
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

	// update loop
	while (isRunning)
	{
		if (GetAsyncKeyState('R') & 0x8000)
		{
			randomizePositions(bodies,  BODY_COUNT, 40);
		}

		freeCam(&camera_pos, &camera_angle, deltaTime);

		//rendering
		if (!renderer_raytrace(bodies, matIDs, mat_list, BODY_COUNT, camera_pos, camera_angle, 90.0))
		{
			printf("RT Error!");
			system("pause");
		}

		if (GetAsyncKeyState('T') & 0x8000)
		{
			BVHNode* node = BVH_createTree(bodies, BODY_COUNT);
			BVH_DebugPrint(node);
			system("pause");
			system("cls");
		}

		// send frame to console
		renderFrame();

		// DEBUG INFO

		printf("object count: %d \n", BODY_COUNT);

		printf("FPS: %.2lf\n", 1000 / deltaTime);

		// frame timing
		printfFrameTimes(16);

		tick++;

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			isRunning = false;
		}
	}

	free(matIDs);
	free_material_list(mat_list, mat_count);

	end();

	return 0;
}

void randomizePositions(Body* bodies, int count, double distance)
{
	for (int i = 0; i < count; i++)
	{
		bodies[i].position = vector3_add((Vector3) { 0.0, 0.0, 0.0 }, vector3_scale(vector3_random(), distance));
	}
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

void freeCam(Vector3* camera_pos, Quaternion* camera_angle, double dt)
{
	const double moveSpeed = 0.25;
	const double rotSpeed = 4.0;

	Vector3 move = { 0 };
	double yawInput = 0;
	double pitchInput = 0;

	// --- Movement input ---
	if (GetAsyncKeyState('W') & 0x8000) move.z += 1;
	if (GetAsyncKeyState('S') & 0x8000) move.z -= 1;
	if (GetAsyncKeyState('A') & 0x8000) move.x -= 1;
	if (GetAsyncKeyState('D') & 0x8000) move.x += 1;
	if (GetAsyncKeyState('E') & 0x8000) move.y += 1;
	if (GetAsyncKeyState('Q') & 0x8000) move.y -= 1;

	// --- Rotation input ---
	if (GetAsyncKeyState(VK_UP) & 0x8000)   pitchInput += 1;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) pitchInput -= 1;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) yawInput -= 1;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) yawInput += 1;

	// --- Apply yaw in world space ---
	Quaternion yawRot = quat_from_axis_angle((Vector3) { 0, 1, 0 }, yawInput* rotSpeed* dt / 16 * (PI / 180.0));
	*camera_angle = quat_multiply(yawRot, *camera_angle);

	// --- Apply pitch in camera's local space ---
	Vector3 right = quat_rotate_vector(*camera_angle, (Vector3) { 1, 0, 0 });
	Quaternion pitchRot = quat_from_axis_angle(right, pitchInput * rotSpeed * dt / 16 * (PI / 180.0));
	*camera_angle = quat_multiply(pitchRot, *camera_angle);

	// --- Normalize quaternion to avoid drift ---
	*camera_angle = quat_normalize(*camera_angle);

	// --- Movement ---
	Vector3 forward = quat_rotate_vector(*camera_angle, (Vector3) { 0, 0, 1 });
	right = quat_rotate_vector(*camera_angle, (Vector3) { 1, 0, 0 });
	Vector3 up = (Vector3){ 0, 1, 0 };

	Vector3 delta = vector3_add(
		vector3_add(
			vector3_scale(forward, move.z * moveSpeed * dt / 16),
			vector3_scale(right, move.x * moveSpeed * dt / 16)
		),
		vector3_scale(up, move.y * moveSpeed * dt / 16)
	);

	*camera_pos = vector3_add(*camera_pos, delta);
}
