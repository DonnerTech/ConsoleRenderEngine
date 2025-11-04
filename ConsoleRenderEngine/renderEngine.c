#define _CRT_SECURE_NO_WARNINGS

#include "renderEngine.h"

//int width, height;
//char* outputFrame;

static Frame outputFrame;

static char* characterBuffer;
static size_t bufferSize;

static clock_t rendertimeClock;
static clock_t executiontimeClock;

HANDLE co_handle;

//create the new frame in a buffer and push it to the console in one call.
void winPrintFrame()
{
	clock_t printStart = clock();

	// generate the buffer
	if (!characterBuffer) return;

	char* ptr = characterBuffer;

	int total = 6;

	// init red to not be the same as the first color so the color switching catches it
	BYTE red = outputFrame.texture.pixeldata[0] == 0 ? 1 : 0, green = 0, blue = 0;

	for (int i = 0; i < outputFrame.texture.imageSize; i += outputFrame.texture.byteCount)
	{
		if (i % outputFrame.texture.stride == 0)
		{
			ptr += sprintf(ptr, "\n");
			total++;
		}

		int num = 0;

		// slightly optomized color switching
		if (red != outputFrame.texture.pixeldata[i] || green != outputFrame.texture.pixeldata[i + 1] || blue != outputFrame.texture.pixeldata[i + 2])
		{
			red = outputFrame.texture.pixeldata[i];
			green = outputFrame.texture.pixeldata[i + 1];
			blue = outputFrame.texture.pixeldata[i + 2];

			num = sprintf(ptr, "\033[48;2;%d;%d;%dm", red, green, blue);
			ptr += num;
			total += num;
		}

		num = sprintf(ptr, "  ");

		ptr += num;
		total += num;
	}

	ptr += sprintf(ptr, "\033[0m\n");

	*ptr = '\0';

	// blit the buffer

	DWORD written;

	SetConsoleCursorPosition(co_handle, outputFrame.position);
	WriteConsoleA(co_handle, characterBuffer, total, &written, NULL);

	printf("blitting time: %d ms\n", clock() - printStart);
}

// overlay color mixing with alpha support
void overlayColor(BYTE RGBAin[4], BYTE RGBAout[4])
{
	//	alpha01 = (1 - a0)·a1 + a0
	float alpha01 = ((255 - (float)RGBAin[3]) * (float)RGBAout[3] + (float)RGBAin[3]);

	//	red01 = ((1 - a0)·a1·r1 + a0·r0) / alpha01
	RGBAout[0] = (BYTE)(((255 - (float)RGBAin[3]) * (float)RGBAout[3] * (float)RGBAout[0] + (float)RGBAin[3] * (float)RGBAin[0]) / alpha01);

	//	green01 = ((1 - a0)·a1·g1 + a0·g0) / alpha01
	RGBAout[1] = (BYTE)(((255 - (float)RGBAin[3]) * (float)RGBAout[3] * (float)RGBAout[1] + (float)RGBAin[3] * (float)RGBAin[1]) / alpha01);

	//	blue01 = ((1 - a0)·a1·b1 + a0·b0) / alpha01
	RGBAout[2] = (BYTE)(((255 - (float)RGBAin[3]) * (float)RGBAout[3] * (float)RGBAout[2] + (float)RGBAin[3] * (float)RGBAin[2]) / alpha01);

	RGBAout[3] = (BYTE)alpha01;
}

// addative color mixing
void addativeColor(BYTE RGBAin[4], BYTE RGBAout[4])
{
	float a0 = RGBAin[3] / 255.0f;
	float a1 = RGBAout[3] / 255.0f;

	float a01 = fminf(1.0f, a0 + a1);

	for (int i = 0; i < 3; i++) {
		float c0 = RGBAin[i] / 255.0f;
		float c1 = RGBAout[i] / 255.0f;
		float c = c0 * a0 + c1 * a1;
		RGBAout[i] = (BYTE)(fminf(1.0f, c) * 255.0f);
	}

	RGBAout[3] = (BYTE)(a01 * 255.0f);
}

