//
//  UICollectionView - A delegate based flow layout control
//
//  Copyright 2016 (c) Kevin Xi(kevinxi.cn@gmail.com)
//	All rights reserved.
//

#include "stdafx.h"
#include "UICollectionViewContentView.h"
#include "UICollectionView.h"
#include "UICollectionViewItem.h"
#include "UICollectionViewLasso.h"
#include "UICollectionViewDelegate.h"

namespace DuiLib
{

// Constructor.
UICollectionViewContentView::UICollectionViewContentView(UICollectionView *pOwner)
	:m_pOwner(pOwner), m_nCount(0), m_nColumns(0), m_nRows(0), m_uMouseState(0),
	 m_pDelegate(nullptr), m_pSelectionLasso(nullptr)
{
	ASSERT(m_pOwner);
	memset(&m_szItem, 0, sizeof(SIZE));
	memset(&m_szItemPadding, 0, sizeof(SIZE));
	memset(&m_szContent, 0, sizeof(SIZE));
	memset(&m_ptViewport, 0, sizeof(POINT));
	memset(&m_rcScrollable, 0, sizeof(RECT));

	m_ItemAttributes = UICollectionViewItemDefaultAttributes();
	m_LassoAttributes = UICollectionViewLassoDefaultAttributes();
}

// Destructor.
UICollectionViewContentView::~UICollectionViewContentView()
{
	for (auto itr = m_Items.begin(); itr != m_Items.end(); itr ++) {
		delete itr->second;
	}
	m_Items.clear();

	while (!m_ItemsPool.empty()) {
		delete m_ItemsPool.top();
		m_ItemsPool.pop();
	}

	m_SelectionIndexes.clear();
	m_LassoPersistedSelectionIndexes.clear();
	if (m_pSelectionLasso) delete m_pSelectionLasso;
}

// Get the delegate.
UICollectionViewDelegate* UICollectionViewContentView::GetDelegate() const
{
	return m_pDelegate;
}

// Set the delegate.
void UICollectionViewContentView::SetDelegate(UICollectionViewDelegate *pDelegate)
{
	m_pDelegate = pDelegate;
}

// Override this method to handle content scrolling.
void UICollectionViewContentView::SetScrollPos(SIZE szPos)
{
	if (!m_pVerticalScrollBar->IsVisible() || m_pVerticalScrollBar->GetScrollPos() == szPos.cy) {
		return;
	}
	m_pVerticalScrollBar->SetScrollPos(szPos.cy);

	// update layout and also update visible items during scroll.
	NeedUpdate();  
}

// Rewrite this method to render item controls inside `m_Items` map.
bool UICollectionViewContentView::DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl)
{
	// make sure the painting area is visible.
	RECT rcTemp = { 0 };
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) {
		return false;
	}

	CControlUI::DoPaint(hDC, rcPaint, nullptr);

#ifdef DEBUG
	//CDuiString csLog; 
	//csLog.Format(L"UICollectionView Visible Items: %ld\n", m_Items.size());
	//::OutputDebugString(csLog);
#endif // DEBUG

	// render visible items onto screen.
	for (auto itr = m_Items.begin(); itr != m_Items.end(); itr ++) {
		if (!::IntersectRect(&rcTemp, &rcPaint, &itr->second->GetPos())) continue;
		if (!::IntersectRect(&rcTemp, &m_rcScrollable, &rcTemp)) continue;
		itr->second->Paint(hDC, rcTemp, nullptr);
	}

	// render scroll bar onto screen.
	if (m_pVerticalScrollBar->IsVisible()) {
		if (::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos())) {
			m_pVerticalScrollBar->Paint(hDC, rcTemp, nullptr);
		}
	}

	// render selection lasso onto screen.
	if (m_pSelectionLasso->IsVisible() && (!m_pDelegate || m_pDelegate->CollectionViewShouldDrawItemSelection(m_pOwner))) {
		if (::IntersectRect(&rcTemp, &rcPaint, &m_pSelectionLasso->GetPos()) &&
			::IntersectRect(&rcTemp, &m_rcScrollable, &rcTemp)) {
			m_pSelectionLasso->Paint(hDC, rcTemp, nullptr);
		}
	}

	return true;
}

