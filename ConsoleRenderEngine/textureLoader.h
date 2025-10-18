#pragma once
#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "texture.h"

// Loads an image to a texture struct
// returns: success state
int texLoader_LoadTexture(Texture* texture, const unsigned short* const fileName);

// Allocs and configures a texture struct
// returns: succeess state
texLoader_generateTexture(Texture* texture, int byteCount, int width, int height);

// frees a texture struct
// Warning: ONLY for dynamically allocated textures
void texLoader_FreeTexture(Texture* texture);

// Demo function for texture loading
void texLoader_test(void);

#endif // TEXTURE_LOADER_H

