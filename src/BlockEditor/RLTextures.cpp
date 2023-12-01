#include "stdafx.h"
#include "RL.h"
#include "stb/stb_image.h"

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                    0x8D64
#endif

#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG  0x8C00
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR     0x93b0
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_8x8_KHR
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR     0x93b7
#endif

unsigned int defaultTextureId;      // Default texture used on shapes/poly drawing (required by shader)
void rlLoadTextureDefault()
{
    // Init default white texture
    unsigned char pixels[4] = { 255, 255, 255, 255 };   // 1 pixel RGBA (4 bytes)
    defaultTextureId = rlLoadTexture(pixels, 1, 1, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

    if (defaultTextureId != 0) TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Default texture loaded successfully", defaultTextureId);
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: Failed to load default texture");
}

// Get pixel data size in bytes (image or texture)
// NOTE: Size depends on pixel format
static int rlGetPixelDataSize(int width, int height, int format)
{
    int dataSize = 0;       // Size in bytes
    int bpp = 0;            // Bits per pixel

    switch (format)
    {
    case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
    case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
    case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
    case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R32: bpp = 32; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 32 * 3; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 32 * 4; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R16: bpp = 16; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: bpp = 16 * 3; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: bpp = 16 * 4; break;
    case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB:
    case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA:
    case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB:
    case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB:
    case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB:
    case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: bpp = 4; break;
    case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA:
    case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA:
    case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
    case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
    case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
    default: break;
    }

    dataSize = width * height * bpp / 8;  // Total data size in bytes

    // Most compressed formats works on 4x4 blocks,
    // if texture is smaller, minimum dataSize is 8 or 16
    if ((width < 4) && (height < 4))
    {
        if ((format >= RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) && (format < RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA)) dataSize = 8;
        else if ((format >= RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA) && (format < RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)) dataSize = 16;
    }
       
    return dataSize;
}

// Get OpenGL internal formats and data type from raylib PixelFormat
void rlGetGlTextureFormats(int format, unsigned int* glInternalFormat, unsigned int* glFormat, unsigned int* glType)
{
    *glInternalFormat = 0;
    *glFormat = 0;
    *glType = 0;

    switch (format)
    {
    case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_R8; *glFormat = GL_RED; *glType = GL_UNSIGNED_BYTE; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_RG8; *glFormat = GL_RG; *glType = GL_UNSIGNED_BYTE; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB565; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB8; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGB5_A1; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA4; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA8; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R32: *glInternalFormat = GL_R32F; *glFormat = GL_RED; *glType = GL_FLOAT; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: *glInternalFormat = GL_RGB32F; *glFormat = GL_RGB; *glType = GL_FLOAT; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: *glInternalFormat = GL_RGBA32F; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R16: *glInternalFormat = GL_R16F; *glFormat = GL_RED; *glType = GL_HALF_FLOAT; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: *glInternalFormat = GL_RGB16F; *glFormat = GL_RGB; *glType = GL_HALF_FLOAT; break;
    case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: *glInternalFormat = GL_RGBA16F; *glFormat = GL_RGBA; *glType = GL_HALF_FLOAT; break;

    case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB: *glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
    case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA: *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
    case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA: *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
    case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA: *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
    case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB: *glInternalFormat = GL_ETC1_RGB8_OES; break;                      // NOTE: Requires OpenGL ES 2.0 or OpenGL 4.3
    case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB: *glInternalFormat = GL_COMPRESSED_RGB8_ETC2; break;               // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
    case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: *glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC; break;     // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
    case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB: *glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break;    // NOTE: Requires PowerVR GPU
    case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: *glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break;  // NOTE: Requires PowerVR GPU
    case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
    case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
    default: TRACELOG(RL_LOG_WARNING, "TEXTURE: Current format not supported (%i)", format); break;
    }
}

// Get name string for pixel format
const char* rlGetPixelFormatName(unsigned int format)
{
    switch (format)
    {
    case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: return "GRAYSCALE"; break;         // 8 bit per pixel (no alpha)
    case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: return "GRAY_ALPHA"; break;       // 8*2 bpp (2 channels)
    case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: return "R5G6B5"; break;               // 16 bpp
    case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: return "R8G8B8"; break;               // 24 bpp
    case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: return "R5G5B5A1"; break;           // 16 bpp (1 bit alpha)
    case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: return "R4G4B4A4"; break;           // 16 bpp (4 bit alpha)
    case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: return "R8G8B8A8"; break;           // 32 bpp
    case RL_PIXELFORMAT_UNCOMPRESSED_R32: return "R32"; break;                     // 32 bpp (1 channel - float)
    case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: return "R32G32B32"; break;         // 32*3 bpp (3 channels - float)
    case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: return "R32G32B32A32"; break;   // 32*4 bpp (4 channels - float)
    case RL_PIXELFORMAT_UNCOMPRESSED_R16: return "R16"; break;                     // 16 bpp (1 channel - half float)
    case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: return "R16G16B16"; break;         // 16*3 bpp (3 channels - half float)
    case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: return "R16G16B16A16"; break;   // 16*4 bpp (4 channels - half float)
    case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB: return "DXT1_RGB"; break;             // 4 bpp (no alpha)
    case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA: return "DXT1_RGBA"; break;           // 4 bpp (1 bit alpha)
    case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA: return "DXT3_RGBA"; break;           // 8 bpp
    case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA: return "DXT5_RGBA"; break;           // 8 bpp
    case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB: return "ETC1_RGB"; break;             // 4 bpp
    case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB: return "ETC2_RGB"; break;             // 4 bpp
    case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: return "ETC2_RGBA"; break;       // 8 bpp
    case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB: return "PVRT_RGB"; break;             // 4 bpp
    case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: return "PVRT_RGBA"; break;           // 4 bpp
    case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: return "ASTC_4x4_RGBA"; break;   // 8 bpp
    case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: return "ASTC_8x8_RGBA"; break;   // 2 bpp
    default: return "UNKNOWN"; break;
    }
}

// Convert image data to OpenGL texture (returns OpenGL valid Id)
unsigned int rlLoadTexture(const void* data, int width, int height, int format, int mipmapCount)
{
    unsigned int id = 0;

    glBindTexture(GL_TEXTURE_2D, 0);    // Free any old binding

    // Check texture format support by OpenGL 1.1 (compressed textures not supported)
    if (((format == RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) || (format == RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA) ||
        (format == RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA) || (format == RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: DXT compressed texture format not supported");
        return id;
    }
    if ((format == RL_PIXELFORMAT_COMPRESSED_ETC1_RGB))
    {
        TRACELOG(RL_LOG_WARNING, "GL: ETC1 compressed texture format not supported");
        return id;
    }

    if (((format == RL_PIXELFORMAT_COMPRESSED_ETC2_RGB) || (format == RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: ETC2 compressed texture format not supported");
        return id;
    }

    if (((format == RL_PIXELFORMAT_COMPRESSED_PVRT_RGB) || (format == RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: PVRT compressed texture format not supported");
        return id;
    }

    if (((format == RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA) || (format == RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: ASTC compressed texture format not supported");
        return id;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &id);              // Generate texture id

    glBindTexture(GL_TEXTURE_2D, id);

    int mipWidth = width;
    int mipHeight = height;
    int mipOffset = 0;          // Mipmap data offset, only used for tracelog

    // NOTE: Added pointer math separately from function to avoid UBSAN complaining
    unsigned char* dataPtr = NULL;
    if (data != NULL) dataPtr = (unsigned char*)data;

    // Load the different mipmap levels
    for (int i = 0; i < mipmapCount; i++)
    {
        unsigned int mipSize = rlGetPixelDataSize(mipWidth, mipHeight, format);

        unsigned int glInternalFormat, glFormat, glType;
        rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

        TRACELOGD("TEXTURE: Load mipmap level %i (%i x %i), size: %i, offset: %i", i, mipWidth, mipHeight, mipSize, mipOffset);

        if (glInternalFormat != 0)
        {
            if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, glFormat, glType, dataPtr);
            else glCompressedTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, mipSize, dataPtr);

            if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
        }

        mipWidth /= 2;
        mipHeight /= 2;
        mipOffset += mipSize;       // Increment offset position to next mipmap
        if (data != NULL) dataPtr += mipSize;         // Increment data pointer to next mipmap

        // Security check for NPOT textures
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }

    // Texture parameters configuration
    // NOTE: glTexParameteri does NOT affect texture uploading, just the way it's used
#if defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: OpenGL ES 2.0 with no GL_OES_texture_npot support (i.e. WebGL) has limited NPOT support, so CLAMP_TO_EDGE must be used
    if (RLGL.ExtSupported.texNPOT)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
    }
    else
    {
        // NOTE: If using negative texture coordinates (LoadOBJ()), it does not work!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);       // Set texture to clamp on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);       // Set texture to clamp on y-axis
    }
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
#endif

    // Magnification and minification filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR

#if defined(GRAPHICS_API_OPENGL_33)
    if (mipmapCount > 1)
    {
        // Activate Trilinear filtering if mipmaps are available
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
#endif

    // At this point we have the texture loaded in GPU and texture parameters configured

    // NOTE: If mipmaps were not in data, they are not generated automatically

    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);

    if (id > 0) TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Texture loaded successfully (%ix%i | %s | %i mipmaps)", id, width, height, rlGetPixelFormatName(format), mipmapCount);
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: Failed to load texture");

    return id;
}

// Get default internal texture (white texture)
// NOTE: Default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
unsigned int rlGetTextureIdDefault(void)
{
    unsigned int id = 0;
    id = defaultTextureId;
    return id;
}

// Unload texture from GPU memory
void rlUnloadTexture(unsigned int id)
{
    glDeleteTextures(1, &id);
}

// Unload texture from GPU memory (VRAM)
void UnloadTexture(RLTexture2D texture)
{
    if (texture.id > 0)
    {
        rlUnloadTexture(texture.id);
        TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Unloaded texture data from VRAM (GPU)", texture.id);
    }
}

// Load a texture from image data
// NOTE: image is not unloaded, it must be done manually
RLTexture2D LoadTextureFromImage(RLImage image)
{
    RLTexture2D texture = { 0 };

    if ((image.width != 0) && (image.height != 0))
    {
        texture.id = rlLoadTexture(image.data, image.width, image.height, image.format, image.mipmaps);
    }
    else TRACELOG(LOG_WARNING, "IMAGE: Data is not valid to load texture");

    texture.width = image.width;
    texture.height = image.height;
    texture.mipmaps = image.mipmaps;
    texture.format = image.format;

    return texture;
}

// Unload image from CPU memory (RAM)
void UnloadImage(RLImage image)
{
    RL_FREE(image.data);
}

// Load texture from file into GPU memory (VRAM)
RLTexture2D LoadTexture(const char* fileName)
{
    RLTexture2D texture = { 0 };

    RLImage image = LoadImage(fileName);

    if (image.data != NULL)
    {
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }

    return texture;
}

// Load image from file into CPU memory (RAM)
RLImage LoadImage(const char* fileName)
{
    RLImage image = { 0 };

#if defined(SUPPORT_FILEFORMAT_PNG) || \
    defined(SUPPORT_FILEFORMAT_BMP) || \
    defined(SUPPORT_FILEFORMAT_TGA) || \
    defined(SUPPORT_FILEFORMAT_JPG) || \
    defined(SUPPORT_FILEFORMAT_GIF) || \
    defined(SUPPORT_FILEFORMAT_PIC) || \
    defined(SUPPORT_FILEFORMAT_HDR) || \
    defined(SUPPORT_FILEFORMAT_PNM) || \
    defined(SUPPORT_FILEFORMAT_PSD)

#define STBI_REQUIRED
#endif

    // Loading file to memory
    int dataSize = 0;
    unsigned char* fileData = LoadFileData(fileName, &dataSize);

    // Loading image from memory data
    if (fileData != NULL) image = LoadImageFromMemory(GetFileExtension(fileName), fileData, dataSize);

    RL_FREE(fileData);

    return image;
}

// Load image from memory buffer, fileType refers to extension: i.e. ".png"
// WARNING: File extension must be provided in lower-case
RLImage LoadImageFromMemory(const char* fileType, const unsigned char* fileData, int dataSize)
{
    RLImage image = { 0 };

    if ((false)
#if defined(SUPPORT_FILEFORMAT_PNG)
        || (strcmp(fileType, ".png") == 0) || (strcmp(fileType, ".PNG") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_BMP)
        || (strcmp(fileType, ".bmp") == 0) || (strcmp(fileType, ".BMP") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_TGA)
        || (strcmp(fileType, ".tga") == 0) || (strcmp(fileType, ".TGA") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_JPG)
        || (strcmp(fileType, ".jpg") == 0) || (strcmp(fileType, ".jpeg") == 0)
        || (strcmp(fileType, ".JPG") == 0) || (strcmp(fileType, ".JPEG") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_GIF)
        || (strcmp(fileType, ".gif") == 0) || (strcmp(fileType, ".GIF") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_PIC)
        || (strcmp(fileType, ".pic") == 0) || (strcmp(fileType, ".PIC") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_PNM)
        || (strcmp(fileType, ".ppm") == 0) || (strcmp(fileType, ".pgm") == 0)
        || (strcmp(fileType, ".PPM") == 0) || (strcmp(fileType, ".PGM") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_PSD)
        || (strcmp(fileType, ".psd") == 0) || (strcmp(fileType, ".PSD") == 0)
#endif
        )
    {
#if defined(STBI_REQUIRED)
        // NOTE: Using stb_image to load images (Supports multiple image formats)

        if (fileData != NULL)
        {
            int comp = 0;
            image.data = stbi_load_from_memory(fileData, dataSize, &image.width, &image.height, &comp, 0);

            if (image.data != NULL)
            {
                image.mipmaps = 1;

                if (comp == 1) image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
                else if (comp == 2) image.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
                else if (comp == 3) image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
                else if (comp == 4) image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            }
        }
#endif
    }
#if defined(SUPPORT_FILEFORMAT_HDR)
    else if ((strcmp(fileType, ".hdr") == 0) || (strcmp(fileType, ".HDR") == 0))
    {
#if defined(STBI_REQUIRED)
        if (fileData != NULL)
        {
            int comp = 0;
            image.data = stbi_loadf_from_memory(fileData, dataSize, &image.width, &image.height, &comp, 0);

            image.mipmaps = 1;

            if (comp == 1) image.format = PIXELFORMAT_UNCOMPRESSED_R32;
            else if (comp == 3) image.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32;
            else if (comp == 4) image.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
            else
            {
                TRACELOG(LOG_WARNING, "IMAGE: HDR file format not supported");
                UnloadImage(image);
            }
        }
#endif
    }
#endif
#if defined(SUPPORT_FILEFORMAT_QOI)
    else if ((strcmp(fileType, ".qoi") == 0) || (strcmp(fileType, ".QOI") == 0))
    {
        if (fileData != NULL)
        {
            qoi_desc desc = { 0 };
            image.data = qoi_decode(fileData, dataSize, &desc, 4);
            image.width = desc.width;
            image.height = desc.height;
            image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            image.mipmaps = 1;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_SVG)
    else if ((strcmp(fileType, ".svg") == 0) || (strcmp(fileType, ".SVG") == 0))
    {
        // Validate fileData as valid SVG string data
        //<svg xmlns="http://www.w3.org/2000/svg" width="2500" height="2484" viewBox="0 0 192.756 191.488">
        if ((fileData != NULL) &&
            (fileData[0] == '<') &&
            (fileData[1] == 's') &&
            (fileData[2] == 'v') &&
            (fileData[3] == 'g'))
        {
            struct NSVGimage* svgImage = nsvgParse(fileData, "px", 96.0f);
            unsigned char* img = RL_MALLOC(svgImage->width * svgImage->height * 4);

            // Rasterize
            struct NSVGrasterizer* rast = nsvgCreateRasterizer();
            nsvgRasterize(rast, svgImage, 0, 0, 1.0f, img, svgImage->width, svgImage->height, svgImage->width * 4);

            // Populate image struct with all data
            image.data = img;
            image.width = svgImage->width;
            image.height = svgImage->height;
            image.mipmaps = 1;
            image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

            nsvgDelete(svgImage);
            nsvgDeleteRasterizer(rast);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_DDS)
    else if ((strcmp(fileType, ".dds") == 0) || (strcmp(fileType, ".DDS") == 0))
    {
        image.data = rl_load_dds_from_memory(fileData, dataSize, &image.width, &image.height, &image.format, &image.mipmaps);
    }
#endif
#if defined(SUPPORT_FILEFORMAT_PKM)
    else if ((strcmp(fileType, ".pkm") == 0) || (strcmp(fileType, ".PKM") == 0))
    {
        image.data = rl_load_pkm_from_memory(fileData, dataSize, &image.width, &image.height, &image.format, &image.mipmaps);
    }
#endif
#if defined(SUPPORT_FILEFORMAT_KTX)
    else if ((strcmp(fileType, ".ktx") == 0) || (strcmp(fileType, ".KTX") == 0))
    {
        image.data = rl_load_ktx_from_memory(fileData, dataSize, &image.width, &image.height, &image.format, &image.mipmaps);
    }
#endif
#if defined(SUPPORT_FILEFORMAT_PVR)
    else if ((strcmp(fileType, ".pvr") == 0) || (strcmp(fileType, ".PVR") == 0))
    {
        image.data = rl_load_pvr_from_memory(fileData, dataSize, &image.width, &image.height, &image.format, &image.mipmaps);
    }
#endif
#if defined(SUPPORT_FILEFORMAT_ASTC)
    else if ((strcmp(fileType, ".astc") == 0) || (strcmp(fileType, ".ASTC") == 0))
    {
        image.data = rl_load_astc_from_memory(fileData, dataSize, &image.width, &image.height, &image.format, &image.mipmaps);
    }
#endif
    else TRACELOG(LOG_WARNING, "IMAGE: Data format not supported");

    if (image.data != NULL) TRACELOG(LOG_INFO, "IMAGE: Data loaded successfully (%ix%i | %s | %i mipmaps)", image.width, image.height, rlGetPixelFormatName(image.format), image.mipmaps);
    else TRACELOG(LOG_WARNING, "IMAGE: Failed to load image data");

    return image;
}

// Select and active a texture slot
void rlActiveTextureSlot(int slot)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glActiveTexture(GL_TEXTURE0 + slot);
#endif
}

// Enable texture cubemap
void rlEnableTextureCubemap(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
#endif
}

// Enable texture
void rlEnableTexture(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, id);
}



// Disable texture cubemap
void rlDisableTextureCubemap(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif
}

// Disable texture
void rlDisableTexture(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Get pixel data size in bytes for certain format
// NOTE: Size can be requested for Image or Texture data
int GetPixelDataSize(int width, int height, int format)
{
    int dataSize = 0;       // Size in bytes
    int bpp = 0;            // Bits per pixel

    switch (format)
    {
    case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
    case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
    case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
    case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
    case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
    case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
    case PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
    case PIXELFORMAT_UNCOMPRESSED_R32: bpp = 32; break;
    case PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 32 * 3; break;
    case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 32 * 4; break;
    case PIXELFORMAT_UNCOMPRESSED_R16: bpp = 16; break;
    case PIXELFORMAT_UNCOMPRESSED_R16G16B16: bpp = 16 * 3; break;
    case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: bpp = 16 * 4; break;
    case PIXELFORMAT_COMPRESSED_DXT1_RGB:
    case PIXELFORMAT_COMPRESSED_DXT1_RGBA:
    case PIXELFORMAT_COMPRESSED_ETC1_RGB:
    case PIXELFORMAT_COMPRESSED_ETC2_RGB:
    case PIXELFORMAT_COMPRESSED_PVRT_RGB:
    case PIXELFORMAT_COMPRESSED_PVRT_RGBA: bpp = 4; break;
    case PIXELFORMAT_COMPRESSED_DXT3_RGBA:
    case PIXELFORMAT_COMPRESSED_DXT5_RGBA:
    case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
    case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
    case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
    default: break;
    }

    dataSize = width * height * bpp / 8;  // Total data size in bytes

    // Most compressed formats works on 4x4 blocks,
    // if texture is smaller, minimum dataSize is 8 or 16
    if ((width < 4) && (height < 4))
    {
        if ((format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) && (format < PIXELFORMAT_COMPRESSED_DXT3_RGBA)) dataSize = 8;
        else if ((format >= PIXELFORMAT_COMPRESSED_DXT3_RGBA) && (format < PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)) dataSize = 16;
    }

    return dataSize;
}

// Unload render texture from GPU memory (VRAM)
void UnloadRenderTexture(RenderTexture target)
{
    if (target.id > 0)
    {
        if (target.texture.id > 0)
        {
            // Color texture attached to FBO is deleted
            rlUnloadTexture(target.texture.id);
        }

        // NOTE: Depth texture/renderbuffer is automatically
        // queried and deleted before deleting framebuffer
        rlUnloadFramebuffer(target.id);
    }
}

// Unload framebuffer from GPU memory
// NOTE: All attached textures/cubemaps/renderbuffers are also deleted
void rlUnloadFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    // Query depth attachment to automatically delete texture/renderbuffer
    int depthType = 0, depthId = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, id);   // Bind framebuffer to query depth texture type
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depthType);

    // TODO: Review warning retrieving object name in WebGL
    // WARNING: WebGL: INVALID_ENUM: getFramebufferAttachmentParameter: invalid parameter name
    // https://registry.khronos.org/webgl/specs/latest/1.0/
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthId);

    unsigned int depthIdU = (unsigned int)depthId;
    if (depthType == GL_RENDERBUFFER) glDeleteRenderbuffers(1, &depthIdU);
    else if (depthType == GL_TEXTURE) glDeleteTextures(1, &depthIdU);

    // NOTE: If a texture object is deleted while its image is attached to the *currently bound* framebuffer,
    // the texture image is automatically detached from the currently bound framebuffer.

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &id);

    TRACELOG(RL_LOG_INFO, "FBO: [ID %i] Unloaded framebuffer from VRAM (GPU)", id);
#endif
}

// Check if a texture is ready
bool IsTextureReady(RLTexture2D texture)
{
    // TODO: Validate maximum texture size supported by GPU?

    return ((texture.id > 0) &&         // Validate OpenGL id
        (texture.width > 0) &&
        (texture.height > 0) &&     // Validate texture size
        (texture.format > 0) &&     // Validate texture pixel format
        (texture.mipmaps > 0));     // Validate texture mipmaps (at least 1 for basic mipmap level)
}

// Load texture for rendering (framebuffer)
// NOTE: Render texture is loaded by default with RGBA color attachment and depth RenderBuffer
RenderTexture LoadRenderTexture(int width, int height)
{
    RenderTexture target = { 0 };

    target.id = rlLoadFramebuffer(width, height);   // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(NULL, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

        // Create depth renderbuffer/texture
        target.depth.id = rlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}

// Load a framebuffer to be used for rendering
// NOTE: No textures attached
unsigned int rlLoadFramebuffer(int width, int height)
{
    unsigned int fboId = 0;

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glGenFramebuffers(1, &fboId);       // Create the framebuffer object
    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // Unbind any framebuffer
#endif

    return fboId;
}

// Enable rendering to texture (fbo)
void rlEnableFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, id);
#endif
}

