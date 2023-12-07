#include "stdafx.h"
#include "AboutDialog.h"
//-----------------------------------------------------------------------------
bool AboutDialog::Draw()
{
	bool open = true;
	ImGui::OpenPopup("ABOUT");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("ABOUT", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), "Editor");
		ImGui::TextUnformatted("");
		ImGui::TextColored(ImColor(0.0f, 0.5f, 0.5f), "Source code: ");

		ImGui::EndPopup();
		return true;
	}
	return open;
}
//-----------------------------------------------------------------------------