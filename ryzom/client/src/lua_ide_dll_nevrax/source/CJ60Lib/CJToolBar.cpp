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

// CJToolBar.cpp : implementation file
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
//  <>  Thanks to Joerg Koenig (Joerg.Koenig@rhein-neckar.de), which is where I
//		got some ideas about drawing 3D looking toolbars, from his article
//		'Another Flat ToolBar (does not require MSIE)'.
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
//			1.02	02 Nov 1998 - Fixed bug with DrawNoGripper() method -
//								  (Christian Skovdal Andersen).
//			1.03	14 Dec 1998 - Changed class to derive from Paul DiLascia's
//								  CFlatToolBar, this class adds gripper and
//								  control insertion.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CJToolBar.h"
#include "CJFlatComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCJToolBar

CCJToolBar::CCJToolBar()
{
	m_pControls = NULL;
	m_bGripper  = TRUE;
	m_clrHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	m_clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	m_clrNormal = ::GetSysColor(COLOR_BTNFACE);
}

CCJToolBar::~CCJToolBar()
{
	if( m_pControls ) {
		for( POSITION pos = m_pControls->GetHeadPosition() ; pos ; ) {
			delete m_pControls->GetNext(pos);
		}
		delete m_pControls;
	}
}

IMPLEMENT_DYNAMIC(CCJToolBar, CFlatToolBar)

BEGIN_MESSAGE_MAP(CCJToolBar, CFlatToolBar)
	//{{AFX_MSG_MAP(CCJToolBar)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCCREATE()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCJToolBar message handlers

//////////////////////////////////////////////////////////////////////
// Inserts a control into the toolbar at the given button id.
// pClass	- runtime class of the control.
// strTitle - window title ( if any ) of the control.
// pRect	- size of the control to be inserted.
// nID		- resource id of the button where the control is to be placed.
// dwStyle  - style flags for the control

CWnd* CCJToolBar::InsertControl(CRuntimeClass * pClass, CString strTitle, CRect & pRect, UINT nID, DWORD dwStyle)
{
	dwStyle |= WS_CHILD | WS_VISIBLE;
	CWnd* pCtrl = 0;
	
	CRect rect = pRect;
	BOOL bCreate = FALSE;
	
	// make sure the id is valid
	ASSERT( CommandToIndex( nID ) >= 0 );
	SetButtonInfo( CommandToIndex( nID ), nID, TBBS_SEPARATOR, pRect.Width());
	
	CString strClass( pClass->m_lpszClassName );
	
	if( strClass == TEXT("CCJFlatComboBox") ) {
		pCtrl = new CCJFlatComboBox();
		bCreate = ((CCJFlatComboBox*)pCtrl)->Create(dwStyle, rect, this, nID);
	}
	
	else if( strClass == TEXT("CComboBox") ) {
		pCtrl = new CComboBox();
		bCreate = ((CComboBox*)pCtrl)->Create(dwStyle, rect, this, nID);
	}
	
	else if( strClass == TEXT("CEdit") ) {
		pCtrl = new CEdit();
		bCreate = ((CEdit*)pCtrl)->Create(dwStyle, rect, this, nID);
	}
	
	else if( strClass == TEXT("CButton") ) {
		pCtrl = new CButton();
		bCreate = ((CButton*)pCtrl)->Create(strTitle,dwStyle, rect, this, nID);
	}
	
	else {
		pCtrl = (CWnd*)pClass->CreateObject();
		bCreate = pCtrl->Create(0, 0, dwStyle, rect, this, nID);
	}
	
	if( !pCtrl )
		return 0;

	if( !bCreate ) {
		delete pCtrl;
		return 0;
	}

	GetItemRect( CommandToIndex(nID), &pRect );
	
	pCtrl->SetWindowPos(0, pRect.left, pRect.top, 0, 0,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOCOPYBITS );
	
	pCtrl->SetFont(GetFont());
	pCtrl->ShowWindow( SW_SHOW );
	
	// we have to remember this control, so we can delete it later
	if( !m_pControls ) {
		m_pControls = new CObList();
		m_pControls->AddTail( pCtrl );
	}

	// David Cofer
	else
		m_pControls->AddTail( pCtrl);
	
	return pCtrl;
}

//////////////////////////////////////////////////////////////////////
// This draws the gripper bars seen to the left and
// top of toolbar

