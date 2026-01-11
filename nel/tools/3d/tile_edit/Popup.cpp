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

#include "stdafx.h"
#include "Popup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Popup

Popup::Popup()
{
}

Popup::~Popup()
{
}

LRESULT Popup::DefWindowProc(UINT message,WPARAM wParam,LPARAM lParam)
{
/*	if (message==WM_INIT)
	{
		int toto = 0;
	}*/
	if (message==WM_PAINT)
	{
		int toto = 0;
	}
	return CWnd::DefWindowProc(message,wParam,lParam);
}

BEGIN_MESSAGE_MAP(Popup, CWnd)
	//{{AFX_MSG_MAP(Popup)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Popup message handlers
