#pragma once

#define MAX_FILEPATH_CAPACITY        8192       // Maximum file paths capacity
#define MAX_FILEPATH_LENGTH          4096       // Maximum length for filepaths (Linux PATH_MAX default value)
#define MAX_TEXTSPLIT_COUNT           128       // Maximum number of substrings to split: TextSplit()

#define SUPPORT_MODULE_RSHAPES          1
#define SUPPORT_MODULE_RTEXTURES        1
#define SUPPORT_MODULE_RTEXT            1       // WARNING: It requires SUPPORT_MODULE_RTEXTURES to load sprite font textures
#define SUPPORT_MODULE_RMODELS          1
#define SUPPORT_MODULE_RAUDIO           1
// Support text management functions
// If not defined, still some functions are supported: TextLength(), TextFormat()
#define SUPPORT_TEXT_MANIPULATION       1

#define _CRT_SECURE_NO_WARNINGS

// Selected desired model fileformats to be supported for loading
#define SUPPORT_FILEFORMAT_OBJ          1
#define SUPPORT_FILEFORMAT_MTL          1
//#define SUPPORT_FILEFORMAT_IQM          1
//#define SUPPORT_FILEFORMAT_GLTF         1
//#define SUPPORT_FILEFORMAT_VOX          1
//#define SUPPORT_FILEFORMAT_M3D          1

// Support framebuffer objects by default
// NOTE: Some driver implementation do not support it, despite they should
#define RLGL_RENDER_TEXTURES_HINT

#define MAX_TEXT_BUFFER_LENGTH       1024       // Size of internal static buffers used on some functions:

// Include pseudo-random numbers generator (rprand.h), based on Xoshiro128** and SplitMix64
#define SUPPORT_RPRAND_GENERATOR        1

#define GRAPHICS_API_OPENGL_33 1
#define SUPPORT_COMPRESSION_API         1
#define MAX_DECOMPRESSION_SIZE         64       // Max size allocated for decompression in MB

#define MAX_MATERIAL_MAPS              12       // Maximum number of shader maps supported
#define MAX_MESH_VERTEX_BUFFERS         7       // Maximum vertex buffers (VBO) per mesh

#define SUPPORT_FILEFORMAT_PNG      1
//#define SUPPORT_FILEFORMAT_BMP      1
//#define SUPPORT_FILEFORMAT_TGA      1
//#define SUPPORT_FILEFORMAT_JPG      1
#define SUPPORT_FILEFORMAT_GIF      1
//#define SUPPORT_FILEFORMAT_QOI      1
//#define SUPPORT_FILEFORMAT_PSD      1
//#define SUPPORT_FILEFORMAT_DDS      1
//#define SUPPORT_FILEFORMAT_HDR      1
//#define SUPPORT_FILEFORMAT_PIC          1
//#define SUPPORT_FILEFORMAT_KTX      1
//#define SUPPORT_FILEFORMAT_ASTC     1
//#define SUPPORT_FILEFORMAT_PKM      1
//#define SUPPORT_FILEFORMAT_PVR      1
//#define SUPPORT_FILEFORMAT_SVG      1

#define SUPPORT_STANDARD_FILEIO         1

#define RMAPI inline

#define RL_MAX_SHADER_LOCATIONS               32      // Maximum number of shader locations supported

// Allow custom memory allocators
// NOTE: Require recompiling raylib sources
#ifndef RL_MALLOC
#define RL_MALLOC(sz)       malloc(sz)
#endif
#ifndef RL_CALLOC
#define RL_CALLOC(n,sz)     calloc(n,sz)
#endif
#ifndef RL_REALLOC
#define RL_REALLOC(ptr,sz)  realloc(ptr,sz)
#endif
#ifndef RL_FREE
#define RL_FREE(ptr)        free(ptr)
#endif

#define MAX_TRACELOG_MSG_LENGTH       256       // Max length of one trace-log message

#ifndef PI
#define PI 3.14159265358979323846f
#endif
#ifndef EPSILON
#define EPSILON 0.000001f
#endif
#ifndef DEG2RAD
#define DEG2RAD (PI/180.0f)
#endif
#ifndef RAD2DEG
#define RAD2DEG (180.0f/PI)
#endif

#define RL_CULL_DISTANCE_NEAR               0.01      // Default projection matrix near cull distance
#define RL_CULL_DISTANCE_FAR              1000.0      // Default projection matrix far cull distance

#define TRACELOG(level, ...) TraceLog(level, __VA_ARGS__)
#define TRACELOGD(...) TraceLog(LOG_DEBUG, __VA_ARGS__)

// Trace log level
// NOTE: Organized by priority level
enum TraceLogLevel {
	LOG_ALL = 0,        // Display all logs
	LOG_TRACE,          // Trace logging, intended for internal use only
	LOG_DEBUG,          // Debug logging, used for internal debugging, it should be disabled on release builds
	LOG_INFO,           // Info logging, used for program execution info
	LOG_WARNING,        // Warning logging, used on recoverable failures
	LOG_ERROR,          // Error logging, used on unrecoverable failures
	LOG_FATAL,          // Fatal logging, used to abort program: exit(EXIT_FAILURE)
	LOG_NONE            // Disable logging
} ;

