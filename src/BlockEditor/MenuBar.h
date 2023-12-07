#pragma once

#include "Settings.h"
#include "Dialogs.h"

class MenuBar final
{
public:
	MenuBar(Settings& settings);
	void Update(float deltaTime);
	void Draw();
	void OpenSaveMapDialog();
	void OpenOpenMapDialog();
	void SaveMap();

	void DisplayStatusMessage(std::string message, float durationSeconds, int priority);
protected:
	Settings& m_settings;

	std::unique_ptr<Dialog> m_activeDialog;

	std::string m_statusMessage;
	float m_messageTimer;
	int m_messagePriority;
};