#pragma once

#include "Tile.h"
#include "Grid.h"

class MapMan;

class TileGrid final : public Grid<Tile>
{
public:
	// Constructs a blank TileGrid with no size
	TileGrid();
	// Constructs a TileGrid full of empty tiles.
	TileGrid(MapMan* mapMan, size_t width, size_t height, size_t length);
	// Constructs a TileGrid filled with the given tile.
	TileGrid(MapMan* mapMan, size_t width, size_t height, size_t length, float spacing, Tile fill);

	Tile GetTile(int i, int j, int k) const;
	Tile GetTile(int flatIndex) const;
	void SetTile(int i, int j, int k, const Tile& tile);
	void SetTile(int flatIndex, const Tile& tile);

	// Sets a range of tiles in the grid inside of the rectangular prism with a corner at (i, j, k) and size (w, h, l).
	void SetTileRect(int i, int j, int k, int w, int h, int l, const Tile& tile);

	// Takes the tiles of `src` and places them in this grid starting at the offset at (i, j, k)
	// If the offset results in `src` exceeding the current grid's boundaries, it is cut off.
	// If `ignoreEmpty` is true, then empty tiles do not overwrite existing tiles.
	void CopyTiles(int i, int j, int k, const TileGrid& src, bool ignoreEmpty = false);

	void UnsetTile(int i, int j, int k);

	// Returns a smaller TileGrid with a copy of the tile data in the rectangle defined by coordinates (i, j, k) and size (w, h, l).
	TileGrid Subsection(int i, int j, int k, int w, int h, int l) const;

	// Draws the tile grid, hiding all layers that are outside of the given y coordinate range.
	void Draw(Vector3 position, int fromY, int toY);
	void Draw(Vector3 position);

	// Returns a base64 encoded string with the binary representations of all tiles.
	std::string GetTileDataBase64() const;

	std::string GetOptimizedTileDataBase64() const;

	// Assigns tiles based on the binary data encoded in base 64. Assumes that the sizes of the data and the current grid are the same.
	void SetTileDataBase64(std::string data);

	//Returns the list of texture and model IDs that are actually used in this tile grid
	std::pair<std::vector<TexID>, std::vector<ModelID>> GetUsedIDs() const;

	const RLModel GetModel();
protected:
	MapMan* _mapMan;

	// Calculates lists of transformations for each tile, separated by texture and shape, to be drawn as instances.
	void _RegenBatches(Vector3 position, int fromY, int toY);
	// Combines all of the tiles into a single model, for export or for preview. When culling is true, redundant faces between tiles are removed.
	RLModel* _GenerateModel(bool culling = true);

	std::map<std::pair<TexID, RLMesh*>, std::vector<Matrix>> _drawBatches;

	Vector3 _batchPosition;
	bool _regenBatches;
	bool _regenModel;
	int _batchFromY;
	int _batchToY;

	RLModel* _model;
	bool _modelCulled;
};