// Vector2, 2 components
struct Vector2 {
	float x;                // Vector x component
	float y;                // Vector y component
};

// Vector3, 3 components
struct Vector3 {
	float x;                // Vector x component
	float y;                // Vector y component
	float z;                // Vector z component
};

// Vector4, 4 components
struct Vector4 {
	float x;                // Vector x component
	float y;                // Vector y component
	float z;                // Vector z component
	float w;                // Vector w component
};

// Quaternion, 4 components (Vector4 alias)
typedef Vector4 Quaternion;

// Rectangle, 4 components
struct Rectangle {
	float x;                // Rectangle top-left corner position x
	float y;                // Rectangle top-left corner position y
	float width;            // Rectangle width
	float height;           // Rectangle height
};

// Matrix, 4x4 components, column major, OpenGL style, right-handed
struct Matrix {
	float m0, m4, m8, m12;  // Matrix first row (4 components)
	float m1, m5, m9, m13;  // Matrix second row (4 components)
	float m2, m6, m10, m14; // Matrix third row (4 components)
	float m3, m7, m11, m15; // Matrix fourth row (4 components)
};

// Color, 4 components, R8G8B8A8 (32bit)
struct RLColor {
	unsigned char r;        // Color red value
	unsigned char g;        // Color green value
	unsigned char b;        // Color blue value
	unsigned char a;        // Color alpha value
};

// Camera projection
enum CameraProjection {
	CAMERA_PERSPECTIVE = 0,         // Perspective projection
	CAMERA_ORTHOGRAPHIC             // Orthographic projection
};

// Camera, defines position/orientation in 3d space
struct Camera3D {
	Vector3 position;       // Camera position
	Vector3 target;         // Camera target it looks-at
	Vector3 up;             // Camera up vector (rotation over its axis)
	float fovy;             // Camera field-of-view aperture in Y (degrees) in perspective, used as near plane width in orthographic
	int projection;         // Camera projection: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
};

// Image, pixel data stored in CPU memory (RAM)
struct RLImage {
	void* data;             // Image raw data
	int width;              // Image base width
	int height;             // Image base height
	int mipmaps;            // Mipmap levels, 1 by default
	int format;             // Data format (PixelFormat type)
};

// Texture, tex data stored in GPU memory (VRAM)
struct RLTexture {
	unsigned int id;        // OpenGL texture id
	int width;              // Texture base width
	int height;             // Texture base height
	int mipmaps;            // Mipmap levels, 1 by default
	int format;             // Data format (PixelFormat type)
};

// Texture2D, same as Texture
typedef RLTexture RLTexture2D;

// GlyphInfo, font characters glyphs info
struct GlyphInfo {
	int value;              // Character value (Unicode)
	int offsetX;            // Character offset X when drawing
	int offsetY;            // Character offset Y when drawing
	int advanceX;           // Character advance position X
	RLImage image;            // Character image data
};

// Font, font texture and GlyphInfo array data
struct Font {
	int baseSize;           // Base size (default chars height)
	int glyphCount;         // Number of glyph characters
	int glyphPadding;       // Padding around the glyph characters
	RLTexture2D texture;      // Texture atlas containing the glyphs
	Rectangle* recs;        // Rectangles in texture for the glyphs
	GlyphInfo* glyphs;      // Glyphs info data
};

// Mesh, vertex data and vao/vbo
struct RLMesh {
	int vertexCount;        // Number of vertices stored in arrays
	int triangleCount;      // Number of triangles stored (indexed or not)

	// Vertex attributes data
	float* vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
	float* texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
	float* texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
	float* normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
	float* tangents;        // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
	unsigned char* colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
	unsigned short* indices;    // Vertex indices (in case vertex data comes indexed)

	// Animation vertex data
	float* animVertices;    // Animated vertex positions (after bones transformations)
	float* animNormals;     // Animated normals (after bones transformations)
	unsigned char* boneIds; // Vertex bone ids, max 255 bone ids, up to 4 bones influence by vertex (skinning)
	float* boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning)

	// OpenGL identifiers
	unsigned int vaoId;     // OpenGL Vertex Array Object id
	unsigned int* vboId;    // OpenGL Vertex Buffer Objects id (default vertex data)
};

// Shader
typedef struct RLShader {
	unsigned int id;        // Shader program id
	int* locs;              // Shader locations array (RL_MAX_SHADER_LOCATIONS)
};

// MaterialMap
struct RLMaterialMap {
	RLTexture2D texture;      // Material map texture
	RLColor color;            // Material map color
	float value;            // Material map value
};

// Material, includes shader and maps
struct RLMaterial {
	RLShader shader;          // Material shader
	RLMaterialMap* maps;      // Material maps array (MAX_MATERIAL_MAPS)
	float params[4];        // Material generic parameters (if required)
};

// Transform, vertex transformation data
struct RLTransform {
	Vector3 translation;    // Translation
	Quaternion rotation;    // Rotation
	Vector3 scale;          // Scale
};

// Bone, skeletal animation bone
struct RLBoneInfo {
	char name[32];          // Bone name
	int parent;             // Bone parent
};

