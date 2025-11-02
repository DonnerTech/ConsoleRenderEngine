
#include "textureLoader.h"



int texLoader_LoadTexture(Texture* texture, const unsigned short* const fileName) 
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return 0;

    IWICImagingFactory *factory = NULL;

    hr = CoCreateInstance(
        &CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_IWICImagingFactory,
        (void**)&factory
    );
    if (FAILED(hr)) return 0;

    printf("Image Factory Init Success\n");

    // Load the image
    IWICBitmapDecoder* decoder = NULL;
    hr = factory->lpVtbl->CreateDecoderFromFilename(
        factory,
        fileName,
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    if (FAILED(hr)) return 0;

    printf("Image Found\n");

    IWICBitmapFrameDecode* frame = NULL;
    decoder->lpVtbl->GetFrame(decoder, 0, &frame);

    UINT width, height;
    frame->lpVtbl->GetSize(frame, &width, &height);

    // Create a format converter
    IWICFormatConverter* converter = NULL;
    factory->lpVtbl->CreateFormatConverter(factory, &converter);

    converter->lpVtbl->Initialize(
        converter,
        (IWICBitmapSource*)frame,
        &GUID_WICPixelFormat32bppRGBA, // output format: 4 bytes per pixel
        WICBitmapDitherTypeNone,
        NULL,
        0.0,
        WICBitmapPaletteTypeCustom
    );

    printf("Image Converter Created\n");

    // Allocate texture data for RGBA pixels
    //texture = (Texture*)malloc(sizeof(Texture));

    if (texture == NULL)
        return 0;

    texture->texMode = TEXMODE_REPEATING;
    texture->uvScale = 1;
    texture->width = width;
    texture->height = height;

    texture->byteCount = 4;
    texture->stride = width * texture->byteCount; // 4 bytes per pixel
    texture->imageSize = texture->stride * height;
    texture->pixeldata = (BYTE*)malloc(texture->imageSize);

    printf("Texture Initialized\n");

    // Copy pixel data into a buffer
    converter->lpVtbl->CopyPixels(
        converter,
        NULL, // entire image
        texture->stride,
        texture->imageSize,
        texture->pixeldata
    );

    printf("Successfully Loaded %ux%u RGBA image.\n", width, height);

    // Cleanup
    converter->lpVtbl->Release(converter);
    frame->lpVtbl->Release(frame);
    decoder->lpVtbl->Release(decoder);

    factory->lpVtbl->Release(factory);
    CoUninitialize();

    printf("Texture Loader Cleanup Complete\n");

    return 1;
}

void texLoader_generateTexture(Texture* texture, int byteCount, int width, int height)
{
    texture->texMode = TEXMODE_REPEATING;
    texture->uvScale = 1;
    texture->width = width;
    texture->height = height;

    texture->byteCount = byteCount;
    texture->stride = width * texture->byteCount;
    texture->imageSize = texture->stride * height;
    texture->pixeldata = (BYTE*)malloc(texture->imageSize);

}

void texLoader_fillTexture(Texture* texture, BYTE* color)
{
    for (int i = 0; i < texture->imageSize; i += texture->byteCount)
    {
        for (int n = 0; n < texture->byteCount; n++)
        {
            texture->pixeldata[i + n] = color[n];
        }
    }
}


void texLoader_FreeTexture(Texture* texture)
{
    free(texture->pixeldata);
    texture->pixeldata = NULL;
    free(texture);
    texture = NULL;
}

void texLoader_test(void)
{
    Texture* textureA = (Texture*)malloc(sizeof(Texture));

    texLoader_LoadTexture(textureA, L"textures\\texture_test.png");


    texture_DebugPrint(textureA, 0);

    texture_DebugPrint(textureA, 1);

    texLoader_FreeTexture(textureA);

    system("pause");
}