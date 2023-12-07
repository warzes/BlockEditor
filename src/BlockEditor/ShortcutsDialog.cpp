#include "stdafx.h"
#include "ShortcutsDialog.h"
//-----------------------------------------------------------------------------
bool ShortcutsDialog::Draw()
{
	const char* SHORTCUT_KEYS[] = {
	"W/A/S/D",
	"Hold Middle click or LEFT ALT+LEFT CLICK",
	"Scroll wheel",
	"Left click",
	"Right click",
	"TAB",
	"LEFT SHIFT+TAB",
	"T (Tile mode)",
	"G (Tile mode)",
	"HOLD LEFT SHIFT",
	"Q",
	"E",
	"R",
	"F",
	"V",
	"H",
	"H (when layers are isolated)",
	"Hold H while using scrollwheel",
	"LEFT SHIFT+B",
	"ESCAPE/BACKSPACE",
	"LEFT CTRL+TAB",
	"LEFT CTRL+E",
	"T/G (Entity mode)",
	"LEFT CTRL+S",
	"LEFT CTRL+Z",
	"LEFT CTRL+Y"
	};

	const char* SHORTCUT_INFO[] = {
	"Move camera",
	"Look around",
	"Move grid up/down",
	"Place tile/entity/brush",
	"Remove tile/entity (Does not work in brush mode)",
	"Switch between texture picker and map editor.",
	"Switch between shape picker and map editor.",
	"Select texture of tile under cursor",
	"Select shape of tile under cursor",
	"Expand cursor to place tiles in bulk.",
	"Turn cursor counterclockwise",
	"Turn cursor clockwise",
	"Reset cursor orientation",
	"Turn cursor upwards",
	"Turn cursor downwards",
	"Isolate the layer of tiles the grid is on.",
	"Unhide hidden layers.",
	"Select multiple layers to isolate.",
	"Capture tiles under cursor as a brush.",
	"Return cursor to tile mode.",
	"Switch between entity editor and map editor.",
	"Put cursor into entity mode.",
	"Copy entity from under cursor.",
	"Save map.",
	"Undo",
	"Redo"
	};

	const int SHORTCUT_COUNT = 26;

	bool open = true;
	ImGui::OpenPopup("SHORTCUTS");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(640.0f, 468.0f), ImVec2(1280.0f, 720.0f));
	if (ImGui::BeginPopupModal("SHORTCUTS", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
	{
		for (int i = 0; i < SHORTCUT_COUNT; ++i)
		{
			ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), SHORTCUT_KEYS[i]);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), "-");
			ImGui::SameLine();
			ImGui::TextUnformatted(SHORTCUT_INFO[i]);
		}

		ImGui::EndPopup();
		return true;
	}
	return open;
}
//-----------------------------------------------------------------------------