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

// CJOutlookBar.cpp : implementation file
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
//	<>  Thanks to Shekar Narayanan (ShekarNarayanan@Hotmail.com) for his article
//		'A Cool Looking Menu For Easier Navigation' which is where I got the idea
//		for this class from.
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
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CJOutlookBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCJOutlookBar

CCJOutlookBar::CCJOutlookBar()
{
    m_bLBDown   = false;
    m_bHilight  = true;
}

CCJOutlookBar::~CCJOutlookBar()
{
	// TODO: add destruction code here.
}

IMPLEMENT_DYNAMIC(CCJOutlookBar, CListBox)

BEGIN_MESSAGE_MAP(CCJOutlookBar, CListBox)
	//{{AFX_MSG_MAP(CCJOutlookBar)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCJOutlookBar message handlers

// Checks mouse movement, sets point member, sets timer.
void CCJOutlookBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	m_point = point;
	SetTimer (1, 10, NULL);
	CListBox::OnMouseMove(nFlags, point);
}

// Sets bLBDown member to true, and sets current selection
void CCJOutlookBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
    m_bLBDown = true;
    SetCurSel(GetCurSel());
	CListBox::OnLButtonDown(nFlags, point);
}

// Sets bLBDown member to false, and sets current selection
void CCJOutlookBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
    m_bLBDown	= false;
    SetCurSel(GetCurSel());
	
	p_Owner->SendMessage(WM_OUTLOOKBAR_NOTIFY,
		OBM_ITEMCLICK, GetCurSel());

	CListBox::OnLButtonUp(nFlags, point);
}

// Sets the hilite flag based on cursor position.
void CCJOutlookBar::OnTimer(UINT nIDEvent) 
{
    BOOL   bOutSide;
    CRect  rItem;
	CRect  rWindow;
	CPoint pt;
    int    nIndex;

    // Get the rect of the current window,
	// get the current cursor position.
	GetWindowRect (rWindow);
    GetCursorPos (&pt);

    // If the mouse is in the rect of this window,
	// set hilite flag to true, get the item that
	// the mouse is over.
	if (rWindow.PtInRect (pt)) {
		m_bHilight = true;
		nIndex = ItemFromPoint (m_point, bOutSide);

		if (nIndex != GetCurSel() && !bOutSide) {
			SetCurSel( nIndex );
		}
    }

    // If the mouse is not in the rect of
	// this window, kill the timer, set hilite flag
	// to false, and return.
	else {
		m_bHilight = false;
        if( !m_bLBDown ) {
          SetCurSel (-1);
        }
        KillTimer(1);
	}
}

//////////////////////////////
// Initial draw routine, draws button on mouse over,
// on mouse press, and on mouse out.

void CCJOutlookBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
    CDC*            pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
    CRect           rcItem(lpDrawItemStruct->rcItem);
    CRect           rClient(rcItem);
    CRect           rText;
    CPoint          Pt(((rcItem.Width() - 32) /2 ), rcItem.top + 15);
    CContentItems   pContents = m_pContents[ lpDrawItemStruct->itemID ];
    HICON           hIcon = m_ImageListNormal.ExtractIcon( pContents.GetImageID());

	// deflate the client rect, and set up item and
	// text rect's.
	rClient.DeflateRect( 8, 5 );
	rcItem.CopyRect( rClient );
    rText.CopyRect( rClient );
    rText.top += 45;
	rText.bottom += 10;

    // set the text color to white, and background mode
	// to transparent.
	pDC->SetTextColor( ::GetSysColor( COLOR_WINDOW ) );
	pDC->SetBkMode( TRANSPARENT );

	// draw the button icon and text.
	if( lpDrawItemStruct->itemAction& ODA_DRAWENTIRE ) {
        pDC->DrawIcon( Pt, hIcon );
        pDC->DrawText( pContents.GetText(), rText,
			DT_CENTER | DT_WORDBREAK );
    }

    // if the left mouse button is pressed, offset
	// the button icon.
	if( m_bLBDown ) {
        Pt.x += 2;
        Pt.y += 2;
        rcItem.OffsetRect( 2, 2 );
    }

	// deflate the button rect, and move up.
	rcItem.DeflateRect( 4, 16 );
	rcItem.top -= 8;

	// Draw the button icon and text, if the mouse is over the
	// button, draw the 3d rect, either pressed,
	// or not pressed depending on left mouse button.
	if(( lpDrawItemStruct->itemState& ODS_SELECTED ) &&
		( lpDrawItemStruct->itemAction& ( ODA_SELECT | ODA_DRAWENTIRE ))) {
        
		rcItem.left = ( rcItem.Width()/2 )-5;
		rcItem.right = rcItem.left+37;

		if( m_bHilight ) {
            if( m_bLBDown ) {
              pDC->Draw3dRect( rcItem, ::GetSysColor( COLOR_3DDKSHADOW ),
				  ::GetSysColor( COLOR_BTNFACE ) );
			}
            else {
              pDC->Draw3dRect( rcItem, ::GetSysColor( COLOR_BTNFACE ),
				  ::GetSysColor( COLOR_3DDKSHADOW ) );
			}
        }

        pDC->DrawIcon( Pt, hIcon );
        pDC->DrawText( pContents.GetText(), rText,
			DT_CENTER | DT_WORDBREAK );
	}

	if( !( lpDrawItemStruct->itemState& ODS_SELECTED ) &&
		( lpDrawItemStruct->itemAction& ODA_SELECT )) {
        pDC->FillSolidRect( rClient, ::GetSysColor( COLOR_3DSHADOW ) );
        pDC->DrawIcon(Pt, hIcon);
        pDC->DrawText(pContents.GetText(), rText, DT_CENTER|DT_WORDBREAK);
    }
}

//////////////////////////////
// Sets item height

void CCJOutlookBar::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	lpMeasureItemStruct->itemHeight = 70;
}

//////////////////////////////
// Sets the background color.

BOOL CCJOutlookBar::OnEraseBkgnd(CDC* pDC) 
{
	// Get the rect of the client area, and fill
	// the background color.
	CRect rcClient;
	GetClientRect( &rcClient );
	pDC->FillRect( rcClient, &CBrush( ::GetSysColor( COLOR_3DSHADOW ) ));
	return TRUE;
}

//////////////////////////////
// Initializes the ContentMenu

void CCJOutlookBar::SetItems(CContentItems * pItems, int nNumItems)
{
	m_pContents = pItems;
	m_nNumItems = nNumItems;

	for( int i = 0; i < nNumItems; i++ ) {
		AddString(m_pContents[i].GetText());
	}
}