// Model, meshes, materials and animation data
struct RLModel {
	Matrix transform;       // Local transform matrix

	int meshCount;          // Number of meshes
	int materialCount;      // Number of materials
	RLMesh* meshes;           // Meshes array
	RLMaterial* materials;    // Materials array
	int* meshMaterial;      // Mesh material number

	// Animation data
	int boneCount;          // Number of bones
	RLBoneInfo* bones;        // Bones information (skeleton)
	RLTransform* bindPose;    // Bones base transformation (pose)
};


// Default shader vertex attribute names to set location points
// NOTE: When a new shader is loaded, the following locations are tried to be set for convenience
#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"    // Bound by default to shader location: 0
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"    // Bound by default to shader location: 1
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"      // Bound by default to shader location: 2
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"       // Bound by default to shader location: 3
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"     // Bound by default to shader location: 4
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"   // Bound by default to shader location: 5

#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               // model-view-projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           // view matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     // projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          // model matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         // normal matrix (transpose(inverse(matModelView))
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          // texture0 (texture slot active 0)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          // texture1 (texture slot active 1)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          // texture2 (texture slot active 2)

#define RLLIGHTGRAY  { 200, 200, 200, 255 }   // Light Gray
#define RLGRAY       { 130, 130, 130, 255 }   // Gray
#define RLDARKGRAY   { 80, 80, 80, 255 }      // Dark Gray
#define RLYELLOW     { 253, 249, 0, 255 }     // Yellow
#define RLGOLD       { 255, 203, 0, 255 }     // Gold
#define RLORANGE     { 255, 161, 0, 255 }     // Orange
#define RLPINK       { 255, 109, 194, 255 }   // Pink
#define RLRED        { 230, 41, 55, 255 }     // Red
#define RLMAROON     { 190, 33, 55, 255 }     // Maroon
#define RLGREEN      { 0, 228, 48, 255 }      // Green
#define RLLIME       { 0, 158, 47, 255 }      // Lime
#define RLDARKGREEN  { 0, 117, 44, 255 }      // Dark Green
#define RLSKYBLUE    { 102, 191, 255, 255 }   // Sky Blue
#define RLBLUE       { 0, 121, 241, 255 }     // Blue
#define RLDARKBLUE   { 0, 82, 172, 255 }      // Dark Blue
#define RLPURPLE     { 200, 122, 255, 255 }   // Purple
#define RLVIOLET     { 135, 60, 190, 255 }    // Violet
#define RLDARKPURPLE { 112, 31, 126, 255 }    // Dark Purple
#define RLBEIGE      { 211, 176, 131, 255 }   // Beige
#define RLBROWN      { 127, 106, 79, 255 }    // Brown
#define RLDARKBROWN  { 76, 63, 47, 255 }      // Dark Brown

#define RLWHITE      { 255, 255, 255, 255 }   // White
#define RLBLACK      { 0, 0, 0, 255 }         // Black
#define RLBLANK      { 0, 0, 0, 0 }           // Blank (Transparent)
#define RLMAGENTA    { 255, 0, 255, 255 }     // Magenta
#define RLRAYWHITE   { 245, 245, 245, 255 }   // My own White (raylib logo)

// Trace log level
// NOTE: Organized by priority level
enum rlTraceLogLevel {
	RL_LOG_ALL = 0,             // Display all logs
	RL_LOG_TRACE,               // Trace logging, intended for internal use only
	RL_LOG_DEBUG,               // Debug logging, used for internal debugging, it should be disabled on release builds
	RL_LOG_INFO,                // Info logging, used for program execution info
	RL_LOG_WARNING,             // Warning logging, used on recoverable failures
	RL_LOG_ERROR,               // Error logging, used on unrecoverable failures
	RL_LOG_FATAL,               // Fatal logging, used to abort program: exit(EXIT_FAILURE)
	RL_LOG_NONE                 // Disable logging
};