// color multiply
void multiplyColor(BYTE RGBAin[4], BYTE RGBAout[4])
{
	float a0 = RGBAin[3] / 255.0f;
	float a1 = RGBAout[3] / 255.0f;
	float a01 = a0 + a1 * (1 - a0);

	for (int i = 0; i < 3; i++) {
		float c0 = RGBAin[i] / 255.0f;
		float c1 = RGBAout[i] / 255.0f;
		float c = (c1 * (1 - a0)) + (c0 * c1) * a0;
		RGBAout[i] = (BYTE)(c * 255.0f);
	}

	RGBAout[3] = (BYTE)(a01 * 255.0f);
}

void sample_mat(Material currentMat, RayHit state, BYTE albedo[4])
{

	// planar projection
	switch (currentMat.projecton)
	{
	case(PROJECT_PLANER):
	{
		texture_sample(currentMat.baseTexture, (Vector2) { state.localPosition.x, -state.localPosition.z }, albedo);
		break;
	}
	case(PROJECT_TRIPLANER):
	{
		/* glsl sudocode:
		vec3 n = abs(normalize(normal));
		vec3 xProj = position.yz;
		vec3 yProj = position.xz;
		vec3 zProj = position.xy;
		vec4 xTex = texture(tex, xProj * scale);
		vec4 yTex = texture(tex, yProj * scale);
		vec4 zTex = texture(tex, zProj * scale);
		vec4 color = (xTex * n.x + yTex * n.y + zTex * n.z) / (n.x + n.y + n.z);
		*/

		Vector3 n = (Vector3){ fabs(state.normal.x), fabs(state.normal.y), fabs(state.normal.z) };
		Vector2 xProj = (Vector2){ state.position.y, state.position.z };
		Vector2 yProj = (Vector2){ state.position.x, state.position.z };
		Vector2 zProj = (Vector2){ state.position.x, state.position.y };

		BYTE cx[4], cy[4], cz[4];
		texture_sample(currentMat.baseTexture, xProj, cx);
		texture_sample(currentMat.baseTexture, yProj, cy);
		texture_sample(currentMat.baseTexture, zProj, cz);

		// average them into one
		float sum = n.x + n.y + n.z;
		if (sum == 0.0f) sum = 1.0f;

		for (int i = 0; i < 4; i++)
		{
			albedo[i] = (BYTE)((cx[i] * n.x + cy[i] * n.y + cz[i] * n.z) / sum);
		}
		break;
	}
	case(PROJECT_LOCAL_TRIPLANER):
	{
		Vector3 n = (Vector3){ fabs(state.normal.x), fabs(state.normal.y), fabs(state.normal.z) };
		Vector2 xProj = (Vector2){ state.localPosition.y, state.localPosition.z };
		Vector2 yProj = (Vector2){ state.localPosition.x, state.localPosition.z };
		Vector2 zProj = (Vector2){ state.localPosition.x, state.localPosition.y };

		BYTE cx[4], cy[4], cz[4];
		texture_sample(currentMat.baseTexture, xProj, cx);
		texture_sample(currentMat.baseTexture, yProj, cy);
		texture_sample(currentMat.baseTexture, zProj, cz);

		// average them into one
		float sum = n.x + n.y + n.z;
		if (sum == 0.0f) sum = 1.0f;

		for (int i = 0; i < 4; i++)
		{
			albedo[i] = (BYTE)((cx[i] * n.x + cy[i] * n.y + cz[i] * n.z) / sum);
		}
		break;
	}
	case(PROJECT_SPHERICAL):
	{
		/* glsl sudocode:
		vec3 p = normalize(position);
		float u = atan(p.z, p.x) / (2.0 * PI) + 0.5;
		float v = asin(p.y) / PI + 0.5;
		vec2 uv = vec2(u, v);
		vec4 color = texture(tex, uv * scale);
		*/
		Vector3 p_norm = vector3_normalize(state.position);
		Vector2 uv = (Vector2){
			atan2f(p_norm.z, p_norm.x) / 2.0f * PI + 0.5f,
			asinf(p_norm.y) / PI + 0.5f
		};
		texture_sample(currentMat.baseTexture, uv, albedo);

		break;
	}
	case(PROJECT_LOCAL_SPHERICAL):
	{
		/* glsl sudocode:
		vec3 p = normalize(position);
		float u = atan(p.z, p.x) / (2.0 * PI) + 0.5;
		float v = asin(p.y) / PI + 0.5;
		vec2 uv = vec2(u, v);
		vec4 color = texture(tex, uv * scale);
		*/
		Vector3 p_norm = vector3_normalize(state.localPosition);
		Vector2 uv = (Vector2){
			atan2f(p_norm.z, p_norm.x) / 2.0f * PI + 0.5f,
			asinf(p_norm.y) / PI + 0.5f
		};
		texture_sample(currentMat.baseTexture, uv, albedo);

		break;
	}
	}
}

