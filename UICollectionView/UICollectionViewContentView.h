//
//  UICollectionView - A delegate based flow layout control
//
//  Copyright 2016 (c) Kevin Xi(kevinxi.cn@gmail.com)
//	All rights reserved.
//

#pragma once

#include "UIlib.h"
#include "UICollectionViewItem.h"
#include "UICollectionViewLasso.h"
#include <map>
#include <set>
#include <stack>

namespace DuiLib
{

// Design Defects:
// 1. We intend to build the content view upon CControlUI instead of CContainerUI, it is
//    because we are going to virtualize and reuse item controls within this container, which 
//    obviously doesn't obey most of interfaces defined in IContainerUI. However, we still
//	  need to use CScrollBarUI to manage content scrolling, currently the CScrollBarUI is only
//    designated to work within a CContainerUI. In order to reuse CScrollBarUI, we decided to
//    build this content view upon CContainerUI, but we will disable most of its interfaces by
//    throwing exceptions at runtime.
//
// 2. We are recycling item controls internally, in order to effectively locate the existed or
//    reusable item controls, we need to replace current `m_items` pointer array in CContainerUI 
//    with std::map<int, UICollectionViewItemPtr>, there were a couple of methods are currently 
//    using with `m_items` array, we will either override or rewrite them.
//
// 3. We start drag selection on top of an item control, and drag scrolling the content view until
//    moving that item control out of the screen. We are virtualizing those item controls, usually 
//    we handle this scenario using two approaches, we either deallocate the invisible items once  
//    scroll them out of the screen, or we keep them in a pool and reuse them to display data of new
//    appeared items. Here we will use the 2nd approach, since after looking into UIManager.cpp, we
//    realized when paint manager handle a `WM_LBUTTONDOWN` event, it will save a reference of the sender
//    of the event, and the paired `WM_LBUTTONUP` event will be fired only if the saved sender control
//    is still available (i.e. we have to keep it in memory).
//
class UICollectionView;
class UICollectionViewItem;
class UICollectionViewLasso;
class UICollectionViewDelegate;
class UICollectionViewContentView : public CContainerUI
{
	friend class UICollectionViewItem;
	friend class UICollectionViewLasso;

public:

	// Constructor.
	UICollectionViewContentView(UICollectionView *pOwner);

	// Destructor.
	~UICollectionViewContentView();

	// UIControl class.
	LPCTSTR GetClass() const { return L"UICollectionViewContentView"; }

	// Get the collection view instance.
	UICollectionView* GetOwner() const { return m_pOwner; }

	// Get the delegate.
	UICollectionViewDelegate* GetDelegate() const;

	// Set the delegate.
	void SetDelegate(UICollectionViewDelegate *pDelegate);

	// Select all items.
	void SelectAll();

	// Deselect all items.
	void DeselectAll();

	// Remove items from specified indexes.
	bool RemoveAt(std::set<int> sIndexes, BOOL bKeepSelections);

	// Remove all items.
	void RemoveAll();

	// Item size (same for all item controls).
	SIZE GetItemSize() const { return m_szItem; }

	// Item padding (same for all item controls).
	SIZE GetItemPadding() const { return m_szItemPadding; }

	// Return the item selection indexes.
	std::set<int> GetSelectionIndexes() const { return m_SelectionIndexes; }

	// Override this method to handle content scrolling.
	virtual void SetScrollPos(SIZE szPos);

	// Rewrite this method to render item controls inside `m_Items` map.
	bool DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl);

	// Rewrite this method to hit test item controls inside `m_Items` map.
	CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

	// Override to dynamically create / destroy / update item controls.
	void SetPos(RECT rc, bool bNeedInvalidate);

	// Override to forward events to UICollectionView.
	void DoEvent(TEventUI& event);

	// Parse XML to configure the UI appearance.
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	// By default refresh internal cache and rebuild the whole view.
	void ReloadData(BOOL bFullReload = TRUE);

protected:

	// Clear all visible item controls.
	void ClearVisibleItems();

protected:

	enum { // scrolling drag selection.
		TIMER_SCROLLUP,
		TIMER_SCROLLDN,
	};

	int m_nCount; // number of items to load.
	int m_nColumns; // virtual columns.
	int m_nRows; // virtual rows.
	UINT m_uMouseState; // mouse (captured) state.
	SIZE m_szItem; // size of each item (fixed).
	SIZE m_szItemPadding; // padding between items.
	SIZE m_szContent; // size of whole virtual area.
	RECT m_rcScrollable; // scroll area (exclude inset and scrollbar).
	POINT m_ptViewport; // origin of virtual area using default axis.

	UICollectionViewItemAttributes m_ItemAttributes; // shared item attributes.
	UICollectionViewLassoAttributes m_LassoAttributes; // selection lasso attributes.

	UICollectionView *m_pOwner; // public visible host.
	UICollectionViewDelegate *m_pDelegate; // collection view's delegate.
	UICollectionViewLasso *m_pSelectionLasso; // drag selection support.
	std::map<int, UICollectionViewItem *> m_Items; // visible items.
	std::stack<UICollectionViewItem *> m_ItemsPool; // recycled items.
	std::set<int> m_SelectionIndexes; // track item selections.
	std::set<int> m_LassoPersistedSelectionIndexes; // save selections before drag selection.

};

}
