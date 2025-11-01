#include "material.h"

void create_material(Material* mat, Projection proj, BYTE specularColor[4], float reflectivity)
{
	mat->projecton = proj;
	mat->reflectivity = reflectivity;
	mat->specularColor[0] = specularColor[0];//4
	mat->specularColor[1] = specularColor[1];//g
	mat->specularColor[2] = specularColor[2];//b
	mat->specularColor[3] = specularColor[3];//a
	mat->baseTexture = (Texture*)malloc(sizeof(Texture));
}

void free_material_list(Material* mat, int count)
{
	for (int i = 0; i < count; i++)
	{
		texLoader_FreeTexture(mat[i].baseTexture);
	}
	free(mat);
}

