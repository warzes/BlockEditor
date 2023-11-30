#pragma once

// This prevents me from mixing types in the malloc statements...
#define SAFE_MALLOC(TYPE, COUNT) (TYPE*) malloc((COUNT) * sizeof(TYPE))
#define SAFE_REALLOC(TYPE, BUFFER, COUNT) (TYPE*) realloc(BUFFER, (COUNT) * sizeof(TYPE))

// RenderTexture, fbo for texture rendering
struct RenderTexture
{
	unsigned int id;        // OpenGL framebuffer object id
	Texture2DRef texture;   // Color buffer attachment texture
	Texture2DRef depth;     // Depth buffer attachment texture
};

// RenderTexture2D, same as RenderTexture
typedef RenderTexture RenderTexture2D;

// Rectangle, 4 components
struct Rectangle {
	float x;                // Rectangle top-left corner position x
	float y;                // Rectangle top-left corner position y
	float width;            // Rectangle width
	float height;           // Rectangle height
};

// GlyphInfo, font characters glyphs info
struct GlyphInfo {
	int value;              // Character value (Unicode)
	int offsetX;            // Character offset X when drawing
	int offsetY;            // Character offset Y when drawing
	int advanceX;           // Character advance position X
	Image image;            // Character image data
};

// Font, font texture and GlyphInfo array data
struct Font {
	int baseSize;           // Base size (default chars height)
	int glyphCount;         // Number of glyph characters
	int glyphPadding;       // Padding around the glyph characters
	Texture2DRef texture;      // Texture atlas containing the glyphs
	Rectangle* recs;        // Rectangles in texture for the glyphs
	GlyphInfo* glyphs;      // Glyphs info data
};

void UploadMesh(NewMesh& mesh, bool dynamic);
void UnloadModel(NewModel& model);

// Returns sections of the input string that are located between occurances of the delimeter.
inline std::vector<std::string> SplitString(const std::string& input, const std::string delimeter)
{
	std::vector<std::string> result;
	size_t tokenStart = 0, tokenEnd = 0;
	while ((tokenEnd = input.find(delimeter, tokenStart)) != std::string::npos)
	{
		result.push_back(input.substr(tokenStart, tokenEnd - tokenStart));
		tokenStart = tokenEnd + delimeter.length();
	}
	// Add the remaining text after the last delimeter
	if (tokenStart < input.length())
		result.push_back(input.substr(tokenStart));
	return result;
}