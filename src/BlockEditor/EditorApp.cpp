#include "stdafx.h"
#include "EditorApp.h"
#include "PlaceMode.h"
#include "PickMode.h"
#include "EntMode.h"
#include "MenuBar.h"
void rlLoadShaderDefault();
void rlLoadTextureDefault();
EditorApp* App = nullptr;
#define SETTINGS_FILE_PATH "settings.json"
//-----------------------------------------------------------------------------
inline void createImgui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
	io.IniFilename = nullptr; // не нужно хранить файл настроек

	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	if (!ImGui_ImplGlfw_InitForOpenGL(GetWindowSystem().GetHandle(), true))
		return ;
	const char* glsl_version = "#version 330";
	if (!ImGui_ImplOpenGL3_Init(glsl_version))
		return ;

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	// - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
	// io.Fonts->AddFontDefault();
	// io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	// ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	// IM_ASSERT(font != NULL);
}
//-----------------------------------------------------------------------------
EditorApp::EditorApp()
	: _settings{
			.texturesDir = "../Data/Textures/Tiles/",
			.shapesDir = "../Data/Models/Shapes/",
			.undoMax = 30UL,
			.mouseSensitivity = 0.5f,
			.exportSeparateGeometry = false,
			.cullFaces = true,
			.defaultTexturePath = "../Data/Textures/Tiles/texel_checker.png",
			.defaultShapePath = "../Data/Models/Shapes/cube.obj",
	}
	,_previewDraw(false)
	,_lastSavedPath()
	,_quit(false)
{
	App = this;
}
//-----------------------------------------------------------------------------
EditorApp::~EditorApp()
{
	App = nullptr;
}
//-----------------------------------------------------------------------------
bool EditorApp::Create()
{
	std::filesystem::directory_entry entry{ SETTINGS_FILE_PATH };
	if (entry.exists())
	{
		LoadSettings();
	}
	else
	{
		SaveSettings();
	}

	_mapMan = std::make_unique<MapMan>();
	_tilePlaceMode = std::make_unique<PlaceMode>(*_mapMan.get());
	_texPickMode = std::make_unique<PickMode>(PickMode::Mode::TEXTURES);
	_shapePickMode = std::make_unique<PickMode>(PickMode::Mode::SHAPES);
	_entMode = std::make_unique<EntMode>();
	_editorMode = _tilePlaceMode.get();
	_menuBar = std::make_unique<MenuBar>(_settings);

	rlLoadShaderDefault();
	rlLoadTextureDefault();
	createImgui();


	ChangeEditorMode(Mode::PLACE_TILE);
	NewMap(100, 5, 100);

	return true;
}
//-----------------------------------------------------------------------------
void EditorApp::Destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
//-----------------------------------------------------------------------------
void EditorApp::Render()
{
	auto& renderSystem = GetRenderSystem();
	auto& graphicsSystem = GetGraphicsSystem();

	if (m_windowWidth != GetWindowWidth() || m_windowHeight != GetWindowHeight())
	{
		m_windowWidth = GetWindowWidth();
		m_windowHeight = GetWindowHeight();
		renderSystem.SetViewport(m_windowWidth, m_windowHeight);
	}
	renderSystem.ClearFrame();

	{
		rlImGuiBegin();
		_editorMode->Draw();
		_menuBar->Draw();
		rlImGuiEnd();
	}

	{
		ImGui::Begin("Window");
		ImGui::Text("Hello window!");
		ImGui::SameLine();
		ImGui::Button("Close");
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
//-----------------------------------------------------------------------------
void EditorApp::Update(float deltaTime)
{
	{
		_menuBar->Update(deltaTime);

		auto& input = GetInputSystem();

		//Mode switching hotkeys
		if (input.IsKeyPressed(Input::KEY_TAB))
		{
			if (input.IsKeyDown(Input::KEY_LEFT_SHIFT))
			{
				if (_editorMode == _tilePlaceMode.get()) ChangeEditorMode(Mode::PICK_SHAPE);
				else ChangeEditorMode(Mode::PLACE_TILE);
			}
			else if (input.IsKeyDown(Input::KEY_LEFT_CONTROL))
			{
				if (_editorMode == _tilePlaceMode.get()) ChangeEditorMode(Mode::EDIT_ENT);
				else if (_editorMode == _entMode.get()) ChangeEditorMode(Mode::PLACE_TILE);
			}
			else
			{
				if (_editorMode == _tilePlaceMode.get()) ChangeEditorMode(Mode::PICK_TEXTURE);
				else ChangeEditorMode(Mode::PLACE_TILE);
			}
		}

		//Save hotkey
		if (input.IsKeyDown(Input::KEY_LEFT_CONTROL) && input.IsKeyPressed(Input::KEY_S))
		{
			if (!GetLastSavedPath().empty())
			{
				TrySaveMap(GetLastSavedPath());
			}
			else
			{
				_menuBar->OpenSaveMapDialog();
			}
		}

		_editorMode->Update(deltaTime);
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	const ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse && !io.WantCaptureKeyboard)
	{
		// обновляем события если не было событий в имгуи
		// TODO: а возможно нужно передавать событие имгуи и апдейт вызывать всегда
	}

	if (GetInputSystem().IsKeyDown(Input::KEY_ESCAPE))
	{
		ExitRequest();
		return;
	}
}
//-----------------------------------------------------------------------------
EditorApp* GetApp()
{
	return App;
}
//-----------------------------------------------------------------------------
void EditorApp::ChangeEditorMode(const Mode newMode)
{
	_editorMode->OnExit();

	if (_editorMode == _texPickMode.get() && _texPickMode->GetPickedTexture())
	{
		_tilePlaceMode->SetCursorTexture(_texPickMode->GetPickedTexture());
	}
	else if (_editorMode == _shapePickMode.get() && _shapePickMode->GetPickedShape())
	{
		_tilePlaceMode->SetCursorShape(_shapePickMode->GetPickedShape());
	}

	switch (newMode)
	{
	case Mode::PICK_SHAPE:
	{
		_editorMode = _shapePickMode.get();
	}
	break;
	case Mode::PICK_TEXTURE:
	{
		_editorMode = _texPickMode.get();
	}
	break;
	case Mode::PLACE_TILE:
	{
		if (_editorMode == _entMode.get())
		{
			_tilePlaceMode->SetCursorEnt(_entMode->GetEnt());
		}
		_editorMode = _tilePlaceMode.get();
	}
	break;
	case Mode::EDIT_ENT:
	{
		if (_editorMode == _tilePlaceMode.get()) _entMode->SetEnt(_tilePlaceMode->GetCursorEnt());
		_editorMode = _entMode.get();
	}
	break;
	}
	_editorMode->OnEnter();
}
//-----------------------------------------------------------------------------
void EditorApp::DisplayStatusMessage(std::string message, float durationSeconds, int priority)
{
	_menuBar->DisplayStatusMessage(message, durationSeconds, priority);
}
//-----------------------------------------------------------------------------
void EditorApp::ResetEditorCamera()
{
	if (_editorMode == _tilePlaceMode.get())
	{
		_tilePlaceMode->ResetCamera();
		_tilePlaceMode->ResetGrid();
	}
}
//-----------------------------------------------------------------------------
void EditorApp::NewMap(int width, int height, int length)
{
	_mapMan->NewMap(width, height, length);
	_tilePlaceMode->ResetCamera();
	_tilePlaceMode->ResetGrid();
	_lastSavedPath = "";
}
//-----------------------------------------------------------------------------
void EditorApp::ExpandMap(Direction axis, int amount)
{
	_mapMan->ExpandMap(axis, amount);
	_tilePlaceMode->ResetGrid();
}
//-----------------------------------------------------------------------------
void EditorApp::ShrinkMap()
{
	_mapMan->ShrinkMap();
	_tilePlaceMode->ResetGrid();
	_tilePlaceMode->ResetCamera();
}
//-----------------------------------------------------------------------------
void EditorApp::TryOpenMap(std::filesystem::path path)
{
	std::filesystem::directory_entry entry{ path };
	if (entry.exists() && entry.is_regular_file())
	{
		if (path.extension() == ".te3")
		{
			if (_mapMan->LoadTE3Map(path))
			{
				_lastSavedPath = path;
				DisplayStatusMessage("Loaded .te3 map '" + path.filename().string() + "'.", 5.0f, 100);
			}
			else
			{
				DisplayStatusMessage("ERROR: Failed to load .te3 map. Check the console.", 5.0f, 100);
			}
			_tilePlaceMode->ResetCamera();
			// Set editor camera to saved position
			_tilePlaceMode->SetCameraOrientation(_mapMan->GetDefaultCameraPosition(), _mapMan->GetDefaultCameraAngles());
			_tilePlaceMode->ResetGrid();
		}
		else if (path.extension() == ".ti")
		{
			if (_mapMan->LoadTE2Map(path))
			{
				_lastSavedPath = "";
				DisplayStatusMessage("Loaded .ti map '" + path.filename().string() + "'.", 5.0f, 100);
			}
			else
			{
				DisplayStatusMessage("ERROR: Failed to load .ti map. Check the console.", 5.0f, 100);
			}
			_tilePlaceMode->ResetCamera();
			_tilePlaceMode->ResetGrid();
		}
		else
		{
			DisplayStatusMessage("ERROR: Invalid file extension.", 5.0f, 100);
		}
	}
	else
	{
		DisplayStatusMessage("ERROR: Invalid file path.", 5.0f, 100);
	}
}
//-----------------------------------------------------------------------------
void EditorApp::TrySaveMap(std::filesystem::path path)
{
	//Add correct extension if no extension is given.
	if (path.extension().empty())
	{
		path += ".te3";
	}

	std::filesystem::directory_entry entry{ path };

	if (path.extension() == ".te3")
	{
		if (_mapMan->SaveTE3Map(path))
		{
			_lastSavedPath = path;
			std::string msg = "Saved .te3 map '";
			msg += path.filename().string();
			msg += "'.";
			DisplayStatusMessage(msg, 5.0f, 100);
		}
		else
		{
			DisplayStatusMessage("ERROR: Map could not be saved. Check the console.", 5.0f, 100);
		}
	}
	else
	{
		DisplayStatusMessage("ERROR: Invalid file extension.", 5.0f, 100);
	}
}
//-----------------------------------------------------------------------------
void EditorApp::TryExportMap(std::filesystem::path path, bool separateGeometry)
{
	//Add correct extension if no extension is given.
	if (path.extension().empty())
	{
		path += ".gltf";
	}

	std::filesystem::directory_entry entry{ path };

	if (path.extension() == ".gltf" || path.extension() == ".glb")
	{
		if (_mapMan->ExportGLTFScene(path, separateGeometry))
		{
			DisplayStatusMessage(std::string("Exported map as ") + path.filename().string(), 5.0f, 100);
		}
		else
		{
			DisplayStatusMessage("ERROR: Map could not be exported. Check the console.", 5.0f, 100);
		}
	}
	else
	{
		DisplayStatusMessage("ERROR: Invalid file extension.", 5.0f, 100);
	}
}
//-----------------------------------------------------------------------------
void EditorApp::SaveSettings()
{
	try
	{
		nlohmann::json jData;
		EditorApp::to_json(jData, _settings);
		std::ofstream file(SETTINGS_FILE_PATH);
		file << jData;
	}
	catch (std::exception e)
	{
		std::cerr << "Error saving settings: " << e.what() << std::endl;
	}
}
//-----------------------------------------------------------------------------
void EditorApp::LoadSettings()
{
	try
	{
		nlohmann::json jData;
		std::ifstream file(SETTINGS_FILE_PATH);
		file >> jData;
		EditorApp::from_json(jData, _settings);
	}
	catch (std::exception e)
	{
		std::cerr << "Error loading settings: " << e.what() << std::endl;
	}
}
//-----------------------------------------------------------------------------