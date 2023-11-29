#include "stdafx.h"
#include "EditorApp.h"
#include "softball_gold_ttf.h"
//-----------------------------------------------------------------------------
inline void createImgui()
{
	ImGui::CreateContext(nullptr);
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();


	ImGuiIO& io = ImGui::GetIO();		
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiBackendFlags_HasMouseCursors;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
	io.IniFilename = nullptr; // не нужно хранить файл настроек

	// ImGUI styling
	{
		ImFontConfig config;
		config.FontDataOwnedByAtlas = false;
		io.FontDefault = io.Fonts->AddFontFromMemoryTTF((void*)Softball_Gold_ttf, Softball_Gold_ttf_len, 24, &config, io.Fonts->GetGlyphRangesCyrillic());
	}


	if (!ImGui_ImplGlfw_InitForOpenGL(GetWindowSystem().GetHandle(), true))
		return;
	const char* glsl_version = "#version 330";
	if (!ImGui_ImplOpenGL3_Init(glsl_version))
		return;

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
#define SETTINGS_FILE_PATH "settings.json"
//-----------------------------------------------------------------------------
EditorApp::EditorApp()
	: m_settings{
		.texturesDir = "../Data/Textures/Tiles/",
		.shapesDir = "../Data/Models/Shapes/",
		.undoMax = 30UL,
		.mouseSensitivity = 0.5f,
		.exportSeparateGeometry = false,
		.cullFaces = true,
		.defaultTexturePath = "../Data/Textures/Tiles/texel_checker.png",
		.defaultShapePath = "../Data/Models/Shapes/cube.obj",
	}
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
}
//-----------------------------------------------------------------------------
bool EditorApp::Create()
{
	auto& renderSystem = GetRenderSystem();

	m_camera.Teleport(glm::vec3(0.0f, 1.0f, -3.0f));

	//GetInputSystem().SetMouseLock(true);

	createImgui();

	ChangeEditorMode(EditorMode::PLACE_TILE);
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
		m_perspective = glm::perspective(glm::radians(65.0f), GetWindowSizeAspect(), 0.01f, 100.f);
		renderSystem.SetViewport(m_windowWidth, m_windowHeight);
	}
	renderSystem.ClearFrame();
	
	{
		ImGui::Begin("Window");
		ImGui::Text("Hello window! Привет мир !");
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

	const float mouseSensitivity = 10.0f * deltaTime;
	const float moveSpeed = 20.0f * deltaTime;

	if (GetInputSystem().IsKeyDown(Input::KEY_W)) m_camera.MoveBy(moveSpeed);
	if (GetInputSystem().IsKeyDown(Input::KEY_S)) m_camera.MoveBy(-moveSpeed);
	if (GetInputSystem().IsKeyDown(Input::KEY_A)) m_camera.StrafeBy(moveSpeed);
	if (GetInputSystem().IsKeyDown(Input::KEY_D)) m_camera.StrafeBy(-moveSpeed);

	glm::vec2 delta = GetInputSystem().GetMouseDeltaPosition();
	if (delta.x != 0.0f)  m_camera.RotateLeftRight(delta.x * mouseSensitivity);
	if (delta.y != 0.0f)  m_camera.RotateUpDown(-delta.y * mouseSensitivity);
}
//-----------------------------------------------------------------------------
void EditorApp::ChangeEditorMode(const EditorMode newMode)
{
	m_editorMode->OnExit();
	// TODO:

	m_editorMode->OnEnter();
}
//-----------------------------------------------------------------------------
void EditorApp::NewMap(int width, int height, int length)
{
	// TODO:
}
//-----------------------------------------------------------------------------
void EditorApp::SaveSettings()
{
	// TODO:
}
//-----------------------------------------------------------------------------
void EditorApp::LoadSettings()
{
	// TODO:
}
//-----------------------------------------------------------------------------