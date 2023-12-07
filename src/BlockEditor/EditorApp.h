#pragma once

#include "IMode.h"
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

	float GetMouseSensitivity() { return m_settings.mouseSensitivity; }
	size_t GetUndoMax() { return m_settings.undoMax; }
	std::string GetTexturesDir() { return m_settings.texturesDir; };
	std::string GetShapesDir() { return m_settings.shapesDir; }
	std::string GetDefaultTexturePath() { return m_settings.defaultTexturePath; }
	std::string GetDefaultShapePath() { return m_settings.defaultShapePath; }
	bool IsCullingEnabled() { return m_settings.cullFaces; }
	Color GetBackgroundColor() { return Color(m_settings.backgroundColor[0], m_settings.backgroundColor[1], m_settings.backgroundColor[2], (uint8_t)255); }

	//Indicates if rendering should be done in "preview mode", i.e. without editor widgets being drawn.
	bool IsPreviewing() const { return m_previewDraw; }
	void SetPreviewing(bool previewDraw) { m_previewDraw = previewDraw; }
	void TogglePreviewing() { m_previewDraw = !m_previewDraw; }
	std::filesystem::path GetLastSavedPath() const { return m_lastSavedPath; }

	bool IsQuitting() const { return m_quit; }
	void Quit() { m_quit = true; }

	void DisplayStatusMessage(std::string message, float durationSeconds, int priority);

	//General map file operations
	const MapMan& GetMapMan() const { return *m_mapManager.get(); }
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

	Settings m_settings;

	std::unique_ptr<MenuBar> m_menuBar;
	std::unique_ptr<MapMan> m_mapManager;

	std::unique_ptr<PlaceMode> m_tilePlaceMode;
	std::unique_ptr<PickMode> m_texPickMode;
	std::unique_ptr<PickMode> m_shapePickMode;
	std::unique_ptr<EntMode> m_entMode;

	IMode* m_editorMode = nullptr;

	std::filesystem::path m_lastSavedPath;
	bool m_previewDraw;
	bool m_quit;
};

EditorApp* GetApp();