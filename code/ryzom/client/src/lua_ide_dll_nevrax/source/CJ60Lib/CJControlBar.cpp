// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// CJControlBar.cpp : implementation file
//  
// DevStudio Style Resizable Docking Control Bar.  
//  
// Copyright (c) 1998-99 Kirk Stowell   
//		mailto:kstowell@codejockeys.com
//		http://www.codejockeys.com/kstowell/
//
// This source code may be used in compiled form in any way you desire. 
// Source file(s) may be redistributed unmodified by any means PROVIDING
// they are not sold for profit without the authors expressed written consent,
// and providing that this notice and the authors name and all copyright
// notices remain intact. If the source code is used in any commercial
// applications then a statement along the lines of:
//
// "Portions Copyright (c) 1998-99 Kirk Stowell" must be included in the
// startup banner, "About" box or printed documentation. An email letting
// me know that you are using it would be nice as well. That's not much to ask
// considering the amount of work that went into this.
//
// This software is provided "as is" without express or implied warranty. Use
// it at your own risk! The author accepts no liability for any damage/loss of
// business that this product may cause.
//  
// ==========================================================================  
//
// Acknowledgements:
//	<>  Many thanks to all of you, who have encouraged me to update my articles
//		and code, and who sent in bug reports and fixes.
//  <>  Many thanks Zafir Anjum (zafir@codeguru.com) for the tremendous job that
//      he has done with codeguru, enough can not be said!
//	<>  Many thanks to Microsoft for making the source code availiable for MFC. 
//		Since most of this work is a modification from existing classes and 
//		methods, this library would not have been possible.
//
// ==========================================================================  
// HISTORY:	  
// ==========================================================================  
//			1.00	17 Oct 1998	- Initial re-write and release.  
//			1.01	20 Oct 1998	- Fixed problem with gripper and buttons  
//								  disappearing when docking toggled. Overloaded  
//								  IsFloating() method from base class.  
//			1.02    22 Nov 1998 - Modified set cursor to display normal size
//								  cursor when static linked.
//			2.00    12 Jan 1999 - Total class re-write, added multiple/side-by-side
//								  controlbar docking! No longer uses CSizingControlBar
//								  base class.
// ==========================================================================  
//  
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CJControlBar.h"
#include "CJDockContext.h"
#include "CJSizeDockBar.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define GRIP_LEFTSPACING	3
#define GRIP_STARTGRIP		8
#define GRIP_SIZE			3
#define GRIP_BUTTONSPACING	3
#define GRIP_INTRASPACING	1

#define _HIDECROSS			0
#define _MAXORZDISABLE		1
#define _MAXORZENABLE		2
#define _MAXVERTDISABLE		3
#define _MAXVERTENABLE		4
#define _NORMORZDISABLE		5
#define _NORMORZENABLE		6
#define _NORMVERTDISABLE	7
#define _NORMVERTENABLE		8

/////////////////////////////////////////////////////////////////////////////
// CCJControlBar

CCJControlBar::CCJControlBar()
{
	m_iTrackBorderSize	= 4;
	m_iAuxImage			= -1;
	m_menuID			= -1;
	m_bToFit			= FALSE;
	m_bOkToDrag			= FALSE;
	m_bDragging			= FALSE;
	m_bMaximized		= FALSE;
	m_bUnique			= FALSE;
	m_bGripper			= TRUE;
	m_bButtons			= TRUE;
	m_sizeDesired		= CSize(200,100);
	m_sizeNormal		= CSize(200,100);

	// Create the image list used by frame buttons.
	m_ImageList.Create(IDB_BUTTON_IMAGES,
		10, 1, RGB(255,255,255));

	m_pChildWnd = NULL;

	m_clrBtnHilite = ::GetSysColor(COLOR_BTNHILIGHT);
	m_clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
	m_clrBtnFace   = ::GetSysColor(COLOR_BTNFACE);
}

CCJControlBar::~CCJControlBar()
{
	m_ImageList.DeleteImageList();
}

IMPLEMENT_DYNAMIC(CCJControlBar, CControlBar)

