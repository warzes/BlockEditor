#pragma once

class Dialog
{
public:
	virtual ~Dialog() = default;
	//Returns false when the dialog should be closed.
	virtual bool Draw() = 0;
};