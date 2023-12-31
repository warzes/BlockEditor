#pragma once

class IMode
{
public:
	virtual ~IMode() = default;

	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;
	virtual void OnEnter() = 0;
	virtual void OnExit() = 0;
};

enum class Mode { PLACE_TILE, PICK_TEXTURE, PICK_SHAPE, EDIT_ENT };