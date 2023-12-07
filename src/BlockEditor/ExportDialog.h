#pragma once

#include "Dialogs.h"
#include "Settings.h"
#include "FileDialog.h"

class ExportDialog final : public Dialog
{
public:
	ExportDialog(Settings& settings);
	virtual bool Draw() override;
protected:
	Settings& m_settings;
	std::unique_ptr<FileDialog> m_dialog;
	char m_filePathBuffer[TEXT_FIELD_MAX];
};