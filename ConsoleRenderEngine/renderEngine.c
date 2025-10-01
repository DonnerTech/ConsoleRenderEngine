#pragma warning(disable : 4996).

#include "renderEngine.h"

#define PI 3.14159
#define TWO_PI 6.28318

#define NUM_THREADS 16

const double TRIANGLE_SIZE = 0.45;
const double TRIANGLE_POS_X = 0;
const double TRIANGLE_POS_Y = 0;

const GROUND_HEIGHT = 1.5;

int width, height;
char* renderArray;

clock_t executiontimeStart;

//create the new frame in a buffer and push it to the console in one call.
void printArray()
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

double area(double x1, double y1, double x2, double y2, double x3, double y3) {
	return fabs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0f);
}

// The point is inside the triangle if the sum of areas of sub-triangles (PAB, PBC, PCA) equals the area of ABC.
bool pointInTriangle(double px, double py,
	double x1, double y1,
	double x2, double y2,
	double x3, double y3)
{
	double A = area(x1, y1, x2, y2, x3, y3);
	double A1 = area(px, py, x2, y2, x3, y3);
	double A2 = area(x1, y1, px, py, x3, y3);
	double A3 = area(x1, y1, x2, y2, px, py);

	return fabs(A - (A1 + A2 + A3)) < 1e-4; // allow for some error
}

void polarToEuler(double r, double theta, double* x, double* y) {
	*x = r * cos(theta);
	*y = r * sin(theta);
}

void rotatingTriangleDemo(int tick)
{
	double x1, y1;
	double x2, y2;
	double x3, y3;

	double rot = (double)tick / 100;
	polarToEuler(TRIANGLE_SIZE, rot, &x1, &y1);
	rot += (TWO_PI / 3.0);
	polarToEuler(TRIANGLE_SIZE, rot, &x2, &y2);
	rot += (TWO_PI / 3.0);
	polarToEuler(TRIANGLE_SIZE, rot, &x3, &y3);

	x1 += TRIANGLE_POS_X;
	x2 += TRIANGLE_POS_X;
	x3 += TRIANGLE_POS_X;

	y1 += TRIANGLE_POS_Y;
	y2 += TRIANGLE_POS_Y;
	y3 += TRIANGLE_POS_Y;

	for (int i = 0; i < width * height; i++)
	{

		if (pointInTriangle(i % width, i / width,
			x1 * width + width / 2, y1 * height + height / 2,
			x2 * width + width / 2, y2 * height + height / 2,
			x3 * width + width / 2, y3 * height + height / 2))
		{
			renderArray[i] = '#';
		}

	}
}

void drawTriangleToArray(double x1, double y1, double x2, double y2, double x3, double y3, char c)
{

	x1 += TRIANGLE_POS_X;
	x2 += TRIANGLE_POS_X;
	x3 += TRIANGLE_POS_X;

	y1 += TRIANGLE_POS_Y;
	y2 += TRIANGLE_POS_Y;
	y3 += TRIANGLE_POS_Y;

	for (int i = 0; i < width * height; i++)
	{


		if (pointInTriangle(i % width, i / width,
			x1 * width + width / 2, y1 * height + height / 2,
			x2 * width + width / 2, y2 * height + height / 2,
			x3 * width + width / 2, y3 * height + height / 2))
		{
			renderArray[i] = c;
		}

	}
}

bool spherePointIntersectionTest(Vector3 pos, double size, Vector3 point)
{
	Vector3 difference = vector3_subtract(pos, point);
	double distance = vector3_magnitude(difference);

	return distance < size;
}

//bool cubePointIntersectionTest(Vector3 pos, )

Vector3 sphereNormal(Vector3 pos, Vector3 point)
{
	Vector3 difference = vector3_subtract(pos, point);
	return vector3_normalize(difference);
}



double GetDistance(Vector3* spheres, double* size, int count, double maxDistance, Vector3 rayPos)
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

	double distance = fabs(GROUND_HEIGHT - rayPos.y);
	if (distance < minDist)
	{
		minDist = distance;
	}

	minDist = smin(distance, minDist, 0.25);
}

//Vector3 GetNormal(Vector3 position, )
//{
//	float distance = GetDist(p);
//	Vector2 epsilon = Vector2(.01, 0);
//	Vector3 n = distance - vec3(
//		GetDist(position - epsilon.xyy), //X Component
//		GetDist(position - epsilon.yxy), //Y Component
//		GetDist(position - epsilon.yyx)  //Z Component
//	);
//
//	return normalize(n);
//}

bool groundIntersectionTest(Vector3 point)
{
	return point.y > GROUND_HEIGHT;
}

// sigmoid
float smin(float a, float b, float k)
{
	k *= log(2.0);
	float x = b - a;
	return a + x / (1.0 - exp2(x / k));
}

//TODO: Implement new smooth minimums code
char traceRay(Vector3* spheres, double* size, int count, Vector3 position, Vector3 direction, double maxDistance)
{

	double reflectiveness = 1000; // [0, infinity]

	Vector3 normalizedDirection = vector3_normalize(direction);
	double stepSize = 0.0;

	Vector3 rayPos;
	rayPos.x = position.x;
	rayPos.y = position.y;
	rayPos.z = position.z;

	int reflection_count = 0;

	for (double i = 0; i < maxDistance;)
	{
		//raymarch distance calculation checked against the spheres and the ground

		//double minDist = GetDistance(spheres, size, count, maxDistance, rayPos);

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

			//normalizedDirection = vector3_normalize(vector3_add(vector3_reflect(normalizedDirection, up), vector3_random()));
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
				//Vector3 sphereNorm = sphereNormal(spheres[z], rayPos);
				//double angle = fabs(vector3_angle(normalizedDirection, sphereNorm));
				//normalizedDirection = vector3_normalize(vector3_add(vector3_reflect(vector3_scale(normalizedDirection, reflectiveness + angle * angle * angle), sphereNorm), vector3_random()));

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

void fsRayTrace(Vector3* spheres, double *size, int count, double fov, double maxDepth)
{
	for (int i = 0; i < width * height; i++)
	{
		Vector3 rayDir;
		rayDir.x = (((i % width)  - (width / 2.0)) / width * 2) * fov/90 * ( (double)width / height);
		rayDir.y = (((i / (double)height) - (height / 2.0)) / height * 2) * fov/90 * ( (double)height / width);
		rayDir.z = 1;
		//Vector3 rayPos;

		Vector3 rayPos;
		rayPos.x = 0;
		rayPos.y = 0;
		rayPos.z = 0;

		renderArray[i] = traceRay(spheres, size, count, rayPos, rayDir, maxDepth);
	}
}

typedef struct {
	Vector3* spheres;
	double* size;
	int count;
	double fov;
	double maxDepth;
	int id;
} RayGroupArgs;

DWORD WINAPI rtWorker(LPVOID arg)
{
	RayGroupArgs* args = (RayGroupArgs*)arg;

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

		renderArray[i] = traceRay(args->spheres, args->size, args->count, rayPos, rayDir, args->maxDepth);
	}

	free(args); // free allocated memory after use
	return 0;
}

int fsRayTraceMultithreaded(Vector3* spheres, double* size, int count, double fov, double maxDepth)
{
	HANDLE threads[NUM_THREADS];

	// Launch threads
	for (int i = 0; i < NUM_THREADS; i++) {
		RayGroupArgs* args = malloc(sizeof(RayGroupArgs));
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
			rtWorker,     // thread function
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


int init()
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
	printArray();
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
