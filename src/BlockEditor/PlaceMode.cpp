#include "stdafx.h"
#include "PlaceMode.h"
#include "EditorApp.h"
#include "RL.h"
#include "RLMath.h"

#define CAMERA_PITCH_LIMIT (PI / 2.5f)
#define CAMERA_MOVE_SPEED_MIN   8.0f
#define CAMERA_MOVE_SPEED_MAX   64.0f
#define CAMERA_ACCELERATION     16.0f

Tile PlaceMode::TileCursor::GetTile(MapMan& mapMan) const
{
	return Tile(
		mapMan.GetOrAddModelID(model->GetPath()),
		angle,
		mapMan.GetOrAddTexID(tex->GetPath()),
		pitch
	);
}

PlaceMode::PlaceMode(MapMan& mapMan)
	: _mapMan(mapMan),
	_layerViewMax(mapMan.Tiles().GetHeight() - 1),
	_layerViewMin(0)
{
	//Setup camera
	_camera = { 0 };
	_camera.up = VEC3_UP;
	_camera.fovy = 70.0f;
	_camera.projection = CAMERA_PERSPECTIVE;
	_cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;

	//Setup cursors
	_tileCursor.tex = nullptr;
	_tileCursor.model = nullptr;
	_tileCursor.angle = _tileCursor.pitch = 0;

	_brushCursor.brush = TileGrid(&mapMan, 1, 1, 1);

	_entCursor.ent = Ent(1.0f);
	_entCursor.ent.properties["name"] = "entity";

	_cursor = &_tileCursor;
	_cursor->position = _tileCursor.endPosition = Vector3Zero();
	_outlineScale = 1.125f;

	_cursorMaterial = LoadMaterialDefault();
	_cursorMaterial.shader = Assets::GetMapShader(false);

	ResetCamera();
	ResetGrid();
}

PlaceMode::~PlaceMode()
{
	//Free the cursor material
	//Do not use UnloadMaterial, because it will free the texture that the material uses, which might be being used somewhere else.
	RL_FREE(_cursorMaterial.maps);
}

void PlaceMode::SetCursorShape(std::shared_ptr<Assets::ModelHandle> shape)
{
	_tileCursor.model = shape;
	_cursor = &_tileCursor;
}

void PlaceMode::SetCursorTexture(std::shared_ptr<Assets::TexHandle> tex)
{
	_tileCursor.tex = tex;
	_cursor = &_tileCursor;
}

void PlaceMode::SetCursorEnt(const Ent& ent)
{
	_entCursor.ent = ent;
	_cursor = &_entCursor;
}

std::shared_ptr<Assets::ModelHandle> PlaceMode::GetCursorShape() const
{
	return _tileCursor.model;
}

std::shared_ptr<Assets::TexHandle> PlaceMode::GetCursorTexture() const
{
	return _tileCursor.tex;
}

const Ent& PlaceMode::GetCursorEnt() const
{
	return _entCursor.ent;
}

void PlaceMode::OnEnter()
{
	if (_tileCursor.tex == nullptr || _tileCursor.model == nullptr)
	{
		_tileCursor.tex = Assets::GetTexture(GetApp()->GetDefaultTexturePath());
		_tileCursor.model = Assets::GetModel(GetApp()->GetDefaultShapePath());
	}
}

void PlaceMode::OnExit()
{
}

void PlaceMode::ResetCamera()
{
	_camera.position = _mapMan.Tiles().GetCenterPos();
	_camera.target = _camera.position;
	_cameraYaw = 0.0f;
	_cameraPitch = -PI / 4.0f;
	_cursor->position = _tileCursor.endPosition = Vector3Zero();
}

Vector3 PlaceMode::GetCameraPosition() const
{
	return _camera.position;
}

Vector3 PlaceMode::GetCameraAngles() const
{
	return Vector3{ _cameraPitch, _cameraYaw, 0.0f };
}

void PlaceMode::SetCameraOrientation(Vector3 position, Vector3 angles)
{
	_camera.position = position;
	_cameraPitch = angles.x;
	_cameraYaw = angles.y;
}

void PlaceMode::ResetGrid()
{
	//Editor grid and plane
	_planeGridPos = Vector3{ (float)_mapMan.Tiles().GetWidth() / 2.0f, 0, (float)_mapMan.Tiles().GetLength() / 2.0f };
	_planeWorldPos = _mapMan.Tiles().GridToWorldPos(_planeGridPos, false);
	_layerViewMin = 0;
	_layerViewMax = _mapMan.Tiles().GetHeight() - 1;
	//Cursor
	_cursor->position = _tileCursor.endPosition = Vector3Zero();
	_cursor = &_tileCursor;
}