// Shader location point type
enum rlShaderLocationIndex {
	RL_SHADER_LOC_VERTEX_POSITION = 0,  // Shader location: vertex attribute: position
	RL_SHADER_LOC_VERTEX_TEXCOORD01,    // Shader location: vertex attribute: texcoord01
	RL_SHADER_LOC_VERTEX_TEXCOORD02,    // Shader location: vertex attribute: texcoord02
	RL_SHADER_LOC_VERTEX_NORMAL,        // Shader location: vertex attribute: normal
	RL_SHADER_LOC_VERTEX_TANGENT,       // Shader location: vertex attribute: tangent
	RL_SHADER_LOC_VERTEX_COLOR,         // Shader location: vertex attribute: color
	RL_SHADER_LOC_MATRIX_MVP,           // Shader location: matrix uniform: model-view-projection
	RL_SHADER_LOC_MATRIX_VIEW,          // Shader location: matrix uniform: view (camera transform)
	RL_SHADER_LOC_MATRIX_PROJECTION,    // Shader location: matrix uniform: projection
	RL_SHADER_LOC_MATRIX_MODEL,         // Shader location: matrix uniform: model (transform)
	RL_SHADER_LOC_MATRIX_NORMAL,        // Shader location: matrix uniform: normal
	RL_SHADER_LOC_VECTOR_VIEW,          // Shader location: vector uniform: view
	RL_SHADER_LOC_COLOR_DIFFUSE,        // Shader location: vector uniform: diffuse color
	RL_SHADER_LOC_COLOR_SPECULAR,       // Shader location: vector uniform: specular color
	RL_SHADER_LOC_COLOR_AMBIENT,        // Shader location: vector uniform: ambient color
	RL_SHADER_LOC_MAP_ALBEDO,           // Shader location: sampler2d texture: albedo (same as: RL_SHADER_LOC_MAP_DIFFUSE)
	RL_SHADER_LOC_MAP_METALNESS,        // Shader location: sampler2d texture: metalness (same as: RL_SHADER_LOC_MAP_SPECULAR)
	RL_SHADER_LOC_MAP_NORMAL,           // Shader location: sampler2d texture: normal
	RL_SHADER_LOC_MAP_ROUGHNESS,        // Shader location: sampler2d texture: roughness
	RL_SHADER_LOC_MAP_OCCLUSION,        // Shader location: sampler2d texture: occlusion
	RL_SHADER_LOC_MAP_EMISSION,         // Shader location: sampler2d texture: emission
	RL_SHADER_LOC_MAP_HEIGHT,           // Shader location: sampler2d texture: height
	RL_SHADER_LOC_MAP_CUBEMAP,          // Shader location: samplerCube texture: cubemap
	RL_SHADER_LOC_MAP_IRRADIANCE,       // Shader location: samplerCube texture: irradiance
	RL_SHADER_LOC_MAP_PREFILTER,        // Shader location: samplerCube texture: prefilter
	RL_SHADER_LOC_MAP_BRDF              // Shader location: sampler2d texture: brdf
};
#define RL_SHADER_LOC_MAP_DIFFUSE       RL_SHADER_LOC_MAP_ALBEDO
#define RL_SHADER_LOC_MAP_SPECULAR      RL_SHADER_LOC_MAP_METALNESS

// Texture pixel formats
// NOTE: Support depends on OpenGL version
enum rlPixelFormat {
	RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
	RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,        // 8*2 bpp (2 channels)
	RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5,            // 16 bpp
	RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8,            // 24 bpp
	RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
	RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
	RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,          // 32 bpp
	RL_PIXELFORMAT_UNCOMPRESSED_R32,               // 32 bpp (1 channel - float)
	RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32,         // 32*3 bpp (3 channels - float)
	RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,      // 32*4 bpp (4 channels - float)
	RL_PIXELFORMAT_UNCOMPRESSED_R16,               // 16 bpp (1 channel - half float)
	RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16,         // 16*3 bpp (3 channels - half float)
	RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,      // 16*4 bpp (4 channels - half float)
	RL_PIXELFORMAT_COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
	RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
	RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA,           // 8 bpp
	RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA,           // 8 bpp
	RL_PIXELFORMAT_COMPRESSED_ETC1_RGB,            // 4 bpp
	RL_PIXELFORMAT_COMPRESSED_ETC2_RGB,            // 4 bpp
	RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
	RL_PIXELFORMAT_COMPRESSED_PVRT_RGB,            // 4 bpp
	RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA,           // 4 bpp
	RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
	RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
};

// Pixel formats
// NOTE: Support depends on OpenGL version and platform
enum PixelFormat {
	PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1, // 8 bit per pixel (no alpha)
	PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,    // 8*2 bpp (2 channels)
	PIXELFORMAT_UNCOMPRESSED_R5G6B5,        // 16 bpp
	PIXELFORMAT_UNCOMPRESSED_R8G8B8,        // 24 bpp
	PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,      // 16 bpp (1 bit alpha)
	PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,      // 16 bpp (4 bit alpha)
	PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,      // 32 bpp
	PIXELFORMAT_UNCOMPRESSED_R32,           // 32 bpp (1 channel - float)
	PIXELFORMAT_UNCOMPRESSED_R32G32B32,     // 32*3 bpp (3 channels - float)
	PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,  // 32*4 bpp (4 channels - float)
	PIXELFORMAT_UNCOMPRESSED_R16,           // 16 bpp (1 channel - half float)
	PIXELFORMAT_UNCOMPRESSED_R16G16B16,     // 16*3 bpp (3 channels - half float)
	PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,  // 16*4 bpp (4 channels - half float)
	PIXELFORMAT_COMPRESSED_DXT1_RGB,        // 4 bpp (no alpha)
	PIXELFORMAT_COMPRESSED_DXT1_RGBA,       // 4 bpp (1 bit alpha)
	PIXELFORMAT_COMPRESSED_DXT3_RGBA,       // 8 bpp
	PIXELFORMAT_COMPRESSED_DXT5_RGBA,       // 8 bpp
	PIXELFORMAT_COMPRESSED_ETC1_RGB,        // 4 bpp
	PIXELFORMAT_COMPRESSED_ETC2_RGB,        // 4 bpp
	PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,   // 8 bpp
	PIXELFORMAT_COMPRESSED_PVRT_RGB,        // 4 bpp
	PIXELFORMAT_COMPRESSED_PVRT_RGBA,       // 4 bpp
	PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,   // 8 bpp
	PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA    // 2 bpp
} ;

