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

// WorkspaceWnd.cpp: implementation of the CWorkspaceWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ide2.h"
#include "WorkspaceWnd.h"
#include "TreeViewFiles.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWorkspaceWnd::CWorkspaceWnd()
{

}

CWorkspaceWnd::~CWorkspaceWnd()
{

}

BOOL CWorkspaceWnd::Create(CWnd *pParentWnd, UINT nID, LPCTSTR lpszWindowName, CSize sizeDefault, DWORD dwStyle)
{
	BOOL bRet = CCJTabCtrlBar::Create(pParentWnd, nID, lpszWindowName, sizeDefault, dwStyle);
	if ( !bRet )
		return FALSE;

	AddView(_T("Files"),    RUNTIME_CLASS(CTreeViewFiles));

	m_tabImages.Create (IDB_IL_TAB, 16, 1, RGB(0,255,0));
	SetTabImageList(&m_tabImages);

	Enable(FALSE);

	return TRUE;
}

CTreeViewFiles* CWorkspaceWnd::GetTreeViewFiles()
{
	return (CTreeViewFiles*)GetView(RUNTIME_CLASS(CTreeViewFiles));
}

void CWorkspaceWnd::Enable(BOOL bEnable)
{
	GetView(0)->EnableWindow(bEnable);
}
