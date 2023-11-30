#pragma once

#define TEXT_FIELD_MAX 512

enum class Direction { Z_POS, Z_NEG, X_POS, X_NEG, Y_POS, Y_NEG };

inline bool IsEngineClose()
{
	// TODO: сделать диалог спрашивающий о том что нужно ли завершать программу
	return false;
}