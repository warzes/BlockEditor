#include "stdafx.h"
#include "ShrinkMapDialog.h"
#include "EditorApp.h"
//-----------------------------------------------------------------------------
bool ShrinkMapDialog::Draw()
{
	bool open = true;
	ImGui::OpenPopup("SHRINK GRID");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("SHRINK GRID", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Shrink the grid to fit around the existing tiles?");

		if (ImGui::Button("Yes"))
		{
			GetApp()->ShrinkMap();
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