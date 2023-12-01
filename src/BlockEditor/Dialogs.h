#pragma once

#include "Tile.h"
#include "Entity.h"
#include "Core.h"
#include "Settings.h"

class Dialog
{
public:
    inline virtual ~Dialog() {}
    //Returns false when the dialog should be closed.
    virtual bool Draw() = 0;
};

class NewMapDialog : public Dialog
{
public:
    NewMapDialog();
    virtual bool Draw() override;
protected:
    int _mapDims[3];
};

class ExpandMapDialog : public Dialog
{
public:
    ExpandMapDialog();
    virtual bool Draw() override;
protected:
    bool _chooserActive;
    bool _spinnerActive;
    int _amount;
    Direction _direction;
};

class ShrinkMapDialog : public Dialog
{
public:
    virtual bool Draw() override;
};

class FileDialog : public Dialog
{
public:
    // If extensions is left blank, then only directories are selectable.
    FileDialog(std::string title, std::initializer_list<std::string> extensions, std::function<void(std::filesystem::path)> callback, bool writeMode);

    virtual bool Draw() override;
protected:
    //Called when a file has been successfully selected.
    std::function<void(std::filesystem::path)> _callback;
    std::string _title;
    std::set<std::string> _extensions;
    std::filesystem::path _currentDir, _overwriteDir;

    char _fileNameBuffer[TEXT_FIELD_MAX];
    bool _overwritePromptOpen, _writeMode;
};

class CloseDialog : public Dialog
{
public:
    CloseDialog();
    virtual bool Draw() override;
protected:
    int _messageIdx;
};

class AssetPathDialog : public Dialog
{
public:
    AssetPathDialog(Settings& settings);
    virtual bool Draw() override;
protected:
    Settings& _settings;
    char _texDirBuffer[TEXT_FIELD_MAX];
    char _shapeDirBuffer[TEXT_FIELD_MAX];
    char _defaultTexBuffer[TEXT_FIELD_MAX];
    char _defaultShapeBuffer[TEXT_FIELD_MAX];
    std::unique_ptr<FileDialog> _fileDialog;
};

class SettingsDialog : public Dialog
{
public:
    SettingsDialog(Settings& settings);
    virtual bool Draw() override;
protected:
    Settings& _settingsOriginal;
    Settings _settingsCopy;
};

class AboutDialog : public Dialog
{
public:
    virtual bool Draw() override;
};

class ShortcutsDialog : public Dialog
{
public:
    virtual bool Draw() override;
};

class InstructionsDialog : public Dialog
{
public:
    virtual bool Draw() override;
};

class ExportDialog : public Dialog
{
public:
    ExportDialog(Settings& settings);
    virtual bool Draw() override;
protected:
    Settings& _settings;
    std::unique_ptr<FileDialog> _dialog;
    char _filePathBuffer[TEXT_FIELD_MAX];
};