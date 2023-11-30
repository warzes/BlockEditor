#pragma once

// Represents a 3 dimensional array of tiles and provides functions for converting coordinates.
template<class Cel>
class Grid
{
public:
	//Constructs a grid filled with the given cel.
	inline Grid(size_t width, size_t height, size_t length, float spacing, const Cel& fill)
	{
		_width = width; _height = height; _length = length; _spacing = spacing;
		_grid.resize(width * height * length);

		for (size_t i = 0; i < _grid.size(); ++i) { _grid[i] = fill; }
	}

	//Constructs a grid full of default-constructed cels.
	inline Grid(size_t width, size_t height, size_t length, float spacing)
		: Grid(width, height, length, spacing, Cel())
	{
	}

	//Constructs a blank grid of zero size.
	inline Grid() : Grid(0, 0, 0, 0.0f)
	{
	}
	virtual ~Grid() {};

	inline glm::vec3 WorldToGridPos(const glm::vec3& worldPos) const
	{
		return { floorf(worldPos.x / _spacing), floorf(worldPos.y / _spacing) , floorf(worldPos.z / _spacing) };
	}

	//Converts (whole number) grid cel coordinates to world coordinates.
	//If `center` is true, then the world coordinate will be in the center of the cel instead of the corner.
	inline glm::vec3 GridToWorldPos(const glm::vec3& gridPos, bool center) const
	{
		if (center)
		{
			return {
				(gridPos.x * _spacing) + (_spacing / 2.0f),
				(gridPos.y * _spacing) + (_spacing / 2.0f),
				(gridPos.z * _spacing) + (_spacing / 2.0f),
			};
		}
		else
		{
			return { gridPos.x * _spacing, gridPos.y * _spacing, gridPos.z * _spacing };
		}
	}

	inline glm::vec3 SnapToCelCenter(glm::vec3 worldPos) const
	{
		worldPos.x = (floorf(worldPos.x / _spacing) * _spacing) + (_spacing / 2.0f);
		worldPos.y = (floorf(worldPos.y / _spacing) * _spacing) + (_spacing / 2.0f);
		worldPos.z = (floorf(worldPos.z / _spacing) * _spacing) + (_spacing / 2.0f);
		return worldPos;
	}

	inline size_t FlatIndex(int i, int j, int k) const
	{
		return i + (k * _width) + (j * _width * _length);
	}

	inline glm::vec3 UnflattenIndex(size_t idx) const
	{
		return {
			(float)(idx % _width),
			(float)(idx / (_width * _length)),
			(float)((idx / _width) % _length)
		};
	}

	inline size_t GetWidth() const { return _width; }
	inline size_t GetHeight() const { return _height; }
	inline size_t GetLength() const { return _length; }
	inline float GetSpacing() const { return _spacing; }

	inline glm::vec3 GetMinCorner() const
	{
		return glm::vec3(0.0f);
	}

	inline glm::vec3 GetMaxCorner() const
	{
		return { (float)_width * _spacing, (float)_height * _spacing, (float)_length * _spacing };
	}

	inline glm::vec3 GetCenterPos() const
	{
		return { (float)_width * _spacing / 2.0f, (float)_height * _spacing / 2.0f, (float)_length * _spacing / 2.0f };
	}

protected:
	inline void SetCel(int i, int j, int k, const Cel& cel)
	{
		_grid[FlatIndex(i, j, k)] = cel;
	}

	inline Cel GetCel(int i, int j, int k) const
	{
		return _grid[FlatIndex(i, j, k)];
	}

	inline void CopyCels(int i, int j, int k, const Grid<Cel>& src)
	{
		assert(i >= 0 && j >= 0 && k >= 0);
		int xEnd = glm::min(i + src._width, _width);
		int yEnd = glm::min(j + src._height, _height);
		int zEnd = glm::min(k + src._length, _length);
		for (int z = k; z < zEnd; ++z)
		{
			for (int y = j; y < yEnd; ++y)
			{
				size_t ourBase = FlatIndex(0, y, z);
				size_t theirBase = src.FlatIndex(0, y - j, z - k);
				for (int x = i; x < xEnd; ++x)
				{
					const Cel& cel = src._grid[theirBase + (x - i)];
					_grid[ourBase + x] = cel;
				}
			}
		}
	}

	inline void SubsectionCopy(int i, int j, int k, int w, int h, int l, Grid<Cel>& out) const
	{
		for (int z = k; z < k + l; ++z)
		{
			for (int y = j; y < j + h; ++y)
			{
				size_t ourBase = FlatIndex(0, y, z);
				size_t theirBase = out.FlatIndex(0, y - j, z - k);
				for (int x = i; x < i + w; ++x)
				{
					out._grid[theirBase + (x - i)] = _grid[ourBase + x];
				}
			}
		}
	}

	std::vector<Cel> _grid;
	size_t _width, _height, _length;
	float _spacing;
};