void PlaceMode::MoveCamera(float deltaTime)
{
	//Camera3D controls
	Vector3 cameraMovement = Vector3Zero();
	auto& input = GetInputSystem();
	if (input.IsKeyDown(Input::KEY_D))
	{
		cameraMovement.x = 1.0f;
	}
	else if (input.IsKeyDown(Input::KEY_A))
	{
		cameraMovement.x = -1.0f;
	}

	if (input.IsKeyDown(Input::KEY_W))
	{
		cameraMovement.z = -1.0f;
	}
	else if (input.IsKeyDown(Input::KEY_S))
	{
		cameraMovement.z = 1.0f;
	}

	if (input.IsKeyDown(Input::KEY_SPACE))
	{
		cameraMovement.y = 1.0f;
	}
	else if (input.IsKeyDown(Input::KEY_C))
	{
		cameraMovement.y = -1.0f;
	}

	if (input.IsKeyDown(Input::KEY_D) || input.IsKeyDown(Input::KEY_A) || input.IsKeyDown(Input::KEY_W) || input.IsKeyDown(Input::KEY_S))
	{
		_cameraMoveSpeed += CAMERA_ACCELERATION * deltaTime;
	}
	else
	{
		_cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;
	}
	_cameraMoveSpeed = Clamp(_cameraMoveSpeed, CAMERA_MOVE_SPEED_MIN, CAMERA_MOVE_SPEED_MAX);

	if (input.IsMouseButtonDown(Input::MOUSE_MIDDLE) || (input.IsMouseButtonDown(Input::MOUSE_LEFT) && input.IsKeyDown(Input::KEY_LEFT_ALT)))
	{
		_cameraYaw -= input.GetMouseDeltaPosition().x * GetApp()->GetMouseSensitivity() * deltaTime;
		_cameraPitch -= input.GetMouseDeltaPosition().y * GetApp()->GetMouseSensitivity() * deltaTime;
		_cameraPitch = Clamp(_cameraPitch, -CAMERA_PITCH_LIMIT, CAMERA_PITCH_LIMIT);
	}

	cameraMovement = Vector3Scale(Vector3Normalize(cameraMovement), _cameraMoveSpeed * deltaTime);
	Matrix cameraRotation = MatrixMultiply(MatrixRotateX(_cameraPitch), MatrixRotateY(_cameraYaw));
	Vector3 rotatedMovement = Vector3Transform(cameraMovement, cameraRotation);

	_camera.position = Vector3Add(_camera.position, rotatedMovement);
	_camera.target = Vector3Add(_camera.position, Vector3Transform(VEC3_FORWARD, cameraRotation));

	// Set the default camera orientation when the map is saved
	_mapMan.SetDefaultCameraPosition(_camera.position);
	_mapMan.SetDefaultCameraAngles(Vector3{ _cameraPitch, _cameraYaw, 0.0f });
}

