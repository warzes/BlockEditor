#pragma once

#include "Dialogs.h"
#include "Base.h"

class ExpandMapDialog final : public Dialog
{
public:
	ExpandMapDialog();
	bool Draw() final;
private:
	bool m_chooserActive;
	bool m_spinnerActive;
	int m_amount;
	Direction m_direction;
};