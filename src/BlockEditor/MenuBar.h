#pragma once

#include "Settings.h"

class MenuBar
{
public:
	MenuBar(Settings& settings);
	void Update();
	void Draw();
	void OpenSaveMapDialog();
	void OpenOpenMapDialog();
	void SaveMap();

	void DisplayStatusMessage(std::string message, float durationSeconds, int priority);
protected:
	Settings& _settings;

	std::unique_ptr<Dialog> _activeDialog;

	std::string _statusMessage;
	float _messageTimer;
	int _messagePriority;
};