#pragma once

#include "Dialogs.h"
#include "Settings.h"

class SettingsDialog final : public Dialog
{
public:
	SettingsDialog(Settings& settings);
	bool Draw() final;
protected:
	Settings& m_settingsOriginal;
	Settings m_settingsCopy;
};