BEGIN_MESSAGE_MAP(CCJControlBar, CControlBar)
	//{{AFX_MSG_MAP(CCJControlBar)
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CREATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDC_BUTTON_HIDE, OnButtonClose)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_HIDE, OnUpdateButtonClose)
	ON_COMMAND(IDC_BUTTON_MINI, OnButtonMinimize)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_MINI, OnUpdateButtonMinimize)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCJControlBar virtual overrides

BOOL CCJControlBar::Create(CWnd *pParentWnd, UINT nID, LPCTSTR lpszWindowName, CSize sizeDefault, DWORD dwStyle)
{
    ASSERT_VALID(pParentWnd);   // must have a parent

    // Set initial control bar style.
    SetBarStyle(dwStyle & CBRS_ALL|CBRS_HIDE_INPLACE|CBRS_SIZE_DYNAMIC|CBRS_FLOAT_MULTI);

    dwStyle &= ~CBRS_ALL;
    dwStyle |= WS_VISIBLE | WS_CHILD;
    
	m_sizeDesired = m_sizeNormal = sizeDefault;

	if (!AfxDeferRegisterClass(AFX_WNDCONTROLBAR_REG))
		return FALSE;
	
	return CWnd::Create(AFX_WNDCONTROLBAR, lpszWindowName,
		dwStyle, CRect(0,0,0,0), pParentWnd, nID);
}

void CCJControlBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	if (m_bButtons)
	{
		m_ToolTip.DelTool(&m_btnMinim, IDS_CONTRACT);
		m_ToolTip.DelTool(&m_btnMinim, IDS_EXPAND);

		UINT NewImage;
		BOOL bEnable = FALSE;

		if (IsHorzDocked())
		{
			if (m_bUnique) {
				m_ToolTip.AddTool (&m_btnMinim, IDS_EXPAND);
				NewImage = _MAXORZDISABLE;
				bEnable = FALSE;
			}

			else if (!m_bMaximized) {
				m_ToolTip.AddTool (&m_btnMinim, IDS_EXPAND);
				NewImage = _MAXORZENABLE;
				bEnable = TRUE;
			}

			else {
				m_ToolTip.AddTool (&m_btnMinim, IDS_CONTRACT);
				NewImage = _NORMORZENABLE;
				bEnable = TRUE;
			}
		}
		
		else if(IsVertDocked())
		{
			if (m_bUnique) {
				m_ToolTip.AddTool (&m_btnMinim, IDS_EXPAND);
				NewImage = _MAXVERTDISABLE;
				bEnable = FALSE;
			}

			else if (!m_bMaximized) {
				m_ToolTip.AddTool (&m_btnMinim, IDS_EXPAND);
				NewImage = _MAXVERTENABLE;
				bEnable = TRUE;
			}

			else {
				m_ToolTip.AddTool (&m_btnMinim, IDS_CONTRACT);
				NewImage = _NORMVERTENABLE;
				bEnable = TRUE;
			}
		}

		if (NewImage != m_iAuxImage) {
			// TMP TMP
			/*			
			m_iAuxImage = NewImage;			
			m_btnMinim.SetIcon(m_ImageList.ExtractIcon(m_iAuxImage), CSize(10,10));
			m_btnMinim.EnableWindow(bEnable);
			*/
		}
	}
}

CSize CCJControlBar::CalcDynamicLayout(int nLength, DWORD nMode)
{
	if(IsFloating())
	{
		if (nMode == LM_HORZ) {
			if (nLength < 50)
				nLength = 50;
			m_sizeDesired.cx = nLength;
		}

		if (nMode == (LM_LENGTHY | LM_HORZ)) {
			if (nLength < 50)
				nLength = 50;
			m_sizeDesired.cy = nLength;
		}
	}

	return m_sizeDesired;
}

BOOL CCJControlBar::PreTranslateMessage(MSG* pMsg) 
{
	if (m_bButtons) m_ToolTip.RelayEvent(pMsg);
	return CControlBar::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CCJControlBar message handlers

void CCJControlBar::OnNcPaint() 
{
	EraseNonClient();
}

void CCJControlBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rc;
	m_iAuxImage = -1;
	GetClientRect(&rc);
	CRect rect;
	GetChildRect(rect);
	dc.ExcludeClipRect(rect);
	DrawBorders(&dc,rc);
}

void CCJControlBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bOkToDrag)
	{
		CFrameWnd *pFrame=GetDockingFrame();
		GetClientRect(m_rcTrack);
		
		if (IsVertDocked()) {
			m_rcTrack.top = m_rcTrack.bottom-m_iTrackBorderSize-2;
			m_rcTrack.bottom-=2;
		}
	
		else if (IsHorzDocked()) {
			m_rcTrack.left = m_rcTrack.right-m_iTrackBorderSize-2;
			m_rcTrack.right-=2;
		}
		
		ClientToScreen(&m_rcTrack);
		pFrame->ScreenToClient(&m_rcTrack);
		ClientToScreen(&point);
		pFrame->ScreenToClient(&point);
		
		m_ptStartDrag = point;
		m_ptCurDrag = point;
		
		SetCapture();
		m_bDragging = TRUE;
		OnInvertTracker(m_rcTrack);
	}

	else if (m_pDockBar) {
		if (OnToolHitTest(point, NULL) == -1) {
			ClientToScreen(&point);
			((CCJDockContext*)m_pDockContext)->StartDragDockBar(point);
		}
	}

	else {
		CControlBar::OnLButtonDown(nFlags, point);
	}
}

void CCJControlBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	HitTest(point);	
	CRect rectDock;
	if( m_bDragging )
	{
		GetParent()->GetClientRect(&rectDock);
		ClientToScreen(&rectDock);
		ClientToScreen(&point);
		
		CFrameWnd *pFrame=GetDockingFrame();
		pFrame -> ScreenToClient(&point);
		pFrame->ScreenToClient(&rectDock);

		point.x = point.x>rectDock.right ? rectDock.right:point.x;
		point.x = point.x<rectDock.left ? rectDock.left:point.x;
		point.y = point.y>rectDock.bottom ? rectDock.bottom:point.y;
		point.y = point.y<rectDock.top ? rectDock.top:point.y;

		OnInvertTracker(m_rcTrack);
		int deltaX = point.x-m_ptCurDrag.x;
		int deltaY = point.y-m_ptCurDrag.y;
		
		if(IsVertDocked()) {
			m_rcTrack.top += deltaY;
			m_rcTrack.bottom += deltaY;
		}

		else if (IsHorzDocked()) {
			m_rcTrack.left += deltaX;
			m_rcTrack.right += deltaX;
		}

		OnInvertTracker(m_rcTrack);
		m_ptCurDrag = point;
	}

	CControlBar::OnMouseMove(nFlags, point);
}

BOOL CCJControlBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	return (!m_bOkToDrag && !m_bDragging)?
		CControlBar::OnSetCursor(pWnd, nHitTest, message):FALSE;
}

void CCJControlBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CRect rectAvail;
	GetParent()->GetClientRect(&rectAvail);
	
	if( m_bDragging )
	{
		ReleaseCapture();
		OnInvertTracker(m_rcTrack);
		m_bDragging = FALSE;
		
		if(IsVertDocked()) {
			int newHeight = m_rcTrack.top-m_ptStartDrag.y;
			m_sizeDesired.cy += newHeight;
			m_sizeDesired.cy = m_sizeDesired.cy > GetMinExt() ? m_sizeDesired.cy : GetMinExt();
			m_sizeDesired.cy = m_sizeDesired.cy > rectAvail.Height() ? rectAvail.Height() : m_sizeDesired.cy;
		}
		
		else if (IsHorzDocked()) {
			int newWidth = m_rcTrack.left-m_ptStartDrag.x;
			m_sizeDesired.cx += newWidth;
			m_sizeDesired.cx = m_sizeDesired.cx > GetMinExt() ? m_sizeDesired.cx : GetMinExt();
			m_sizeDesired.cy = m_sizeDesired.cx > rectAvail.Width() ? rectAvail.Width() : m_sizeDesired.cx;
		}

		((CCJSizeDockBar *)GetParent())->CalcSizeBarLayout();
	}

	CControlBar::OnLButtonUp(nFlags, point);
}

