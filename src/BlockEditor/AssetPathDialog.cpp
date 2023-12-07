#include "stdafx.h"
#include "AssetPathDialog.h"
#include "EditorApp.h"
//-----------------------------------------------------------------------------
#define ASSET_PATH_FILE_DIALOG_CALLBACK(buffer)                                               \
	[&](std::filesystem::path path)                                                           \
	{                                                                                         \
		auto relativePath = std::filesystem::relative(path, std::filesystem::current_path()); \
		strcpy(buffer, relativePath.generic_string().c_str());                                \
	}
//-----------------------------------------------------------------------------
AssetPathDialog::AssetPathDialog(Settings& settings)
	: m_settings(settings)
	, m_fileDialog(nullptr)
{
	strcpy(m_texDirBuffer, GetApp()->GetTexturesDir().c_str());
	strcpy(m_shapeDirBuffer, GetApp()->GetShapesDir().c_str());
	strcpy(m_defaultTexBuffer, GetApp()->GetDefaultTexturePath().c_str());
	strcpy(m_defaultShapeBuffer, GetApp()->GetDefaultShapePath().c_str());
}
//-----------------------------------------------------------------------------
bool AssetPathDialog::Draw()
{
	if (m_fileDialog)
	{
		bool open = m_fileDialog->Draw();
		if (!open) m_fileDialog.reset(nullptr);
		return true;
	}

	bool open = true;
	ImGui::OpenPopup("CONFIGURE ASSET PATHS");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("CONFIGURE ASSET PATHS", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("Browse##texdir"))
		{
			m_fileDialog.reset(new FileDialog("Select textures directory", {}, ASSET_PATH_FILE_DIALOG_CALLBACK(m_texDirBuffer), false));
		}
		ImGui::SameLine();
		ImGui::InputText("Textures Directory", m_texDirBuffer, TEXT_FIELD_MAX);

		if (ImGui::Button("Browse##deftex"))
		{
			m_fileDialog.reset(new FileDialog("Select default texture", { ".png" }, ASSET_PATH_FILE_DIALOG_CALLBACK(m_defaultTexBuffer), false));
		}
		ImGui::SameLine();
		ImGui::InputText("Default RLTexture", m_defaultTexBuffer, TEXT_FIELD_MAX);

		if (ImGui::Button("Browse##shapedir"))
		{
			m_fileDialog.reset(new FileDialog("Select shapes directory", {}, ASSET_PATH_FILE_DIALOG_CALLBACK(m_shapeDirBuffer), false));
		}
		ImGui::SameLine();
		ImGui::InputText("Shapes Directory", m_shapeDirBuffer, TEXT_FIELD_MAX);

		if (ImGui::Button("Browse##defshape"))
		{
			m_fileDialog.reset(new FileDialog("Select default shape", { ".obj" }, ASSET_PATH_FILE_DIALOG_CALLBACK(m_defaultShapeBuffer), false));
		}
		ImGui::SameLine();
		ImGui::InputText("Default Shape", m_defaultShapeBuffer, TEXT_FIELD_MAX);

		if (ImGui::Button("Confirm"))
		{
			// Validate all of the entered paths
			bool bad = false;
			std::filesystem::directory_entry texEntry{ m_texDirBuffer };
			if (!texEntry.exists() || !texEntry.is_directory())
			{
				strcpy(m_texDirBuffer, "Invalid!");
				bad = true;
			}
			std::filesystem::directory_entry defTexEntry{ m_defaultTexBuffer };
			if (!defTexEntry.exists() || !defTexEntry.is_regular_file())
			{
				strcpy(m_defaultTexBuffer, "Invalid!");
				bad = true;
			}
			std::filesystem::directory_entry shapeEntry{ m_shapeDirBuffer };
			if (!shapeEntry.exists() || !shapeEntry.is_directory())
			{
				strcpy(m_shapeDirBuffer, "Invalid!");
				bad = true;
			}
			std::filesystem::directory_entry defShapeEntry{ m_defaultShapeBuffer };
			if (!defShapeEntry.exists() || !defShapeEntry.is_regular_file())
			{
				strcpy(m_defaultShapeBuffer, "Invalid!");
				bad = true;
			}
			if (!bad)
			{
				m_settings.texturesDir = texEntry.path().string();
				m_settings.defaultTexturePath = defTexEntry.path().string();
				m_settings.shapesDir = shapeEntry.path().string();
				m_settings.defaultShapePath = defShapeEntry.path().string();
				GetApp()->SaveSettings();
				ImGui::EndPopup();
				return false;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
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