#pragma once

#include "Dialogs.h"
#include "Settings.h"
#include "FileDialog.h"

class AssetPathDialog final : public Dialog
{
public:
	AssetPathDialog(Settings& settings);
	bool Draw() final;
private:
	Settings& m_settings;
	char m_texDirBuffer[TEXT_FIELD_MAX];
	char m_shapeDirBuffer[TEXT_FIELD_MAX];
	char m_defaultTexBuffer[TEXT_FIELD_MAX];
	char m_defaultShapeBuffer[TEXT_FIELD_MAX];
	std::unique_ptr<FileDialog> m_fileDialog;
};