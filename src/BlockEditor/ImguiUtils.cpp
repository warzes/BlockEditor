#include "stdafx.h"
#include "ImguiUtils.h"

static ImGuiMouseCursor CurrentMouseCursor = ImGuiMouseCursor_COUNT;

static void rlImGuiNewFrame(float deltaTime)
{
	ImGuiIO& io = ImGui::GetIO();
	auto& input = GetInputSystem();

	//if (IsWindowFullscreen())
	//{
	//	int monitor = GetCurrentMonitor();
	//	io.DisplaySize.x = float(GetMonitorWidth(monitor));
	//	io.DisplaySize.y = float(GetMonitorHeight(monitor));
	//}
	//else
	//{
		io.DisplaySize.x = float(GetWindowWidth());
		io.DisplaySize.y = float(GetWindowHeight());
	//}

	int width = int(io.DisplaySize.x), height = int(io.DisplaySize.y);
#ifdef PLATFORM_DESKTOP
	glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
#endif
	if (width > 0 && height > 0) {
		io.DisplayFramebufferScale = ImVec2(width / io.DisplaySize.x, height / io.DisplaySize.y);
	}
	else {
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	}

	io.DeltaTime = deltaTime;

	if (io.WantSetMousePos)
	{
		input.SetMousePosition((int)io.MousePos.x, (int)io.MousePos.y);
	}
	else
	{
		io.MousePos.x = (float)input.GetMousePosition().x;
		io.MousePos.y = (float)input.GetMousePosition().y;
	}

	io.MouseDown[0] = input.IsMouseButtonDown(Input::MOUSE_LEFT);
	io.MouseDown[1] = input.IsMouseButtonDown(Input::MOUSE_RIGHT);
	io.MouseDown[2] = input.IsMouseButtonDown(Input::MOUSE_MIDDLE);

	{
		glm::vec2 mouseWheel = input.GetMouseWheelMoveV();
		io.MouseWheel += mouseWheel.y;
		io.MouseWheelH += mouseWheel.x;
	}

	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
	{
		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		if (imgui_cursor != CurrentMouseCursor || io.MouseDrawCursor)
		{
			CurrentMouseCursor = imgui_cursor;
			if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
			{
				HideCursor();
			}
			else
			{
				ShowCursor();

				if (!(io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
				{
					SetMouseCursor((imgui_cursor > -1 && imgui_cursor < ImGuiMouseCursor_COUNT) ? MouseCursorMap[imgui_cursor] : MOUSE_CURSOR_DEFAULT);
				}
			}
		}
	}
}

static void rlImGuiEvents()
{
	ImGuiIO& io = ImGui::GetIO();

	bool focused = IsWindowFocused();
	if (focused != LastFrameFocused)
		io.AddFocusEvent(focused);
	LastFrameFocused = focused;

	// handle the modifyer key events so that shortcuts work
	bool ctrlDown = rlImGuiIsControlDown();
	if (ctrlDown != LastControlPressed)
		io.AddKeyEvent(ImGuiMod_Ctrl, ctrlDown);
	LastControlPressed = ctrlDown;

	bool shiftDown = rlImGuiIsShiftDown();
	if (shiftDown != LastShiftPressed)
		io.AddKeyEvent(ImGuiMod_Shift, ctrlDown);
	LastShiftPressed = shiftDown;

	bool altDown = rlImGuiIsAltDown();
	if (altDown != LastAltPressed)
		io.AddKeyEvent(ImGuiMod_Alt, altDown);
	LastAltPressed = altDown;

	bool superDown = rlImGuiIsSuperDown();
	if (superDown != LastSuperPressed)
		io.AddKeyEvent(ImGuiMod_Super, ctrlDown);
	LastSuperPressed = superDown;

	// get the pressed keys, they are in event order
	int keyId = GetKeyPressed();
	while (keyId != 0)
	{
		auto keyItr = RaylibKeyMap.find(KeyboardKey(keyId));
		if (keyItr != RaylibKeyMap.end())
			io.AddKeyEvent(keyItr->second, true);
		keyId = GetKeyPressed();
	}

	for (auto keyItr : RaylibKeyMap)
		io.KeysData[keyItr.second].Down = IsKeyDown(keyItr.first);

	// look for any keys that were down last frame and see if they were down and are released
	for (const auto keyItr : RaylibKeyMap)
	{
		if (IsKeyReleased(keyItr.first))
			io.AddKeyEvent(keyItr.second, false);
	}

	// add the text input in order
	unsigned int pressed = GetCharPressed();
	while (pressed != 0)
	{
		io.AddInputCharacter(pressed);
		pressed = GetCharPressed();
	}
}


void rlImGuiBegin()
{
	rlImGuiNewFrame();
	rlImGuiEvents();
	ImGui::NewFrame();
}

void rlImGuiEnd()
{
	ImGui::Render();
	rlRenderData(ImGui::GetDrawData());
}

bool rlImGuiImageButton(const char* name, const RLTexture* image)
{
	return ImGui::ImageButton(name, (ImTextureID)image, ImVec2(float(image->width), float(image->height)));
}