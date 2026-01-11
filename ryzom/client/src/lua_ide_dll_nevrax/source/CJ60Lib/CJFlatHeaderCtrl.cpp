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

// CJFlatHeaderCtrl.cpp : implementation file
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
//      he has done with codeguru, enough can not be said, and for his article
//		'Indicating sort order in header control' which is where the code for
//		the sorting arrows in the header comes from.
//	<>  Many thanks to Microsoft for making the source code availiable for MFC. 
//		Since most of this work is a modification from existing classes and 
//		methods, this library would not have been possible.
//
// ==========================================================================  
// HISTORY:	  
// ==========================================================================  
//			1.00	16 Jan 1999	- Initial release.  
// ==========================================================================  
//  
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CJFlatHeaderCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJFlatHeaderCtrl

CCJFlatHeaderCtrl::CCJFlatHeaderCtrl()
{
	m_nSortCol	  = -1;
	m_bFlatHeader = FALSE;
	m_bBoldFont	  = FALSE;
}

CCJFlatHeaderCtrl::~CCJFlatHeaderCtrl()
{
}

BEGIN_MESSAGE_MAP(CCJFlatHeaderCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(CCJFlatHeaderCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCJFlatHeaderCtrl message handlers

void CCJFlatHeaderCtrl::FlatHeader(BOOL bBoldFont/*=TRUE*/)
{
	if (bBoldFont)
	{
		m_boldFont.CreateFont( -11, 0, 0, 0, FW_BOLD, 0, 0, 0,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH, _T("Tahoma"));
		SetFont(&m_boldFont);
	}

	m_bBoldFont   = bBoldFont;
	m_bFlatHeader = TRUE;
}

int CCJFlatHeaderCtrl::SetSortImage(int nCol, BOOL bAsc)
{
	int nPrevCol = m_nSortCol;
	
	m_nSortCol = nCol;
	m_bSortAsc = bAsc;
	
	// Change the item to ownder drawn
	HD_ITEM hditem;
	
	hditem.mask = HDI_FORMAT;
	GetItem( nCol, &hditem );
	hditem.fmt |= HDF_OWNERDRAW;
	SetItem( nCol, &hditem );
	
	// Invalidate header control so that it gets redrawn
	Invalidate();
	return nPrevCol;
}

void CCJFlatHeaderCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );
	
	// Get the column rect
	CRect rcLabel( lpDrawItemStruct->rcItem );
	
	// Save DC
	int nSavedDC = dc.SaveDC();
	
	// Set clipping region to limit drawing within column
	CRgn rgn;
	rgn.CreateRectRgnIndirect( &rcLabel );
	dc.SelectObject( &rgn );
	rgn.DeleteObject();
	
	// Draw the background
	dc.FillRect(rcLabel, &CBrush(::GetSysColor(COLOR_3DFACE)));
	dc.SetBkMode(TRANSPARENT);

	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	int offset = dc.GetTextExtent(_T(" "), 1 ).cx*2;
	
	// Get the column text and format
	TCHAR buf[256];
	HD_ITEM hditem;
	
	hditem.mask = HDI_TEXT | HDI_FORMAT;
	hditem.pszText = buf;
	hditem.cchTextMax = 255;
	
	GetItem( lpDrawItemStruct->itemID, &hditem );
	
	// Determine format for drawing column label
	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP 
		| DT_VCENTER | DT_END_ELLIPSIS ;
	
	if( hditem.fmt & HDF_CENTER)
		uFormat |= DT_CENTER;
	else if( hditem.fmt & HDF_RIGHT)
		uFormat |= DT_RIGHT;
	else
		uFormat |= DT_LEFT;
	
	// Adjust the rect if the mouse button is pressed on it
	if( lpDrawItemStruct->itemState == ODS_SELECTED )
	{
		rcLabel.left++;
		rcLabel.top += 2;
		rcLabel.right++;
	}
	
	// Adjust the rect further if Sort arrow is to be displayed
	if( lpDrawItemStruct->itemID == (UINT)m_nSortCol )
	{
		rcLabel.right -= 3 * offset;
	}
	
	rcLabel.left += offset;
	rcLabel.right -= offset;

	// Draw column label
	if( rcLabel.left < rcLabel.right )
	{
		if (m_bBoldFont) {
			dc.SelectObject(&m_boldFont);
		}
		dc.DrawText(buf,-1,rcLabel, uFormat);
	}

	// Draw the Sort arrow
	if( lpDrawItemStruct->itemID == (UINT)m_nSortCol )
	{
		CRect rcIcon( lpDrawItemStruct->rcItem );
		
		// Set up pens to use for drawing the triangle
		CPen penLight(PS_SOLID, 1, ::GetSysColor(COLOR_3DHILIGHT));
		CPen penShadow(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
		CPen *pOldPen = dc.SelectObject( &penLight );
		
		if( m_bSortAsc )
		{
			// Draw triangle pointing upwards
			dc.MoveTo( rcIcon.right - 2*offset, offset-1);
			dc.LineTo( rcIcon.right - 3*offset/2, rcIcon.bottom - offset );
			dc.LineTo( rcIcon.right - 5*offset/2-2, rcIcon.bottom - offset );
			dc.MoveTo( rcIcon.right - 5*offset/2-1, rcIcon.bottom - offset-1 );
			
			dc.SelectObject( &penShadow );
			dc.LineTo( rcIcon.right - 2*offset, offset-2);
		}
		else
		{
			// Draw triangle pointing downwords
			dc.MoveTo( rcIcon.right - 3*offset/2, offset-1);
			dc.LineTo( rcIcon.right - 2*offset-1, rcIcon.bottom - offset + 1 );
			dc.MoveTo( rcIcon.right - 2*offset-1, rcIcon.bottom - offset );
			
			dc.SelectObject( &penShadow );
			dc.LineTo( rcIcon.right - 5*offset/2-1, offset -1 );
			dc.LineTo( rcIcon.right - 3*offset/2, offset -1);
		}
		
		// Restore the pen
		dc.SelectObject( pOldPen );
	}

	// Restore dc
	dc.RestoreDC( nSavedDC );
	
	// Detach the dc before returning
	dc.Detach();
}

void CCJFlatHeaderCtrl::OnPaint() 
{
	Default();
	
	if (m_bFlatHeader)
		DrawFlatBorder();
}

void CCJFlatHeaderCtrl::DrawFlatBorder()
{
	CDC* pDC = GetDC();

	CRect rci;
	GetWindowRect(&rci);
	ScreenToClient(&rci);

	// Cover up thick 3D border.
	pDC->Draw3dRect(rci, ::GetSysColor(COLOR_3DFACE),
		::GetSysColor(COLOR_3DFACE));
	rci.DeflateRect(1,1);
	pDC->Draw3dRect(rci, ::GetSysColor(COLOR_3DFACE),
		::GetSysColor(COLOR_3DFACE));

	// Draw flat style border around entire header.
	rci.InflateRect(1,1);
	pDC->Draw3dRect(rci, ::GetSysColor(COLOR_3DHILIGHT),
		::GetSysColor(COLOR_3DSHADOW));

	// Create the pens for further cover-up.
	CPen penLight(PS_SOLID, 1, ::GetSysColor(COLOR_3DHILIGHT));
	CPen penShadow(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
	CPen penButton(PS_SOLID, 1, ::GetSysColor(COLOR_3DFACE));
	CPen *pOldPen = pDC->SelectObject( &penLight );

	pDC->SelectObject(&penButton);
	pDC->MoveTo(rci.right-1, 2);
	pDC->LineTo(rci.right-1, rci.bottom-2);

	// Set up the header item struct.
	HD_ITEM hdi;
	memset (&hdi, 0, sizeof(HD_ITEM));
	hdi.fmt  = HDF_STRING | HDF_LEFT | HDF_OWNERDRAW;
	hdi.mask = HDI_WIDTH | HDI_TEXT | HDI_FORMAT;
	int cx = 0;

	// For each header item found, do further cover up.
	for (int i = 0; i < GetItemCount(); ++i)
	{
		GetItem(i, &hdi);
		cx += hdi.cxy;

		pDC->SelectObject(&penShadow);
		pDC->MoveTo(cx, 2);
		pDC->LineTo(cx, rci.bottom-2);

		pDC->SelectObject(&penLight);
		pDC->MoveTo(cx+1, 2);
		pDC->LineTo(cx+1, rci.bottom-2);

		pDC->SelectObject(&penButton);
		pDC->MoveTo(cx-1, 2);
		pDC->LineTo(cx-1, rci.bottom-2);

		pDC->SelectObject(&penButton);
		pDC->MoveTo(cx-2, 2);
		pDC->LineTo(cx-2, rci.bottom-2);
	}

	// Restore the pen and release device context.
	pDC->SelectObject( pOldPen );
	ReleaseDC(pDC);
}
