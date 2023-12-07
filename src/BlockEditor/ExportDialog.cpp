#include "stdafx.h"
#include "ExportDialog.h"
#include "EditorApp.h"
//-----------------------------------------------------------------------------
ExportDialog::ExportDialog(Settings& settings)
	: m_settings(settings)
{
	strcpy(m_filePathBuffer, m_settings.exportFilePath.c_str());
}
//-----------------------------------------------------------------------------
bool ExportDialog::Draw()
{
	if (m_dialog)
	{
		if (!m_dialog->Draw())
			m_dialog.reset(nullptr);
		return true;
	}

	bool open = true;
	ImGui::OpenPopup("EXPORT .GLTF/.GLB SCENE");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("EXPORT .GLTF/.GLB SCENE", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(".gltf and .glb are both supported!");
		ImGui::InputText("File path", m_filePathBuffer, TEXT_FIELD_MAX);
		ImGui::SameLine();
		if (ImGui::Button("Browse##gltfexport"))
		{
			m_dialog.reset(new FileDialog(std::string("Save .GLTF or .GLB file"), { std::string(".gltf"), std::string(".glb") }, [&](std::filesystem::path path) 
				{
				m_settings.exportFilePath = std::filesystem::relative(path).string();
				strcpy(m_filePathBuffer, m_settings.exportFilePath.c_str());
				}, true));
		}
		m_settings.exportFilePath = m_filePathBuffer;

		ImGui::Checkbox("Seperate nodes for each texture", &m_settings.exportSeparateGeometry);
		ImGui::Checkbox("Cull redundant faces between tiles", &m_settings.cullFaces);

		if (ImGui::Button("Export##exportgltf"))
		{
			GetApp()->TryExportMap(std::filesystem::path(m_settings.exportFilePath), m_settings.exportSeparateGeometry);
			GetApp()->SaveSettings();
			ImGui::EndPopup();
			return false;
		}

		ImGui::EndPopup();
		return true;
	}

	return open;
}
//-----------------------------------------------------------------------------