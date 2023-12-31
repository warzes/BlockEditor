#pragma once

#include "IMode.h"
#include "FileDialog.h"
#include "Entity.h"

#define TEXT_FIELD_MAX 512

class EntMode : public IMode
{
public:
    EntMode();
    virtual ~EntMode() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;

    inline const Ent& GetEnt() const { return _ent; }
    inline void SetEnt(const Ent& ent) { _ent = ent; }
protected:
    std::unique_ptr<FileDialog> _fileDialog;

    Ent _ent;

    char _texturePathBuffer[TEXT_FIELD_MAX];
    char _modelPathBuffer[TEXT_FIELD_MAX];

    char _newKeyBuffer[TEXT_FIELD_MAX];
    char _newValBuffer[TEXT_FIELD_MAX];

    // Character buffers for each property's value (indexed by the key)
    std::map<std::string, char*> _valBuffers;
};