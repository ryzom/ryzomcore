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

// CallStack.cpp: implementation of the CCallStack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ide2.h"
#include "CallStack.h"

#include "ScintillaView.h"
#include "MainFrame.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCallStack::CCallStack()
{

}

CCallStack::~CCallStack()
{

}

BOOL CCallStack::Create(CWnd *pParentWnd, UINT nID, LPCTSTR lpszWindowName, CSize sizeDefault, DWORD dwStyle)
{
	BOOL bRet = CCJTabCtrlBar::Create(pParentWnd, nID, lpszWindowName, sizeDefault, dwStyle);
	if ( !bRet )
		return FALSE;

	AddView(_T("Call Stack"),    RUNTIME_CLASS(CScintillaView));

	CLuaEditor* pEditor = ((CScintillaView*)GetView(0))->GetEditor();
	pEditor->SetCallStackMargins();

	Clear();
	return TRUE;
}

int CCallStack::OnSci(CScintillaView* pView, SCNotification* pNotify)
{
	CLuaEditor* pEditor = ((CScintillaView*)GetView(0))->GetEditor();

	CPoint pt;
	int nLine;
	CString strLine;
	switch (pNotify->nmhdr.code)
	{
		case SCN_DOUBLECLICK:
			GetCursorPos(&pt);
			pEditor->ScreenToClient(&pt);
			nLine = pEditor->LineFromPoint(pt);
			GotoStackTraceLevel(nLine-1);
		break;
	};

	return 0;
}

void CCallStack::Clear()
{
	((CScintillaView*)GetView(0))->Clear();

	m_nCurrentLevel = -1;
	m_lines.RemoveAll();
	m_files.RemoveAll();
}

void CCallStack::Add(const char *szDesc, const char *szFile, int nLine)
{
	((CScintillaView*)GetView(0))->Write(CString(szDesc)+"\n");

	m_files.Add(szFile);
	m_lines.Add(nLine);
}

void CCallStack::GotoStackTraceLevel(int nLevel)
{
	if ( nLevel<0 || nLevel>=m_files.GetSize() )
		return;

	m_nCurrentLevel = nLevel;

	CLuaEditor* pEditor = ((CScintillaView*)GetView(0))->GetEditor();
	pEditor->SetStackTraceLevel(nLevel);

	((CMainFrame*)AfxGetMainWnd())->GotoFileLine(m_files[nLevel], m_lines[nLevel]);	
	((CMainFrame*)AfxGetMainWnd())->GetDebugger()->StackLevelChanged();
}
