#include "stdafx.h"
#include "RL.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"  // Required for: stbir_resize_uint8_linear() [ImageResize()]

#ifndef PIXELFORMAT_UNCOMPRESSED_R5G5B5A1_ALPHA_THRESHOLD
#define PIXELFORMAT_UNCOMPRESSED_R5G5B5A1_ALPHA_THRESHOLD  50    // Threshold over 255 to set alpha as 0
#endif

// From https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion/60047308#60047308

static float HalfToFloat(unsigned short x) {
    const unsigned int e = (x & 0x7C00) >> 10; // exponent
    const unsigned int m = (x & 0x03FF) << 13; // mantissa
    const float fm = (float)m;
    const unsigned int v = (*(unsigned int*)&fm) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
    const unsigned int r = (x & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) | ((e == 0) & (m != 0)) * ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000)); // sign : normalized : denormalized
    return *(float*)&r;
}

static unsigned short FloatToHalf(float x) {
    const unsigned int b = (*(unsigned int*)&x) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
    const unsigned int e = (b & 0x7F800000) >> 23; // exponent
    const unsigned int m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
    return (b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF; // sign : normalized : denormalized : saturate
}

// Get pixel data from image as Vector4 array (float normalized)
static Vector4* LoadImageDataNormalized(RLImage image)
{
    Vector4* pixels = (Vector4*)RL_MALLOC(image.width * image.height * sizeof(Vector4));

    if (image.format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "IMAGE: Pixel data retrieval not supported for compressed image formats");
    else
    {
        for (int i = 0, k = 0; i < image.width * image.height; i++)
        {
            switch (image.format)
            {
            case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            {
                pixels[i].x = (float)((unsigned char*)image.data)[i] / 255.0f;
                pixels[i].y = (float)((unsigned char*)image.data)[i] / 255.0f;
                pixels[i].z = (float)((unsigned char*)image.data)[i] / 255.0f;
                pixels[i].w = 1.0f;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            {
                pixels[i].x = (float)((unsigned char*)image.data)[k] / 255.0f;
                pixels[i].y = (float)((unsigned char*)image.data)[k] / 255.0f;
                pixels[i].z = (float)((unsigned char*)image.data)[k] / 255.0f;
                pixels[i].w = (float)((unsigned char*)image.data)[k + 1] / 255.0f;

                k += 2;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            {
                unsigned short pixel = ((unsigned short*)image.data)[i];

                pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11) * (1.0f / 31);
                pixels[i].y = (float)((pixel & 0b0000011111000000) >> 6) * (1.0f / 31);
                pixels[i].z = (float)((pixel & 0b0000000000111110) >> 1) * (1.0f / 31);
                pixels[i].w = ((pixel & 0b0000000000000001) == 0) ? 0.0f : 1.0f;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
            {
                unsigned short pixel = ((unsigned short*)image.data)[i];

                pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11) * (1.0f / 31);
                pixels[i].y = (float)((pixel & 0b0000011111100000) >> 5) * (1.0f / 63);
                pixels[i].z = (float)(pixel & 0b0000000000011111) * (1.0f / 31);
                pixels[i].w = 1.0f;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            {
                unsigned short pixel = ((unsigned short*)image.data)[i];

                pixels[i].x = (float)((pixel & 0b1111000000000000) >> 12) * (1.0f / 15);
                pixels[i].y = (float)((pixel & 0b0000111100000000) >> 8) * (1.0f / 15);
                pixels[i].z = (float)((pixel & 0b0000000011110000) >> 4) * (1.0f / 15);
                pixels[i].w = (float)(pixel & 0b0000000000001111) * (1.0f / 15);

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            {
                pixels[i].x = (float)((unsigned char*)image.data)[k] / 255.0f;
                pixels[i].y = (float)((unsigned char*)image.data)[k + 1] / 255.0f;
                pixels[i].z = (float)((unsigned char*)image.data)[k + 2] / 255.0f;
                pixels[i].w = (float)((unsigned char*)image.data)[k + 3] / 255.0f;

                k += 4;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            {
                pixels[i].x = (float)((unsigned char*)image.data)[k] / 255.0f;
                pixels[i].y = (float)((unsigned char*)image.data)[k + 1] / 255.0f;
                pixels[i].z = (float)((unsigned char*)image.data)[k + 2] / 255.0f;
                pixels[i].w = 1.0f;

                k += 3;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32:
            {
                pixels[i].x = ((float*)image.data)[k];
                pixels[i].y = 0.0f;
                pixels[i].z = 0.0f;
                pixels[i].w = 1.0f;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            {
                pixels[i].x = ((float*)image.data)[k];
                pixels[i].y = ((float*)image.data)[k + 1];
                pixels[i].z = ((float*)image.data)[k + 2];
                pixels[i].w = 1.0f;

                k += 3;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            {
                pixels[i].x = ((float*)image.data)[k];
                pixels[i].y = ((float*)image.data)[k + 1];
                pixels[i].z = ((float*)image.data)[k + 2];
                pixels[i].w = ((float*)image.data)[k + 3];

                k += 4;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16:
            {
                pixels[i].x = HalfToFloat(((unsigned short*)image.data)[k]);
                pixels[i].y = 0.0f;
                pixels[i].z = 0.0f;
                pixels[i].w = 1.0f;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            {
                pixels[i].x = HalfToFloat(((unsigned short*)image.data)[k]);
                pixels[i].y = HalfToFloat(((unsigned short*)image.data)[k + 1]);
                pixels[i].z = HalfToFloat(((unsigned short*)image.data)[k + 2]);
                pixels[i].w = 1.0f;

                k += 3;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            {
                pixels[i].x = HalfToFloat(((unsigned short*)image.data)[k]);
                pixels[i].y = HalfToFloat(((unsigned short*)image.data)[k + 1]);
                pixels[i].z = HalfToFloat(((unsigned short*)image.data)[k + 2]);
                pixels[i].w = HalfToFloat(((unsigned short*)image.data)[k + 3]);

                k += 4;
            } break;
            default: break;
            }
        }
    }

    return pixels;
}

// Resize and image to new size
// NOTE: Uses stb default scaling filters (both bicubic):
// STBIR_DEFAULT_FILTER_UPSAMPLE    STBIR_FILTER_CATMULLROM
// STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_MITCHELL   (high-quality Catmull-Rom)
void ImageResize(RLImage* image, int newWidth, int newHeight)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    // Check if we can use a fast path on image scaling
    // It can be for 8 bit per channel images with 1 to 4 channels per pixel
    if ((image->format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) ||
        (image->format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) ||
        (image->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8) ||
        (image->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8))
    {
        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
        unsigned char* output = (unsigned char*)RL_MALLOC(newWidth * newHeight * bytesPerPixel);

        switch (image->format)
        {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: stbir_resize_uint8_linear((unsigned char*)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)1); break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: stbir_resize_uint8_linear((unsigned char*)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)2); break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: stbir_resize_uint8_linear((unsigned char*)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)3); break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: stbir_resize_uint8_linear((unsigned char*)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)4); break;
        default: break;
        }

        RL_FREE(image->data);
        image->data = output;
        image->width = newWidth;
        image->height = newHeight;
    }
    else
    {
        // Get data as Color pixels array to work with it
        Color* pixels = LoadImageColors(*image);
        Color* output = (Color*)RL_MALLOC(newWidth * newHeight * sizeof(Color));

        // NOTE: Color data is cast to (unsigned char *), there shouldn't been any problem...
        stbir_resize_uint8_linear((unsigned char*)pixels, image->width, image->height, 0, (unsigned char*)output, newWidth, newHeight, 0, (stbir_pixel_layout)4);

        int format = image->format;

        UnloadImageColors(pixels);
        RL_FREE(image->data);

        image->data = output;
        image->width = newWidth;
        image->height = newHeight;
        image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

        RLImageFormat(image, format);  // Reformat 32bit RGBA image to original format
    }
}

// Load color data from image as a Color array (RGBA - 32bit)
// NOTE: Memory allocated should be freed using UnloadImageColors();
Color* LoadImageColors(RLImage image)
{
    if ((image.width == 0) || (image.height == 0)) return NULL;

    Color* pixels = (Color*)RL_MALLOC(image.width * image.height * sizeof(Color));

    if (image.format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "IMAGE: Pixel data retrieval not supported for compressed image formats");
    else
    {
        if ((image.format == PIXELFORMAT_UNCOMPRESSED_R32) ||
            (image.format == PIXELFORMAT_UNCOMPRESSED_R32G32B32) ||
            (image.format == PIXELFORMAT_UNCOMPRESSED_R32G32B32A32)) TRACELOG(LOG_WARNING, "IMAGE: Pixel format converted from 32bit to 8bit per channel");

        if ((image.format == PIXELFORMAT_UNCOMPRESSED_R16) ||
            (image.format == PIXELFORMAT_UNCOMPRESSED_R16G16B16) ||
            (image.format == PIXELFORMAT_UNCOMPRESSED_R16G16B16A16)) TRACELOG(LOG_WARNING, "IMAGE: Pixel format converted from 16bit to 8bit per channel");

        for (int i = 0, k = 0; i < image.width * image.height; i++)
        {
            switch (image.format)
            {
            case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            {
                pixels[i].r = ((unsigned char*)image.data)[i];
                pixels[i].g = ((unsigned char*)image.data)[i];
                pixels[i].b = ((unsigned char*)image.data)[i];
                pixels[i].a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            {
                pixels[i].r = ((unsigned char*)image.data)[k];
                pixels[i].g = ((unsigned char*)image.data)[k];
                pixels[i].b = ((unsigned char*)image.data)[k];
                pixels[i].a = ((unsigned char*)image.data)[k + 1];

                k += 2;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            {
                unsigned short pixel = ((unsigned short*)image.data)[i];

                pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11) * (255 / 31));
                pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111000000) >> 6) * (255 / 31));
                pixels[i].b = (unsigned char)((float)((pixel & 0b0000000000111110) >> 1) * (255 / 31));
                pixels[i].a = (unsigned char)((pixel & 0b0000000000000001) * 255);

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
            {
                unsigned short pixel = ((unsigned short*)image.data)[i];

                pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11) * (255 / 31));
                pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111100000) >> 5) * (255 / 63));
                pixels[i].b = (unsigned char)((float)(pixel & 0b0000000000011111) * (255 / 31));
                pixels[i].a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            {
                unsigned short pixel = ((unsigned short*)image.data)[i];

                pixels[i].r = (unsigned char)((float)((pixel & 0b1111000000000000) >> 12) * (255 / 15));
                pixels[i].g = (unsigned char)((float)((pixel & 0b0000111100000000) >> 8) * (255 / 15));
                pixels[i].b = (unsigned char)((float)((pixel & 0b0000000011110000) >> 4) * (255 / 15));
                pixels[i].a = (unsigned char)((float)(pixel & 0b0000000000001111) * (255 / 15));

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            {
                pixels[i].r = ((unsigned char*)image.data)[k];
                pixels[i].g = ((unsigned char*)image.data)[k + 1];
                pixels[i].b = ((unsigned char*)image.data)[k + 2];
                pixels[i].a = ((unsigned char*)image.data)[k + 3];

                k += 4;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            {
                pixels[i].r = (unsigned char)((unsigned char*)image.data)[k];
                pixels[i].g = (unsigned char)((unsigned char*)image.data)[k + 1];
                pixels[i].b = (unsigned char)((unsigned char*)image.data)[k + 2];
                pixels[i].a = 255;

                k += 3;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32:
            {
                pixels[i].r = (unsigned char)(((float*)image.data)[k] * 255.0f);
                pixels[i].g = 0;
                pixels[i].b = 0;
                pixels[i].a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            {
                pixels[i].r = (unsigned char)(((float*)image.data)[k] * 255.0f);
                pixels[i].g = (unsigned char)(((float*)image.data)[k + 1] * 255.0f);
                pixels[i].b = (unsigned char)(((float*)image.data)[k + 2] * 255.0f);
                pixels[i].a = 255;

                k += 3;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            {
                pixels[i].r = (unsigned char)(((float*)image.data)[k] * 255.0f);
                pixels[i].g = (unsigned char)(((float*)image.data)[k] * 255.0f);
                pixels[i].b = (unsigned char)(((float*)image.data)[k] * 255.0f);
                pixels[i].a = (unsigned char)(((float*)image.data)[k] * 255.0f);

                k += 4;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16:
            {
                pixels[i].r = (unsigned char)(HalfToFloat(((unsigned short*)image.data)[k]) * 255.0f);
                pixels[i].g = 0;
                pixels[i].b = 0;
                pixels[i].a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            {
                pixels[i].r = (unsigned char)(HalfToFloat(((unsigned short*)image.data)[k]) * 255.0f);
                pixels[i].g = (unsigned char)(HalfToFloat(((unsigned short*)image.data)[k + 1]) * 255.0f);
                pixels[i].b = (unsigned char)(HalfToFloat(((unsigned short*)image.data)[k + 2]) * 255.0f);
                pixels[i].a = 255;

                k += 3;
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            {
                pixels[i].r = (unsigned char)(HalfToFloat(((unsigned short*)image.data)[k]) * 255.0f);
                pixels[i].g = (unsigned char)(HalfToFloat(((unsigned short*)image.data)[k]) * 255.0f);
                pixels[i].b = (unsigned char)(HalfToFloat(((unsigned short*)image.data)[k]) * 255.0f);
                pixels[i].a = (unsigned char)(HalfToFloat(((unsigned short*)image.data)[k]) * 255.0f);

                k += 4;
            } break;
            default: break;
            }
        }
    }

    return pixels;
}

// Unload color data loaded with LoadImageColors()
void UnloadImageColors(Color* colors)
{
    RL_FREE(colors);
}

// Convert image data to desired format
void RLImageFormat(RLImage* image, int newFormat)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if ((newFormat != 0) && (image->format != newFormat))
    {
        if ((image->format < PIXELFORMAT_COMPRESSED_DXT1_RGB) && (newFormat < PIXELFORMAT_COMPRESSED_DXT1_RGB))
        {
            Vector4* pixels = LoadImageDataNormalized(*image);     // Supports 8 to 32 bit per channel

            RL_FREE(image->data);      // WARNING! We loose mipmaps data --> Regenerated at the end...
            image->data = NULL;
            image->format = newFormat;

            switch (image->format)
            {
            case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            {
                image->data = (unsigned char*)RL_MALLOC(image->width * image->height * sizeof(unsigned char));

                for (int i = 0; i < image->width * image->height; i++)
                {
                    ((unsigned char*)image->data)[i] = (unsigned char)((pixels[i].x * 0.299f + pixels[i].y * 0.587f + pixels[i].z * 0.114f) * 255.0f);
                }

            } break;
            case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            {
                image->data = (unsigned char*)RL_MALLOC(image->width * image->height * 2 * sizeof(unsigned char));

                for (int i = 0, k = 0; i < image->width * image->height * 2; i += 2, k++)
                {
                    ((unsigned char*)image->data)[i] = (unsigned char)((pixels[k].x * 0.299f + (float)pixels[k].y * 0.587f + (float)pixels[k].z * 0.114f) * 255.0f);
                    ((unsigned char*)image->data)[i + 1] = (unsigned char)(pixels[k].w * 255.0f);
                }

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
            {
                image->data = (unsigned short*)RL_MALLOC(image->width * image->height * sizeof(unsigned short));

                unsigned char r = 0;
                unsigned char g = 0;
                unsigned char b = 0;

                for (int i = 0; i < image->width * image->height; i++)
                {
                    r = (unsigned char)(round(pixels[i].x * 31.0f));
                    g = (unsigned char)(round(pixels[i].y * 63.0f));
                    b = (unsigned char)(round(pixels[i].z * 31.0f));

                    ((unsigned short*)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;
                }

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            {
                image->data = (unsigned char*)RL_MALLOC(image->width * image->height * 3 * sizeof(unsigned char));

                for (int i = 0, k = 0; i < image->width * image->height * 3; i += 3, k++)
                {
                    ((unsigned char*)image->data)[i] = (unsigned char)(pixels[k].x * 255.0f);
                    ((unsigned char*)image->data)[i + 1] = (unsigned char)(pixels[k].y * 255.0f);
                    ((unsigned char*)image->data)[i + 2] = (unsigned char)(pixels[k].z * 255.0f);
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            {
                image->data = (unsigned short*)RL_MALLOC(image->width * image->height * sizeof(unsigned short));

                unsigned char r = 0;
                unsigned char g = 0;
                unsigned char b = 0;
                unsigned char a = 0;

                for (int i = 0; i < image->width * image->height; i++)
                {
                    r = (unsigned char)(round(pixels[i].x * 31.0f));
                    g = (unsigned char)(round(pixels[i].y * 31.0f));
                    b = (unsigned char)(round(pixels[i].z * 31.0f));
                    a = (pixels[i].w > ((float)PIXELFORMAT_UNCOMPRESSED_R5G5B5A1_ALPHA_THRESHOLD / 255.0f)) ? 1 : 0;

                    ((unsigned short*)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;
                }

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            {
                image->data = (unsigned short*)RL_MALLOC(image->width * image->height * sizeof(unsigned short));

                unsigned char r = 0;
                unsigned char g = 0;
                unsigned char b = 0;
                unsigned char a = 0;

                for (int i = 0; i < image->width * image->height; i++)
                {
                    r = (unsigned char)(round(pixels[i].x * 15.0f));
                    g = (unsigned char)(round(pixels[i].y * 15.0f));
                    b = (unsigned char)(round(pixels[i].z * 15.0f));
                    a = (unsigned char)(round(pixels[i].w * 15.0f));

                    ((unsigned short*)image->data)[i] = (unsigned short)r << 12 | (unsigned short)g << 8 | (unsigned short)b << 4 | (unsigned short)a;
                }

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            {
                image->data = (unsigned char*)RL_MALLOC(image->width * image->height * 4 * sizeof(unsigned char));

                for (int i = 0, k = 0; i < image->width * image->height * 4; i += 4, k++)
                {
                    ((unsigned char*)image->data)[i] = (unsigned char)(pixels[k].x * 255.0f);
                    ((unsigned char*)image->data)[i + 1] = (unsigned char)(pixels[k].y * 255.0f);
                    ((unsigned char*)image->data)[i + 2] = (unsigned char)(pixels[k].z * 255.0f);
                    ((unsigned char*)image->data)[i + 3] = (unsigned char)(pixels[k].w * 255.0f);
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32:
            {
                // WARNING: Image is converted to GRAYSCALE equivalent 32bit

                image->data = (float*)RL_MALLOC(image->width * image->height * sizeof(float));

                for (int i = 0; i < image->width * image->height; i++)
                {
                    ((float*)image->data)[i] = (float)(pixels[i].x * 0.299f + pixels[i].y * 0.587f + pixels[i].z * 0.114f);
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            {
                image->data = (float*)RL_MALLOC(image->width * image->height * 3 * sizeof(float));

                for (int i = 0, k = 0; i < image->width * image->height * 3; i += 3, k++)
                {
                    ((float*)image->data)[i] = pixels[k].x;
                    ((float*)image->data)[i + 1] = pixels[k].y;
                    ((float*)image->data)[i + 2] = pixels[k].z;
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            {
                image->data = (float*)RL_MALLOC(image->width * image->height * 4 * sizeof(float));

                for (int i = 0, k = 0; i < image->width * image->height * 4; i += 4, k++)
                {
                    ((float*)image->data)[i] = pixels[k].x;
                    ((float*)image->data)[i + 1] = pixels[k].y;
                    ((float*)image->data)[i + 2] = pixels[k].z;
                    ((float*)image->data)[i + 3] = pixels[k].w;
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16:
            {
                // WARNING: Image is converted to GRAYSCALE equivalent 16bit

                image->data = (unsigned short*)RL_MALLOC(image->width * image->height * sizeof(unsigned short));

                for (int i = 0; i < image->width * image->height; i++)
                {
                    ((unsigned short*)image->data)[i] = FloatToHalf((float)(pixels[i].x * 0.299f + pixels[i].y * 0.587f + pixels[i].z * 0.114f));
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            {
                image->data = (unsigned short*)RL_MALLOC(image->width * image->height * 3 * sizeof(unsigned short));

                for (int i = 0, k = 0; i < image->width * image->height * 3; i += 3, k++)
                {
                    ((unsigned short*)image->data)[i] = FloatToHalf(pixels[k].x);
                    ((unsigned short*)image->data)[i + 1] = FloatToHalf(pixels[k].y);
                    ((unsigned short*)image->data)[i + 2] = FloatToHalf(pixels[k].z);
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            {
                image->data = (unsigned short*)RL_MALLOC(image->width * image->height * 4 * sizeof(unsigned short));

                for (int i = 0, k = 0; i < image->width * image->height * 4; i += 4, k++)
                {
                    ((unsigned short*)image->data)[i] = FloatToHalf(pixels[k].x);
                    ((unsigned short*)image->data)[i + 1] = FloatToHalf(pixels[k].y);
                    ((unsigned short*)image->data)[i + 2] = FloatToHalf(pixels[k].z);
                    ((unsigned short*)image->data)[i + 3] = FloatToHalf(pixels[k].w);
                }
            } break;
            default: break;
            }

            RL_FREE(pixels);
            pixels = NULL;

            // In case original image had mipmaps, generate mipmaps for formatted image
            // NOTE: Original mipmaps are replaced by new ones, if custom mipmaps were used, they are lost
            if (image->mipmaps > 1)
            {
                image->mipmaps = 1;
#if defined(SUPPORT_IMAGE_MANIPULATION)
                if (image->data != NULL) ImageMipmaps(image);
#endif
            }
        }
        else TRACELOG(LOG_WARNING, "IMAGE: Data format is compressed, can not be converted");
    }
}