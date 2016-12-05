//
//  UICollectionView - A delegate based flow layout control
//
//  Copyright 2016 (c) Kevin Xi(kevinxi.cn@gmail.com)
//	All rights reserved.
//

#pragma once

#include "UIlib.h"

namespace DuiLib
{

// Define UI attributes for UICollectionViewLasso view.
struct UICollectionViewLassoAttributes
{
	// attribute					//	default value

	// lasso
	int nLassoBorderWidth;			// 1
	DWORD dwLassoBdColor;			// 0xFF3399FF
	DWORD dwLassoBkColor;			// 0x883399FF
};

// Default UI attributes for UICollectionViewLasso view.
struct UICollectionViewLassoDefaultAttributes : public UICollectionViewLassoAttributes
{
	UICollectionViewLassoDefaultAttributes()
	{
		nLassoBorderWidth = 1;
		dwLassoBkColor = 0x883399FF;
		dwLassoBdColor = 0xFF3399FF;
	}
};

// The lasso control is used to enable mouse dragging selection.
class UICollectionViewContentView;
class UICollectionViewLasso : public CControlUI
{
public:

	// Constructor.
	UICollectionViewLasso(UICollectionViewContentView *pContentView);

	// UIControl class.
	LPCTSTR GetClass() const { return L"UICollectionViewLasso"; }

	// Get the point where mouse down.
	POINT GetMouseDownPos() { return m_ptDown; }

	// Get the point where mouse move.
	POINT GetMouseMovePos() { return m_ptMove; }

	// Get the related collection content view.
	UICollectionViewContentView *GetContentView() { return m_pContentView; }

	// Set the related collection content view.
	void SetContentView(UICollectionViewContentView *pContent) { m_pContentView = pContent; }

	// Set the point where mouse down.
	void SetMouseDownPos(POINT ptDown);

	// Set the point where mouse move.
	void SetMouseMovePos(POINT ptMove);

	// Override to customize painting.
	bool DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl);

private:

	SIZE m_szScroll;
	POINT m_ptDown;
	POINT m_ptMove;
	UICollectionViewContentView *m_pContentView;

};

}
