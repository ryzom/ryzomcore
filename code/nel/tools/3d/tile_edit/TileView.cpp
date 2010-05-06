// TileView.cpp : implementation file
//

#include "stdafx.h"
#include "tile_edit.h"
#include "TileView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TileView

TileView::TileView()
{
}

TileView::~TileView()
{
}


BEGIN_MESSAGE_MAP(TileView, CListCtrl)
	//{{AFX_MSG_MAP(TileView)
	ON_WM_DROPFILES()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TileView message handlers

void TileView::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	 *parent=this->GetParent();
	if (parent) parent->
	
	CListCtrl::OnDropFiles(hDropInfo);
}

void TileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

int TileView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CListCtrl::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

BOOL TileView::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CListCtrl::OnCommand(wParam, lParam);
}

LRESULT TileView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CListCtrl::WindowProc(message, wParam, lParam);
}

