#pragma once

#include "Grid.h"

class MapMan;

typedef int TexID;
typedef int ModelID;

#define TILE_SPACING_DEFAULT 2.0f
#define NO_TEX -1
#define NO_MODEL -1

struct Tile
{
	inline Tile() : shape(NO_MODEL), angle(0), texture(NO_TEX), pitch(0) {}
	inline Tile(ModelID s, int a, TexID t, int p) : shape(s), angle(a), texture(t), pitch(p) {}

	inline operator bool() const
	{
		return shape > NO_MODEL && texture > NO_TEX;
	}

	ModelID shape;
	int angle; //Yaw in whole number of degrees
	TexID texture;
	int pitch; //Pitch in whole number of degrees

	// bool flipped; //True if flipped vertically
	// char padding[3]; //This is here to ensure that the byte layout is consistent across compilers.
};

inline bool operator==(const Tile& lhs, const Tile& rhs)
{
	return (lhs.shape == rhs.shape) && (lhs.texture == rhs.texture) && (lhs.angle == rhs.angle) && (lhs.pitch == rhs.pitch);
}

inline bool operator!=(const Tile& lhs, const Tile& rhs)
{
	return !(lhs == rhs);
}

inline glm::mat4 TileRotationMatrix(const Tile& tile)
{
	glm::mat4 rotx = glm::rotate(glm::mat4(1.0f), glm::radians(float(-tile.pitch)), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 roty = glm::rotate(glm::mat4(1.0f), glm::radians(float(-tile.angle)), glm::vec3(0.0f, 1.0f, 0.0f));
	return rotx * roty;
}

class TileGrid : public Grid<Tile>
{
public:
	//Constructs a blank TileGrid with no size
	TileGrid();
	//Constructs a TileGrid full of empty tiles.
	TileGrid(MapMan* mapMan, size_t width, size_t height, size_t length);
	//Constructs a TileGrid filled with the given tile.
	TileGrid(MapMan* mapMan, size_t width, size_t height, size_t length, float spacing, Tile fill);
	~TileGrid();

	Tile GetTile(int i, int j, int k) const;
	Tile GetTile(int flatIndex) const;
	void SetTile(int i, int j, int k, const Tile& tile);
	void SetTile(int flatIndex, const Tile& tile);

	//Sets a range of tiles in the grid inside of the rectangular prism with a corner at (i, j, k) and size (w, h, l).
	void SetTileRect(int i, int j, int k, int w, int h, int l, const Tile& tile);

	//Takes the tiles of `src` and places them in this grid starting at the offset at (i, j, k)
	//If the offset results in `src` exceeding the current grid's boundaries, it is cut off.
	//If `ignoreEmpty` is true, then empty tiles do not overwrite existing tiles.
	void CopyTiles(int i, int j, int k, const TileGrid& src, bool ignoreEmpty = false);


	void UnsetTile(int i, int j, int k);

	//Returns a smaller TileGrid with a copy of the tile data in the rectangle defined by coordinates (i, j, k) and size (w, h, l).
	TileGrid Subsection(int i, int j, int k, int w, int h, int l) const;

	//Draws the tile grid, hiding all layers that are outside of the given y coordinate range.
	void Draw(const glm::vec3& position, int fromY, int toY);
	void Draw(const glm::vec3& position);

	//Returns a base64 encoded string with the binary representations of all tiles.
	std::string GetTileDataBase64() const;

	std::string GetOptimizedTileDataBase64() const;

	//Assigns tiles based on the binary data encoded in base 64. Assumes that the sizes of the data and the current grid are the same.
	void SetTileDataBase64(std::string data);

	//Returns the list of texture and model IDs that are actually used in this tile grid
	std::pair<std::vector<TexID>, std::vector<ModelID>> GetUsedIDs() const;

	const StaticModelRef GetModel();
protected:
	MapMan* _mapMan;

	// Calculates lists of transformations for each tile, separated by texture and shape, to be drawn as instances.
	void _RegenBatches(const glm::vec3& position, int fromY, int toY);
	// Combines all of the tiles into a single model, for export or for preview. When culling is true, redundant faces between tiles are removed.
	StaticModelRef _GenerateModel(bool culling = true);

	std::map<std::pair<TexID, StaticMesh*>, std::vector<glm::mat4>> _drawBatches;

	//const glm::vec3& _batchPosition;
	glm::vec3 _batchPosition;

	bool _regenBatches;
	bool _regenModel;
	int _batchFromY;
	int _batchToY;

	StaticModel* _model;
	bool _modelCulled;
};