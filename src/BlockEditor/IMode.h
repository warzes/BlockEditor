#pragma once

enum class EditorMode
{
	PLACE_TILE,
	PICK_TEXTURE,
	PICK_SHAPE,
	EDIT_ENT
};

//Mode implementation
class ModeImpl
{
public:
	inline virtual ~ModeImpl() {}
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void OnEnter() = 0;
	virtual void OnExit() = 0;
};