// Load depth texture/renderbuffer (to be attached to fbo)
// WARNING: OpenGL ES 2.0 requires GL_OES_depth_texture and WebGL requires WEBGL_depth_texture extensions
unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // In case depth textures not supported, we force renderbuffer usage
    useRenderBuffer = true;

    // NOTE: We let the implementation to choose the best bit-depth
    // Possible formats: GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32 and GL_DEPTH_COMPONENT32F
    unsigned int glInternalFormat = GL_DEPTH_COMPONENT;

#if (defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_ES3))
    // WARNING: WebGL platform requires unsized internal format definition (GL_DEPTH_COMPONENT)
    // while other platforms using OpenGL ES 2.0 require/support sized internal formats depending on the GPU capabilities
    if (!RLGL.ExtSupported.texDepthWebGL || useRenderBuffer)
    {
        if (RLGL.ExtSupported.maxDepthBits == 32) glInternalFormat = GL_DEPTH_COMPONENT32_OES;
        else if (RLGL.ExtSupported.maxDepthBits == 24) glInternalFormat = GL_DEPTH_COMPONENT24_OES;
        else glInternalFormat = GL_DEPTH_COMPONENT16;
    }
#endif

    if (!useRenderBuffer)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        TRACELOG(RL_LOG_INFO, "TEXTURE: Depth texture loaded successfully");
    }
    else
    {
        // Create the renderbuffer that will serve as the depth attachment for the framebuffer
        // NOTE: A renderbuffer is simpler than a texture and could offer better performance on embedded devices
        glGenRenderbuffers(1, &id);
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        glRenderbufferStorage(GL_RENDERBUFFER, glInternalFormat, width, height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        //TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Depth renderbuffer loaded successfully (%i bits)", id, (RLGL.ExtSupported.maxDepthBits >= 24) ? RLGL.ExtSupported.maxDepthBits : 16);
    }
#endif

    return id;
}

