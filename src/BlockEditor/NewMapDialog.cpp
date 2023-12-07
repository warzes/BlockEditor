#include "stdafx.h"
#include "NewMapDialog.h"
#include "Tile.h"
#include "EditorApp.h"
#include "MapMan.h"
//-----------------------------------------------------------------------------
NewMapDialog::NewMapDialog()
{
	const TileGrid& map = GetApp()->GetMapMan().Tiles();
	m_mapDims[0] = map.GetWidth();
	m_mapDims[1] = map.GetHeight();
	m_mapDims[2] = map.GetLength();
}
//-----------------------------------------------------------------------------
bool NewMapDialog::Draw()
{
	bool open = true;
	ImGui::OpenPopup("NEW MAP");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("NEW MAP", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("NEW GRID SIZE:");
		ImGui::InputInt3("X, Y, Z", m_mapDims);

		if (ImGui::Button("CREATE"))
		{
			GetApp()->NewMap(m_mapDims[0], m_mapDims[1], m_mapDims[2]);
			ImGui::EndPopup();
			return false;
		}

		ImGui::EndPopup();
		return true;
	}
	return open;
}
//-----------------------------------------------------------------------------