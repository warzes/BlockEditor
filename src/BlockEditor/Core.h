#pragma once

#include "RL.h"
#include "RLMath.h"

#define TEXT_FIELD_MAX 512

// This prevents me from mixing types in the malloc statements...
#define SAFE_MALLOC(TYPE, COUNT) (TYPE*) malloc((COUNT) * sizeof(TYPE))
#define SAFE_REALLOC(TYPE, BUFFER, COUNT) (TYPE*) realloc(BUFFER, (COUNT) * sizeof(TYPE))

const Vector3 VEC3_UP = Vector3{ 0.0f, 1.0f, 0.0f };
const Vector3 VEC3_FORWARD = Vector3{ 0.0f, 0.0f, -1.0f };

//Returns the parameter that is lower in value.
inline int Min(int a, int b)
{
	return a < b ? a : b;
}

//Returns the parameter that is lower in value.
inline float Minf(float a, float b)
{
	return a < b ? a : b;
}

//Returns the parameter that is higher in value.
inline int Max(int a, int b)
{
	return a < b ? b : a;
}

//Returns the parameter that is higher in value.
inline float Maxf(float a, float b)
{
	return a < b ? b : a;
}

//Returns -1 if `a` is negative, 1 if `a` is positive, and 0 otherwise.
inline int Sign(int a) {
	if (a == 0) return 0;
	return a < 0 ? -1 : 1;
}

inline Rectangle CenteredRect(float x, float y, float w, float h)
{
	return Rectangle{ x - (w / 2.0f), y - (h / 2.0f), w, h };
}

inline float ToRadians(float degrees)
{
	return degrees * PI / 180.0f;
}

inline float ToDegrees(float radians)
{
	return (radians / PI) * 180.0f;
}

//Ensures that the added degrees stays in the range [0, 360)
inline int OffsetDegrees(int base, int add)
{
	return (base + add >= 0) ? (base + add) % 360 : (360 + (base + add));
}

inline Matrix MatrixRotYDeg(float degrees)
{
	return MatrixRotateY(ToRadians(degrees));
}

//Modified version of Raylib's GetWorldToScreen(), but returns the NDC coordinates so that the program can tell what's behind the camera.
inline Vector3 GetWorldToNDC(Vector3 position, Camera3D camera)
{
	int width = GetWindowWidth();
	int height = GetWindowHeight();

	// Calculate projection matrix (from perspective instead of frustum
	Matrix matProj = MatrixIdentity();

	if (camera.projection == CAMERA_PERSPECTIVE)
	{
		// Calculate projection matrix from perspective
		matProj = MatrixPerspective(camera.fovy * DEG2RAD, ((double)width / (double)height), RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
	}
	else if (camera.projection == CAMERA_ORTHOGRAPHIC)
	{
		float aspect = (float)width / (float)height;
		double top = camera.fovy / 2.0;
		double right = top * aspect;

		// Calculate projection matrix from orthographic
		matProj = MatrixOrtho(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
	}

	// Calculate view matrix from camera look at (and transpose it)
	Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

	// TODO: Why not use Vector3Transform(Vector3 v, Matrix mat)?

	// Convert world position vector to quaternion
	Quaternion worldPos = { position.x, position.y, position.z, 1.0f };

	// Transform world position to view
	worldPos = QuaternionTransform(worldPos, matView);

	// Transform result to projection (clip space position)
	worldPos = QuaternionTransform(worldPos, matProj);

	// Calculate normalized device coordinates (inverted y)
	Vector3 ndcPos = { worldPos.x / worldPos.w, -worldPos.y / worldPos.w, worldPos.z / worldPos.w };

	return ndcPos;
}

inline std::string BuildPath(std::initializer_list<std::string> components) {
	std::string output;
	for (const std::string& s : components) {
		output += s;
		if (s != *(components.end() - 1) && s[s.length() - 1] != '/') {
			output += '/';
		}
	}
	return output;
}

//Returns the approximate width, in pixels, of a string written in the given font.
//Based off of Raylib's DrawText functions
inline int GetStringWidth(Font font, float fontSize, const std::string& string)
{
	float scaleFactor = fontSize / font.baseSize;
	int maxWidth = 0;
	int lineWidth = 0;
	for (int i = 0; i < int(string.size());)
	{
		int codepointByteCount = 0;
		int codepoint = GetCodepoint(&string[i], &codepointByteCount);
		int g = GetGlyphIndex(font, codepoint);

		if (codepoint == 0x3f) codepointByteCount = 1;

		if (codepoint == '\n')
		{
			maxWidth = Max(lineWidth, maxWidth);
			lineWidth = 0;
		}
		else
		{
			if (font.glyphs[g].advanceX == 0)
				lineWidth += int(font.recs[g].width * scaleFactor);
			else
				lineWidth += int(font.glyphs[g].advanceX * scaleFactor);
		}

		i += codepointByteCount;
	}
	maxWidth = Max(lineWidth, maxWidth);
	return maxWidth;
}

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