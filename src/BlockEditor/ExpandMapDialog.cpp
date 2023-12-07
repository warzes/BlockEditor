#include "stdafx.h"
#include "ExpandMapDialog.h"
#include "EditorApp.h"
//-----------------------------------------------------------------------------
ExpandMapDialog::ExpandMapDialog()
	: m_spinnerActive(false)
	, m_chooserActive(false)
	, m_amount(0)
	, m_direction(Direction::Z_NEG)
{
}
//-----------------------------------------------------------------------------
bool ExpandMapDialog::Draw()
{
	bool open = true;
	ImGui::OpenPopup("EXPAND GRID");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("EXPAND GRID", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Combo("Direction", (int*)(&m_direction), "Back (+Z)\0Front (-Z)\0Right (+X)\0Left (-X)\0Top (+Y)\0Bottom (-Y)\0");

		ImGui::InputInt("# of grid cels", &m_amount, 1, 10);

		if (ImGui::Button("EXPAND"))
		{
			GetApp()->ExpandMap(m_direction, m_amount);
			ImGui::EndPopup();
			return false;
		}

		ImGui::EndPopup();
		return true;
	}
	return open;
}
//-----------------------------------------------------------------------------