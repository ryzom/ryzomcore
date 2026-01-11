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

// MDIClientWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "MDIClientWnd.h"

#include "MainFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMDIClientWnd

CMDIClientWnd::CMDIClientWnd()
{
}

CMDIClientWnd::~CMDIClientWnd()
{
}


BEGIN_MESSAGE_MAP(CMDIClientWnd, CWnd)
	//{{AFX_MSG_MAP(CMDIClientWnd)
	ON_MESSAGE(WM_MDISETMENU, OnMDISetMenu)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMDIClientWnd message handlers

LRESULT CMDIClientWnd::OnMDISetMenu(WPARAM wParam, LPARAM lParam)
{
	Default();

	ResetMenu();

	return 0;
}

void CMDIClientWnd::PreSubclassWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	VERIFY(m_menuBuild.LoadMenu(IDR_SUBMENU_BUILD));
	VERIFY(m_menuDebug.LoadMenu(IDR_SUBMENU_DEBUG));
	VERIFY(m_menuProject.LoadMenu(IDR_PROJECT_MENU));
	
	CWnd::PreSubclassWindow();
}

BOOL CMDIClientWnd::OnEraseBkgnd(CDC* pDC) 
{
/*      CBrush backBrush(RGB(58, 111, 165));

      // Save old brush
      CBrush* pOldBrush = pDC->SelectObject(&backBrush);

      CRect rect;
      pDC->GetClipBox(&rect);     // Erase the area needed

      pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(),
          PATCOPY);
      pDC->SelectObject(pOldBrush);
      return TRUE;
*/	
	return CWnd::OnEraseBkgnd(pDC);
}

void CMDIClientWnd::ResetMenu()
{
	CMenu* pMenu = AfxGetMainWnd()->GetMenu();

	CString strMenu;
	int nMenuItemCount = pMenu->GetMenuItemCount();

	for ( int i=0; i<nMenuItemCount; ++i )
	{
		pMenu->GetMenuString(i, strMenu, MF_BYPOSITION);
		if ( strMenu=="&Build" || strMenu=="&Debug" || strMenu=="&Project")
		{
			pMenu->RemoveMenu(i, MF_BYPOSITION);
			--i;
			--nMenuItemCount;
		}
	}

	pMenu->GetMenuString(1, strMenu, MF_BYPOSITION);
	int nPos = 2;
	if ( strMenu=="&Edit" )
		++nPos;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if ( pFrame->GetMode() != pFrame->modeNoProject )
	{
		pMenu->InsertMenu(nPos, MF_BYPOSITION|MF_POPUP|MF_ENABLED, 
			(UINT)m_menuProject.GetSubMenu(0)->GetSafeHmenu(), "&Project");
		++nPos;
	}

	if ( pFrame->GetMode() == pFrame->modeBuild )
		pMenu->InsertMenu(nPos, MF_BYPOSITION|MF_POPUP|MF_ENABLED, 
			(UINT)m_menuBuild.GetSubMenu(0)->GetSafeHmenu(), "&Build");
	else if ( pFrame->GetMode()==pFrame->modeDebug || pFrame->GetMode()==pFrame->modeDebugBreak )
		pMenu->InsertMenu(nPos, MF_BYPOSITION|MF_POPUP|MF_ENABLED, 
			(UINT)m_menuDebug.GetSubMenu(0)->GetSafeHmenu(), "&Debug");
}
