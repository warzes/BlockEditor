#pragma once

#include "Grid.h"
#include "Assets.h"

#define ENT_SPACING_DEFAULT 2.0f

//This is short for "entity", because "entity" is difficult to type.
struct Ent
{
    enum class DisplayMode {
        SPHERE,
        MODEL,
        SPRITE,
    };

    bool active;

    DisplayMode display;
    Color color;
    float radius;
    Vector3 position; //World space coordinates
    int yaw, pitch; //Degrees angle orientation

    std::shared_ptr<Assets::ModelHandle> model;
    std::shared_ptr<Assets::TexHandle> texture;

    //Entity properties are key/value pairs. No data types are enforced, values are parsed as strings.
    std::map<std::string, std::string> properties;

    Ent();
    Ent(float radius);

    Matrix GetMatrix() const;

    void Draw(const bool drawAxes) const;
};

void to_json(nlohmann::json& j, const Ent& ent);
void from_json(const nlohmann::json& j, Ent& ent);

//This represents a grid of entities. 
//Instead of the grid storing entities directly, it stores iterators into the `_ents` array to save on memory.
class EntGrid : public Grid<Ent>
{
public:
    //Creates an empty entgrid of zero size
    EntGrid();

    //Creates ent grid of given dimensions, default spacing.
    EntGrid(size_t width, size_t height, size_t length);

    //Will set the given ent to occupy the grid space, replacing any existing entity in that space.
    inline void AddEnt(int i, int j, int k, Ent ent)
    {
        ent.position = GridToWorldPos(Vector3{ (float)i, (float)j, (float)k }, true);
        setCel(i, j, k, ent);
    }

    inline void RemoveEnt(int i, int j, int k)
    {
        setCel(i, j, k, Ent());
    }

    inline bool HasEnt(int i, int j, int k) const
    {
        return getCel(i, j, k).active;
    }

    inline Ent GetEnt(int i, int j, int k) const
    {
        assert(HasEnt(i, j, k));
        return getCel(i, j, k);
    }

    inline void CopyEnts(int i, int j, int k, const EntGrid& src)
    {
        return copyCels(i, j, k, src);
    }

    //Returns a smaller grid with a copy of the ent data in the rectangle defined by coordinates (i, j, k) and size (w, h, l).
    inline EntGrid Subsection(int i, int j, int k, int w, int h, int l) const
    {
        assert(i >= 0 && j >= 0 && k >= 0);
        assert(i + w <= int(m_width) && j + h <= int(m_height) && k + l <= int(m_length));

        EntGrid newGrid(w, h, l);

        subsectionCopy(i, j, k, w, h, l, newGrid);

        return newGrid;
    }

    //Returns a contiguous array of all active entities.
    inline std::vector<Ent> GetEntList() const
    {
        std::vector<Ent> out;
        for (const Ent& ent : m_grid)
        {
            if (ent.active) out.push_back(ent);
        }
        return out;
    }

    void Draw(Camera3D& camera, int fromY, int toY);
    void DrawLabels(Camera3D& camera, int fromY, int toY);
private:
    std::vector<std::pair<Vector3, std::string>> _labelsToDraw;
};