// Override to dynamically create / destroy / update item controls.
void UICollectionViewContentView::SetPos(RECT rc, bool bNeedInvalidate)
{
	// this is a window based axis
	CControlUI::SetPos(rc, bNeedInvalidate);

	// apply inset
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;

	// update view port
	m_ptViewport.x = rc.left; 
	m_ptViewport.y = rc.top;

	// optimize speed, return directly
	if (m_nCount == 0 || !m_pOwner || !m_pDelegate) {
		ClearVisibleItems();
		m_pVerticalScrollBar->SetVisible(false);
		m_pVerticalScrollBar->SetScrollPos(0);
		m_pVerticalScrollBar->SetScrollRange(0);
		m_rcScrollable = rc; // allow drag selection on an empty view.
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// calculate total rows and columns based on data source, support item paddings
	m_nColumns = ((rc.right - rc.left + m_szItemPadding.cx) / (m_szItem.cx + m_szItemPadding.cx));
	if (m_nColumns == 0) m_nColumns = 1;
	m_nRows = (m_nCount % m_nColumns) ? (m_nCount / m_nColumns + 1) : (m_nCount / m_nColumns);

	// calculate the scrollable content size
	m_szContent.cx = (rc.right - rc.left);
	m_szContent.cy = m_nRows * (m_szItem.cy + m_szItemPadding.cy) - m_szItemPadding.cy;
	//////////////////////////////////////////////////////////////////////////

	// re-calculate, if vertical scroll is required
	if (m_szContent.cy > (rc.bottom - rc.top)) {
		// correct the right edge
		rc.right -= m_pVerticalScrollBar->GetFixedWidth();

		//////////////////////////////////////////////////////////////////////////
		// calculate total rows and columns based on data source, support item paddings
		m_nColumns = ((rc.right - rc.left + m_szItemPadding.cx) / (m_szItem.cx + m_szItemPadding.cx));
		if (m_nColumns == 0) m_nColumns = 1;
		m_nRows = (m_nCount % m_nColumns) ? (m_nCount / m_nColumns + 1) : (m_nCount / m_nColumns);

		// calculate the scrollable content size
		m_szContent.cx = (rc.right - rc.left);
		m_szContent.cy = m_nRows * (m_szItem.cy + m_szItemPadding.cy) - m_szItemPadding.cy;
		//////////////////////////////////////////////////////////////////////////

		// correct vertical scroll bar
		RECT rcScrollBarPos = { rc.right, rc.top, rc.right + m_pVerticalScrollBar->GetFixedWidth(), rc.bottom };
		m_pVerticalScrollBar->SetPos(rcScrollBarPos);
		m_pVerticalScrollBar->SetVisible(true);
		m_pVerticalScrollBar->SetScrollRange(m_szContent.cy - (rc.bottom - rc.top));
		if (m_pVerticalScrollBar->GetScrollPos() > m_pVerticalScrollBar->GetScrollRange()) {
			m_pVerticalScrollBar->SetScrollPos(m_pVerticalScrollBar->GetScrollRange());
		}
		m_ptViewport.y -= m_pVerticalScrollBar->GetScrollPos();
	}
	else {
		// correct vertical scroll bar
		m_pVerticalScrollBar->SetVisible(false);
		m_pVerticalScrollBar->SetScrollPos(0);
		m_pVerticalScrollBar->SetScrollRange(0);
	}

	// save scrollable area rect.
	m_rcScrollable = rc;

	// calculate index range of visible items by dividing scroll pos with item height
	int nIndexFirst = (m_pVerticalScrollBar->GetScrollPos() / (m_szItem.cy + m_szItemPadding.cy)) * m_nColumns;
	if (nIndexFirst < 0) nIndexFirst = 0;
	int nIndexLast = ((m_pVerticalScrollBar->GetScrollPos() + rc.bottom - rc.top) / (m_szItem.cy + m_szItemPadding.cy) + 1) * m_nColumns - 1;
	if (nIndexLast > m_nCount - 1) nIndexLast = (m_nCount - 1);
	ASSERT(nIndexLast >= 0 && nIndexFirst >= 0 && nIndexLast >= nIndexFirst);

	// put items averagely on the X axis, an extra padding fix is required.
	int nPaddingFix = (m_szContent.cx - (m_nColumns * (m_szItem.cx + m_szItemPadding.cx) - m_szItemPadding.cx)) / \
		((m_nColumns - 1) > 0 ? (m_nColumns - 1) : 2);

	// lambda to calculate item position.
	auto GetItemPos = [&](int nRow, int nColumn) {
		// code `(m_nColumns > 1)` is used to special handle single column.
		RECT rcCell = {
			((m_nColumns > 1) ? (nColumn * (m_szItem.cx + m_szItemPadding.cx + nPaddingFix) + m_ptViewport.x) : (nPaddingFix + m_ptViewport.x)),
			nRow * (m_szItem.cy + m_szItemPadding.cy) + m_ptViewport.y,
			((m_nColumns > 1) ? (nColumn * (m_szItem.cx + m_szItemPadding.cx + nPaddingFix) + m_ptViewport.x) : (nPaddingFix + m_ptViewport.x)) + m_szItem.cx,
			nRow * (m_szItem.cy + m_szItemPadding.cy) + m_ptViewport.y + m_szItem.cy
		};

		return rcCell;
	};

	// update selection indexes with lasso selection area.
	if (m_pSelectionLasso && m_pSelectionLasso->IsVisible() && m_pDelegate->CollectionViewShouldDrawItemSelection(m_pOwner)) {
		RECT rcSel = m_pSelectionLasso->GetPos();
		RECT rcIdx = {-1, -1, -1, -1};

		// calculate selection index range in X axis.
		for (int nColumn = 0; nColumn < m_nColumns; nColumn ++) {
			RECT rcCell = GetItemPos(0, nColumn);

			// nearest cell, its right border is larger than rcSel.left.
			if (rcCell.right > rcSel.left && rcIdx.left < 0)
				rcIdx.left = nColumn; /* first matched */

			// nearest cell, its left border is smaller than rcSel.right.
			if (rcCell.left < rcSel.right)
				rcIdx.right = nColumn; /* last matched */
		}

		// calculate selection index range in Y axis.
		if (rcSel.top - m_ptViewport.y > 0) {
			rcIdx.top = (rcSel.top - m_ptViewport.y) / (m_szItem.cy + m_szItemPadding.cy);
			if ((rcSel.top - m_ptViewport.y) % (m_szItem.cy + m_szItemPadding.cy) > m_szItem.cy)
				rcIdx.top ++;
		} else {
			rcIdx.top = 0;
		}
		if (rcSel.bottom - m_ptViewport.y > 0) {
			rcIdx.bottom = (rcSel.bottom - m_ptViewport.y) / (m_szItem.cy + m_szItemPadding.cy);
		}

#ifdef DEBUG
		//CDuiString csLog;
		//csLog.Format(L"Index Range: X:%d:%d - Y:%d:%d\n", rcIdx.left, rcIdx.right, rcIdx.top, rcIdx.bottom);
		//::OutputDebugString(csLog);
#endif // DEBUG

		// save a copy of previous index set before making changes.
		std::set<int> sTempIndexes = m_SelectionIndexes;

		// CTRL is not pressed.
		if (::GetKeyState(VK_CONTROL) >= 0) { 
			m_SelectionIndexes.clear();
			if (rcIdx.left >= 0 && rcIdx.right >= 0 && rcIdx.top >= 0 && rcIdx.bottom >= 0) {
				for (int i = rcIdx.left; i <= rcIdx.right; i ++) {
					for (int j = rcIdx.top; j <= rcIdx.bottom; j ++) {
						int nIndex = (j * m_nColumns + i);
						if (nIndex < m_nCount) { /* boundary validation */
							m_SelectionIndexes.insert(nIndex);
						}
					}
				}
			}

		// CTRL can be used to do reverse selection.
		} else {
			m_SelectionIndexes = m_LassoPersistedSelectionIndexes;
			if (rcIdx.left >= 0 && rcIdx.right >= 0 && rcIdx.top >= 0 && rcIdx.bottom >= 0) {
				for (int i = rcIdx.left; i <= rcIdx.right; i ++) {
					for (int j = rcIdx.top; j <= rcIdx.bottom; j ++) {
						int nIndex = (j * m_nColumns + i);
						if (m_LassoPersistedSelectionIndexes.count(nIndex)) {
							m_SelectionIndexes.erase(nIndex);
						} else if (nIndex < m_nCount) { /* boundary validation */
							m_SelectionIndexes.insert(nIndex);
						}	
					}
				}
			}
		}

		// notify selection changes.
		if (m_SelectionIndexes != sTempIndexes) {
			m_pDelegate->CollectionViewSelectionDidChange(m_pOwner, sTempIndexes, m_SelectionIndexes);
		}
	}

	// recycle those invisible items.
	for (auto itr = m_Items.begin(); itr != m_Items.end();) {
		if (itr->first < nIndexFirst || itr->first > nIndexLast) {
			m_pDelegate->CollectionViewWillRecycleItem(m_pOwner, itr->second);
			m_ItemsPool.push(itr->second);
			itr = m_Items.erase(itr);
		} else {
			itr ++;
		}
	}

	// layout the visible items.
	for (int i = nIndexFirst; i <= nIndexLast; i ++) {
		UICollectionViewItem *pItem = nullptr;

		// make sure we are reusing the existed items in current pool.
		if (m_Items.count(i) <= 0) {
			if (!m_ItemsPool.empty()) { // use recycle pool.
				pItem = m_ItemsPool.top();
				m_ItemsPool.pop();
			} else { // create a new item control using template.
				pItem = m_pDelegate->CollectionViewReusableItemTemplate(m_pOwner);
				pItem->SetContentView(this);
				m_pManager->InitControls(pItem, this);
			}

			// request latest data via delegate, and fill it into the item.
			pItem->DoInit(); pItem->SetIndex(i);
			m_pDelegate->CollectionViewWillDisplayItem(m_pOwner, pItem, i);

			m_Items[i] = pItem;
		} else {
			pItem = m_Items[i];
		}

		// calculate item pos (zero based, row, column)
		pItem->SetPos(GetItemPos((i / m_nColumns), (i % m_nColumns)));

		// notify item layout updates.
		m_pDelegate->CollectionViewDidUpdateItemLayout(m_pOwner, pItem, i);
	}
}

// Override to forward events to UICollectionView.
void UICollectionViewContentView::DoEvent(TEventUI& event)
{
	// use timer to scroll drag selection.
	if (event.Type == UIEVENT_TIMER) {
		if (event.wParam == TIMER_SCROLLDN) { LineDown(); }
		else if (event.wParam == TIMER_SCROLLUP) { LineUp(); }
		m_pSelectionLasso->SetMouseMovePos(m_pSelectionLasso->GetMouseMovePos());
		return;
	}
	
	// filter, only handle mouse events.
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
		if (m_pParent != NULL) m_pParent->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}

	// handle mouse events.
	if (event.Type == UIEVENT_BUTTONDOWN) {
		if (::PtInRect(&m_rcScrollable, event.ptMouse) && (!m_pDelegate || m_pDelegate->CollectionViewShouldDrawItemSelection(m_pOwner))) {
			m_uMouseState |= UISTATE_CAPTURED;
			if (::GetKeyState(VK_CONTROL) >= 0)
				m_SelectionIndexes.clear();
			m_pSelectionLasso->SetMouseDownPos(event.ptMouse); // start selection.
			m_pSelectionLasso->SetVisible(true);
			m_LassoPersistedSelectionIndexes = m_SelectionIndexes;
		}

	} else if (event.Type == UIEVENT_BUTTONUP) {
		if ((m_uMouseState & UISTATE_CAPTURED) != 0)
			m_uMouseState &= ~UISTATE_CAPTURED;
		m_pManager->KillTimer(this);
		m_pSelectionLasso->SetVisible(false); // end selection.
		m_LassoPersistedSelectionIndexes.clear();

	} else if (event.Type == UIEVENT_MOUSEMOVE) {
		if ((m_uMouseState & UISTATE_CAPTURED) != 0) {
			m_pSelectionLasso->SetVisible(true); // update selection.
			if (event.ptMouse.y > m_rcScrollable.bottom)
				m_pManager->SetTimer(this, TIMER_SCROLLDN, 30);
			else if (event.ptMouse.y < m_rcScrollable.top)
				m_pManager->SetTimer(this, TIMER_SCROLLUP, 30);
			else
				m_pManager->KillTimer(this);
			POINT ptMouse = event.ptMouse;
			ptMouse.x = max(ptMouse.x, m_rcScrollable.left);
			ptMouse.x = min(ptMouse.x, m_rcScrollable.right);
			//ptMouse.y = max(ptMouse.y, m_rcScrollable.top);
			//ptMouse.y = min(ptMouse.y, m_rcScrollable.bottom);
			m_pSelectionLasso->SetMouseMovePos(ptMouse);
		}

	} else if (event.Type == UIEVENT_KEYDOWN) {
		if (::GetKeyState(VK_CONTROL) < 0 && event.chKey == 'A') {
			SelectAll(); // CTRL+A
		}

	} else {
		CContainerUI::DoEvent(event);
	}
}

