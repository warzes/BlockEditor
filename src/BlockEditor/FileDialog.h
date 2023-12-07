#pragma once

#include "Dialogs.h"
#include "Base.h"

class FileDialog final : public Dialog
{
public:
	// If extensions is left blank, then only directories are selectable.
	FileDialog(std::string title, std::initializer_list<std::string> extensions, std::function<void(std::filesystem::path)> callback, bool writeMode);

	bool Draw() final;
private:
	//Called when a file has been successfully selected.
	std::function<void(std::filesystem::path)> m_callback;
	std::string m_title;
	std::set<std::string> m_extensions;
	std::filesystem::path m_currentDir, m_overwriteDir;

	char m_fileNameBuffer[TEXT_FIELD_MAX];
	bool m_overwritePromptOpen, m_writeMode;
};