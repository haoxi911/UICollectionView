//
//  UICollectionView - A delegate based flow layout control
//
//  Copyright 2016 (c) Kevin Xi(kevinxi.cn@gmail.com)
//	All rights reserved.
//

#include "stdafx.h"
#include "UICollectionViewItem.h"
#include "UICollectionViewLasso.h"
#include "UICollectionViewContentView.h"
#include "UICollectionViewDelegate.h"

namespace DuiLib
{

// Constructor.
UICollectionViewItem::UICollectionViewItem() : 
	m_nIndex(-1),
	m_uMouseState(0),
	m_pCaption(nullptr),
	m_pPreview(nullptr),
	m_pContentView(nullptr)
{
}

// Save item index into item control.
void UICollectionViewItem::DoInit()
{
	m_nIndex = -1;
	m_uMouseState = 0;

	m_pCaption = dynamic_cast<CLabelUI *>(FindSubControl(UICollectionViewItemCaption));
	m_pPreview = dynamic_cast<CControlUI *>(FindSubControl(UICollectionViewItemPreview));
}

// Return TRUE if item is selected by collection view.
BOOL UICollectionViewItem::IsSelected()
{
	return (m_pContentView && m_pContentView->m_SelectionIndexes.count(m_nIndex) != 0);
}

// Handle mouse events.
void UICollectionViewItem::DoEvent(TEventUI& event)
{
	// filter mouse events.
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
		if (m_pContentView) m_pContentView->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}

	// handle item double click.
	if (event.Type == UIEVENT_DBLCLICK && IsEnabled()) {
		if (m_pContentView && m_pContentView->m_pDelegate)
			m_pContentView->m_pDelegate->CollectionViewDidDoubleClickItem(m_pContentView->m_pOwner, this, m_nIndex);
	}
	
	// handle item hover state.
	else if( event.Type == UIEVENT_MOUSEENTER && IsEnabled()) {
		m_uMouseState |= UISTATE_HOT;
		if (m_pContentView && m_pContentView->m_pDelegate && m_pContentView->m_pDelegate->\
			CollectionViewShouldDrawItemHover(m_pContentView->m_pOwner)) {
			Invalidate();
		}
	}
	else if (event.Type == UIEVENT_MOUSELEAVE && IsEnabled()) {
		if ((m_uMouseState & UISTATE_HOT) != 0) m_uMouseState &= ~UISTATE_HOT;
		if (m_pContentView && m_pContentView->m_pDelegate && m_pContentView->m_pDelegate->\
			CollectionViewShouldDrawItemHover(m_pContentView->m_pOwner)) {
			Invalidate();
		}
	}

	// forward events to content view.
	else if (m_pContentView) m_pContentView->DoEvent(event); 
	else CContainerUI::DoEvent(event);
}

// Apply item color before paint it.
bool UICollectionViewItem::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI *pStopControl)
{
	if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) return false;
	if (m_pContentView == nullptr) return false;

	UICollectionViewItemAttributes *pInfo = &m_pContentView->m_ItemAttributes;
	static const SIZE sz = {3,3}; // corner.
	m_cxyBorderRound = sz;
	m_rcBorderSize.top = m_rcBorderSize.left = m_rcBorderSize.right =
		m_rcBorderSize.bottom = pInfo->nBorderWidth;

	// support item disabled state.
	if (!IsEnabled()) {
		m_dwBackColor = pInfo->dwDisabledBkColor;
		m_dwBackColor2 = pInfo->dwDisabledBkColor;
		m_dwBorderColor = pInfo->dwDisabledBdColor;
	} 
	
	// support item selection state.
	else if (m_pContentView->m_SelectionIndexes.count(m_nIndex) != 0 && m_pContentView && m_pContentView->m_pDelegate && 
		m_pContentView->m_pDelegate->CollectionViewShouldDrawItemSelection(m_pContentView->m_pOwner)) {
		m_dwBackColor = pInfo->dwSelectedBkColor;
		m_dwBackColor2 = pInfo->dwSelectedBkColor;
		m_dwBorderColor = pInfo->dwSelectedBdColor;
	}
	
	// support item hover state.
	else if ((m_uMouseState & UISTATE_HOT) != 0 && m_pContentView && m_pContentView->m_pDelegate && 
		m_pContentView->m_pDelegate->CollectionViewShouldDrawItemHover(m_pContentView->m_pOwner)) {
		m_dwBackColor = pInfo->dwHotBkColor;
		m_dwBackColor2 = pInfo->dwHotBkColor;
		m_dwBorderColor = pInfo->dwHotBdColor;
	}

	// support default state.
	else {
		m_dwBackColor = pInfo->dwBkColor;
		m_dwBackColor2 = pInfo->dwBkColor;
		m_dwBorderColor = pInfo->dwBdColor;
	}	

	return CContainerUI::DoPaint(hDC, rcPaint, nullptr);
}

// Override to reduce paint area.
void UICollectionViewItem::Invalidate()
{
	RECT rcPaint = {0};

	if (m_pContentView) {
		// crop paint area with content view's client rect.
		if (!::IntersectRect(&rcPaint, &m_rcItem, &m_pContentView->m_rcScrollable)) 
			return;

		// crop paint area with all parents' client area.
		CControlUI *pParent = m_pContentView;
		RECT rcTemp;
		RECT rcParent;
		while (pParent = pParent->GetParent()) {
			rcTemp = rcPaint;
			rcParent = pParent->GetPos();
			if (!::IntersectRect(&rcPaint, &rcTemp, &rcParent)) 
				return;
		}

		if (m_pManager)
			m_pManager->Invalidate(rcPaint);
		return;
	}

	CContainerUI::Invalidate();
}

// Override to disable those unsupported attributes.
void UICollectionViewItem::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	// disable those unsupported attributes.
	if (_tcscmp(pstrName, _T("pos")) == 0 ||
		_tcscmp(pstrName, _T("relativepos")) == 0 ||
		_tcscmp(pstrName, _T("padding")) == 0 ||
		_tcscmp(pstrName, _T("width")) == 0 ||
		_tcscmp(pstrName, _T("height")) == 0 ||
		_tcscmp(pstrName, _T("minwidth")) == 0 ||
		_tcscmp(pstrName, _T("minheight")) == 0 ||
		_tcscmp(pstrName, _T("maxwidth")) == 0 ||
		_tcscmp(pstrName, _T("maxheight")) == 0 ||
		_tcscmp(pstrName, _T("visible")) == 0 ||
		_tcscmp(pstrName, _T("float")) == 0
		) return;

	CContainerUI::SetAttribute(pstrName, pstrValue);
}

}