// Texture parameters: filter mode
// NOTE 1: Filtering considers mipmaps if available in the texture
// NOTE 2: Filter is accordingly set for minification and magnification
enum rlTextureFilter {
	RL_TEXTURE_FILTER_POINT = 0,        // No filter, just pixel approximation
	RL_TEXTURE_FILTER_BILINEAR,         // Linear filtering
	RL_TEXTURE_FILTER_TRILINEAR,        // Trilinear filtering (linear with mipmaps)
	RL_TEXTURE_FILTER_ANISOTROPIC_4X,   // Anisotropic filtering 4x
	RL_TEXTURE_FILTER_ANISOTROPIC_8X,   // Anisotropic filtering 8x
	RL_TEXTURE_FILTER_ANISOTROPIC_16X,  // Anisotropic filtering 16x
};


// GL equivalent data types
#define RL_UNSIGNED_BYTE                        0x1401      // GL_UNSIGNED_BYTE
#define RL_FLOAT                                0x1406      // GL_FLOAT

// Shader attribute data types
enum rlShaderAttributeDataType {
	RL_SHADER_ATTRIB_FLOAT = 0,         // Shader attribute type: float
	RL_SHADER_ATTRIB_VEC2,              // Shader attribute type: vec2 (2 float)
	RL_SHADER_ATTRIB_VEC3,              // Shader attribute type: vec3 (3 float)
	RL_SHADER_ATTRIB_VEC4               // Shader attribute type: vec4 (4 float)
} ;

// Shader attribute data types
enum ShaderAttributeDataType {
	SHADER_ATTRIB_FLOAT = 0,        // Shader attribute type: float
	SHADER_ATTRIB_VEC2,             // Shader attribute type: vec2 (2 float)
	SHADER_ATTRIB_VEC3,             // Shader attribute type: vec3 (3 float)
	SHADER_ATTRIB_VEC4              // Shader attribute type: vec4 (4 float)
} ;

// Shader location index
enum ShaderLocationIndex {
	SHADER_LOC_VERTEX_POSITION = 0, // Shader location: vertex attribute: position
	SHADER_LOC_VERTEX_TEXCOORD01,   // Shader location: vertex attribute: texcoord01
	SHADER_LOC_VERTEX_TEXCOORD02,   // Shader location: vertex attribute: texcoord02
	SHADER_LOC_VERTEX_NORMAL,       // Shader location: vertex attribute: normal
	SHADER_LOC_VERTEX_TANGENT,      // Shader location: vertex attribute: tangent
	SHADER_LOC_VERTEX_COLOR,        // Shader location: vertex attribute: color
	SHADER_LOC_MATRIX_MVP,          // Shader location: matrix uniform: model-view-projection
	SHADER_LOC_MATRIX_VIEW,         // Shader location: matrix uniform: view (camera transform)
	SHADER_LOC_MATRIX_PROJECTION,   // Shader location: matrix uniform: projection
	SHADER_LOC_MATRIX_MODEL,        // Shader location: matrix uniform: model (transform)
	SHADER_LOC_MATRIX_NORMAL,       // Shader location: matrix uniform: normal
	SHADER_LOC_VECTOR_VIEW,         // Shader location: vector uniform: view
	SHADER_LOC_COLOR_DIFFUSE,       // Shader location: vector uniform: diffuse color
	SHADER_LOC_COLOR_SPECULAR,      // Shader location: vector uniform: specular color
	SHADER_LOC_COLOR_AMBIENT,       // Shader location: vector uniform: ambient color
	SHADER_LOC_MAP_ALBEDO,          // Shader location: sampler2d texture: albedo (same as: SHADER_LOC_MAP_DIFFUSE)
	SHADER_LOC_MAP_METALNESS,       // Shader location: sampler2d texture: metalness (same as: SHADER_LOC_MAP_SPECULAR)
	SHADER_LOC_MAP_NORMAL,          // Shader location: sampler2d texture: normal
	SHADER_LOC_MAP_ROUGHNESS,       // Shader location: sampler2d texture: roughness
	SHADER_LOC_MAP_OCCLUSION,       // Shader location: sampler2d texture: occlusion
	SHADER_LOC_MAP_EMISSION,        // Shader location: sampler2d texture: emission
	SHADER_LOC_MAP_HEIGHT,          // Shader location: sampler2d texture: height
	SHADER_LOC_MAP_CUBEMAP,         // Shader location: samplerCube texture: cubemap
	SHADER_LOC_MAP_IRRADIANCE,      // Shader location: samplerCube texture: irradiance
	SHADER_LOC_MAP_PREFILTER,       // Shader location: samplerCube texture: prefilter
	SHADER_LOC_MAP_BRDF             // Shader location: sampler2d texture: brdf
};

#define SHADER_LOC_MAP_DIFFUSE      SHADER_LOC_MAP_ALBEDO
#define SHADER_LOC_MAP_SPECULAR     SHADER_LOC_MAP_METALNESS

