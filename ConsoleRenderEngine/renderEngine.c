#define _CRT_SECURE_NO_WARNINGS

#include "renderEngine.h"

#define NUM_THREADS 32

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

	BYTE red = 0, green = 0, blue = 0;

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

void raytrace(BYTE RGBAout[4], BVHNode* BVHroot, Body* bodies, BYTE* textureIDs, Texture* textures, int count, Ray ray)
{
	const double depthScalar = 6e-1;

	RayHit i = BVH_traverse(BVHroot, &ray, bodies, 1e30);

	Vector3 localHitPoint = { 0 };
	double minDist = 1e30;
	double dist = 0;

	// inside hvh
	if (i.hit_id != -1)
	{
		if (bodies[i.hit_id].type == SHAPE_SPHERE && raySphereIntersection(bodies[i.hit_id], ray, &dist))
		{
			minDist = dist;

			//RGBAout[0] = (BYTE)max(255 - (minDist * depthScalar), 0);
			//RGBAout[1] = (BYTE)max(255 - (minDist * depthScalar), 0);
			//RGBAout[2] = (BYTE)max(255 - (minDist * depthScalar), 0);
			RGBAout[0] = 20;
			RGBAout[1] = 20;
			RGBAout[2] = 200;
			RGBAout[3] = 255;
			return;
		}
		else if (bodies[i.hit_id].type == SHAPE_BOX /*&& rayBoxIntersection(bodies[i.hit_id], ray, &dist, &localHitPoint)*/)
		{
			minDist = dist;

			RGBAout[0] = 20;
			RGBAout[1] = 200;
			RGBAout[2] = 20;
			RGBAout[3] = 255;
			return;
		}
	}

	// handle planes (not in bvh)
	for (int i = 0; i < count; i++)
	{

		if (bodies[i].type != SHAPE_PLANE) continue;

		Vector3 localHitPoint = { 0, 0, 0 };


		if (rayPlaneIntersection(bodies[i], ray, &dist, &localHitPoint) && dist < minDist)
		{
			minDist = dist;
			//texture_sample(textures, (Vector2) { localHitPoint.x, -localHitPoint.z }, RGBAout);


			if (abs((int)localHitPoint.x - localHitPoint.z) % 20 > 0 && abs((int)localHitPoint.z + localHitPoint.x) % 20 > 0)
			{
				RGBAout[0] = (BYTE)max(255 - (minDist * depthScalar), 0);
				RGBAout[1] = (BYTE)max(255 - (minDist * depthScalar), 0);
				RGBAout[2] = (BYTE)max(255 - (minDist * depthScalar), 0);
				RGBAout[3] = 255;
			}
		}
	}

	return;
}


typedef struct {
	Body* bodies;
	BVHNode* BVHroot;
	int* textureIDs;
	Texture* textures;
	Vector3 camera_pos;
	Quaternion camera_angle;
	int count;
	int id;
	double fov;
} RaytraceGroupArgs;

DWORD WINAPI raytraceWorker(LPVOID arg)
{
	RaytraceGroupArgs* args = (RaytraceGroupArgs*)arg;

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

		raytrace(RGBA,args->BVHroot, args->bodies, args->textureIDs, args->textures, args->count, ray);

		for (int j = 0; j < outputFrame.texture.byteCount; j++)
		{
			outputFrame.texture.pixeldata[i * outputFrame.texture.byteCount + j] = RGBA[j];
		}
	}

	free(args); // free allocated memory after use
	return 0;
}


int renderer_raytrace(Body* bodies, int* textureIDs, Texture* textures, int count, Vector3 cameraPos, Quaternion cameraAngle, double fov)
{
	rendertimeClock = clock();

	//create bvh tree
	BVHNode* BVHroot = BVH_createTree(bodies, count-1);

	//BVH_DebugPrint(BVHroot);
	//system("pause");

	if (BVHroot == NULL)
	{
		fprintf(stderr, "Error creating BVH Tree\n");
		system("pause");
		return 1;
	}

	HANDLE threads[NUM_THREADS];

	// Launch threads
	for (int i = 0; i < NUM_THREADS; i++) {
		RaytraceGroupArgs* args = malloc(sizeof(RaytraceGroupArgs));
		if (!args) return 1;

		args->bodies = bodies;
		args->BVHroot = BVHroot;
		args->textureIDs = textureIDs;
		args->textures = textures;
		args->camera_pos = cameraPos;
		args->camera_angle = cameraAngle;
		args->count = count;
		args->fov = fov;

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
			return 1;
		}
	}

	// Wait for threads
	WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

	// Clean up handles
	for (int i = 0; i < NUM_THREADS; i++) CloseHandle(threads[i]);

	BVH_freeTree(BVHroot);

	return 0;
}

int renderer_raytrace_b(Body* bodies, int* textureIDs, Texture* textures, int count, Vector3 cameraPos, Quaternion cameraAngle, double fov)
{
	rendertimeClock = clock();

	//create bvh tree
	BVHNode* BVHroot = BVH_createTree(bodies, count);

	BVH_DebugPrint(BVHroot);
	//system("pause");

	if (BVHroot == NULL)
	{
		fprintf(stderr, "Error creating BVH Tree\n");
		system("pause");
		return 1;
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

		raytrace(RGBA, bodies, BVHroot, textureIDs, textures, count, ray);

		for (int j = 0; j < outputFrame.texture.byteCount; j++)
		{
			outputFrame.texture.pixeldata[i * outputFrame.texture.byteCount + j] = RGBA[j];
		}
	}

	BVH_freeTree(BVHroot);

	return 0;
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
	//printFrame();
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

	printf("Nodes visited: %d, Leaves visited: %d\n",nodesVisited, leavesVisited);
	nodesVisited = leavesVisited = 0;

	if (exec_time < targetms && exec_time > 0)
		Sleep(targetms - (int)exec_time); // 16ms per frame = 60 fps
	executiontimeClock = clock();

}

void end()
{
	//end program (cleanup)
	free(outputFrame.texture.pixeldata);
	outputFrame.texture.pixeldata = NULL;

	free(characterBuffer);
	characterBuffer = NULL;
}
