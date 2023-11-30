#include "stdafx.h"
#include "Dialog.h"
#include "Temp.h"

inline Rectangle CenteredRect(float x, float y, float w, float h)
{
	return Rectangle{ x - (w / 2.0f), y - (h / 2.0f), w, h };
}

Rectangle DialogRec(float w, float h)
{
	return CenteredRect((float)GetWindowWidth() / 2.0f, (float)GetWindowHeight() / 2.0f, w, h);
}


NewMapDialog::NewMapDialog()
{
}

bool NewMapDialog::Draw()
{
	return false;
}

ExpandMapDialog::ExpandMapDialog()
{
}

bool ExpandMapDialog::Draw()
{
	return false;
}

bool ShrinkMapDialog::Draw()
{
	return false;
}

FileDialog::FileDialog(std::string title, std::initializer_list<std::string> extensions, std::function<void(std::filesystem::path)> callback, bool writeMode)
{
}

bool FileDialog::Draw()
{
	return false;
}

CloseDialog::CloseDialog()
{
}

bool CloseDialog::Draw()
{
	return false;
}

AssetPathDialog::AssetPathDialog(Settings& settings) : _settings(settings)
{
}

bool AssetPathDialog::Draw()
{
	return false;
}

SettingsDialog::SettingsDialog(Settings& settings) : _settingsOriginal(settings)
{
}

bool SettingsDialog::Draw()
{
	return false;
}

bool AboutDialog::Draw()
{
	return false;
}

bool ShortcutsDialog::Draw()
{
	return false;
}

bool InstructionsDialog::Draw()
{
	return false;
}

ExportDialog::ExportDialog(Settings& settings) : _settings(settings)
{
}

bool ExportDialog::Draw()
{
	return false;
}
