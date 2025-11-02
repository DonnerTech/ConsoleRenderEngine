#pragma once
#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#ifndef _WIN32
#include "stb_image.h"
#endif // !_WIN32

#include "texture.h"

// Loads an image to a texture struct
// returns: success state
int texLoader_LoadTexture(Texture* texture, const unsigned short* const fileName);

// Allocs and configures a texture struct
// returns: succeess state
void texLoader_generateTexture(Texture* texture, int byteCount, int width, int height);

// Fills a texture struct with a single color
// precondition: the color must have the same BYTE count as the colors in the texture
void texLoader_fillTexture(Texture* texture, BYTE* color);

// frees a texture struct
// Warning: ONLY for dynamically allocated textures
void texLoader_FreeTexture(Texture* texture);

// Demo function for texture loading
void texLoader_test(void);

#endif // TEXTURE_LOADER_H

