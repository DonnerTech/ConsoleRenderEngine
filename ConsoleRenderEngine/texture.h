#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <wincodec.h>
#include <Windows.h>
#include <stdio.h>
#include "vector2.h"

#pragma comment(lib, "windowscodecs.lib")

#define TEXMODE_CLAMPED 0
#define TEXMODE_REPEATING 1
#define RGBA_WHITE {255,255,255,255}

typedef struct {

	float uvScale;
	BYTE texMode;
	BYTE byteCount;
	UINT width;		// image width
	UINT height;	// image height

	UINT stride;	// image data width
	UINT imageSize; // the total 1D length of the image data

	// The array of raw pixel data. A sequence of BYTE values for each channel.
	BYTE* pixeldata;

} Texture;

#define CHAR_COUNT 10

char texture_ValToChar(BYTE value);

int texture_sample(Texture* texture_ptr, Vector2 pos, BYTE RGBA[4]);

void texture_DebugPrint(Texture *texture_ptr, int colorMode);


#endif // TEXTURE_H