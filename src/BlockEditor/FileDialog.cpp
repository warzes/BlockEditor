#include "stdafx.h"
#include "FileDialog.h"
//-----------------------------------------------------------------------------
FileDialog::FileDialog(std::string title, std::initializer_list<std::string> extensions, std::function<void(std::filesystem::path)> callback, bool writeMode)
	: m_title(title)
	, m_extensions(extensions)
	, m_callback(callback)
	, m_currentDir(std::filesystem::current_path())
	, m_overwritePromptOpen(false)
	, m_writeMode(writeMode)
{
	memset(&m_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
}
//-----------------------------------------------------------------------------
bool FileDialog::Draw()
{
	bool open = true;
	if (!m_overwritePromptOpen) ImGui::OpenPopup(m_title.c_str());
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(m_title.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Parent directory button & other controls
		if (ImGui::Button("Parent directory") && m_currentDir.has_parent_path())
		{
			m_currentDir = m_currentDir.parent_path();
		}

		if (ImGui::BeginListBox("Files", ImVec2(504.0f, 350.0f)))
		{
			// File list
			std::filesystem::directory_iterator fileIter;
			try
			{
				fileIter = std::filesystem::directory_iterator{ m_currentDir };
			}
			catch (...)
			{
				// For some reason, the directory iterator will show special system folders that aren't even accessible in the file explorer.
				// Navigating into such a folder causes a crash, which I am preventing with this try/catch block.
				// So, going into these folders will just show nothing instead of crashing.
			}

			// Store found files and directories in these sets so that they get automatically sorted (and we can put directories in front of files)
			std::set<std::filesystem::path> foundDirectories;
			std::set<std::filesystem::path> foundFiles;

			// Get files/directories from the file system
			std::error_code osError;
			for (auto i = std::filesystem::begin(fileIter); i != std::filesystem::end(fileIter); i = i.increment(osError))
			{
				if (osError) break;
				auto entry = *i;
				if (entry.is_directory())
				{
					foundDirectories.insert(entry.path());
				}
				if (entry.is_regular_file() && m_extensions.find(entry.path().extension().string()) != m_extensions.end())
				{
					foundFiles.insert(entry.path());
				}
			}

			// Display the directories
			for (std::filesystem::path entry : foundDirectories)
			{
				std::string entry_str = std::string("[") + entry.stem().string() + "]";
				// Directories are yellow
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

				if (ImGui::Selectable(entry_str.c_str()))
				{
					m_currentDir = entry;
					memset(m_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
				}
				ImGui::PopStyleColor();
			}

			// Display the files
			for (std::filesystem::path entry : foundFiles)
			{
				std::string entry_str = entry.filename().string();
				// Files are white
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

				if (ImGui::Selectable(entry_str.c_str()))
				{
					strcpy(m_fileNameBuffer, entry_str.c_str());
				}
				ImGui::PopStyleColor();
			}

			ImGui::EndListBox();
		}

		ImGui::InputText("File name", m_fileNameBuffer, TEXT_FIELD_MAX);

		if (ImGui::Button("SELECT") && (strlen(m_fileNameBuffer) > 0 || m_extensions.empty()))
		{
			std::filesystem::path newPath = m_currentDir / m_fileNameBuffer;
			if (m_writeMode && std::filesystem::exists(newPath))
			{
				m_overwritePromptOpen = true;
				m_overwriteDir = newPath;
			}
			else
			{
				m_callback(newPath);

				ImGui::EndPopup();
				return false;
			}
		}

		ImGui::EndPopup();
	}

	if (m_overwritePromptOpen) ImGui::OpenPopup("CONFIRM OVERWRITE?");
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("CONFIRM OVERWRITE?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(m_overwriteDir.string().c_str());
		ImGui::TextUnformatted("Do you DARE overwrite this file?");

		if (ImGui::Button("YES"))
		{
			m_callback(m_overwriteDir);

			ImGui::EndPopup();
			return false;
		}
		ImGui::SameLine();
		if (ImGui::Button("NO"))
		{
			ImGui::CloseCurrentPopup();
			m_overwritePromptOpen = false;
		}

		ImGui::EndPopup();
	}

	return open;
}
//-----------------------------------------------------------------------------