// Select all items.
void UICollectionViewContentView::SelectAll()
{
	// check if item selection is enabled or not.
	if (m_pDelegate && !m_pDelegate->CollectionViewShouldDrawItemSelection(m_pOwner))
		throw std::exception("Return TRUE in delegate method `CollectionViewShouldDrawItemSelection` to enable selection.");

	// save a copy of previous index set before making changes.
	std::set<int> sTempIndexes = m_SelectionIndexes;

	m_SelectionIndexes.clear();
	for (int i = 0; i < m_nCount; i ++)
		m_SelectionIndexes.insert(i);

	// notify selection changes.
	if (m_pDelegate && m_SelectionIndexes != sTempIndexes) {
		m_pDelegate->CollectionViewSelectionDidChange(m_pOwner, sTempIndexes, m_SelectionIndexes);
	}

	Invalidate();
}

// Deselect all items.
void UICollectionViewContentView::DeselectAll()
{
	// check if item selection is enabled or not.
	if (m_pDelegate && !m_pDelegate->CollectionViewShouldDrawItemSelection(m_pOwner))
		throw std::exception("Return TRUE in delegate method `CollectionViewShouldDrawItemSelection` to enable selection.");

	// save a copy of previous index set before making changes.
	std::set<int> sTempIndexes = m_SelectionIndexes;

	m_SelectionIndexes.clear();

	// notify selection changes.
	if (m_pDelegate && m_SelectionIndexes != sTempIndexes) {
		m_pDelegate->CollectionViewSelectionDidChange(m_pOwner, sTempIndexes, m_SelectionIndexes);
	}

	Invalidate();
}