// Shader uniform data type
enum ShaderUniformDataType {
	SHADER_UNIFORM_FLOAT = 0,       // Shader uniform type: float
	SHADER_UNIFORM_VEC2,            // Shader uniform type: vec2 (2 float)
	SHADER_UNIFORM_VEC3,            // Shader uniform type: vec3 (3 float)
	SHADER_UNIFORM_VEC4,            // Shader uniform type: vec4 (4 float)
	SHADER_UNIFORM_INT,             // Shader uniform type: int
	SHADER_UNIFORM_IVEC2,           // Shader uniform type: ivec2 (2 int)
	SHADER_UNIFORM_IVEC3,           // Shader uniform type: ivec3 (3 int)
	SHADER_UNIFORM_IVEC4,           // Shader uniform type: ivec4 (4 int)
	SHADER_UNIFORM_SAMPLER2D        // Shader uniform type: sampler2d
} ;

// Shader uniform data type
enum rlShaderUniformDataType {
	RL_SHADER_UNIFORM_FLOAT = 0,        // Shader uniform type: float
	RL_SHADER_UNIFORM_VEC2,             // Shader uniform type: vec2 (2 float)
	RL_SHADER_UNIFORM_VEC3,             // Shader uniform type: vec3 (3 float)
	RL_SHADER_UNIFORM_VEC4,             // Shader uniform type: vec4 (4 float)
	RL_SHADER_UNIFORM_INT,              // Shader uniform type: int
	RL_SHADER_UNIFORM_IVEC2,            // Shader uniform type: ivec2 (2 int)
	RL_SHADER_UNIFORM_IVEC3,            // Shader uniform type: ivec3 (3 int)
	RL_SHADER_UNIFORM_IVEC4,            // Shader uniform type: ivec4 (4 int)
	RL_SHADER_UNIFORM_SAMPLER2D         // Shader uniform type: sampler2d
} ;

// Ray, ray for raycasting
struct RLRay {
	Vector3 position;       // Ray position (origin)
	Vector3 direction;      // Ray direction
};


struct float3 {
	float v[3];
};

struct float16 {
	float v[16];
};

// RayCollision, ray hit information
struct RayCollision {
	bool hit;               // Did the ray hit something?
	float distance;         // Distance to the nearest hit
	Vector3 point;          // Point of the nearest hit
	Vector3 normal;         // Surface normal of hit
};

// RenderTexture, fbo for texture rendering
struct RenderTexture {
	unsigned int id;        // OpenGL framebuffer object id
	RLTexture texture;        // Color buffer attachment texture
	RLTexture depth;          // Depth buffer attachment texture
};

// Framebuffer attachment type
// NOTE: By default up to 8 color channels defined, but it can be more
enum rlFramebufferAttachType {
	RL_ATTACHMENT_COLOR_CHANNEL0 = 0,       // Framebuffer attachment type: color 0
	RL_ATTACHMENT_COLOR_CHANNEL1 = 1,       // Framebuffer attachment type: color 1
	RL_ATTACHMENT_COLOR_CHANNEL2 = 2,       // Framebuffer attachment type: color 2
	RL_ATTACHMENT_COLOR_CHANNEL3 = 3,       // Framebuffer attachment type: color 3
	RL_ATTACHMENT_COLOR_CHANNEL4 = 4,       // Framebuffer attachment type: color 4
	RL_ATTACHMENT_COLOR_CHANNEL5 = 5,       // Framebuffer attachment type: color 5
	RL_ATTACHMENT_COLOR_CHANNEL6 = 6,       // Framebuffer attachment type: color 6
	RL_ATTACHMENT_COLOR_CHANNEL7 = 7,       // Framebuffer attachment type: color 7
	RL_ATTACHMENT_DEPTH = 100,              // Framebuffer attachment type: depth
	RL_ATTACHMENT_STENCIL = 200,            // Framebuffer attachment type: stencil
} ;

// Framebuffer texture attachment type
 enum rlFramebufferAttachTextureType {
	RL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,   // Framebuffer texture attachment type: cubemap, +X side
	RL_ATTACHMENT_CUBEMAP_NEGATIVE_X = 1,   // Framebuffer texture attachment type: cubemap, -X side
	RL_ATTACHMENT_CUBEMAP_POSITIVE_Y = 2,   // Framebuffer texture attachment type: cubemap, +Y side
	RL_ATTACHMENT_CUBEMAP_NEGATIVE_Y = 3,   // Framebuffer texture attachment type: cubemap, -Y side
	RL_ATTACHMENT_CUBEMAP_POSITIVE_Z = 4,   // Framebuffer texture attachment type: cubemap, +Z side
	RL_ATTACHMENT_CUBEMAP_NEGATIVE_Z = 5,   // Framebuffer texture attachment type: cubemap, -Z side
	RL_ATTACHMENT_TEXTURE2D = 100,          // Framebuffer texture attachment type: texture2d
	RL_ATTACHMENT_RENDERBUFFER = 200,       // Framebuffer texture attachment type: renderbuffer
} ;

#define RLAPI


void TraceLog(int logLevel, const char* text, ...);         // Show trace log messages (LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR...)

