#pragma once

#include "UIlib.h"

namespace DuiLib
{

class CIconUI : public CControlUI
{

public:

	// Default Constructor
	CIconUI();

	// Default Destructor
	virtual ~CIconUI();

	// Set class name for this control.
	virtual LPCTSTR GetClass() const { return L"CIconUI"; }

	// Set the icon image to display.
	virtual VOID SetIcon(HICON hIcon);

protected:

	// Paint icon as background image.
	virtual void PaintBkImage(HDC hDC);

private:

	HICON m_hIcon;
};

}
