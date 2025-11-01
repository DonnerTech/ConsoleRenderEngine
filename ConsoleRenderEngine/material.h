#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include "texture.h"
#include "textureLoader.h"

typedef enum Projection {
	PROJECT_PLANER,
	PROJECT_TRIPLANER,
	PROJECT_SPHERICAL
} Projection;

typedef struct Material {

	Texture* baseTexture;
	BYTE specularColor[4];
	float reflectivity;
	Projection projecton;

} Material;

// Initializes the properties of a matiral struct
// and allocates the base texture
void create_material(Material* mat, Projection proj, BYTE specularColor[4], float reflectivity);

void free_material_list(Material* mat, int count);

#endif // !MATERIAL_H
