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

// Predefined item control's name (optional use)
static const LPCTSTR UICollectionViewItemCaption		= (L"itemcaption");
static const LPCTSTR UICollectionViewItemPreview		= (L"itempreview");

// Define UI attributes for UICollectionViewItem view.
struct UICollectionViewItemAttributes
{
	// attribute					//	default value

	// item border			
	int nBorderWidth;				// 1
	DWORD dwBdColor;				// 0xFFFFFFFF
	DWORD dwHotBdColor;				// 0xFFB8D6FB
	DWORD dwSelectedBdColor;		// 0xFF84ACDD
	DWORD dwDisabledBdColor;		// 0xFFFFFFFF

	// item background
	DWORD dwBkColor;				// 0xFFFFFFFF
	DWORD dwHotBkColor;				// 0x66B8D6FB
	DWORD dwSelectedBkColor;		// 0x6684ACDD
	DWORD dwDisabledBkColor;		// 0xFFFFFFFF
};

// Default UI attributes for UICollectionViewItem view.
struct UICollectionViewItemDefaultAttributes : public UICollectionViewItemAttributes
{
	UICollectionViewItemDefaultAttributes()
	{
		dwBdColor = dwDisabledBdColor =
			dwBkColor = dwDisabledBkColor = 0xFFFFFFFF;
		nBorderWidth = 1;
		dwSelectedBkColor = 0x6684ACDD;
		dwSelectedBdColor = 0xFF84ACDD;
		dwHotBkColor = 0x66B8D6FB;
		dwHotBdColor = 0xFFB8D6FB;
	}
};

// The default implementation for item control.
class UICollectionViewContentView;
class UICollectionViewItem : public CContainerUI 
{
	friend class UICollectionViewContentView;

public:

	// Constructor.
	UICollectionViewItem();

	// UIControl class.
	LPCTSTR GetClass() const { return L"UICollectionViewItem"; }

	// Get item index.
	virtual int GetIndex() const { return m_nIndex; }

	// Get item caption control.
	virtual CLabelUI* GetCaption() { return m_pCaption; }

	// Get item preview control.
	virtual CControlUI* GetPreview() { return m_pPreview; }

	// Return TRUE if item is selected by collection view.
	virtual BOOL IsSelected();

	// Handle mouse events.
	void DoEvent(TEventUI &event);

	// Apply item color before paint it.
	bool DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl);

	// Override to reduce paint area.
	void Invalidate(); 

protected:

	// Get the related collection content view.
	UICollectionViewContentView* GetContentView() { return m_pContentView; }

	// Set the related collection content view.
	void SetContentView(UICollectionViewContentView *pContent) { m_pContentView = pContent; }

	// Save item index into item control.
	virtual void SetIndex(int nIndex) { if (nIndex >= 0) m_nIndex = nIndex; }

	// Initialize item before use or reuse.
	virtual void DoInit();

	// Override to disable those unsupported attributes.
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

private:

	int  m_nIndex; // item index within collection view.
	UINT m_uMouseState; // mouse state flags.

private:

	CLabelUI	*m_pCaption; // item label
	CControlUI	*m_pPreview; // item icon
	UICollectionViewContentView *m_pContentView;
};

}