int GetGlyphIndex(Font font, int codepoint);                                          // Get glyph index position in font for a codepoint (unicode character), fallback to '?' if not found
int GetCodepoint(const char* text, int* codepointSize);           // Get next codepoint in a UTF-8 encoded string, 0x3f('?') is returned on failure


void rlUnloadTexture(unsigned int id);                              // Unload texture from GPU memory
void UnloadTexture(RLTexture2D texture);

unsigned int rlCompileShader(const char* shaderCode, int type);
unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId);
int* rlGetShaderLocsDefault();                // Get default shader locations
unsigned int rlGetShaderIdDefault();          // Get default shader id
unsigned int rlGetTextureIdDefault();         // Get default texture id
int rlGetPixelDataSize(int width, int height, int format);   // Get pixel data size in bytes (image or texture)
unsigned int rlLoadTexture(const void* data, int width, int height, int format, int mipmapCount); // Load texture data
void rlGetGlTextureFormats(int format, unsigned int* glInternalFormat, unsigned int* glFormat, unsigned int* glType); // Get OpenGL internal formats
const char* rlGetPixelFormatName(unsigned int format);              // Get name string for pixel format
RLMaterial LoadMaterialDefault();


void UploadMesh(RLMesh* mesh, bool dynamic);

unsigned int rlLoadVertexArray();             // Load vertex array (vao) if supported
bool rlEnableVertexArray(unsigned int vaoId);     // Enable vertex array (VAO, if supported)
unsigned int rlLoadVertexBuffer(const void* buffer, int size, bool dynamic); // Load a vertex buffer object
void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, const void* pointer); // Set vertex attribute data configuration
void rlEnableVertexAttribute(unsigned int index); // Enable vertex attribute index
void rlSetVertexAttributeDefault(int locIndex, const void* value, int attribType, int count); // Set vertex attribute default value, when attribute to provided
void rlDisableVertexAttribute(unsigned int index); // Disable vertex attribute index
unsigned int rlLoadVertexBufferElement(const void* buffer, int size, bool dynamic); // Load vertex buffer elements object
void rlDisableVertexArray(void);

void UnloadModel(RLModel model);
void UnloadMesh(RLMesh mesh);

void rlUnloadVertexArray(unsigned int vaoId);     // Unload vertex array (vao)
void rlUnloadVertexBuffer(unsigned int vboId);
RLAPI RLTexture2D LoadTextureFromImage(RLImage image);
RLAPI RLShader LoadShaderFromMemory(const char* vsCode, const char* fsCode); // Load shader from code strings and bind default locations
RLAPI unsigned int rlLoadShaderCode(const char* vsCode, const char* fsCode);    // Load shader from code strings
RLAPI int rlGetLocationAttrib(unsigned int shaderId, const char* attribName);   // Get shader location attribute
RLAPI int rlGetLocationUniform(unsigned int shaderId, const char* uniformName); // Get shader location uniform
RLAPI int GetShaderLocation(RLShader shader, const char* uniformName);       // Get shader uniform location
int GetShaderLocationAttrib(RLShader shader, const char* attribName);

RLAPI RLModel LoadModelFromMesh(RLMesh mesh);

RLAPI RLMesh GenMeshSphere(float radius, int rings, int slices);                              // Generate sphere mesh (standard sphere)

RLAPI unsigned char* DecompressData(const unsigned char* compData, int compDataSize, int* dataSize);  // Decompress data (DEFLATE algorithm), memory must be MemFree()

RLAPI void UnloadImage(RLImage image);

RLAPI RLTexture2D LoadTexture(const char* fileName);
RLAPI RLImage LoadImage(const char* fileName);

RLAPI unsigned char* LoadFileData(const char* fileName, int* dataSize); // Load file data as byte array (read)
RLAPI RLImage LoadImageFromMemory(const char* fileType, const unsigned char* fileData, int dataSize);      // Load image from memory buffer, fileType refers to extension: i.e. '.png'
RLAPI const char* GetFileExtension(const char* fileName);         // Get pointer to extension for a filename string (includes dot: '.png')

RLAPI void SetMaterialTexture(RLMaterial* material, int mapType, RLTexture2D texture);          // Set texture for a material map type (MATERIAL_MAP_DIFFUSE, MATERIAL_MAP_SPECULAR...)

RLAPI void DrawMeshInstanced(RLMesh mesh, RLMaterial material, const Matrix* transforms, int instances); // Draw multiple mesh instances with material and different transforms

RLAPI void rlEnableShader(unsigned int id);             // Enable shader program

RLAPI void DrawModel(RLModel model, Vector3 position, float scale, RLColor tint);               // Draw a model (with texture if set)
RLAPI void DrawModelEx(RLModel model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, RLColor tint); // Draw a model with extended parameters
RLAPI void DrawMesh(RLMesh mesh, RLMaterial material, Matrix transform);                        // Draw a 3d mesh with material and transform

RLAPI void rlSetUniform(int locIndex, const void* value, int uniformType, int count); // Set shader value uniform

RLAPI Matrix rlGetMatrixModelview(void);                                  // Get internal modelview matrix
RLAPI Matrix rlGetMatrixProjection(void);                                 // Get internal projection matrix

