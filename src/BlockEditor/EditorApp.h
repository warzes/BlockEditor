#pragma once

#include "IMode.h"
#include "Settings.h"
#include "MenuBar.h"

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
	bool Create() final;
	void Destroy() final;

	void Render() final;
	void Update(float deltaTime) final;

	//Handles transition and data flow from one editor state to the next.
	void ChangeEditorMode(const EditorMode newMode);

	//General map file operations
	void NewMap(int width, int height, int length);

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
	ModeImpl* m_editorMode = nullptr;
};