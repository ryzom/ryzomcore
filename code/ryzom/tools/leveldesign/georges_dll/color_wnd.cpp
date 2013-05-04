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

// color_wnd.cpp : implementation file
//

#include "stdafx.h"
#include "georges_edit.h"
#include "color_wnd.h"

using namespace NLMISC;

/////////////////////////////////////////////////////////////////////////////
// CColorWnd

CColorWnd::CColorWnd()
{
	Color = CRGBA::Black;
}

CColorWnd::~CColorWnd()
{
}


BEGIN_MESSAGE_MAP(CColorWnd, CWnd)
	//{{AFX_MSG_MAP(CColorWnd)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CColorWnd message handlers

void CColorWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Get client dc
	RECT client;
	GetClientRect (&client);
	
	COLORREF color = RGB (Color.R, Color.G, Color.B);

	CBrush brush;
	brush.CreateSolidBrush (color);
	CBrush *old = dc.SelectObject (&brush);
	dc.Rectangle (&client);
	dc.SelectObject (old);

	if (GetFocus ()== this)
		dc.DrawFocusRect (&client);
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CColorWnd::create (DWORD wStyle, RECT &pos, CWnd *parent, uint dialogIndex)
{
	Id = dialogIndex;
	// Register window class
	LPCTSTR className = AfxRegisterWndClass(CS_DBLCLKS); 

	// Create this window
	if (CWnd::Create (className, "empty", wStyle, pos, parent, dialogIndex))
	{
	}
}


void CColorWnd::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	// Force redraw
	Invalidate ();

	// Send the notify message to the parent
	NMHDR strNotify;
	strNotify.hwndFrom = *this;
	strNotify.idFrom = Id;
	strNotify.code = NM_SETFOCUS;
	GetParent ()->SendMessage (WM_NOTIFY, Id, (LPARAM)&strNotify);
}

void CColorWnd::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	
	// Force redraw
	Invalidate ();

	// Send the notify message to the parent
	NMHDR strNotify;
	strNotify.hwndFrom = *this;
	strNotify.idFrom = Id;
	strNotify.code = NM_KILLFOCUS;
	GetParent ()->SendMessage (WM_NOTIFY, Id, (LPARAM)&strNotify);
}

void CColorWnd::setColor (const NLMISC::CRGBA &color)
{
	Color = color;

	char buffer[256];
	sprintf(buffer, "%d,%d,%d", Color.R, Color.G, Color.B);
	SetWindowText(buffer);
	
	Invalidate ();
	GetParent()->PostMessage (CBN_CHANGED, 0, 0);
}

NLMISC::CRGBA CColorWnd::getColor () const
{
	return Color;
}

void CColorWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// Select a color
	if (theApp.getColor (Color))
	{
		colorChanged ();
	}
 	
	CWnd::OnLButtonDblClk(nFlags, point);
}

BOOL CColorWnd::PreTranslateMessage(MSG* pMsg) 
{
	if ( (pMsg->message==WM_KEYDOWN) && (pMsg->wParam == VK_RETURN))
	{
		if (theApp.getColor (Color))
		{
			colorChanged ();
		}
	}
	
	return CWnd::PreTranslateMessage(pMsg);
}

void CColorWnd::colorChanged ()
{
	updateEdit();
	Invalidate ();

	char buffer[256];
	sprintf(buffer, "%d,%d,%d", Color.R, Color.G, Color.B);
	SetWindowText(buffer);

	CWnd *wnd = GetParent ();
	if (wnd)
	{
		wnd->SendMessage (CL_CHANGED, Id, 0);
		wnd->PostMessage (CBN_CHANGED, 0, 0);
	}
}

void CColorWnd::updateEdit()
{
	char buffer[256];
	sprintf(buffer, "%d,%d,%d", Color.R, Color.G, Color.B);
	pEdit->SetWindowText(buffer);
}