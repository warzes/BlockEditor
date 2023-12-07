#pragma once

#include "Core.h"

// Represents a 3 dimensional array of tiles and provides functions for converting coordinates.
template<class Cel>
class Grid
{
public:
	// Constructs a grid filled with the given cel.
	Grid(size_t width, size_t height, size_t length, float spacing, const Cel& fill)
	{
		m_width = width; m_height = height; m_length = length; m_spacing = spacing;
		m_grid.resize(width * height * length);

		for (size_t i = 0; i < m_grid.size(); ++i) { m_grid[i] = fill; }
	}
	// Constructs a grid full of default-constructed cels.
	Grid(size_t width, size_t height, size_t length, float spacing) : Grid(width, height, length, spacing, Cel()) {}
	virtual ~Grid() = default;

	Vector3 WorldToGridPos(Vector3 worldPos) const
	{
		return { floorf(worldPos.x / m_spacing), floorf(worldPos.y / m_spacing) , floorf(worldPos.z / m_spacing) };
	}

	// Converts (whole number) grid cel coordinates to world coordinates.
	// If `center` is true, then the world coordinate will be in the center of the cel instead of the corner.
	Vector3 GridToWorldPos(Vector3 gridPos, bool center) const
	{
		if (center)
		{
			return {
				(gridPos.x * m_spacing) + (m_spacing / 2.0f),
				(gridPos.y * m_spacing) + (m_spacing / 2.0f),
				(gridPos.z * m_spacing) + (m_spacing / 2.0f),
			};
		}
		else
		{
			return { gridPos.x * m_spacing, gridPos.y * m_spacing, gridPos.z * m_spacing };
		}
	}

	Vector3 SnapToCelCenter(Vector3 worldPos) const
	{
		worldPos.x = (floorf(worldPos.x / m_spacing) * m_spacing) + (m_spacing / 2.0f);
		worldPos.y = (floorf(worldPos.y / m_spacing) * m_spacing) + (m_spacing / 2.0f);
		worldPos.z = (floorf(worldPos.z / m_spacing) * m_spacing) + (m_spacing / 2.0f);
		return worldPos;
	}

	size_t FlatIndex(int i, int j, int k) const
	{
		return i + (k * m_width) + (j * m_width * m_length);
	}

	Vector3 UnflattenIndex(size_t idx) const
	{
		return {
			(float)(idx % m_width),
			(float)(idx / (m_width * m_length)),
			(float)((idx / m_width) % m_length)
		};
	}

	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }
	size_t GetLength() const { return m_length; }
	float GetSpacing() const { return m_spacing; }

	Vector3 GetMinCorner() const
	{
		return Vector3Zero();
	}

	Vector3 GetMaxCorner() const
	{
		return { (float)m_width * m_spacing, (float)m_height * m_spacing, (float)m_length * m_spacing };
	}

	Vector3 GetCenterPos() const
	{
		return { (float)m_width * m_spacing / 2.0f, (float)m_height * m_spacing / 2.0f, (float)m_length * m_spacing / 2.0f };
	}

protected:
	void setCel(int i, int j, int k, const Cel& cel)
	{
		m_grid[FlatIndex(i, j, k)] = cel;
	}

	Cel getCel(int i, int j, int k) const
	{
		return m_grid[FlatIndex(i, j, k)];
	}

	void copyCels(int i, int j, int k, const Grid<Cel>& src)
	{
		assert(i >= 0 && j >= 0 && k >= 0);
		int xEnd = Min(i + src.m_width, m_width);
		int yEnd = Min(j + src.m_height, m_height);
		int zEnd = Min(k + src.m_length, m_length);
		for (int z = k; z < zEnd; ++z)
		{
			for (int y = j; y < yEnd; ++y)
			{
				size_t ourBase = FlatIndex(0, y, z);
				size_t theirBase = src.FlatIndex(0, y - j, z - k);
				for (int x = i; x < xEnd; ++x)
				{
					const Cel& cel = src.m_grid[theirBase + (x - i)];
					m_grid[ourBase + x] = cel;
				}
			}
		}
	}

	void subsectionCopy(int i, int j, int k, int w, int h, int l, Grid<Cel>& out) const
	{
		for (int z = k; z < k + l; ++z)
		{
			for (int y = j; y < j + h; ++y)
			{
				size_t ourBase = FlatIndex(0, y, z);
				size_t theirBase = out.FlatIndex(0, y - j, z - k);
				for (int x = i; x < i + w; ++x)
				{
					out.m_grid[theirBase + (x - i)] = m_grid[ourBase + x];
				}
			}
		}
	}

	std::vector<Cel> m_grid;
	size_t m_width, m_height, m_length;
	float m_spacing;
};