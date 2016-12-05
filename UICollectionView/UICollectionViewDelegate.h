//
//  UICollectionView - A delegate based flow layout control
//
//  Copyright 2016 (c) Kevin Xi(kevinxi.cn@gmail.com)
//	All rights reserved.
//

#pragma once

#include "UIlib.h"
#include <set>

namespace DuiLib
{

// Delegate methods to lazy loading items and their data.
class UICollectionView;
class UICollectionViewItem;
class UICollectionViewDelegate
{
public:

	// Return the number of items in the collection view.
	virtual int CollectionViewItemsCount(UICollectionView *pCollectionView) = 0;

	// Return an empty item template view, this might not be visible to the user immediately.
	virtual UICollectionViewItem* CollectionViewReusableItemTemplate(UICollectionView *pCollectionView) = 0;

	// The collection view is about to display an item. Use this method to fill data into the item view.
	virtual void CollectionViewWillDisplayItem(UICollectionView *pCollectionView, UICollectionViewItem *pItemView, int nItemIndex) = 0;

public: // Optional

	// Collection view assumes all items are in the same size and will be resized automatically.
	virtual SIZE CollectionViewItemSize(UICollectionView *pCollectionView) { SIZE szItem = {0, 0}; return szItem; }

	// Collection view assumes all items are using the same paddings between each other.
	virtual SIZE CollectionViewItemPadding(UICollectionView *pCollectionView) { SIZE szPadding = {0, 0}; return szPadding; }

	// User is explicitly removing one or many items. Make sure you've updated your data source accordingly within this method.
	virtual void CollectionViewWillRemoveItemsAtIndexes(UICollectionView *pCollectionView, std::set<int> indexes) {}

	// The collection view is about to recycle an item for reuse. Use this method to clean up resources.
	virtual void CollectionViewWillRecycleItem(UICollectionView *pCollectionView, UICollectionViewItem *pItemView) {}

	// User selected or deselected one or many items. The internal selection indexes were updated before visiting this method.
	virtual void CollectionViewSelectionDidChange(UICollectionView *pCollectionView, std::set<int> sOldIndexes, std::set<int> sNewIndexes) {}

	// Visited when the collection view updated item's position. Don't put any time consuming code in this method.
	virtual void CollectionViewDidUpdateItemLayout(UICollectionView *pCollectionView, UICollectionViewItem *pItemView, int nItemIndex) {}

	// User double clicked on an item.
	virtual void CollectionViewDidDoubleClickItem(UICollectionView *pCollectionView, UICollectionViewItem *pItemView, int nItemIndex) {}

	// User has explicitly removed one or many items from collection view.
	virtual void CollectionViewDidRemoveItemsAtIndexes(UICollectionView *pCollectionView, std::set<int> indexes) {}

	// Return FALSE to disable item hover state.
	virtual BOOL CollectionViewShouldDrawItemHover(UICollectionView *pCollectionView) { return FALSE; }

	// Return FALSE to disable item selection feature.
	virtual BOOL CollectionViewShouldDrawItemSelection(UICollectionView *pCollectionView) { return TRUE; }

};

}
