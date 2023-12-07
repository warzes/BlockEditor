#pragma once

#include "Dialogs.h"

class NewMapDialog final : public Dialog
{
public:
	NewMapDialog();
	bool Draw() final;
private:
	int m_mapDims[3];
};