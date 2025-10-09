#pragma warning(disable : 4996).

#include "renderEngine.h"

#define PI 3.14159
#define TWO_PI 6.28318

#define NUM_THREADS 32

const double TRIANGLE_SIZE = 0.45;
const double TRIANGLE_POS_X = 0;
const double TRIANGLE_POS_Y = 0;

const GROUND_HEIGHT = 1.5;

int width, height;
char* renderArray;

clock_t executiontimeStart;

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
void printFrame()
{
	// Each cell is (2 chars) + newline per row + null terminator
	size_t bufferSize = (width * 2 + 1) * height + 1;
	char* buffer = (char*)malloc(bufferSize);
	if (!buffer) return;

	char* ptr = buffer;

	for (int i = 0; i < width * height; i++)
	{
		ptr += sprintf(ptr, "%c ", renderArray[i]);

		if (i % width == width - 1)
			ptr += sprintf(ptr, "\n");
	}

	*ptr = '\0'; // Null terminate

	system("cls");
	printf("%s", buffer);

	free(buffer);
}

bool spherePointIntersectionTest(Vector3 pos, double size, Vector3 point)
{
	Vector3 difference = vector3_subtract(pos, point);
	double distance = vector3_magnitude(difference);

	return distance < size;
}
Vector3 sphereNormal(Vector3 pos, Vector3 point)
{
	Vector3 difference = vector3_subtract(pos, point);
	return vector3_normalize(difference);
}

bool groundIntersectionTest(Vector3 point)
{
	return point.y > GROUND_HEIGHT;
}

char raymarch(Vector3* spheres, double* size, int count, Vector3 position, Vector3 direction, double maxDistance)
{
	Vector3 normalizedDirection = vector3_normalize(direction);
	double stepSize = 0.0;

	Vector3 rayPos;
	rayPos.x = position.x;
	rayPos.y = position.y;
	rayPos.z = position.z;

	int reflection_count = 0;

	for (double i = 0; i < maxDistance;)
	{
		double minDist = maxDistance + 1;
		for (int z = 0; z < count; z++)
		{
			double distance = vector3_magnitude(vector3_subtract(spheres[z], rayPos)) - size[z];
			if (distance < minDist)
			{
				minDist = distance;
			}
		}

		double distance = GROUND_HEIGHT - rayPos.y;
		if (distance < minDist)
		{
			minDist = distance;
		}

		stepSize = (minDist < 0.01) ? 0.01 : minDist;

		rayPos = vector3_add(rayPos, vector3_scale(normalizedDirection, stepSize));
		
		// ground intersection test
		if (groundIntersectionTest(rayPos))
		{
			if (reflection_count == 3)
			{
				return '.';
			}
			reflection_count++;

			rayPos = vector3_subtract(rayPos, vector3_scale(normalizedDirection, stepSize));

			Vector3 up = {0, 1, 0};
			normalizedDirection = vector3_reflect(normalizedDirection, up);
		}

		// sphere intersection tests
		for (int z = 0; z < count; z++)
		{
			if (spherePointIntersectionTest(spheres[z], size[z], rayPos))
			{
				if (reflection_count == 3)
				{
					return '.';
				}
				reflection_count++;

				rayPos = vector3_subtract(rayPos, vector3_scale(normalizedDirection, stepSize));
				normalizedDirection = vector3_reflect(normalizedDirection, sphereNormal(spheres[z], rayPos));
			}
		}

		i += stepSize;
	}

	if (reflection_count > 2)
	{
		return '^';
	}
	if (reflection_count > 1)
	{
		return '*';
	}
	if (reflection_count > 0)
		return '#';

	return ' ';
}

typedef struct {
	Vector3* spheres;
	double* size;
	int count;
	double fov;
	double maxDepth;
	int id;
} RaymarchGroupArgs;

DWORD WINAPI raymarchWorker(LPVOID arg)
{
	RaymarchGroupArgs* args = (RaymarchGroupArgs*)arg;

	int arrayLength = width * height;

	int start = (arrayLength / NUM_THREADS) * args->id;
	int end = (args->id == NUM_THREADS - 1) ? arrayLength : start + (arrayLength / NUM_THREADS);

	for (int i = start; i < end; i++)
	{
		Vector3 rayDir;
		rayDir.x = (((i % width) - (width / 2.0)) / width * 2) * args->fov / 90 * ((double)width / height);
		rayDir.y = (((i / (double)height) - (height / 2.0)) / height * 2) * args->fov / 90 * ((double)height / width);
		rayDir.z = 1;
		//Vector3 rayPos;

		Vector3 rayPos;
		rayPos.x = 0;
		rayPos.y = 0;
		rayPos.z = 0;

		renderArray[i] = raymarch(args->spheres, args->size, args->count, rayPos, rayDir, args->maxDepth);
	}

	free(args); // free allocated memory after use
	return 0;
}