//TODO: Separate cursor updates into virtual functions
void PlaceMode::UpdateCursor()
{
	auto& input = GetInputSystem();
	if (input.IsKeyPressed(Input::KEY_ESCAPE) || input.IsKeyPressed(Input::KEY_BACKSPACE))
	{
		_cursor = &_tileCursor;
	}
	else if (input.IsKeyPressed(Input::KEY_E) && input.IsKeyDown(Input::KEY_LEFT_CONTROL))
	{
		_cursor = &_entCursor;
	}

	//Position cursor
	RLRay pickRay = GetMouseRay({ input.GetMousePosition().x, input.GetMousePosition().y }, _camera);
	Vector3 gridMin = _mapMan.Tiles().GetMinCorner();
	Vector3 gridMax = _mapMan.Tiles().GetMaxCorner();
	RayCollision col = GetRayCollisionQuad(pickRay,
		Vector3{ gridMin.x, _planeWorldPos.y, gridMin.z },
		Vector3{ gridMax.x, _planeWorldPos.y, gridMin.z },
		Vector3{ gridMax.x, _planeWorldPos.y, gridMax.z },
		Vector3{ gridMin.x, _planeWorldPos.y, gridMax.z });
	if (col.hit)
	{
		_cursor->position = _mapMan.Tiles().SnapToCelCenter(col.point);
		_cursor->position.y = _planeWorldPos.y + _mapMan.Tiles().GetSpacing() / 2.0f;
	}

	//Handle box selection/fill in tile mode.
	bool multiSelect = false;
	if (_cursor == &_tileCursor)
	{
		if (!input.IsKeyDown(Input::KEY_LEFT_SHIFT))
		{
			_tileCursor.endPosition = _tileCursor.position;
		}
		else
		{
			multiSelect = true;
		}
	}

	//Perform Tile operations
	Vector3 cursorStartGridPos = _mapMan.Tiles().WorldToGridPos(_cursor->position);
	Vector3 cursorEndGridPos = _mapMan.Tiles().WorldToGridPos(_cursor->endPosition);
	Vector3 gridPosMin = Vector3Min(cursorStartGridPos, cursorEndGridPos);
	Vector3 gridPosMax = Vector3Max(cursorStartGridPos, cursorEndGridPos);
	size_t i = (size_t)gridPosMin.x;
	size_t j = (size_t)gridPosMin.y;
	size_t k = (size_t)gridPosMin.z;
	size_t w = (size_t)gridPosMax.x - i + 1;
	size_t h = (size_t)gridPosMax.y - j + 1;
	size_t l = (size_t)gridPosMax.z - k + 1;
	Tile underTile = _mapMan.Tiles().GetTile(i, j, k); // * hi. my name's sans undertile...you get it?

	//Press Shift+B to enter brush mode, copying the currently selected rectangle of tiles.
	if (_cursor == &_tileCursor && input.IsKeyPressed(Input::KEY_B) && input.IsKeyDown(Input::KEY_LEFT_SHIFT))
	{
		_cursor = &_brushCursor;
		_brushCursor.brush = _mapMan.Tiles().Subsection(i, j, k, w, h, l);
		_brushCursor.position = _tileCursor.position;
		_brushCursor.endPosition = _tileCursor.endPosition;
	}

	if (_cursor == &_brushCursor)
	{
		//Put the end position at the other extent of the bounding box so that a border can be drawn later
		_brushCursor.endPosition = Vector3{
		_brushCursor.position.x + ((_brushCursor.brush.GetWidth() - 1) * _brushCursor.brush.GetSpacing()),
		_brushCursor.position.y + ((_brushCursor.brush.GetHeight() - 1) * _brushCursor.brush.GetSpacing()),
		_brushCursor.position.z + ((_brushCursor.brush.GetLength() - 1) * _brushCursor.brush.GetSpacing()),
		};
		if (input.IsMouseButtonPressed(Input::MOUSE_LEFT) && !input.IsKeyDown(Input::KEY_LEFT_ALT))
		{
			_mapMan.ExecuteTileAction(i, j, k, w, h, l, _brushCursor.brush);
		}
	}
	else if (_cursor == &_tileCursor)
	{
		//Rotate cursor
		if (input.IsKeyPressed(Input::KEY_Q))
		{
			_tileCursor.angle = OffsetDegrees(_tileCursor.angle, -90);
		}
		else if (!input.IsKeyDown(Input::KEY_LEFT_CONTROL) && input.IsKeyPressed(Input::KEY_E))
		{
			_tileCursor.angle = OffsetDegrees(_tileCursor.angle, 90);
		}
		if (input.IsKeyPressed(Input::KEY_F))
		{
			_tileCursor.pitch = OffsetDegrees(_tileCursor.pitch, 90);
		}
		else if (input.IsKeyPressed(Input::KEY_V))
		{
			_tileCursor.pitch = OffsetDegrees(_tileCursor.pitch, -90);
		}
		//Reset tile orientation
		if (input.IsKeyPressed(Input::KEY_R))
		{
			_tileCursor.angle = _tileCursor.pitch = 0;
		}

		Tile cursorTile = _tileCursor.GetTile(_mapMan);

		if (input.IsMouseButtonDown(Input::MOUSE_LEFT) && !input.IsKeyDown(Input::KEY_LEFT_ALT) && !multiSelect)
		{
			//Place tiles
			if (underTile != cursorTile)
			{
				_mapMan.ExecuteTileAction(i, j, k, 1, 1, 1, cursorTile);
			}
		}
		else if (input.IsMouseButtonReleased(Input::MOUSE_LEFT) && multiSelect)
		{
			//Place tiles rectangle
			_mapMan.ExecuteTileAction(i, j, k, w, h, l, cursorTile);
		}
		else if (input.IsMouseButtonDown(Input::MOUSE_RIGHT) && !multiSelect)
		{
			//Remove tiles
			if (underTile)
			{
				_mapMan.ExecuteTileAction(i, j, k, 1, 1, 1, Tile{ NO_MODEL, 0, NO_TEX, false });
			}
		}
		else if (input.IsMouseButtonReleased(Input::MOUSE_RIGHT) && multiSelect)
		{
			//Remove tiles RECTANGLE
			_mapMan.ExecuteTileAction(i, j, k, w, h, l, Tile{ NO_MODEL, 0, NO_TEX, false });
		}
		else if (input.IsKeyPressed(Input::KEY_G) && !multiSelect)
		{
			//(G)rab the shape from the tile under the cursor
			if (underTile)
			{
				std::cout << "ID: " << underTile.shape << std::endl;
				auto path = _mapMan.PathFromModelID(underTile.shape);
				std::cout << "PATH: " << path << std::endl;
				_tileCursor.model = Assets::GetModel(path);
				_tileCursor.angle = underTile.angle;
				_tileCursor.pitch = underTile.pitch;
			}
		}
		else if (input.IsKeyPressed(Input::KEY_T) && !multiSelect)
		{
			//Pick the (T)exture from the tile under the cursor.
			if (underTile)
			{
				std::cout << "ID: " << underTile.texture << std::endl;
				auto path = _mapMan.PathFromTexID(underTile.texture);
				std::cout << "PATH: " << path << std::endl;
				_tileCursor.tex = Assets::GetTexture(path);
			}
		}
	}
	else if (_cursor == &_entCursor)
	{
		_entCursor.endPosition = _entCursor.position;
		_entCursor.ent.position = _entCursor.position;

		//Turn entity
		const int ANGLE_INC = input.IsKeyDown(Input::KEY_LEFT_SHIFT) ? 15 : 45;
		if (input.IsKeyPressed(Input::KEY_Q))
		{
			_entCursor.ent.yaw = OffsetDegrees(_entCursor.ent.yaw, -ANGLE_INC);
		}
		else if (!input.IsKeyDown(Input::KEY_LEFT_CONTROL) && input.IsKeyPressed(Input::KEY_E))
		{
			_entCursor.ent.yaw = OffsetDegrees(_entCursor.ent.yaw, ANGLE_INC);
		}
		if (input.IsKeyPressed(Input::KEY_F))
		{
			_entCursor.ent.pitch = OffsetDegrees(_entCursor.ent.pitch, ANGLE_INC);
		}
		else if (input.IsKeyPressed(Input::KEY_V))
		{
			_entCursor.ent.pitch = OffsetDegrees(_entCursor.ent.pitch, -ANGLE_INC);
		}

		//Reset ent orientation
		if (input.IsKeyPressed(Input::KEY_R))
		{
			_entCursor.ent.yaw = _entCursor.ent.pitch = 0;
		}

		if (input.IsMouseButtonPressed(Input::MOUSE_LEFT) && !input.IsKeyDown(Input::KEY_LEFT_ALT))
		{
			//Placement
			_mapMan.ExecuteEntPlacement(i, j, k, _entCursor.ent);
		}
		else if (input.IsMouseButtonPressed(Input::MOUSE_RIGHT))
		{
			//Removal
			if (_mapMan.Ents().HasEnt(i, j, k))
			{
				_mapMan.ExecuteEntRemoval(i, j, k);
			}
		}

		if (input.IsKeyPressed(Input::KEY_T) || input.IsKeyPressed(Input::KEY_G))
		{
			//Copy the entity under the cursor
			if (_mapMan.Ents().HasEnt(i, j, k))
			{
				_entCursor.ent = _mapMan.Ents().GetEnt(i, j, k);
			}
		}
	}
}