int raysShot = 0;

void raytrace(BYTE RGBAout[4], BVHNode* BVHroot, Body* bodies, short* matIDs, Material* mats, int count, Ray ray, int depth)
{
	const double EPS = 1e-8;

	// initialize color
	RGBAout[0] = 0;
	RGBAout[1] = 0;
	RGBAout[2] = 0;
	RGBAout[3] = 255;

	BYTE albedo[4] = { 255,255,255,255 };
	BYTE indirect_lighting[4] = { 255,255,255,255 };

	BYTE skyCol[4] = {	200, 200, 230, 255 };

	raysShot++;

	const double depthScalar = 4e-1;

	RayHit state = (RayHit){ .dist = 1e30, .hit_id = NO_HIT, .localPosition = (Vector3){0,0,0}, .position = (Vector3){0,0,0}, .normal = (Vector3){0,0,0} };
	Material* mat_ptr = mats;

	BVH_traverse(BVHroot, ray, bodies, &state);

	double minDist = 1e30;
	double dist = 0;

	// inside hvh
	if (state.hit_id != NO_HIT)
	{
		if (bodies[state.hit_id].type == SHAPE_SPHERE)
		{
			minDist = state.dist;
		}
		else if (bodies[state.hit_id].type == SHAPE_BOX)
		{
			minDist = state.dist;
		}

		mat_ptr = &mats[matIDs[state.hit_id]];
	}

	// handle planes (not in bvh)
	for (int i = 0; i < count; i++)
	{
		if (bodies[i].type != SHAPE_PLANE) continue;

		RayHit planeHit  = intersectBody(bodies[i], i, ray);

		if (planeHit.hit_id != NO_HIT && planeHit.dist < minDist)
		{
			//BYTE planeCol[4] = { 255, 255, 255, 255 };
			//sample_mat(mats[matIDs[planeHit.hit_id]], state, planeCol);
			//if(planeCol[3] > 127)
			//{
				minDist = dist;

				state = planeHit;
				mat_ptr = &mats[matIDs[planeHit.hit_id]];
			//}
		}
	}

	// ---texture mapping---
	if (state.hit_id != NO_HIT)
	{
		sample_mat(*mat_ptr, state, albedo);
	}
	
	// ---direct lighting---
	if (state.hit_id != NO_HIT)
	{

		Vector3 ref_dir = (Vector3){ 0.4, -0.8, 0.4472135 };// sun direction (normalized)
		Ray rayR;

		Vector3 pos = vector3_add(state.position, vector3_scale(state.normal, EPS));
		create_ray(&rayR, pos, ref_dir);

		// trace the sun ray
		RayHit dl = (RayHit){ .dist = 1e30, .hit_id = NO_HIT, .localPosition = (Vector3){0,0,0}, .position = (Vector3){0,0,0}, .normal = (Vector3){0,0,0} };
		BVH_traverse(BVHroot, rayR, bodies, &dl);

		// if we hit nothing apply the sky color
		if (dl.hit_id == NO_HIT)
		{
			addativeColor(skyCol, indirect_lighting);
		}
		else
		{
			// set to black
			for (int i = 0; i < 3; i++)
			{
				indirect_lighting[i] = 0;
			}
		}
	}

	// ---recursive reflections---
	if (state.hit_id != NO_HIT && mat_ptr->reflectivity > 0 && depth < MAX_RT_DEPTH)
	{

		Vector3 ref_dir = vector3_reflect(ray.direction, state.normal);
		Ray rayR;

		Vector3 pos = vector3_add(state.position, vector3_scale(state.normal, EPS));
		create_ray(&rayR, pos, ref_dir);

		BYTE ref_color[4];

		raytrace(ref_color, BVHroot, bodies, matIDs, mats, count, rayR, depth + 1);

		multiplyColor(mat_ptr->specularColor, ref_color);

		ref_color[0] *= mat_ptr->reflectivity;
		ref_color[1] *= mat_ptr->reflectivity;
		ref_color[2] *= mat_ptr->reflectivity;
		ref_color[3] *= mat_ptr->reflectivity;

		addativeColor(ref_color, albedo);
		//multiplyColor(ref_color, indirect_lighting);
	}
	

	// ---Global Illumination---
	if (state.hit_id != NO_HIT && depth < MAX_RT_DEPTH)
	{
		for (int i = 0; i < RTGI_SAMPLES; i++)
		{
			// TODO: add cosine distribution
			//TODO: fix cubic distribution
			Vector3 ray_dir = vector3_normalize(vector3_random());

			// if the ray points into the surface flip it
			// this gets us a half hemisphere
			double dot = vector3_dot(ray_dir, state.normal);
			if (dot < 0)
			{
				ray_dir = vector3_reflect(ray_dir, state.normal);
			}

			Vector3 pos = vector3_add(state.position, vector3_scale(state.normal, EPS));

			Ray rayR;
			create_ray(&rayR, pos, ray_dir);

			BYTE ref_color[4];
			raytrace(ref_color, BVHroot, bodies, matIDs, mats, count, rayR, MAX_RT_DEPTH); // only one bounce

#if RTGI_SAMPLES > 0
			multiplyColor((BYTE[4]){ 
					RTGI_BRIGHTNESS / RTGI_SAMPLES, 
					RTGI_BRIGHTNESS / RTGI_SAMPLES, 
					RTGI_BRIGHTNESS / RTGI_SAMPLES, 
					255},
				ref_color);
#endif // RTGI_SAMPLES > 0

			addativeColor(ref_color, indirect_lighting);
		}
	}

	// ---ambient sky light---
	if (state.hit_id == NO_HIT)
	{
		for (int i = 0; i < 4; i++)
		{
			indirect_lighting[i] = skyCol[i];
		}
	}

	// apply indirect lighting
	multiplyColor(indirect_lighting, albedo);

	// return the lighting
	for (int i = 0; i < 4; i++)
	{
		RGBAout[i] = albedo[i];
	}

#if _DEBUG || _BENCHMARK
	// render Bounding Volume Hierarchy
	ray_bvh(RGBAout, BVHroot, ray, 0);
#endif
}