int renderer_raymarch(Vector3* spheres, double* size, int count, double fov, double maxDepth)
{
	HANDLE threads[NUM_THREADS];

	// Launch threads
	for (int i = 0; i < NUM_THREADS; i++) {
		RaymarchGroupArgs* args = malloc(sizeof(RaymarchGroupArgs));
		if (!args) return 1;

		args->spheres = spheres;
		args->size = size;
		args->count = count;
		args->fov = fov;
		args->maxDepth = maxDepth;

		args->id = i;

		threads[i] = CreateThread(
			NULL,       // default security
			0,          // default stack size
			raymarchWorker,     // thread function
			args,        // argument
			0,          // run immediately
			NULL        // thread id not needed
		);

		if (threads[i] == NULL) {
			fprintf(stderr, "Error creating thread %d\n", i);
			return 1;
		}
	}

	// Wait for threads
	WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

	// Clean up handles
	for (int i = 0; i < NUM_THREADS; i++) CloseHandle(threads[i]);

	return 0;
}

// doesent even find the closest one lol... This is like super bare bones

bool raySphereIntersection(Body sphere, Ray ray, double* dist_ptr)
{
	double direct_dist = vector3_magnitude(vector3_subtract(sphere.position, ray.origin));
	double angle = vector3_angle(vector3_subtract(sphere.position, ray.origin), ray.direction);
	
	// if the distance to the point perpendicular to the sphere is less than the radius and it is in front of the camera
	// trig stuff :>

	*dist_ptr = direct_dist - cos(angle) * sphere.sphere.radius;

	double a = sin(angle) * direct_dist;
	if (a < sphere.sphere.radius && cos(angle) > 0)
	{
		return true;
	}

	return false;
}

bool rayBoxIntersection(Body box, Ray ray, double* dist_ptr)
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

	return true;
}

bool rayPlaneIntersection(Body plane, Ray ray, double *dist_ptr)
{
	double denom = vector3_dot(plane.plane.normal, ray.direction);

	// If the ray is parallel to the plane, no intersection
	if (fabs(denom) < 1e-8)
		return false;

	// Compute dist for the intersection point
	*dist_ptr = -(vector3_dot(plane.plane.normal, ray.origin) - plane.plane.offset) / denom;

	// Intersection must be in front of the ray
	if (*dist_ptr < 0.0)
		return false;

	return true;
}

char raytrace(Body* body, int count, Ray ray)
{
	char displayChar = ' ';

	double minDist = 1e30;
	double dist = 0;
	double* dist_ptr = &dist;

	for (int i = 0; i < count; i++)
	{
		if (body[i].type == SHAPE_SPHERE)
		{
			if (raySphereIntersection(body[i], ray, dist_ptr))
			{
				if (dist < minDist)
				{
					minDist = dist;
					displayChar = '#';
				}
			}
		}
		else if (body[i].type == SHAPE_BOX)
		{
			if (rayBoxIntersection(body[i], ray, dist_ptr))
			{
				if (dist < minDist)
				{
					minDist = dist;
					displayChar = 'X';
				}
			}
		}
		else if (body[i].type == SHAPE_PLANE)
		{
			if (rayPlaneIntersection(body[i], ray, dist_ptr))
			{
				if (dist < minDist)
				{
					minDist = dist;
					displayChar = '-';
				}
			}
		}
		else
		{
			// unknown shape
			displayChar = '?';
		}
	}

	return displayChar;
}

typedef struct {
	Body* bodies;
	int count;
	double fov;
	int id;
} RaytraceGroupArgs;

DWORD WINAPI raytraceWorker(LPVOID arg)
{
	RaytraceGroupArgs* args = (RaytraceGroupArgs*)arg;

	int arrayLength = width * height;

	int start = (arrayLength / NUM_THREADS) * args->id;
	int end = (args->id == NUM_THREADS - 1) ? arrayLength : start + (arrayLength / NUM_THREADS);

	for (int i = start; i < end; i++)
	{
		Vector3 rayDir;
		rayDir.x = (((i % width) - (width / 2.0)) / width * 2) * args->fov / 90 * ((double)width / height);
		rayDir.y = (((i / (double)height) - (height / 2.0)) / height * 2) * args->fov / 90 * ((double)height / width);
		rayDir.z = 1;

		Vector3 rayPos;
		rayPos.x = 0;
		rayPos.y = 0;
		rayPos.z = 0;

		Ray ray;
		rayDir = vector3_normalize(rayDir);
		create_ray(&ray, rayPos, rayDir);

		renderArray[i] = raytrace(args->bodies, args->count, ray);
	}

	free(args); // free allocated memory after use
	return 0;
}


