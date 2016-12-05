//
//  UICollectionView - A delegate based flow layout control
//
//  Copyright 2016 (c) Kevin Xi(kevinxi.cn@gmail.com)
//	All rights reserved.
//

#include "stdafx.h"
#include "UICollectionView.h"
#include "UICollectionViewContentView.h"

namespace DuiLib
{

// Constructor.
UICollectionView::UICollectionView()
{
	// add content view as the only child control.
	m_pContentView = new UICollectionViewContentView(this);
	CContainerUI::Add(m_pContentView);
}

// By default refresh internal cache and rebuild the whole view.
void UICollectionView::ReloadData(BOOL bFullReload)
{
	m_pContentView->ReloadData(bFullReload);
}

// Remove item at particular index.
bool UICollectionView::RemoveAt(int nIndex, BOOL bKeepSelections)
{
	std::set<int> sIndexes; 
	sIndexes.insert(nIndex);
	return RemoveAt(sIndexes, bKeepSelections);
}

// Remove items from specified indexes.
bool UICollectionView::RemoveAt(std::set<int> sIndexes, BOOL bKeepSelections)
{
	return m_pContentView->RemoveAt(sIndexes, bKeepSelections);
}

// Remove all items.
void UICollectionView::RemoveAll()
{
	m_pContentView->RemoveAll();
}

// Return the item selection indexes.
std::set<int> UICollectionView::GetSelectionIndexes() const
{
	return m_pContentView->GetSelectionIndexes();
}

// Get the delegate.
UICollectionViewDelegate* UICollectionView::GetDelegate() const
{
	return m_pContentView->GetDelegate();
}

// Set the delegate.
void UICollectionView::SetDelegate(UICollectionViewDelegate *pDelegate)
{
	m_pContentView->SetDelegate(pDelegate);
}

// Select all items.
void UICollectionView::SelectAll()
{
	m_pContentView->SelectAll();
}

// Deselect all items.
void UICollectionView::DeselectAll()
{
	m_pContentView->DeselectAll();
}

// Get inset rect.
RECT UICollectionView::GetInset() const 
{
	return m_pContentView->GetInset();
}
// Set inset rect.
void UICollectionView::SetInset(RECT rcInset)
{
	return m_pContentView->SetInset(rcInset);
}

// Apply xml attributes.
void UICollectionView::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	// disable those unsupported attributes.
	if (_tcscmp(pstrName, _T("hscrollbar")) == 0 ||
		_tcscmp(pstrName, _T("hscrollbarstyle")) == 0 ||
		_tcscmp(pstrName, _T("childpadding")) == 0
		) return;

	m_pContentView->SetAttribute(pstrName, pstrValue);
}

// Throw exception on those unsupported method.
CControlUI* UICollectionView::GetItemAt(int nIndex) const { 
	throw std::exception("UICollectionView doesn't support this method. The item controls have been virtualized thus may not exist at a particular index."); 
}
bool UICollectionView::SetItemIndex(CControlUI *pControl, int nIndex) {
	throw std::exception("UICollectionView doesn't support this method. The item controls have been virtualized thus may not exist at a particular index."); 
}
bool UICollectionView::Add(CControlUI *pControl) {
	throw std::exception("UICollectionView doesn't support this method. The item controls have been virtualized and can't be modified externally."); 
}
bool UICollectionView::AddAt(CControlUI *pControl, int nIndex) {
	throw std::exception("UICollectionView doesn't support this method. The item controls have been virtualized and can't be modified externally."); 
}
bool UICollectionView::Remove(CControlUI *pControl) {
	throw std::exception("UICollectionView doesn't support this method. The item controls have been virtualized and can't be modified externally."); 
}
int UICollectionView::GetItemIndex(CControlUI *pControl) const {
	throw std::exception("UICollectionView doesn't support this method. You can access equivalent data via delegate methods in UICollectionViewDelegate."); 
}
int UICollectionView::GetCount() const {
	throw std::exception("UICollectionView doesn't support this method. You can access equivalent data via delegate methods in UICollectionViewDelegate."); 
}
int UICollectionView::GetChildPadding() const {
	throw std::exception("UICollectionView doesn't support this method. You can access equivalent data via delegate methods in UICollectionViewDelegate."); 
}
void UICollectionView::SetChildPadding(int nPadding) {
	throw std::exception("UICollectionView doesn't support this method. Please use `itempadding` attribute instead, it is a `POINT` type and support both X and Y axis paddings."); 
}

}