// Attach color buffer texture to an fbo (unloads previous attachment)
// NOTE: Attach type: 0-Color, 1-Depth renderbuffer, 2-Depth texture
void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    switch (attachType)
    {
    case RL_ATTACHMENT_COLOR_CHANNEL0:
    case RL_ATTACHMENT_COLOR_CHANNEL1:
    case RL_ATTACHMENT_COLOR_CHANNEL2:
    case RL_ATTACHMENT_COLOR_CHANNEL3:
    case RL_ATTACHMENT_COLOR_CHANNEL4:
    case RL_ATTACHMENT_COLOR_CHANNEL5:
    case RL_ATTACHMENT_COLOR_CHANNEL6:
    case RL_ATTACHMENT_COLOR_CHANNEL7:
    {
        if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_2D, texId, mipLevel);
        else if (texType == RL_ATTACHMENT_RENDERBUFFER) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_RENDERBUFFER, texId);
        else if (texType >= RL_ATTACHMENT_CUBEMAP_POSITIVE_X) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_CUBE_MAP_POSITIVE_X + texType, texId, mipLevel);

    } break;
    case RL_ATTACHMENT_DEPTH:
    {
        if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
        else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texId);

    } break;
    case RL_ATTACHMENT_STENCIL:
    {
        if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
        else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, texId);

    } break;
    default: break;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

// Verify render texture is complete
bool rlFramebufferComplete(unsigned int id)
{
    bool result = false;

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
        case GL_FRAMEBUFFER_UNSUPPORTED: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer is unsupported", id); break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete attachment", id); break;
#if defined(GRAPHICS_API_OPENGL_ES2)
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete dimensions", id); break;
#endif
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer has a missing attachment", id); break;
        default: break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result = (status == GL_FRAMEBUFFER_COMPLETE);
#endif

    return result;
}

// Disable rendering to texture
void rlDisableFramebuffer(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}