int renderer_raytrace(Body* bodies, int count, double fov)
{
	HANDLE threads[NUM_THREADS];

	// Launch threads
	for (int i = 0; i < NUM_THREADS; i++) {
		RaytraceGroupArgs* args = malloc(sizeof(RaytraceGroupArgs));
		if (!args) return 1;

		args->bodies = bodies;
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

void test_intersection_cases()
{
	Vector3 spherePos; // exactly 1 unit away from (0, 0, 0)
	spherePos.x = 0;
	spherePos.y = 0.6;
	spherePos.z = 0.8;

	Vector3 rayPos;
	rayPos.x = 0;
	rayPos.y = 0;
	rayPos.z = 0;

	printf("Test One (False): %d\n", spherePointIntersectionTest(spherePos, 0.1, rayPos));
	printf("Test One (False): %d\n", spherePointIntersectionTest(spherePos, 0.99, rayPos));
	printf("Test One (True) : %d\n", spherePointIntersectionTest(spherePos, 1.01, rayPos));
}

void test_vector3_random()
{
	srand(0);
	Vector3 a = vector3_random();
	printf("Vector3 random A: %f %f %f \n", a.x, a.y, a.z);
	a = vector3_random();
	printf("Vector3 random B: %f %f %f \n", a.x, a.y, a.z);
	printf("Reset random \n");
	srand(0);
	a = vector3_random();
	printf("Vector3 random A: %f %f %f \n", a.x, a.y, a.z);
	a = vector3_random();
	printf("Vector3 random B: %f %f %f \n", a.x, a.y, a.z);
	printf("Seed Random \n");
	int seed = time(clock());
	srand(seed);
	printf("Vector3 random %d: %f %f %f \n", seed, a.x, a.y, a.z);
}

void renderer_unit_tests()
{
	test_intersection_cases();
	test_vector3_random();
}

int init(int w, int h)
{
	width = w;
	height = h;
	// dynamically allocates the memory for an array of boolians the size of the output display.
	// calloc initalizes all the references to zero.
	renderArray = (char*)calloc(width * height, sizeof(char));

	if (renderArray == NULL)
	{
		printf("Memory Allocation Error\n");
		return 2;
	}

	renderArray[width / 2] = '#'; // half width point
	renderArray[height / 2 * width] = '#'; // half height point
	renderArray[height / 2 * width + width / 2] = '#'; // center point

	return 0;
}

int userInit()
{
	char useStandardDimensions = '\0';

	// get the users input
	printf("Hello I'm Rendy! An askii rendering engine.\n");
	printf("Would you like to use the standard dimenstions? (y/n): ");
	scanf("%c", &useStandardDimensions);

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

	printf("Array memory size: %lld bytes \n", sizeof(renderArray) * width * height);

	printf("\nPress enter to begin rendering: ");


	// wait for user input to contine
	_getch();

	return init(width, height);
}

void resetDeltaTime()
{
	deltaTime = 0;
	executiontimeStart = clock();
}


void blank()
{
	deltaTime = executiontimeStart;
	executiontimeStart = clock(); // mesure frame times

	deltaTime = (double)(executiontimeStart - deltaTime); // find the time difference between every blank (A frame behind)

	for (int i = 0; i < width * height; i++)
	{
		renderArray[i] = '.';
	}
}

void renderFrame(void)
{
	printFrame();
}

void printfFrameTimes(double targetms, int tick)
{
	clock_t executiontimeEnd = clock();   // End timing
	double time_elapsed = (double)(executiontimeEnd - executiontimeStart); // Calculate elapsed frametime
	printf("Execution time: %f milliseconds\n", time_elapsed);

	double frameTime = targetms < time_elapsed ? time_elapsed : targetms;
	printf("Frame time: %f milliseconds\n", frameTime);

	printf("Delta time: %f miliseconds\n", deltaTime);

	if (time_elapsed < targetms)
		_sleep(targetms - (int)time_elapsed); // 16ms per frame = 60 fps

}

void end()
{
	//end program (cleanup)
	free(renderArray);
}
