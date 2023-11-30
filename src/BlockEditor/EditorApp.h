#pragma once

#include "IMode.h"
#include "Settings.h"
#include "MenuBar.h"
#include "MapMan.h"

#include "PlaceMode.h"
#include "PickMode.h"
#include "EntMode.h"

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
	void ChangeEditorMode(const EditorMode newMode);

	inline float       GetMouseSensitivity() { return m_settings.mouseSensitivity; }
	inline size_t      GetUndoMax() { return m_settings.undoMax; }
	inline std::string GetTexturesDir() { return m_settings.texturesDir; };
	inline std::string GetShapesDir() { return m_settings.shapesDir; }
	inline std::string GetDefaultTexturePath() { return m_settings.defaultTexturePath; }
	inline std::string GetDefaultShapePath() { return m_settings.defaultShapePath; }
	inline bool        IsCullingEnabled() { return m_settings.cullFaces; }
	inline Color       GetBackgroundColor() { return Color{ m_settings.backgroundColor[0], m_settings.backgroundColor[1], m_settings.backgroundColor[2], 255 }; }

	//Indicates if rendering should be done in "preview mode", i.e. without editor widgets being drawn.
	inline bool IsPreviewing() const { return m_previewDraw; }
	inline void SetPreviewing(bool p) { m_previewDraw = p; }
	inline void TogglePreviewing() { m_previewDraw = !m_previewDraw; }
	inline std::filesystem::path GetLastSavedPath() const { return m_lastSavedPath; }

	void DisplayStatusMessage(std::string message, float durationSeconds, int priority);

	//General map file operations
	inline const MapMan& GetMapMan() const { return *m_mapMan.get(); }
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
	glm::mat4 m_perspective;
	Camera m_camera;

	Settings m_settings;

	std::unique_ptr<MenuBar> m_menuBar;
	std::unique_ptr<MapMan> m_mapMan;

	std::unique_ptr<PlaceMode> m_tilePlaceMode;
	std::unique_ptr<PickMode> m_texPickMode;
	std::unique_ptr<PickMode> m_shapePickMode;
	std::unique_ptr<EntMode> m_entMode;

	ModeImpl* m_editorMode = nullptr;

	std::filesystem::path m_lastSavedPath{};
	bool m_previewDraw = false;
};

EditorApp* GetApp();