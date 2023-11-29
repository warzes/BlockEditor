#include "stdafx.h"
#include "GameApp.h"
#include "BaseSceneShader.h"
//-----------------------------------------------------------------------------
namespace
{
	ShaderProgramRef NewMeshShader;
	Uniform NewMeshShaderUniformProjectionMatrix;
	Uniform NewMeshShaderUniformViewMatrix;
	Uniform NewMeshShaderWorldViewMatrix;

	NewModel tempModel4;

	Texture2DRef TestTexture;
}
void createImgui()
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
bool GameApp::Create()
{
	auto& renderSystem = GetRenderSystem();

	NewMeshShader = CreateNewMeshSceneShader();
	NewMeshShaderUniformProjectionMatrix = renderSystem.GetUniform(NewMeshShader, "uProjection");
	NewMeshShaderUniformViewMatrix = renderSystem.GetUniform(NewMeshShader, "uView");
	NewMeshShaderWorldViewMatrix = renderSystem.GetUniform(NewMeshShader, "uWorld");

	tempModel4 = LoadModel("../Data/Models/crate.obj");

	m_camera.Teleport(glm::vec3(0.0f, 1.0f, -3.0f));

	GetInputSystem().SetMouseLock(true);
		
	TestTexture = renderSystem.CreateTexture2D("../Data/Textures/texel_checker.png");


	createImgui();

	return true;
}
//-----------------------------------------------------------------------------
void GameApp::Destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	TestTexture.reset();
}
//-----------------------------------------------------------------------------
void GameApp::Render()
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
		
	renderSystem.Bind(NewMeshShader);
	renderSystem.Bind(TestTexture);
	renderSystem.SetUniform(NewMeshShaderUniformProjectionMatrix, m_perspective);
	renderSystem.SetUniform(NewMeshShaderUniformViewMatrix, m_camera.GetViewMatrix());
	renderSystem.SetUniform(NewMeshShaderWorldViewMatrix, glm::mat4(1.0f));
	renderSystem.SetUniform("Texture", 0);

	renderSystem.SetUniform(NewMeshShaderWorldViewMatrix, glm::translate(glm::mat4(1.0f), glm::vec3(-9.0f, 0.0f, 40.0f)));
	for (size_t i = 0; i < tempModel4.meshes.size(); i++)
	{
		renderSystem.Draw(tempModel4.meshes[i].geometry);
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
void GameApp::Update(float deltaTime)
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