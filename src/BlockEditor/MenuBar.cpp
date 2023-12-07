#include "stdafx.h"
#include "MenuBar.h"
#include "EditorApp.h"
#include "IMode.h"
#include "FileDialog.h"
#include "NewMapDialog.h"
#include "ExportDialog.h"
#include "ExpandMapDialog.h"
#include "ShrinkMapDialog.h"
#include "AssetPathDialog.h"
#include "SettingsDialog.h"
#include "AboutDialog.h"
#include "ShortcutsDialog.h"
#include "InstructionsDialog.h"
//-----------------------------------------------------------------------------
MenuBar::MenuBar(Settings& settings)
	: m_settings(settings)
	, m_activeDialog(nullptr)
	, m_messageTimer(0.0f)
	, m_messagePriority(0)
{
}
//-----------------------------------------------------------------------------
void MenuBar::DisplayStatusMessage(std::string message, float durationSeconds, int priority)
{
	if (priority >= m_messagePriority)
	{
		m_statusMessage = message;
		m_messageTimer = durationSeconds;
		m_messagePriority = priority;
	}
}
//-----------------------------------------------------------------------------
void MenuBar::OpenOpenMapDialog()
{
	auto callback = [](std::filesystem::path path)
		{
			GetApp()->TryOpenMap(path);
		};
	m_activeDialog.reset(new FileDialog("Open Map (*.te3)", { ".te3" }, callback, false));
}
//-----------------------------------------------------------------------------
void MenuBar::OpenSaveMapDialog()
{
	auto callback = [](std::filesystem::path path)
		{
			GetApp()->TrySaveMap(path);
		};
	m_activeDialog.reset(new FileDialog("Save Map (*.te3)", { ".te3" }, callback, true));
}
//-----------------------------------------------------------------------------
void MenuBar::SaveMap()
{
	if (!GetApp()->GetLastSavedPath().empty())
	{
		GetApp()->TrySaveMap(GetApp()->GetLastSavedPath());
	}
	else
	{
		OpenSaveMapDialog();
	}
}
//-----------------------------------------------------------------------------
void MenuBar::Update(float deltaTime)
{
	if (m_messageTimer > 0.0f)
	{
		m_messageTimer -= deltaTime;
		if (m_messageTimer <= 0.0f)
		{
			m_messageTimer = 0.0f;
			m_messagePriority = 0;
		}
	}

	//Show exit confirmation dialog
	// TODO: диалог при закрытии приложения
	//if (WindowShouldClose())
	//{
	//    m_activeDialog.reset(new CloseDialog());
	//}
}
//-----------------------------------------------------------------------------
void MenuBar::Draw()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("MAP"))
		{
			if (ImGui::MenuItem("NEW")) m_activeDialog.reset(new NewMapDialog());
			if (ImGui::MenuItem("OPEN")) OpenOpenMapDialog();
			if (ImGui::MenuItem("SAVE")) SaveMap();
			if (ImGui::MenuItem("SAVE AS")) OpenSaveMapDialog();
			if (ImGui::MenuItem("EXPORT")) m_activeDialog.reset(new ExportDialog(m_settings));
			if (ImGui::MenuItem("EXPAND GRID")) m_activeDialog.reset(new ExpandMapDialog());
			if (ImGui::MenuItem("SHRINK GRID")) m_activeDialog.reset(new ShrinkMapDialog());
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("VIEW"))
		{
			if (ImGui::MenuItem("MAP EDITOR"))     GetApp()->ChangeEditorMode(Mode::PLACE_TILE);
			if (ImGui::MenuItem("TEXTURE PICKER")) GetApp()->ChangeEditorMode(Mode::PICK_TEXTURE);
			if (ImGui::MenuItem("SHAPE PICKER"))   GetApp()->ChangeEditorMode(Mode::PICK_SHAPE);
			if (ImGui::MenuItem("ENTITY EDITOR"))  GetApp()->ChangeEditorMode(Mode::EDIT_ENT);
			if (ImGui::MenuItem("RESET CAMERA"))   GetApp()->ResetEditorCamera();
			if (ImGui::MenuItem("TOGGLE PREVIEW")) GetApp()->TogglePreviewing();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("CONFIG"))
		{
			if (ImGui::MenuItem("ASSET PATHS")) m_activeDialog.reset(new AssetPathDialog(m_settings));
			if (ImGui::MenuItem("SETTINGS"))    m_activeDialog.reset(new SettingsDialog(m_settings));
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("INFO"))
		{
			if (ImGui::MenuItem("ABOUT"))          m_activeDialog.reset(new AboutDialog());
			if (ImGui::MenuItem("KEYS/SHORTCUTS")) m_activeDialog.reset(new ShortcutsDialog());
			if (ImGui::MenuItem("INSTRUCTIONS"))   m_activeDialog.reset(new InstructionsDialog());
			ImGui::EndMenu();
		}

		ImGui::SameLine();
		ImGui::TextUnformatted(" | ");
		if (m_messageTimer > 0.0f)
		{
			ImGui::SameLine();
			ImGui::TextUnformatted(m_statusMessage.c_str());
		}

		ImGui::EndMainMenuBar();
	}

	//Draw modal dialogs
	if (m_activeDialog.get())
	{
		if (!m_activeDialog->Draw()) m_activeDialog.reset(nullptr);
	}
}
//-----------------------------------------------------------------------------