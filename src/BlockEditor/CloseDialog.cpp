#include "stdafx.h"
#include "CloseDialog.h"
#include "EditorApp.h"
//-----------------------------------------------------------------------------
bool CloseDialog::Draw()
{
	bool open = true;
	ImGui::OpenPopup("REALLY QUIT?");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("REALLY QUIT?", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("Quit"))
		{
			GetApp()->Quit();
			ImGui::EndPopup();
			return false;
		}
		ImGui::SameLine();
		if (ImGui::Button("No"))
		{
			ImGui::EndPopup();
			return false;
		}

		ImGui::EndPopup();
		return true;
	}
	return open;
}
//-----------------------------------------------------------------------------