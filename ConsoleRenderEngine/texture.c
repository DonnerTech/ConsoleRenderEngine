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
    float scale = texture_ptr->uvScale;
    int index = 0;
    switch (texture_ptr->texMode)
    {
        case 0:
            index = (int)(pos.y * scale * texture_ptr->height) * texture_ptr->stride + (int)(pos.x* scale * texture_ptr->width) * texture_ptr->byteCount;
            break;
        case 1:
            index = (int)(((pos.y * scale < 0) + fmod(pos.y * scale, 1)) * texture_ptr->height) * texture_ptr->stride + (int)(((pos.x * scale < 0) + fmod(pos.x * scale, 1)) * texture_ptr->width) * texture_ptr->byteCount;
            break;
    }
    // snap to pixel
    index /= 4;
    index *= 4;

    if (index - 4 > texture_ptr->imageSize || index < 0)
        return 0;

    RGBA[0] = texture_ptr->pixeldata[index];
    RGBA[1] = texture_ptr->pixeldata[index+1];
    RGBA[2] = texture_ptr->pixeldata[index+2];
    RGBA[3] = texture_ptr->pixeldata[index+3];

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