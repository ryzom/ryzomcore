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

// bar.cpp : implementation file
//

#include "stdafx.h"
#include "client_config.h"
#include "bar.h"

// ***************************************************************************
// CBar
// ***************************************************************************

CBar::CBar()
{
}

CBar::~CBar()
{
}


BEGIN_MESSAGE_MAP(CBar, CWnd)
	//{{AFX_MSG_MAP(CBar)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ***************************************************************************
// CBar message handlers
// ***************************************************************************

void CBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	RECT client;
	GetClientRect (&client);

	CPen hilight (PS_SOLID, 1, GetSysColor (COLOR_3DHILIGHT));
	CPen shadow (PS_SOLID, 1, GetSysColor (COLOR_3DSHADOW));

	if (client.right - client.left > client.bottom - client.top)
	{
		dc.SelectObject (shadow);
		dc.MoveTo (client.left, client.top);
		dc.LineTo (client.right, client.top);
		dc.SelectObject (hilight);
		dc.MoveTo (client.left, client.bottom);
		dc.LineTo (client.right, client.bottom);
	}
	else
	{
		dc.SelectObject (hilight);
		dc.MoveTo (client.left, client.top);
		dc.LineTo (client.left, client.bottom);
		dc.SelectObject (shadow);
		dc.MoveTo (client.right, client.top);
		dc.LineTo (client.right, client.bottom);
	}
	
	// Do not call CWnd::OnPaint() for painting messages
}

// ***************************************************************************

BOOL CBar::Create (CRect &rect, CWnd *parent)
{
	return CWnd::Create ("Static", "", WS_CHILD|WS_VISIBLE, rect, parent, 0);
}