// Parse XML to configure the UI appearance.
void UICollectionViewContentView::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	// we will initialize vertical scroll bar by default.
	if (!m_pVerticalScrollBar) {
		m_pVerticalScrollBar = new CScrollBarUI;
		m_pVerticalScrollBar->SetOwner(this);
		m_pVerticalScrollBar->SetManager(m_pManager, nullptr, false);
		if (m_pManager) {
			LPCTSTR pDefaultAttributes = m_pManager->GetDefaultAttributeList(L"VScrollBar");
			if (pDefaultAttributes) {
				m_pVerticalScrollBar->SetAttributeList(pDefaultAttributes);
			}
		}
	}

	// we will initialize drag selection lasso by default.
	if (!m_pSelectionLasso) {
		m_pSelectionLasso = new UICollectionViewLasso(this);
		m_pSelectionLasso->SetManager(m_pManager, this, false);
	}

	if (_tcscmp(pstrName, _T("inset")) == 0) {
		LPTSTR pstr = NULL;
		m_rcInset.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		m_rcInset.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		m_rcInset.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		m_rcInset.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		NeedUpdate();
	} else if (_tcscmp(pstrName, _T("itemsize")) == 0) {
		LPTSTR pstr = NULL;
		m_szItem.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		m_szItem.cy = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr);     
		NeedUpdate();
	} else if (_tcscmp(pstrName, _T("itempadding")) == 0) {
		LPTSTR pstr = NULL;
		m_szItemPadding.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		m_szItemPadding.cy = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr);     
		NeedUpdate();
	} else if (_tcscmp(pstrName, _T("itembkcolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_ItemAttributes.dwBkColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("itemselectedbkcolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_ItemAttributes.dwSelectedBkColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("itemhotbkcolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_ItemAttributes.dwHotBkColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("itemdisabledbkcolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_ItemAttributes.dwDisabledBkColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("itembordersize")) == 0) {
		m_ItemAttributes.nBorderWidth = (_ttoi(pstrValue));
		Invalidate();
	} else if (_tcscmp(pstrName, _T("itembordercolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_ItemAttributes.dwBdColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("itemselectedbordercolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_ItemAttributes.dwSelectedBdColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("itemhotbordercolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_ItemAttributes.dwHotBdColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("itemdisabledbordercolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_ItemAttributes.dwDisabledBdColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("lassobkcolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_LassoAttributes.dwLassoBkColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("lassobordercolor")) == 0) {
		LPTSTR pstr = NULL;
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		m_LassoAttributes.dwLassoBdColor = _tcstoul(pstrValue, &pstr, 16);
		Invalidate();
	} else if (_tcscmp(pstrName, _T("lassobordersize")) == 0) {
		m_LassoAttributes.nLassoBorderWidth = (_ttoi(pstrValue));
		Invalidate();
	}

	CControlUI::SetAttribute(pstrName, pstrValue);
}

// Remove items from specified indexes.
bool UICollectionViewContentView::RemoveAt(std::set<int> sIndexes, BOOL bKeepSelections)
{
	// construct an index set with user selections.
	std::set<int> sTempIndexes;
	for (int i : sIndexes) {
		if (i >= 0 && i < m_nCount) {
			sTempIndexes.insert(i);
		}
	}

	// notify delegate to update data source.
	if (m_pDelegate) m_pDelegate->CollectionViewWillRemoveItemsAtIndexes(m_pOwner, sTempIndexes);

	// lambda to decrease indexes in selection index set.
	auto UpdateIndexset = [](std::set<int> &sIndexes, int nBound) {
		// find all indexes greater than the bound and decrease them by 1.
		std::set<int> sNewIndexes;
		for (auto itr = sIndexes.begin(); itr != sIndexes.lower_bound(nBound); itr ++)
			sNewIndexes.insert(*itr);
		for (auto itr = sIndexes.upper_bound(nBound); itr != sIndexes.end(); itr ++)
			sNewIndexes.insert(*itr - 1);
		sIndexes = sNewIndexes;
	};

	// lambda to decrease indexes in item map keys.
	auto UpdateItemsMap = [&](int nBound) {
		// find all indexes greater than the bound and decrease them by 1.
		std::map<int, UICollectionViewItem *> mNewMap;
		for (auto itr : m_Items) {
			if (itr.first < nBound) {
				mNewMap[itr.first] = itr.second;
			} else if (itr.first > nBound) {
				mNewMap[itr.first - 1] = itr.second;
				if (itr.second) itr.second->SetIndex(itr.first - 1);
			} else {
				if (m_pDelegate) m_pDelegate->CollectionViewWillRecycleItem(m_pOwner, itr.second);
				m_ItemsPool.push(itr.second);
			}
		}
		m_Items = mNewMap;
	};

	// simply reset item selections if `bKeepSelections` is not set.
	if (!bKeepSelections) {
		m_SelectionIndexes.clear();
		m_LassoPersistedSelectionIndexes.clear();
	}

	// loop through all indexes, and adjust them accordingly to represent the correct
	// indexes after items deletion.
	for (auto itr = sTempIndexes.rbegin(); itr != sTempIndexes.rend(); itr ++) {
		if (bKeepSelections) {
			UpdateIndexset(m_SelectionIndexes, *itr);
			UpdateIndexset(m_LassoPersistedSelectionIndexes, *itr);
		}
		UpdateItemsMap(*itr);
	}

	// reduce total count.
	m_nCount -= sTempIndexes.size();
	if (m_nCount < 0) m_nCount = 0;

	NeedUpdate();

	// notify delegate at the end of removal.
	if (m_pDelegate) m_pDelegate->CollectionViewDidRemoveItemsAtIndexes(m_pOwner, sTempIndexes);

	return true;
}

// Remove all items.
void UICollectionViewContentView::RemoveAll()
{
	// construct an index set with all items.
	std::set<int> sTempIndexes;
	for (int i = 0; i < m_nCount; i ++) {
		sTempIndexes.insert(i);
	}

	// notify delegate to update data source.
	if (m_pDelegate) m_pDelegate->CollectionViewWillRemoveItemsAtIndexes(m_pOwner, sTempIndexes);

	// empty cached items.
	ClearVisibleItems();

	// setting count to zero.
	m_nCount = 0;

	NeedUpdate();

	// notify delegate at the end of removal.
	if (m_pDelegate) m_pDelegate->CollectionViewDidRemoveItemsAtIndexes(m_pOwner, sTempIndexes);
}

// By default refresh internal cache and rebuild the whole view.
void UICollectionViewContentView::ReloadData(BOOL bFullReload)
{
	if (bFullReload) {

		// empty cached items.
		ClearVisibleItems();

		// reset vertical scroll bar.
		m_pVerticalScrollBar->SetVisible(false);
		m_pVerticalScrollBar->SetScrollPos(0);
		m_pVerticalScrollBar->SetScrollRange(0);
	}

	if (!m_pOwner || !m_pDelegate) {
		return;
	}

	// we can set item size using either delegate or XML attribute.
	SIZE szItem = m_pDelegate->CollectionViewItemSize(m_pOwner);
	if (szItem.cx > 0 && szItem.cy > 0) {
		m_szItem = szItem;
	}

	// we can set item padding using either delegate or XML attribute.
	SIZE szPadding = m_pDelegate->CollectionViewItemPadding(m_pOwner);
	if (szPadding.cx > 0 && szPadding.cy > 0) {
		m_szItemPadding = szPadding;
	}

	// we only update file count when reload is explicitly called. 
	m_nCount = m_pDelegate->CollectionViewItemsCount(m_pOwner);

	NeedUpdate();
}

// Clear all visible item controls.
void UICollectionViewContentView::ClearVisibleItems()
{
	// recycle all visible item controls.
	for (auto itr = m_Items.begin(); itr != m_Items.end(); itr ++) {
		if (m_pDelegate) m_pDelegate->CollectionViewWillRecycleItem(m_pOwner, itr->second);
		m_ItemsPool.push(itr->second);
	}
	m_Items.clear();
	m_SelectionIndexes.clear();
	m_LassoPersistedSelectionIndexes.clear();
}

// Rewrite this method to hit test item controls inside `m_Items` map.
CControlUI* UICollectionViewContentView::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
	// validate control with flags.
	if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) return nullptr;
	if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) return nullptr;
	if ((uFlags & UIFIND_HITTEST) != 0 && !::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData)))) return nullptr;

	CControlUI *pResult = nullptr;
	
	// is it the vertical scroll bar?
	if (m_pVerticalScrollBar) pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
	if (pResult) return pResult;

	// support `UIFIND_ME_FIRST` flag.
	if ((uFlags & UIFIND_ME_FIRST) != 0) {
		pResult = CControlUI::FindControl(Proc, pData, uFlags);
		if (pResult) return pResult;
	}

	// is it an item control?
	for (auto itr = m_Items.begin(); itr != m_Items.end(); itr ++) {
		CControlUI *pControl = static_cast<CControlUI *>(itr->second)->FindControl(Proc, pData, uFlags);
		if (pControl) {
			if ((uFlags & UIFIND_HITTEST) != 0 && !::PtInRect(&m_rcScrollable, *(static_cast<LPPOINT>(pData))))
				continue;
			pResult = pControl;
			break;
		} 
	}

	// return self.
	if (!pResult && (uFlags & UIFIND_ME_FIRST) == 0) pResult = CControlUI::FindControl(Proc, pData, uFlags);
	return pResult;
}

}