#pragma once

#include "IMode.h"
#include "MapMan.h"

class PlaceMode : public IMode {
public:
	PlaceMode(MapMan& mapMan);
	~PlaceMode();

	virtual void Update(float deltaTime) override;
	virtual void Draw() override;
	virtual void OnEnter() override;
	virtual void OnExit() override;

	void SetCursorShape(std::shared_ptr<Assets::ModelHandle> shape);
	void SetCursorTexture(std::shared_ptr<Assets::TexHandle> tex);
	void SetCursorEnt(const Ent& ent);
	std::shared_ptr<Assets::ModelHandle> GetCursorShape() const;
	std::shared_ptr<Assets::TexHandle> GetCursorTexture() const;
	const Ent& GetCursorEnt() const;

	void ResetCamera();
	Vector3 GetCameraPosition() const;
	Vector3 GetCameraAngles() const;
	void SetCameraOrientation(Vector3 position, Vector3 angles);
	void ResetGrid();
protected:
	struct Cursor
	{
		Vector3 position;
		Vector3 endPosition;
	};

	struct TileCursor : public Cursor
	{
		std::shared_ptr<Assets::ModelHandle> model;
		std::shared_ptr<Assets::TexHandle> tex;
		int angle, pitch;

		Tile GetTile(MapMan& mapMan) const;
	};

	struct BrushCursor : public Cursor
	{
		TileGrid brush;
	};

	struct EntCursor : public Cursor
	{
		Ent ent;
	};

	void MoveCamera(float deltaTime);
	void UpdateCursor();

	MapMan& _mapMan;

	Camera3D _camera;
	float _cameraYaw;
	float _cameraPitch;
	float _cameraMoveSpeed;

	Cursor* _cursor; //The current cursor being used in the editor. Will point to one of: _tileCursor, _brushCursor, _entCursor
	TileCursor _tileCursor;
	BrushCursor _brushCursor;
	EntCursor _entCursor;

	RLMaterial _cursorMaterial; //RLMaterial used to render the cursor
	float _outlineScale; //How much the wire box around the cursor is larger than its contents

	int _layerViewMin, _layerViewMax;

	Vector3 _planeGridPos;
	Vector3 _planeWorldPos;
};