void PlaceMode::Update(float deltaTime)
{
	auto& input = GetInputSystem();

	// Don't update this when using the GUI
	if (auto io = ImGui::GetIO(); io.WantCaptureMouse || io.WantCaptureKeyboard)
	{
		return;
	}

	MoveCamera(deltaTime);

	if (!GetApp()->IsPreviewing())
	{
		//Move editing plane
		if (input.GetMouseWheelMove() > EPSILON || input.GetMouseWheelMove() < -EPSILON)
		{
			_planeGridPos.y = Clamp(_planeGridPos.y + Sign(input.GetMouseWheelMove()), 0.0f, _mapMan.Tiles().GetHeight() - 1);

			//Reveal hidden layers when the grid is over them and holding H
			if (input.IsKeyDown(Input::KEY_H))
			{
				int planeLayer = (int)_planeGridPos.y;
				if (planeLayer < _layerViewMin) _layerViewMin = planeLayer;
				if (planeLayer > _layerViewMax) _layerViewMax = planeLayer;
			}
			else
			{
				//Prevent the user from editing hidden layers.
				_planeGridPos.y = Clamp(_planeGridPos.y, _layerViewMin, _layerViewMax);
			}
		}
		_planeWorldPos = _mapMan.Tiles().GridToWorldPos(_planeGridPos, false);

		//Layer hiding
		if (input.IsKeyPressed(Input::KEY_H))
		{
			if (_layerViewMin == 0 && _layerViewMax == _mapMan.Tiles().GetHeight() - 1)
			{
				_layerViewMax = _layerViewMin = (int)_planeGridPos.y;
			}
			else
			{
				_layerViewMin = 0;
				_layerViewMax = _mapMan.Tiles().GetHeight() - 1;
			}
		}

		if (_layerViewMin > 0 || _layerViewMax < _mapMan.Tiles().GetHeight() - 1)
		{
			GetApp()->DisplayStatusMessage("PRESS H TO UNHIDE LAYERS", 0.25f, 1);
		}

		//Update cursor
		UpdateCursor();

		//Undo and redo
		if (input.IsKeyDown(Input::KEY_LEFT_CONTROL))
		{
			if (input.IsKeyPressed(Input::KEY_Z)) _mapMan.Undo();
			else if (input.IsKeyPressed(Input::KEY_Y)) _mapMan.Redo();
		}
	}
}

