//
//  UICollectionView - A delegate based flow layout control
//
//  Copyright 2016 (c) Kevin Xi(kevinxi.cn@gmail.com)
//	All rights reserved.
//

#pragma once

#include "UICollectionViewItem.h"
#include "UICollectionViewDelegate.h"

namespace DuiLib
{

// Provide a DuiLib style alias for public used classes.
typedef UICollectionView CCollectionViewUI;
typedef UICollectionViewItem CCollectionViewItemUI;
typedef UICollectionViewDelegate CCollectionViewUIDelegate;

// The UICollectionView.
class UICollectionViewContentView;
class UICollectionView : public CContainerUI
{
	friend class UICollectionViewContentView;

public:

	// Constructor.
	UICollectionView();

	// UIControl class.
	LPCTSTR GetClass() const { return L"UICollectionView"; }

	// UIControl flags.
	UINT GetControlFlags() const { return UIFLAG_TABSTOP; }

	// Usually, you may want use full reload, as it will remove all visible items from screen, create new items based on
	// the latest data provided by your delegate methods, the scroll pos will also be reset in this case.
	// However, sometimes you are very sure that you haven't made any changes to the data source, and you want to update
	// the item layout for some reason (e.g. zoom in & out items), in this case, we recommend you to disable full reload
	// as it will ideally result a better performance.
	void ReloadData(BOOL bFullReload = TRUE);

	// UICollection allows you to configure UI appearance by using the following attributes:
	// - itemsize / itempadding: Item size and padding between them, you can also specify them via delegate methods.
	// - itembkcolor / itemselectedbkcolor / itemhotbkcolor / itemdisabledbkcolor: Item background color.
	// - itembordersize / itembordercolor / itemselectedbordercolor / itemhotbordercolor / itemdisabledbordercolor: Item border size & color.
	// - lassobkcolor / lassobordercolor / lassobordersize: Apperance of drag selection lasso view.
	//
	// UICollection also disabled the following existed attributes thus you should not use:
	// - hscrollbar / hscrollbarstyle: Horizontal scrolling is not supported.
	// - childpadding: Replaced with better `itempadding` attribute.
	//
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	// When user removed some items from data source and needs to reflect the changes onto collection view, they can fully
	// reloading data which will work, but obviously it is too slow.
	// The `Remove` methods below are designated to better handle this use case, user can visit these methods to remove the
	// corresponding items at specified indexes from collection view. In order to on-demand load correct data from delegate
	// methods, please make sure you've updated your data source properly, a recommended way is implementing delegate method
	// `CollectionViewWillRemoveItemsAtIndexes` and put related code there.

	// The `bKeepSelections` option is set to FALSE by default, which will optimize the items removal speed, it will be helpful
	// especially when users remove a large set of items at a time. By default, it will simply reset item selection indexes while
	// remove items from collection view, set it to TRUE if you want to keep user selections during removal process.

	// Remove item at particular index.
	bool RemoveAt(int nIndex, BOOL bKeepSelections = FALSE);

	// Remove items from specified indexes.
	bool RemoveAt(std::set<int> sIndexes, BOOL bKeepSelections = FALSE);

	// Remove all items.
	void RemoveAll();

	// Return the item selection indexes.
	std::set<int> GetSelectionIndexes() const;

	// Get the delegate.
	UICollectionViewDelegate* GetDelegate() const;

	// Set the delegate.
	void SetDelegate(UICollectionViewDelegate* pDelegate);

	// Select all items.
	void SelectAll();

	// Deselect all items.
	void DeselectAll();

	// Get inset rect.
	RECT GetInset() const;

	// Set inset rect.
	void SetInset(RECT rcInset);

	// Throw exception on those unsupported method.
	CControlUI* GetItemAt(int nIndex) const;
	bool SetItemIndex(CControlUI *pControl, int nIndex);
	bool Add(CControlUI *pControl);
	bool AddAt(CControlUI *pControl, int nIndex);
	bool Remove(CControlUI *pControl);
	int GetItemIndex(CControlUI *pControl) const;
	int GetCount() const;
	int GetChildPadding() const;
	void SetChildPadding(int nPadding);

private:
	
	UICollectionViewContentView *m_pContentView;
};

}
