#pragma warning(disable : 4996).

#include "physicsTest.h"

#include "textureLoader.h"

int main(void)
{
	// seed the random number generator
	srand((unsigned int)time(NULL));

	printf("Welcome to CoralEngine. The Console based game engine.\n \n");

	system("pause");

	Texture *textureA = (Texture*)malloc(sizeof(Texture));

	texLoader_LoadImage(textureA, L"texture_test.png");


	texture_DebugPrint(textureA, 0);

	texture_DebugPrint(textureA, 1);

	texLoader_FreeTexture(textureA);

	system("pause");

	physics_test();

	return 0;
}