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

// creates a ray from an origin and direction
void create_ray(Ray* ray, Vector3 origin, Vector3 direction)
{
	ray->origin = origin;
	ray->direction = direction;
	ray->invdir.x = 1.0 / direction.x;
	ray->invdir.y = 1.0 / direction.y;
	ray->invdir.z = 1.0 / direction.z;
	ray->sign[0] = (ray->invdir.x < 0);
	ray->sign[1] = (ray->invdir.y < 0);
	ray->sign[2] = (ray->invdir.z < 0);
}

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

bool raySphereIntersection(Body sphere, Ray ray, double* dist_ptr)
{
	double direct_dist = vector3_magnitude(vector3_subtract(sphere.position, ray.origin));
	double angle = vector3_angle(vector3_subtract(sphere.position, ray.origin), ray.direction);
	
	// if the distance to the point perpendicular to the sphere is less than the radius and it is in front of the camera
	// trig stuff :>

	*dist_ptr = direct_dist - fast_cos(angle) * sphere.sphere.radius;

	double a = fast_sin(angle) * direct_dist;
	if (a < sphere.sphere.radius && fast_cos(angle) > 0)
	{
		return true;
	}

	return false;
}

bool rayBoxIntersection(Body box, Ray ray, double* dist_ptr, Vector3* localHitPoint)
{
	*dist_ptr = 1e30;

	Ray ba_ray;

	// convert the ray to the box's local space
	Vector3 origin = vector3_subtract(ray.origin, box.position);
	origin = quat_rotate_vector(quat_conjugate(box.orientation), origin);

	// convert direction to local space
	Vector3 direction = quat_rotate_vector(quat_conjugate(box.orientation), ray.direction);

	create_ray(&ba_ray, origin, direction);

	// do ray intersection with axis alligned bounding box
	// Credit to Scratchapixel for the AABB-Ray optimized intersection algorithm
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html

	Vector3 zero = (Vector3){ 0,0,0 };

	Vector3 bounds[2];
	bounds[0] = vector3_subtract(zero, box.box.half_extents); // min
	bounds[1] = vector3_add(zero, box.box.half_extents); // max

	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (bounds[ba_ray.sign[0]].x - ba_ray.origin.x) * ba_ray.invdir.x;
	tmax = (bounds[1 - ba_ray.sign[0]].x - ba_ray.origin.x) * ba_ray.invdir.x;
	tymin = (bounds[ba_ray.sign[1]].y - ba_ray.origin.y) * ba_ray.invdir.y;
	tymax = (bounds[1 - ba_ray.sign[1]].y - ba_ray.origin.y) * ba_ray.invdir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	tzmin = (bounds[ba_ray.sign[2]].z - ba_ray.origin.z) * ba_ray.invdir.z;
	tzmax = (bounds[1 - ba_ray.sign[2]].z - ba_ray.origin.z) * ba_ray.invdir.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	// Compute intersection distance
	if (tmax < 0)
		return false; // box is behind ray

	if (tmin < 0)
		*dist_ptr = tmax; // inside box
	else
		*dist_ptr = tmin; // first hit

	*localHitPoint = vector3_add(ba_ray.origin, vector3_scale(ba_ray.direction, *dist_ptr));

	return true;
}

bool rayPlaneIntersection(Body plane, Ray ray, double *dist_ptr, Vector3* localHitPoint)
{
	double denom = vector3_dot(plane.plane.normal, ray.direction);

	// If the ray is parallel to the plane, no intersection
	if (fabs(denom) < 1e-16)
		return false;

	// Compute dist for the intersection point
	*dist_ptr = -(vector3_dot(plane.plane.normal, ray.origin) - plane.plane.offset) / denom;

	// Intersection must be in front of the ray
	if (*dist_ptr < 0.0)
		return false;

	// set the hit position (currently global)
	*localHitPoint = vector3_add(ray.origin, vector3_scale(ray.direction, *dist_ptr));

	return true;
}

void raytrace(BYTE RGBAout[4], Body* bodies, BYTE* textureIDs, Texture* textures, int count, Ray ray)
{

	double minDist = 1e30;
	double dist = 0;
	double* dist_ptr = &dist;

	for (int i = 0; i < count; i++)
	{
		if (bodies[i].type == SHAPE_SPHERE)
		{
			if (raySphereIntersection(bodies[i], ray, dist_ptr))
			{
				if (dist < minDist)
				{
					minDist = dist;

					RGBAout[0] = 50;
					RGBAout[1] = 50;
					RGBAout[2] = 200;
					RGBAout[3] = 255;
				}
			}
		}
		else if (bodies[i].type == SHAPE_BOX)
		{
			Vector3 localHitPoint = { 0 };

			if (rayBoxIntersection(bodies[i], ray, dist_ptr, &localHitPoint))
			{
				if (dist < minDist)
				{
					minDist = dist;
					RGBAout[0] = 255;
					RGBAout[1] = 0;
					RGBAout[2] = 0;
					RGBAout[3] = 255;

					// determine which face was hit

					// basic shading
					if (fabs(localHitPoint.x - bodies[i].box.half_extents.x) < 1e-6) RGBAout[0] = 220;
					else if (fabs(localHitPoint.x + bodies[i].box.half_extents.x) < 1e-6) RGBAout[0] = 200;
					else if (fabs(localHitPoint.y - bodies[i].box.half_extents.y) < 1e-6) RGBAout[0] = 180;
					else if (fabs(localHitPoint.y + bodies[i].box.half_extents.y) < 1e-6) RGBAout[0] = 160;
					else if (fabs(localHitPoint.z - bodies[i].box.half_extents.z) < 1e-6) RGBAout[0] = 140;
					else if (fabs(localHitPoint.z + bodies[i].box.half_extents.z) < 1e-6) RGBAout[0] = 120;
				}
			}
		}
		else if (bodies[i].type == SHAPE_PLANE)
		{
			Vector3 localHitPoint = { 0, 0, 0 };

			if (rayPlaneIntersection(bodies[i], ray, dist_ptr, &localHitPoint))
			{
				if (dist < minDist)
				{
					minDist = dist;
					texture_sample(textures, (Vector2){ localHitPoint.x, -localHitPoint.z }, RGBAout);


					int c = (int)localHitPoint.x - (int)localHitPoint.z;

					if (abs(c) % 2 > 0)
					{
						RGBAout[0] = 200;
						RGBAout[1] = 220;
						RGBAout[2] = 200;
						RGBAout[3] = 255;
					}
				}
			}
		}
		else
		{
			// unknown shape
			RGBAout[0] = 200;
			RGBAout[1] = 0;
			RGBAout[2] = 200;
			RGBAout[3] = 255;
		}
	}
}

typedef struct {
	Body* bodies;
	int* textureIDs;
	Texture* textures;
	Vector3 camera_pos;
	Quaternion camera_angle;
	int count;
	double fov;
	int id;
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

		raytrace(RGBA,args->bodies, args->textureIDs, args->textures, args->count, ray);

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

	HANDLE threads[NUM_THREADS];

	// Launch threads
	for (int i = 0; i < NUM_THREADS; i++) {
		RaytraceGroupArgs* args = malloc(sizeof(RaytraceGroupArgs));
		if (!args) return 1;

		args->bodies = bodies;
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

	printf("Smeep time: %f milliseconds\n", deltaTime - exec_time);

	printf("Delta time: %f miliseconds\n", deltaTime);

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