static void ray_bvh(BYTE RGBAout[4], BVHNode* node, Ray ray, int depth)
{
	if (node == NULL)
		return;

	double d;
	if (node->ids[0] != -1 &&  ray_aabb(ray, node->bounds.min, node->bounds.max, 1e30, &d))
	{
		//BYTE color[4] = { max(255 - depth*10,0), 20, min(depth*10,255), 240 };
		BYTE color[4] = { 255, 20, 255, 252 };
		overlayColor(color, RGBAout);
	}

	ray_bvh(RGBAout, node->left_ptr, ray, depth + 1);

	ray_bvh(RGBAout, node->right_ptr, ray, depth + 1);
}


typedef struct {
	Body* bodies;
	BVHNode* BVHroot;
	short* matIDs;
	Material* mats;
	Vector3 camera_pos;
	Quaternion camera_angle;
	int count;
	double fov;
	int id;
	int rand_seed;
} RaytraceGroupArgs;

DWORD WINAPI raytraceWorker(LPVOID arg)
{
	RaytraceGroupArgs* args = (RaytraceGroupArgs*)arg;

	srand(args->rand_seed);

	int width = outputFrame.texture.width;
	int height = outputFrame.texture.height;
	int pixelCount = width * height;

	int start = (pixelCount / NUM_THREADS) * args->id;
	int end = (args->id == NUM_THREADS - 1) ? pixelCount : start + (pixelCount / NUM_THREADS);

	for (int i = start; i < end; i++)
	{
		Vector3 rayDir;
		rayDir.x = (((i % width) - (width / 2.0)) / width * 2) * args->fov / 90 * ((double)width / height);
		rayDir.y = (((i / (double)height) - (height / 2.0)) / height * 2) * args->fov / 90 * ((double)height / width);
		rayDir.z = 1;

		Ray ray;
		rayDir = vector3_normalize(rayDir);

		rayDir = quat_rotate_vector(args->camera_angle, rayDir);

		create_ray(&ray, args->camera_pos, rayDir);

		BYTE RGBA[4] = { 0 };

		raytrace(RGBA,args->BVHroot, args->bodies, args->matIDs, args->mats, args->count, ray, 0);

		for (int j = 0; j < outputFrame.texture.byteCount; j++)
		{
			outputFrame.texture.pixeldata[i * outputFrame.texture.byteCount + j] = RGBA[j];
		}
	}

	free(args); // free allocated memory after use
	return 0;
}


