#pragma once

#include "ModeImpl.h"
#include "Tile.h"
#include "Settings.h"

class PlaceMode;
class PickMode;
class EntMode;
class MenuBar;
class MapMan;

class EditorApp final : public IApp
{
public:
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        Settings,
        texturesDir,
        shapesDir,
        undoMax,
        mouseSensitivity,
        exportSeparateGeometry,
        cullFaces,
        exportFilePath,
        defaultTexturePath,
        defaultShapePath,
        backgroundColor);

	EditorApp();
	~EditorApp();
	bool Create() final;
	void Destroy() final;

	void Render() final;
	void Update(float deltaTime) final;

    //Handles transition and data flow from one editor state to the next.
    void ChangeEditorMode(const Mode newMode);

    inline float       GetMouseSensitivity() { return _settings.mouseSensitivity; }
    inline size_t      GetUndoMax() { return _settings.undoMax; }
    inline std::string GetTexturesDir() { return _settings.texturesDir; };
    inline std::string GetShapesDir() { return _settings.shapesDir; }
    inline std::string GetDefaultTexturePath() { return _settings.defaultTexturePath; }
    inline std::string GetDefaultShapePath() { return _settings.defaultShapePath; }
    inline bool        IsCullingEnabled() { return _settings.cullFaces; }
    inline RLColor       GetBackgroundColor() { return RLColor{ _settings.backgroundColor[0], _settings.backgroundColor[1], _settings.backgroundColor[2], 255 }; }

    //Indicates if rendering should be done in "preview mode", i.e. without editor widgets being drawn.
    inline bool IsPreviewing() const { return _previewDraw; }
    inline void SetPreviewing(bool p) { _previewDraw = p; }
    inline void TogglePreviewing() { _previewDraw = !_previewDraw; }
    inline std::filesystem::path GetLastSavedPath() const { return _lastSavedPath; }

    inline bool IsQuitting() const { return _quit; }
    inline void Quit() { _quit = true; }

    void DisplayStatusMessage(std::string message, float durationSeconds, int priority);

    //General map file operations
    inline const MapMan& GetMapMan() const { return *_mapMan.get(); }
    void ResetEditorCamera();
    void NewMap(int width, int height, int length);
    void ExpandMap(Direction axis, int amount);
    void ShrinkMap();
    void TryOpenMap(std::filesystem::path path);
    void TrySaveMap(std::filesystem::path path);
    void TryExportMap(std::filesystem::path path, bool separateGeometry);

    //Serializes settings into JSON file and exports.
    void SaveSettings();
    //Deserializes settings from JSON file
    void LoadSettings();

private:
	int m_windowWidth = 0;
	int m_windowHeight = 0;

    Settings _settings;

    std::unique_ptr<MenuBar> _menuBar;
    std::unique_ptr<MapMan> _mapMan;

    std::unique_ptr<PlaceMode> _tilePlaceMode;
    std::unique_ptr<PickMode> _texPickMode;
    std::unique_ptr<PickMode> _shapePickMode;
    std::unique_ptr<EntMode> _entMode;

    ModeImpl* _editorMode;

    std::filesystem::path _lastSavedPath;
    bool _previewDraw;

    bool _quit;
};

EditorApp* GetApp();