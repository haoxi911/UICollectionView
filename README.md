# UICollectionView

This project intends to bring the easy-to-use iOS UICollectionView component onto Windows. It is designed to work with a lot of items and only loads the views that it actually needs. UICollectionView for Windows can smoothly displays 20,000+ items.

## High-level concept overview

The delegate provides empty UICollectionViewItem for the UICollectionView that will be reused as often as needed; you can style the view in it to a creation degree, but the delegate doesn't know yet which item it will be displayed later.

    UICollectionViewItem* CollectionViewReusableItemTemplate(UICollectionView *pCollectionView);

You will also want to tell UICollectionView how many items you have, and what is the size of an UICollectionViewItem through the following delegate methods.
    
    int CollectionViewItemsCount(UICollectionView *pCollectionView);
    SIZE CollectionViewItemSize(UICollectionView *pCollectionView);
   
Before UICollectionView shows items, it asks the delegate to populate an UICollectionViewItem with an index; this is the time to set labels or images that are dependent on the index of item its supposed to represent.
	  
    void CollectionViewWillDisplayItem(UICollectionView *pCollectionView, UICollectionViewItem *pItemView, int nItemIndex);

When the user scrolls the view, some views will become invisible. UICollectionView removes them from view and stores them for later use. The delegate has a chance to unload any item-specific resources from the item, possibly to save memory or do kinds of cleanup.
	  
    void CollectionViewWillRecycleItem(UICollectionView *pCollectionView, UICollectionViewItem *pItemView);

The user can use either mouse or keyboard to select items. By default, UICollectionView will draw a nice selection background under each selected items. The delegate below can disable this behavior by return FALSE in:
	  
    BOOL CollectionViewShouldDrawItemSelection(UICollectionView *pCollectionView);

The method below intends to inform the delegate of changes during user selections. Using this method to style the UICollectionViewItem might break when the views are being reused at a later point, so don't do that.
	  
    void CollectionViewSelectionDidChange(UICollectionView *pCollectionView, std::set<int> sOldIndexes, std::set<int> sNewIndexes);

## License

UICollectionView is licensed under the MIT license.