RLAPI void rlSetUniformMatrix(int locIndex, Matrix mat);                        // Set shader value matrix
RLAPI void rlSetVertexAttributeDivisor(unsigned int index, int divisor); // Set vertex attribute data divisor
RLAPI void rlDisableVertexBuffer(void);                 // Disable vertex buffer (VBO)
RLAPI Matrix rlGetMatrixTransform(void);

RLAPI void rlActiveTextureSlot(int slot);               // Select and active a texture slot
RLAPI void rlEnableTextureCubemap(unsigned int id);     // Enable texture cubemap
RLAPI void rlEnableTexture(unsigned int id);            // Enable texture
RLAPI void rlEnableVertexBuffer(unsigned int id);       // Enable vertex buffer (VBO)
RLAPI void rlEnableVertexBufferElement(unsigned int id); // Enable vertex buffer element (VBO element)

RLAPI void rlViewport(int x, int y, int width, int height); // Set the viewport area
int rlGetFramebufferWidth(void);
int rlGetFramebufferHeight(void);
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye);
RLAPI Matrix rlGetMatrixProjectionStereo(int eye);

RLAPI void rlDrawVertexArrayElementsInstanced(int offset, int count, const void* buffer, int instances); // Draw vertex array elements with instancing
void rlDrawVertexArrayInstanced(int offset, int count, int instances);
RLAPI void rlDisableTextureCubemap(void);               // Disable texture cubemap
RLAPI void rlDisableTexture(void);                      // Disable texture
void rlDisableVertexBufferElement(void);
RLAPI void rlDisableShader(void);                       // Disable shader program
void rlDrawVertexArrayElements(int offset, int count, const void* buffer);
RLAPI void rlDrawVertexArray(int offset, int count);    // Draw vertex array (currently active vao)
RLAPI void rlSetMatrixProjection(Matrix proj);                            // Set a custom projection matrix (replaces internal projection matrix)
RLAPI void rlSetMatrixModelview(Matrix view);                             // Set a custom modelview matrix (replaces internal modelview matrix)

RLAPI int GetRandomValue(int min, int max);                       // Get a random value between min and max (both included)

RLAPI RLRay GetMouseRay(Vector2 mousePosition, Camera3D camera);      // Get a ray trace from mouse position

RLAPI RayCollision GetRayCollisionTriangle(RLRay ray, Vector3 p1, Vector3 p2, Vector3 p3);            // Get collision info between ray and triangle
RLAPI RayCollision GetRayCollisionQuad(RLRay ray, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4);    // Get collision info between ray and quad

RLAPI void ImageResize(RLImage* image, int newWidth, int newHeight);                                       // Resize image (Bicubic scaling algorithm)
RLAPI int GetPixelDataSize(int width, int height, int format);              // Get pixel data size in bytes for certain format
RLAPI RLColor* LoadImageColors(RLImage image);                                                               // Load color data from image as a Color array (RGBA - 32bit)
RLAPI void UnloadImageColors(RLColor* colors);                                                             // Unload color data loaded with LoadImageColors()
RLAPI void RLImageFormat(RLImage* image, int newFormat);                                                     // Convert image data to desired format

RLAPI const char* TextToLower(const char* text);                      // Get lower case version of provided string
RLAPI bool TextIsEqual(const char* text1, const char* text2);                               // Check if two text string are equal

RLAPI void UnloadRenderTexture(RenderTexture target);

RLAPI void rlUnloadFramebuffer(unsigned int id);                          // Delete framebuffer from GPU

RLAPI bool IsTextureReady(RLTexture2D texture);                                                            // Check if a texture is ready

RLAPI RenderTexture LoadRenderTexture(int width, int height);                                          // Load texture for rendering (framebuffer)

RLAPI unsigned int rlLoadFramebuffer(int width, int height);              // Load an empty framebuffer
RLAPI void rlEnableFramebuffer(unsigned int id);        // Enable render texture (fbo)
RLAPI unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer); // Load depth texture/renderbuffer (to be attached to fbo)
RLAPI void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel); // Attach texture/renderbuffer to a framebuffer
RLAPI bool rlFramebufferComplete(unsigned int id);                        // Verify framebuffer is complete
RLAPI void rlDisableFramebuffer(void);                  // Disable render texture (fbo), return to default framebuffer

RLAPI RLModel RLLoadModel(const char* fileName);                                                // Load model from files (meshes and materials)

RLAPI bool IsFileExtension(const char* fileName, const char* ext); // Check file extension (including point: .png, .wav)
RLAPI const char** TextSplit(const char* text, char delimiter, int* count);                 // Split text into multiple strings

RLAPI char* LoadFileText(const char* fileName);                   // Load text data from file (read), returns a '\0' terminated string

const char* GetWorkingDirectory(void);

RLAPI const char* GetDirectoryPath(const char* filePath);         // Get full path for a given fileName with path (uses static string)
RLAPI void UnloadFileText(char* text);                            // Unload file text data allocated by LoadFileText()

RLAPI char* TextReplace(char* text, const char* replace, const char* by);                   // Replace text string (WARNING: memory must be freed!)
RLAPI unsigned int TextLength(const char* text);                                            // Get text length, checks for '\0' ending