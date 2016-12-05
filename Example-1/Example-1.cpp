// Example-1.cpp : Loads system icons into the UICollectionView, demonstrate drag selection, select
// all, deselect all, delete items, etc.
//

#include "stdafx.h"
#include "Example-1.h"
#include "Shlobj.h"
#include "Shellapi.h"
#include "CommonControls.h"
#include "UIlib.h"
#include "UICollectionView.h"
#include "UICollectionViewDelegate.h"
#include "UIIcon.h"

using namespace DuiLib;

class ExampleWindow : public CWindowWnd, public INotifyUI, public IDialogBuilderCallback, public UICollectionViewDelegate
{

public:

    LPCTSTR GetWindowClassName() const { 
        return _T("ExampleWindow"); 
    };

    UINT GetClassStyle() const { 
        return UI_CLASSSTYLE_FRAME;
    };

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {

		if (uMsg == WM_CREATE) {
			m_PaintMgr.Init(m_hWnd);

			// !!!it is important to set delegate correctly.
			m_pCollectionView = new UICollectionView;
			m_pCollectionView->SetDelegate(this);

			CDialogBuilder builder;
			CControlUI *pRoot = builder.Create(_T("example-1.xml"), (UINT)0, this, &m_PaintMgr);
			m_PaintMgr.AttachDialog(pRoot);
			m_PaintMgr.AddNotifier(this);

			// vista or later, load the large icons.
			::SHGetImageList(SHIL_JUMBO, IID_IImageList, (void **)&m_pImageList);
	
			// !!!data source is ready, reload.
			m_pCollectionView->ReloadData();
	
			return 0L;

		} else if (uMsg == WM_DESTROY) {
			::PostQuitMessage(0L);
			return 0L;
		}
		
		LRESULT lRes = 0;
		if (m_PaintMgr.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
		else return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

    void Notify(TNotifyUI &msg) {
		if (msg.sType == _T("click")) {
            if (msg.pSender->GetName() == _T("deletebtn")) { 

				// delete selected items.
				m_pCollectionView->RemoveAt(m_pCollectionView->GetSelectionIndexes());
			} else if (msg.pSender->GetName() == _T("selectbtn")) {

				// select all, deselect all.
				if (msg.pSender->GetText() == _T("Select All")) {
					msg.pSender->SetText(_T("Deselect All"));
					m_pCollectionView->SelectAll();
				} else {
					msg.pSender->SetText(_T("Select All"));
					m_pCollectionView->DeselectAll();
				}
			}
        }
    }

	CControlUI* CreateControl(LPCTSTR pstrClass) {
		if (_tcsicmp(pstrClass, _T("UICollectionView")) == 0)
			return m_pCollectionView;
		else if (_tcsicmp(pstrClass, _T("UICollectionViewItem")) == 0)
			return new UICollectionViewItem;
		else if (_tcsicmp(pstrClass, _T("Icon")) == 0)
			return new CIconUI;
		return nullptr;
	}

	// Return the number of items in the collection view.
	int CollectionViewItemsCount(UICollectionView *pCollectionView) {
		int nCount = 0;
		m_pImageList->GetImageCount(&nCount);
		return nCount;
	}

	// Collection view assumes all items are in the same size and will be resized automatically.
	SIZE CollectionViewItemSize(UICollectionView *pCollectionView) {
		SIZE szItem = { 100, 100 };
		return szItem;
	}

	// Return an empty item template view, this might not be visible to the user immediately.
	UICollectionViewItem* CollectionViewReusableItemTemplate(UICollectionView *pCollectionView) {
		
		UICollectionViewItem *pItem = nullptr;
		static CDialogBuilder builder;
		if (!builder.GetMarkup()->IsValid())
			pItem = (UICollectionViewItem *)builder.Create(L"example-1_item.xml", (UINT)0, this, &m_PaintMgr);
		else
			pItem = (UICollectionViewItem *)builder.Create(this, &m_PaintMgr);
		return pItem;
	}

	// The collection view is about to display an item. Use this method to fill data into the item view.
	void CollectionViewWillDisplayItem(UICollectionView *pCollectionView, UICollectionViewItem *pItemView, int nItemIndex) {

		CIconUI *pIconUI = dynamic_cast<CIconUI *>(pItemView->GetPreview());
		if (pIconUI) {
			HICON hIcon = NULL;
			m_pImageList->GetIcon(nItemIndex, 0, &hIcon);
			pIconUI->SetIcon(hIcon);
		}
	}

protected:

    CPaintManagerUI m_PaintMgr;
	UICollectionView *m_pCollectionView;
	IImageList *m_pImageList;
};

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("..\\Example-1\\Resources"));

	ExampleWindow *pWnd = new ExampleWindow();
	if (pWnd == NULL) return 0;
	pWnd->Create(NULL, _T("UICollectionView - Example 1"), UI_WNDSTYLE_FRAME | WS_CLIPCHILDREN, WS_EX_WINDOWEDGE);
	pWnd->CenterWindow();
	pWnd->ShowWindow(true);

	CPaintManagerUI::MessageLoop();

	delete pWnd;
	return 0;
}
