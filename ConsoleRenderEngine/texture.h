#pragma once
#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <wincodec.h>
#include <Windows.h>
#include <stdio.h>

#pragma comment(lib, "windowscodecs.lib")

typedef struct {

	UINT bitCount;
	UINT width;		// image width
	UINT height;	// image height

	UINT stride;	// image data width
	UINT imageSize; // the total 1D length of the image data

	// The array of raw pixel data. A sequence of R, G, B, A BYTE values.
	BYTE* pixeldata;

} Texture;

#define CHAR_COUNT 10

char texture_ValToChar(BYTE value);

void texture_DebugPrint(Texture *texture_ptr, int colorMode);


#endif // TEXTURE_H