int renderer_raytrace(BVHNode* BVHroot, Body* bodies, short* matIDs, Material* mats, int count, Vector3 cameraPos, Quaternion cameraAngle, double fov)
{
	rendertimeClock = clock();

	//create bvh tree
	//BVHNode* BVHroot = BVH_createTree(bodies, count);

	if (!BVH_validateTree(BVHroot)) return 0;

	//BVH_DebugPrint(BVHroot);
	//system("pause");

	if (BVHroot == NULL)
	{
		fprintf(stderr, "Error creating BVH Tree\n");
		system("pause");
		return 0;
	}

	HANDLE threads[NUM_THREADS];

	// Launch threads
	for (int i = 0; i < NUM_THREADS; i++) {
		RaytraceGroupArgs* args = malloc(sizeof(RaytraceGroupArgs));
		if (!args) return 0;

		args->bodies = bodies;
		args->BVHroot = BVHroot;
		args->matIDs = matIDs;
		args->mats = mats;
		args->camera_pos = cameraPos;
		args->camera_angle = cameraAngle;
		args->count = count;
		args->fov = fov;

		args->rand_seed = rand();
		args->id = i;

		threads[i] = CreateThread(
			NULL,       // default security
			0,          // default stack size
			raytraceWorker,     // thread function
			args,        // argument
			0,          // run immediately
			NULL        // thread id not needed
		);

		if (threads[i] == NULL) {
			fprintf(stderr, "Error creating thread %d\n", i);
			system("pause");
			return 0;
		}
	}

	// Wait for threads
	WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

	// Clean up handles
	for (int i = 0; i < NUM_THREADS; i++) CloseHandle(threads[i]);

	return 1;
}

int renderer_raytrace_b(Body* bodies, short* matIDs, Material* mats, int count, Vector3 cameraPos, Quaternion cameraAngle, double fov)
{
	rendertimeClock = clock();

	//create bvh tree
	BVHNode* BVHroot = BVH_createTree(bodies, count);

	//BVH_DebugPrint(BVHroot);
	//system("pause");

	if (BVHroot == NULL)
	{
		fprintf(stderr, "Error creating BVH Tree\n");
		system("pause");
		return 0;
	}

	int width = outputFrame.texture.width;
	int height = outputFrame.texture.height;
	int pixelCount = width * height;

	int start = 0;
	int end = pixelCount;

	for (int i = start; i < end; i++)
	{
		Vector3 rayDir;
		rayDir.x = (((i % width) - (width / 2.0)) / width * 2) * fov / 90 * ((double)width / height);
		rayDir.y = (((i / (double)height) - (height / 2.0)) / height * 2) * fov / 90 * ((double)height / width); 
		rayDir.z = 1;

		Ray ray;
		rayDir = vector3_normalize(rayDir);

		rayDir = quat_rotate_vector(cameraAngle, rayDir);

		create_ray(&ray, cameraPos, rayDir);

		BYTE RGBA[4] = { 0 };

		raytrace(RGBA, BVHroot, bodies, matIDs, mats, count, ray, 0);

		for (int j = 0; j < outputFrame.texture.byteCount; j++)
		{
			outputFrame.texture.pixeldata[i * outputFrame.texture.byteCount + j] = RGBA[j];
		}
	}

	BVH_freeTree(BVHroot);

	return 1; 
}

