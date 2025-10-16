
#include "textureLoader.h"


// returns false if the decoder failed to load the image
int texLoader_LoadImage(Texture* texture, const unsigned short* const fileName) 
{
    CoInitialize(NULL);
    IWICImagingFactory *factory = NULL;

    HRESULT hr = CoCreateInstance(
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

    texture->width = width;
    texture->height = height;

    texture->bitCount = 4;
    texture->stride = width * texture->bitCount; // 4 bytes per pixel
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

void texLoader_FreeTexture(Texture* texture)
{
    free(texture->pixeldata);
    free(texture);
}