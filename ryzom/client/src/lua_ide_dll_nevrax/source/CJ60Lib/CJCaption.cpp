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

// CJCaption.cpp : implementation file
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
// ==========================================================================  
// HISTORY:	  
// ==========================================================================  
//			1.00	16 Jan 1999	- Initial first release.  
// ==========================================================================  
//  
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CJCaption.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCJCaption

CCJCaption::CCJCaption()
{
}

CCJCaption::~CCJCaption()
{
}


BEGIN_MESSAGE_MAP(CCJCaption, CStatic)
	//{{AFX_MSG_MAP(CCJCaption)
	ON_WM_PAINT()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCJCaption message handlers

void CCJCaption::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// Get the client rect for this control.
	CRect rc;
	GetClientRect(&rc);
	
	// Set the background to transparent, and draw a 3D
	// border around the control.
	dc.SetBkMode(TRANSPARENT);
	dc.FillSolidRect(rc, ::GetSysColor(COLOR_BTNFACE));

	rc.bottom += 2;
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW),
		::GetSysColor(COLOR_3DHILIGHT));
	rc.DeflateRect(1,1);
	rc.bottom += 2;
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DHILIGHT),
		::GetSysColor(COLOR_3DSHADOW));
	
	// Get the log font.
	CFont newFont, *oldFont;
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
		sizeof(NONCLIENTMETRICS), &ncm, 0));
	newFont.CreateFontIndirect(&ncm.lfMessageFont);
	
	// Get the window text for this control.
	CString strText;
	GetWindowText(strText);
	rc.left += 10;

	// Shuffle fonts and paint the text.
	oldFont = dc.SelectObject(&newFont);
	dc.DrawText(strText, rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	dc.SelectObject(oldFont);
}

void CCJCaption::OnSysColorChange() 
{
	CStatic::OnSysColorChange();
}