int init(int w, int h)
{
	outputFrame.position = (COORD){ 0, 0 };
	texLoader_generateTexture(&outputFrame.texture, 3, w, h);

	bufferSize = (outputFrame.texture.width * 2 + 1) * outputFrame.texture.height * sizeof("\033[48;2;000;000;000m ") + sizeof("\033[0m") + sizeof(char);
	characterBuffer = (char*)malloc(bufferSize);

	if (characterBuffer == NULL)
		return 1;

	for (int i = 0; i < bufferSize; i++)
	{
		characterBuffer[i] = '\0';
	}

	// get the console handle for clean screen blitting
	co_handle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (co_handle == INVALID_HANDLE_VALUE)
		return 1;

	return 0;
}

int userInit()
{
	char useStandardDimensions = '\0';

	// get the users input
	printf("Hello I'm Rendy! The askii renderer config manager.\n");
	printf("Would you like to use the standard dimenstions? (y/n): ");
	scanf("%c", &useStandardDimensions);

	int width, height;

	if(useStandardDimensions == 'n' || useStandardDimensions == 'N')
	{ 
		printf("Some reccomended dimentions are 852x480, 192x108, 160x90, and 80x45\n");
		printf("Enter the canvas width: ");
		scanf("%d", &width);
		printf("Enter the canvas height: ");
		scanf("%d", &height);
	}
	else
	{
		width = 192;
		height = 108;
	}

	//free(useStandardDimensions);

	if (width <= 0 || height <= 0)
	{
		printf("Invalid Dimensions\n");
		return 1;
	}

	int sizeEST = (width * 3 * 2 + 1) * height * sizeof("\033[48;2;000;000;000m ") + sizeof("\033[0m") + sizeof(char);

	printf("display memory size: %0.2lfMB \n", sizeEST/1000000.0);


	printf("\nPress enter to begin rendering: ");


	// wait for user input to contine
	_getch();

	return init(width, height);
}

void resetDeltaTime()
{
	deltaTime = 0;
	executiontimeClock = clock();
}

void renderFrame(void)
{
	winPrintFrame();
}

void printfFrameTimes(double targetms)
{
	clock_t temp = executiontimeClock;
	executiontimeClock = clock();   // End timing
	double exec_time = (double)(executiontimeClock - temp); // Calculate elapsed loop time

	temp = rendertimeClock;
	rendertimeClock = clock();
	double render_time = (double)(executiontimeClock - temp);

	printf("Render exec time: %f milliseconds\n", render_time);

	deltaTime = targetms < exec_time ? exec_time : targetms;

	printf("Other time: %f milliseconds\n", exec_time - render_time);

	printf("Sleep time: %f milliseconds\n", deltaTime - exec_time);

	printf("Delta time: %f miliseconds\n", deltaTime);

	//printf("Nodes visited: %d, Leaves visited: %d Rays Shot: %d\n",nodesVisited, leavesVisited, raysShot);
	printf("Nodes per Ray: %f\n", (float)nodesVisited / raysShot);
	printf("Leaves per Ray: %f\n", (float)leavesVisited / raysShot);

	nodesVisited = leavesVisited = raysShot = 0;

	if (exec_time < targetms && exec_time > 0)
		Sleep(targetms - (int)exec_time); // 16ms per frame = 60 fps
	executiontimeClock = clock();

}

void end()
{
	free(outputFrame.texture.pixeldata);
	outputFrame.texture.pixeldata = NULL;

	free(characterBuffer);
	characterBuffer = NULL;
}
