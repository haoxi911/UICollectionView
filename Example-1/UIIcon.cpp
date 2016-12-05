#include "stdafx.h"
#include "UIIcon.h"

namespace DuiLib
{

// Default Constructor
CIconUI::CIconUI() : m_hIcon(NULL)
{
}

// Default Destructor
CIconUI::~CIconUI()
{
	::DestroyIcon(m_hIcon);
}

// Set the icon image to display.
VOID CIconUI::SetIcon(HICON hIcon)
{
	if (m_hIcon != hIcon) {
		::DestroyIcon(m_hIcon);
		m_hIcon = hIcon;
		Invalidate();
	}
}

// Paint icon as background image.
void CIconUI::PaintBkImage(HDC hDC)
{
	if (!m_hIcon) return;

	// It is better if we can draw the icons using CRenderEngine.
	::DrawIconEx(
		hDC, m_rcItem.left, m_rcItem.top, m_hIcon, m_rcItem.right - m_rcItem.left, 
		m_rcItem.bottom - m_rcItem.top, 0, 0, DI_NORMAL
		);
}

}

