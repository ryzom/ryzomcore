// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


// color_button.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "color_button.h"


WNDPROC CColorButton::_BasicButtonWndProc = NULL  ;


/////////////////////////////////////////////////////////////////////////////
// CColorButton

CColorButton::CColorButton() : _Color(CRGBA::White)
{		
}

CColorButton::~CColorButton()
{
}


BEGIN_MESSAGE_MAP(CColorButton, CButton)
	//{{AFX_MSG_MAP(CColorButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorButton message handlers

void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	HDC dc = lpDrawItemStruct->hDC ;
	
	RECT r ;

	GetClientRect(&r) ;

	r.left += 4 ;
	r.top +=  4 ;
	r.bottom -= 4 ;
	r.right -= 4 ;
	GetClientRect(&r) ;
	CBrush b ;
	b.CreateSolidBrush(RGB(_Color.R, _Color.G, _Color.B)) ;
	::FillRect(dc, &r, (HBRUSH) b) ;	


}