UINT CCJControlBar::OnNcHitTest(CPoint point) 
{
	return HTCLIENT;
}

BOOL CCJControlBar::OnEraseBkgnd(CDC* pDC) 
{
	int result = CControlBar::OnEraseBkgnd(pDC);

	CRect rect;
	GetChildRect(rect);
	pDC->Draw3dRect(rect, m_clrBtnShadow, m_clrBtnHilite);
	return result;
}

void CCJControlBar::OnSysColorChange() 
{
	CControlBar::OnSysColorChange();
	
	m_clrBtnHilite = ::GetSysColor(COLOR_BTNHILIGHT);
	m_clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
	m_clrBtnFace   = ::GetSysColor(COLOR_BTNFACE);
}

int CCJControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (m_bButtons)
	{
		if(!m_btnClose.Create(NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP |
			BS_NOTIFY | BS_OWNERDRAW | BS_ICON,
			CRect(0,0,0,0), this, IDC_BUTTON_HIDE ))
		{
			TRACE0("Unable to create close button\n");
			return -1;
		}


		if(!m_btnMinim.Create(NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP |
			BS_NOTIFY | BS_OWNERDRAW | BS_ICON,
			CRect(0,0,0,0), this, IDC_BUTTON_MINI ))
		{
			TRACE0("Unable to create minimize button\n");
			return -1;
		}

		m_btnClose.DisableFlatLook();
		m_btnMinim.DisableFlatLook();

		
		// TMP TMP
		//	m_btnClose.SetIcon(m_ImageList.ExtractIcon(0), CSize(10,10));
		
		// Create the ToolTip control.
		m_ToolTip.Create(this);
		m_ToolTip.Activate(TRUE);
		
		m_ToolTip.AddTool (&m_btnClose, IDS_HIDE);
		m_ToolTip.AddTool (&m_btnMinim, IDS_EXPAND);
	}

	return 0;
}

void CCJControlBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CControlBar::OnWindowPosChanged(lpwndpos);

	if (m_bButtons)
	{
		if (IsFloating())
		{
			m_btnClose.ShowWindow(SW_HIDE);
			m_btnMinim.ShowWindow(SW_HIDE);
		}
		
		else
		{
			m_btnClose.ShowWindow(SW_SHOW);
			m_btnMinim.ShowWindow(SW_SHOW);

			CRect rcClose(GetButtonRect());
			CRect rcMinim(GetButtonRect());

			if (IsHorzDocked()) {
				rcMinim.OffsetRect(0,14);
			}

			else {
				rcClose.OffsetRect(14,0);
			}

			m_btnClose.MoveWindow(rcClose);
			m_btnMinim.MoveWindow(rcMinim);
		}
	}

	if (m_pChildWnd->GetSafeHwnd()) {
		CRect rc;
		GetChildRect(rc);
		m_pChildWnd->MoveWindow(rc);
	}
}

void CCJControlBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// if no menu, just return.
	if (m_menuID == -1 ) {
		TRACE0("Warning: No control bar menu defined.\n");
		return;
	}

	if (point.x == -1 && point.y == -1)
	{
		//keystroke invocation
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);
		
		point = rect.TopLeft();
		point.Offset(5, 5);
	}
	
	CMenu menu;
	VERIFY(menu.LoadMenu(m_menuID));
	
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();
	
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);
}

void CCJControlBar::OnButtonClose()
{
	GetDockingFrame()->ShowControlBar(this,FALSE,FALSE);
}

void CCJControlBar::OnUpdateButtonClose(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
}

void CCJControlBar::OnButtonMinimize()
{
	if (!m_bMaximized) {
		((CCJSizeDockBar *)GetParent())->Maximize(this);
		m_bMaximized = TRUE;
	}
	
	else {
		((CCJSizeDockBar *)GetParent())->Normalize(this);
		m_bMaximized = FALSE;
	}
}

void CCJControlBar::OnUpdateButtonMinimize(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
}

/////////////////////////////////////////////////////////////////////////////
// CCJControlBar operations

void CCJControlBar::EraseNonClient()
{
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.IntersectClipRect(rectWindow);
	SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);
}

