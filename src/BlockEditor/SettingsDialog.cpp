#include "stdafx.h"
#include "SettingsDialog.h"
#include "EditorApp.h"
//-----------------------------------------------------------------------------
SettingsDialog::SettingsDialog(Settings& settings)
	: m_settingsOriginal(settings)
	, m_settingsCopy(settings)
{
}
//-----------------------------------------------------------------------------
bool SettingsDialog::Draw()
{
	bool open = true;
	ImGui::OpenPopup("SETTINGS");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("SETTINGS", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		int undoMax = (int)m_settingsCopy.undoMax;
		ImGui::InputInt("Maximum undo count", &undoMax, 1, 10);
		if (undoMax < 0) undoMax = 0;
		m_settingsCopy.undoMax = undoMax;

		ImGui::SliderFloat("Mouse sensitivity", &m_settingsCopy.mouseSensitivity, 0.05f, 10.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat);

		float bgColorf[3] = {
		(float)m_settingsCopy.backgroundColor[0] / 255.0f,
		(float)m_settingsCopy.backgroundColor[1] / 255.0f,
		(float)m_settingsCopy.backgroundColor[2] / 255.0f
		};
		ImGui::SetNextItemWidth(256.0f);
		ImGui::ColorPicker3("Background color", bgColorf, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB);
		m_settingsCopy.backgroundColor[0] = (int)(bgColorf[0] * 255.0f);
		m_settingsCopy.backgroundColor[1] = (int)(bgColorf[1] * 255.0f);
		m_settingsCopy.backgroundColor[2] = (int)(bgColorf[2] * 255.0f);

		if (ImGui::Button("Confirm"))
		{
			m_settingsOriginal = m_settingsCopy;
			GetApp()->SaveSettings();
			ImGui::EndPopup();
			return false;
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