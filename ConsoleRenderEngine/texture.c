#include "texture.h"

char texture_ValToChar(BYTE value)
{
    const char chars[CHAR_COUNT] = { ' ', '.', ':', '-', '=', '+', '*', '#', '%', '@' };

    int index = ((int)value * CHAR_COUNT) / 255;

    index = index > (CHAR_COUNT - 1) ? (CHAR_COUNT - 1) : index;
    index = index < 0 ? 0 : index;

    return chars[index];
}

int texture_sample(Texture* texture_ptr, Vector2 pos, BYTE RGBA[4])
{
    RGBA[3] = 255;

    float scale = texture_ptr->uvScale;
    float u = pos.x * scale;
    float v = pos.y * scale;

    // Handle wrapping / clamping
    switch (texture_ptr->texMode)
    {
    case TEXMODE_CLAMPED:
        if (u < 0 || u > 1 || v < 0 || v > 1)
        {
            for (int n = 0; n < 4; n++)
                RGBA[n] = 0;
            return 1;
        }
        u = fminf(fmaxf(u, 0.0f), 0.999999f);
        v = fminf(fmaxf(v, 0.0f), 0.999999f);
        break;

    case TEXMODE_REPEATING:
        u = u - floorf(u); // wrap to [0,1)
        v = v - floorf(v); // wrap to [0,1)
        break;
    }

    // Compute integer pixel coordinates
    int x = (int)(u * texture_ptr->width);
    int y = (int)(v * texture_ptr->height);

    // Texture bounds Clamping
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= (int)texture_ptr->width)  x = texture_ptr->width - 1;
    if (y >= (int)texture_ptr->height) y = texture_ptr->height - 1;

    int index = y * texture_ptr->stride + x * texture_ptr->byteCount;

    if (index < 0 || index + texture_ptr->byteCount >(int)texture_ptr->imageSize)
        return 0;

    // Copy pixel bytes
    for (int n = 0; n < texture_ptr->byteCount; n++)
        RGBA[n] = texture_ptr->pixeldata[index + n];

    return 1;
}

void texture_DebugPrint(Texture* texture_ptr, int colorMode)
{
    // black and white print
    switch (colorMode)
    {
    case 0:
        for (int i = 0; i < texture_ptr->imageSize; i += 4)
        {
            if (i % texture_ptr->stride == 0)
            {
                printf("\n");
            }
            printf("%c", texture_ValToChar(texture_ptr->pixeldata[i]));
        }
        break;
    case 1:
        // color print
        for (int i = 0; i < texture_ptr->imageSize - 4; i += 4)
        {
            if (i % texture_ptr->stride == 0)
            {
                printf("\n");
            }

            printf("\033[48;2;%d;%d;%dm \033[0m", texture_ptr->pixeldata[i], texture_ptr->pixeldata[i + 1], texture_ptr->pixeldata[i + 2]);

        }
        break;
    }
}