void CCJControlBar::DrawBorders(CDC * pDC, CRect & rect)
{
	CRect rc;
	rc.CopyRect(&rect);

	CRect rcTrack;
	rcTrack.CopyRect(&rc);

	if(IsVertDocked())
	{
		if (!m_bToFit) {
			rcTrack.top = rc.bottom-m_iTrackBorderSize;
			rcTrack.top-=2;
			rcTrack.bottom-=1;
			pDC->FillSolidRect(rcTrack, m_clrBtnFace);
			pDC->Draw3dRect(rcTrack, m_clrBtnHilite, m_clrBtnShadow);
		}
	}
	
	else if (IsHorzDocked())
	{
		if (!m_bToFit) {
			rcTrack.left = rc.right-m_iTrackBorderSize;
			rcTrack.left-=2;
			rcTrack.right-=1;
			pDC->FillSolidRect(rcTrack, m_clrBtnFace);
			pDC->Draw3dRect(rcTrack, m_clrBtnHilite, m_clrBtnShadow);
		}
	}

	DrawGripper(pDC);
}

void CCJControlBar::HitTest(CPoint & point)
{
	CRect rcWin;
	GetClientRect(&rcWin);
	HCURSOR hCur;
	BOOL bHit = FALSE;

	if (IsVertDocked()) {
		rcWin.top = rcWin.bottom-m_iTrackBorderSize;
		hCur = AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR);
		bHit = rcWin.PtInRect(point) && !m_bToFit;
	}
	
	else if (IsHorzDocked()) {
		rcWin.left = rcWin.right-m_iTrackBorderSize;
		hCur = AfxGetApp()->LoadCursor(AFX_IDC_HSPLITBAR);
		bHit = rcWin.PtInRect(point) && !m_bToFit;	
	}

	if( bHit )
		SetCursor(hCur);

	else {
		hCur = ::LoadCursor(NULL,IDC_ARROW);
		SetCursor(hCur);
	}

	m_bOkToDrag = bHit;

}

void CCJControlBar::OnInvertTracker(const CRect & rect)
{
	ASSERT_VALID(this);
	ASSERT(!rect.IsRectEmpty());
	CFrameWnd *pFrame=GetDockingFrame();	
	CDC* pDC = pFrame->GetDC();
	CBrush* pBrush = CDC::GetHalftoneBrush();
	HBRUSH hOldBrush = NULL;
	if (pBrush != NULL)
		hOldBrush = (HBRUSH)SelectObject(pDC->m_hDC, pBrush->m_hObject);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
	if (hOldBrush != NULL)
		SelectObject(pDC->m_hDC, hOldBrush);
	pFrame->ReleaseDC(pDC);
}

void CCJControlBar::SetChild(CWnd * pWnd)
{
	m_pChildWnd = pWnd;
}

void CCJControlBar::DrawGripper(CDC * pDC)
{
	if (!IsFloating() && m_bGripper)
	{
		// draw the gripper.
		CRect pRect(GetGripperRect());
		pDC->Draw3dRect( pRect, ::GetSysColor(COLOR_BTNHIGHLIGHT),
			::GetSysColor(COLOR_BTNSHADOW) );
		
		if (IsHorzDocked())
			pRect.OffsetRect(4,0);
		else
			pRect.OffsetRect(0,4);
		
		pDC->Draw3dRect( pRect, ::GetSysColor(COLOR_BTNHIGHLIGHT),
			::GetSysColor(COLOR_BTNSHADOW) );
	}
}

CRect CCJControlBar::GetButtonRect()
{
	CRect pRect;
	GetClientRect(pRect);
	pRect.OffsetRect(-pRect.left,-pRect.top);

	if (IsHorzDocked()) {
		pRect.top	 += 3;
		pRect.bottom = pRect.top+12;
		pRect.left  += 2;
		pRect.right  = pRect.left+12;
	}

	else {
		pRect.right -= 19;
		pRect.left   = pRect.right-12;
		pRect.top   += 3;
		pRect.bottom = pRect.top+12;
	}
	
	return pRect;
}

