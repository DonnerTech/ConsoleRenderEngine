#pragma once
#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "texture.h"


int texLoader_LoadImage(Texture* texture, const unsigned short* const fileName);

void texLoader_FreeTexture(Texture* texture);

#endif // TEXTURELOADER_H

