#pragma once

#include "ModeImpl.h"
#include "Assets.h"

#define SEARCH_BUFFER_SIZE 256

class PickMode : public ModeImpl
{
public:
    //Represents a selectable frame in the list or grid of the picker
    struct Frame
    {
        std::filesystem::path        filePath;
        std::string     label;
        RLTexture         texture;

        Frame();
        Frame(const std::filesystem::path filePath, const std::filesystem::path rootDir);
    };

    enum class Mode { TEXTURES, SHAPES };

    PickMode(Mode mode);
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;

    inline Mode GetMode() const { return _mode; }

    std::shared_ptr<Assets::TexHandle> GetPickedTexture() const;
    std::shared_ptr<Assets::ModelHandle> GetPickedShape() const;

protected:
    //Retrieves files, recursively, and generates frames for each.
    void _GetFrames();

    //Load or retrieve cached texture
    RLTexture2D       _GetTexture(const std::filesystem::path path);
    //Load or retrieve cached model
    RLModel           _GetModel(const std::filesystem::path path);
    //Load or retrieve cached render texture
    RenderTexture _GetIcon(const std::filesystem::path path);

    std::map<std::filesystem::path, RLTexture2D> _loadedTextures;
    std::map<std::filesystem::path, RLModel> _loadedModels;
    std::map<std::filesystem::path, RenderTexture> _loadedIcons;
    std::set<std::filesystem::path> _foundFiles;
    std::vector<Frame> _frames;

    Frame _selectedFrame;
    Camera3D _iconCamera; //Camera3D for rendering 3D shape preview icons

    char _searchFilterBuffer[SEARCH_BUFFER_SIZE];
    char _searchFilterPrevious[SEARCH_BUFFER_SIZE];

    Mode _mode;
    std::filesystem::path _rootDir;
};
