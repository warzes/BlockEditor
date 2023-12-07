#pragma once

#include "Assets.h"
#include "Base.h"
#include "Core.h"

typedef int TexID;
typedef int ModelID;

#define TILE_SPACING_DEFAULT 2.0f
#define NO_TEX -1
#define NO_MODEL -1

struct Tile
{
	Tile() = default;
	Tile(ModelID s, int a, TexID t, int p) : shape(s), angle(a), texture(t), pitch(p) {}

	operator bool() const
	{
		return shape > NO_MODEL && texture > NO_TEX;
	}

	ModelID shape = NO_MODEL;
	int angle = 0; // Yaw in whole number of degrees
	TexID texture = NO_TEX;
	int pitch = 0; // Pitch in whole number of degrees
};

inline Matrix TileRotationMatrix(const Tile& tile)
{
	return MatrixMultiply(MatrixRotateX(ToRadians(float(-tile.pitch))), MatrixRotYDeg(float(-tile.angle)));
}