void CCJToolBar::DrawGripper(CDC & dc) const
{
	if (m_bGripper)
	{
		CRect rcGripper;
		GetWindowRect(rcGripper);
		ScreenToClient(rcGripper);
		rcGripper.OffsetRect(-rcGripper.left, -rcGripper.top);
		
		if( m_dwStyle & CBRS_ORIENT_HORZ ) {
			
			// Gripper at left
			rcGripper.DeflateRect(3, 3);
			rcGripper.right = rcGripper.left+3;
			dc.Draw3dRect(rcGripper, m_clrHilite, m_clrShadow );
			
			rcGripper.OffsetRect(4, 0);
			dc.Draw3dRect(rcGripper, m_clrHilite, m_clrShadow);
		}
		
		else {
			
			// Gripper at top
			rcGripper.DeflateRect(4, 4);
			rcGripper.bottom = rcGripper.top+3;
			dc.Draw3dRect(rcGripper, m_clrHilite, m_clrShadow);
			
			rcGripper.OffsetRect(0, 4);
			dc.Draw3dRect(rcGripper, m_clrHilite, m_clrShadow);
		}
	}
}

void CCJToolBar::EraseNonClient(BOOL)
{
	if (IsFloating()) {
		Default();
	}

	else
	{
		// get window DC that is clipped to the non-client area
		CWindowDC dc(this);
		CRect rectClient;
		GetClientRect(rectClient);
		CRect rectWindow;
		GetWindowRect(rectWindow);
		ScreenToClient(rectWindow);
		rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
		dc.ExcludeClipRect(rectClient);     // draw borders in non-client area

		// draw borders in non-client area
		rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
		DrawBorders(&dc, rectWindow);     // erase parts not drawn
		dc.IntersectClipRect(rectWindow);
		SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);
		DrawGripper(dc);
	}
}

void CCJToolBar::DrawBorders(CDC * pDC, CRect & rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	DWORD dwStyle = m_dwStyle;
	if (!(dwStyle & CBRS_BORDER_ANY))
		return;

	// prepare for dark lines
	ASSERT(rect.top == 0 && rect.left == 0);

	if(dwStyle & CBRS_BORDER_RIGHT)  // right
	{
		pDC->FillSolidRect(rect.right-1, 0, rect.right, rect.bottom, m_clrShadow);
		rect.right--;
	}
	if(dwStyle & CBRS_BORDER_BOTTOM)  // bottom
	{
		pDC->FillSolidRect(0, rect.bottom-1, rect.right, rect.bottom, m_clrShadow);
		rect.bottom--;
	}
	if(dwStyle & CBRS_BORDER_TOP) // top
	{
		pDC->FillSolidRect(0, 0, rect.right, 1, m_clrHilite);
		rect.top++;
	}
	if(dwStyle & CBRS_BORDER_LEFT) // left
	{
		pDC->FillSolidRect(0, 0, 1, rect.bottom, m_clrHilite);
		rect.left++;
	}
}

//////////////////////////////////////////////////////////////////////
// This offsets the non-client area to allow enough
// room for the gripper to be drawn

void CCJToolBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	if (!IsFloating())
	{
		if (m_bGripper)
		{	
			if( m_dwStyle & CBRS_ORIENT_HORZ ) {

				lpncsp->rgrc[0].left  += 4;
				lpncsp->rgrc[0].right += 4;
			}
			else {
				lpncsp->rgrc[0].top    += 6;
				lpncsp->rgrc[0].bottom += 6;
			}
		}
	}

	CFlatToolBar::OnNcCalcSize(bCalcValidRects, lpncsp);
}

//////////////////////////////////////////////////////////////////////
// This will draw the gripper on the toolbar then repaints
// client areas.

void CCJToolBar::OnNcPaint() 
{
	CControlBar::EraseNonClient();
	CWindowDC dc(this);
	
	CRect pRect;
	GetClientRect( &pRect );
	InvalidateRect( &pRect, TRUE );
	EraseNonClient(FALSE);
	
	// Do not call CFlatToolBar::OnNcPaint() for painting messages
}

void CCJToolBar::OnSysColorChange() 
{
	CFlatToolBar::OnSysColorChange();
	m_clrHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	m_clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	m_clrNormal = ::GetSysColor(COLOR_BTNFACE);
}