void PlaceMode::Draw()
{
	auto& input = GetInputSystem();

	//BeginMode3D(_camera);
	{

		//Draw map
		if (!GetApp()->IsPreviewing())
		{
			// Grid
			//rlDrawRenderBatchActive();
			//rlSetLineWidth(1.0f);
			//DrawGridEx(
			//    Vector3Add(_planeWorldPos, Vector3{ 0.0f, 0.05f, 0.0f }), //Adding the offset to prevent Z-fighting 
			//    _mapMan.Tiles().GetWidth() + 1, _mapMan.Tiles().GetLength() + 1,
			//    _mapMan.Tiles().GetSpacing());
			//// We need to draw the grid BEFORE the map, or there will be visual errors with entity billboards
			//rlDrawRenderBatchActive();
		}

		_mapMan.DrawMap(_camera, _layerViewMin, _layerViewMax);

		if (!GetApp()->IsPreviewing())
		{
			//Draw cursor
			if (_cursor == &_tileCursor)
			{
				if (!input.IsKeyDown(Input::KEY_LEFT_SHIFT))
				{
					Matrix cursorTransform = MatrixMultiply(
						TileRotationMatrix(_tileCursor.GetTile(_mapMan)),
						MatrixTranslate(_cursor->position.x, _cursor->position.y, _cursor->position.z));

					_cursorMaterial.maps[MATERIAL_MAP_ALBEDO].texture = _tileCursor.tex->GetTexture();
					const RLModel& shape = _tileCursor.model->GetModel();
					for (size_t m = 0; m < shape.meshCount; ++m)
					{
						DrawMesh(shape.meshes[m], _cursorMaterial, cursorTransform);
					}
				}
			}
			else if (_cursor == &_brushCursor)
			{
				// Draw the tile grid within the brush
				Vector3 brushOffset = Vector3Min(_brushCursor.position, _brushCursor.endPosition);
				brushOffset.x -= _brushCursor.brush.GetSpacing() / 2.0f;
				brushOffset.y -= _brushCursor.brush.GetSpacing() / 2.0f;
				brushOffset.z -= _brushCursor.brush.GetSpacing() / 2.0f;
				_brushCursor.brush.Draw(brushOffset);
			}
			else if (_cursor == &_entCursor)
			{
				_entCursor.ent.Draw(true);
			}

			// Draw pink border around the cursor
			/*rlDrawRenderBatchActive();
			rlDisableDepthTest();
			rlSetLineWidth(2.0f);
			DrawCubeWires(
			Vector3Scale(Vector3Add(_cursor->position, _cursor->endPosition), 0.5f),
			fabs(_cursor->endPosition.x - _cursor->position.x) + _mapMan.Tiles().GetSpacing() * _outlineScale,
			fabs(_cursor->endPosition.y - _cursor->position.y) + _mapMan.Tiles().GetSpacing() * _outlineScale,
			fabs(_cursor->endPosition.z - _cursor->position.z) + _mapMan.Tiles().GetSpacing() * _outlineScale,
			MAGENTA);*/

			/*  DrawAxes3D(Vector3{ 1.0f, 1.0f, 1.0f }, 10.0f);
			rlDrawRenderBatchActive();
			rlEnableDepthTest();*/
		}
	}
	//EndMode3D();

	_mapMan.Draw2DElements(_camera, _layerViewMin, _layerViewMax);
}