CRect CCJControlBar::GetGripperRect()
{
	CRect pRect;
	GetClientRect(pRect);
	pRect.OffsetRect(-pRect.left,-pRect.top);

	if (IsHorzDocked()) {
		pRect.DeflateRect(3,3);
		pRect.left		+= 1;
		pRect.right		 = pRect.left+3;
		pRect.bottom	-= 1;
		pRect.top		+= m_bButtons?30:1;
	}
	
	else {
		pRect.DeflateRect(4,4);
		pRect.top		+= 2;
		pRect.bottom	 = pRect.top+3;
		pRect.right		-= m_bButtons?30:2;
	}

	return pRect;
}

UINT CCJControlBar::GetMenuID()
{
	return m_menuID;
}

void CCJControlBar::SetMenuID(UINT nID)
{
	m_menuID = nID;
}

void CCJControlBar::ShowFrameControls(BOOL bGripper, BOOL bButtons)
{
	m_bGripper = bGripper;
	m_bButtons = bButtons;
}

void CCJControlBar::EnableDockingOnSizeBar(DWORD dwDockStyle)
{
	ASSERT(m_pDockContext == NULL);
	m_dwDockStyle = dwDockStyle;

	if (m_pDockContext == NULL)
		m_pDockContext = new CCJDockContext(this);

	if (m_hWndOwner == NULL)
		m_hWndOwner = ::GetParent(m_hWnd);
}

void CCJControlBar::Maximize(int size)
{
	m_sizeNormal = m_sizeDesired;
	if (IsHorzDocked()) {
		m_sizeDesired.cx = size;
	}

	else if (IsVertDocked()) {
		m_sizeDesired.cy = size;
	}

	m_bMaximized = TRUE;
}

void CCJControlBar::Minimize()
{
	m_sizeNormal = m_sizeDesired;
	if (IsHorzDocked()) {
		m_sizeDesired.cx = GetMinExt();
	}

	else if (IsVertDocked()) {
		m_sizeDesired.cy = GetMinExt();
	}

	m_bMaximized = FALSE;
}

void CCJControlBar::Normalize()
{
	if (IsHorzDocked()) {
		m_sizeDesired.cx = m_sizeNormal.cx;
	}

	else if (IsVertDocked()) {
		m_sizeDesired.cy = m_sizeNormal.cy;
	}

	m_bMaximized = FALSE;
}

void CCJControlBar::SetNormalSize(const CSize & cs)
{
	m_sizeDesired = m_sizeNormal = cs;
}

void CCJControlBar::GetChildRect(CRect &rect)
{
	GetClientRect(&rect);
	
	if (!IsFloating())
	{
		if (IsVertDocked()) {
			rect.top += (GetMinExt()-5);
			rect.bottom-=(m_iTrackBorderSize+4);
		}

		else if(IsHorzDocked()) {
			rect.left += (GetMinExt()-5);
			rect.right-=(m_iTrackBorderSize+4);
		}

		if( rect.left > rect.right || rect.top > rect.bottom )
			rect = CRect(0,0,0,0);
	}
}

BOOL CCJControlBar::IsLeftDocked()
{
	return m_pDockBar?(m_pDockBar->GetDlgCtrlID()
		== AFX_IDW_SIZEBAR_LEFT):FALSE;
}

BOOL CCJControlBar::IsRightDocked()
{
	return m_pDockBar?(m_pDockBar->GetDlgCtrlID()
		== AFX_IDW_SIZEBAR_RIGHT):FALSE;
}

BOOL CCJControlBar::IsTopDocked()
{
	return m_pDockBar?(m_pDockBar->GetDlgCtrlID()
		== AFX_IDW_SIZEBAR_TOP):FALSE;
}

BOOL CCJControlBar::IsBottomDocked()
{
	return m_pDockBar?(m_pDockBar->GetDlgCtrlID()
		== AFX_IDW_SIZEBAR_BOTTOM):FALSE;
}

int CCJControlBar::GetMinExt()
{
	int nRet = m_iTrackBorderSize;

	if (m_bGripper || m_bButtons) {
		nRet += GRIP_STARTGRIP;
		nRet += GRIP_SIZE*2;
		nRet += GRIP_INTRASPACING*3+2;
	}

	else
		nRet += 2;

	return nRet;
}
