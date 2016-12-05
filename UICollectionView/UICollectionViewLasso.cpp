//
//  UICollectionView - A delegate based flow layout control
//
//  Copyright 2016 (c) Kevin Xi(kevinxi.cn@gmail.com)
//	All rights reserved.
//

#include "stdafx.h"
#include "UICollectionViewLasso.h"
#include "UICollectionViewContentView.h"

namespace DuiLib
{

// Constructor.
UICollectionViewLasso::UICollectionViewLasso(UICollectionViewContentView *pContentView)
	:m_pContentView(pContentView)
{
	m_bVisible = false;
	m_bFloat = true;
}

// Set the point where mouse down.
void UICollectionViewLasso::SetMouseDownPos(POINT ptDown)
{
	if (!m_pContentView) return;

	// track start point and scroll pos.
	m_szScroll = m_pContentView->GetScrollPos();
	m_ptDown = ptDown;
	SetMouseMovePos(ptDown);
}

// Set the point where mouse move.
void UICollectionViewLasso::SetMouseMovePos(POINT ptMove)
{
	if (!m_pContentView) return;

	// calculate scroll offset.
	SIZE szOffset = {0, 0};
	szOffset.cx = m_pContentView->GetScrollPos().cx - m_szScroll.cx;
	szOffset.cy = m_pContentView->GetScrollPos().cy - m_szScroll.cy;

	// calculate lasso position.
	POINT pt1 = ptMove;
	POINT pt2 = {m_ptDown.x - szOffset.cx, m_ptDown.y - szOffset.cy};
	m_ptMove = ptMove;
	m_rcItem.left = min(pt1.x, pt2.x);
	m_rcItem.top = min(pt1.y, pt2.y);
	m_rcItem.right = max(pt1.x, pt2.x);
	m_rcItem.bottom = max(pt1.y, pt2.y);

	// update the selection area.
	SetPos(m_rcItem);
	
	// update the background.
	m_pContentView->NeedUpdate();
}

// Override to customize painting.
bool UICollectionViewLasso::DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl)
{
	if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) return false;
	if (!m_pContentView) return false;

	// read lasso attributes.
	m_dwBackColor = m_pContentView->m_LassoAttributes.dwLassoBkColor;
	m_dwBackColor2 = m_pContentView->m_LassoAttributes.dwLassoBkColor;
	m_rcBorderSize.left = m_rcBorderSize.top = m_rcBorderSize.right
		= m_rcBorderSize.bottom = m_pContentView->m_LassoAttributes.nLassoBorderWidth;
	m_dwBorderColor = m_pContentView->m_LassoAttributes.dwLassoBdColor;

	// BUG: we can't use `CControlUI::DoPaint()` to render this control, this
	// is because the default implementation will always paint into the whole
	// client area, while we only want to paint the intersect rect.
	
	// fill the intersect rect.
	CRenderEngine::DrawGradient(hDC, m_rcPaint, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), true, 16);

	// always draw left & right border.
	RECT rcBorder;
	rcBorder = m_rcPaint; rcBorder.right = m_rcPaint.left;
	CRenderEngine::DrawLine(hDC, rcBorder, m_rcBorderSize.left, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
	rcBorder = m_rcPaint; rcBorder.left	= m_rcPaint.right;
	CRenderEngine::DrawLine(hDC, rcBorder, m_rcBorderSize.right, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);

	// draw top & bottom border conditionally.
	if (m_rcItem.top >= m_rcPaint.top) {
		rcBorder = m_rcItem; rcBorder.bottom = m_rcItem.top;
		CRenderEngine::DrawLine(hDC, rcBorder, m_rcBorderSize.top, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
	}
	if (m_rcItem.bottom <= m_rcPaint.bottom) {
		rcBorder = m_rcItem; rcBorder.top = m_rcItem.bottom;
		CRenderEngine::DrawLine(hDC, rcBorder, m_rcBorderSize.bottom, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
	}

	return true;
}

}