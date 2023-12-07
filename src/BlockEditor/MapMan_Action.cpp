#include "stdafx.h"
#include "MapMan.h"
#include "EditorApp.h"

// ======================================================================
// TILE ACTION
// ======================================================================

MapMan::TileAction::TileAction(size_t i, size_t j, size_t k, TileGrid prevState, TileGrid newState)
	: _i(i), _j(j), _k(k),
	_prevState(prevState),
	_newState(newState)
{}

void MapMan::TileAction::Do(MapMan& map) const
{
	map._tileGrid.CopyTiles(_i, _j, _k, _newState);
}

void MapMan::TileAction::Undo(MapMan& map) const
{
	map._tileGrid.CopyTiles(_i, _j, _k, _prevState);
}

// ======================================================================
// ENT ACTION
// ======================================================================

MapMan::EntAction::EntAction(size_t i, size_t j, size_t k, bool overwrite, bool removed, Ent oldEnt, Ent newEnt)
	: _i(i), _j(j), _k(k),
	_overwrite(overwrite),
	_removed(removed),
	_oldEnt(oldEnt),
	_newEnt(newEnt)
{}

void MapMan::EntAction::Do(MapMan& map) const
{
	if (_removed)
	{
		map._entGrid.RemoveEnt(_i, _j, _k);
	}
	else
	{
		map._entGrid.AddEnt(_i, _j, _k, _newEnt);
	}
}

void MapMan::EntAction::Undo(MapMan& map) const
{
	if (_overwrite || _removed)
	{
		map._entGrid.AddEnt(_i, _j, _k, _oldEnt);
	}
	else
	{
		map._entGrid.RemoveEnt(_i, _j, _k);
	}
}