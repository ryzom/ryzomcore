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

// OutputWnd.cpp: implementation of the COutputWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ide2.h"
#include "OutputWnd.h"
#include "ScintillaView.h"
#include "LuaHelper.h"
#include "MainFrame.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COutputWnd::COutputWnd()
{

}

COutputWnd::~COutputWnd()
{

}

BOOL COutputWnd::Create(CWnd *pParentWnd, UINT nID, LPCTSTR lpszWindowName, CSize sizeDefault, DWORD dwStyle)
{
	BOOL bRet = CCJTabCtrlBar::Create(pParentWnd, nID, lpszWindowName, sizeDefault, dwStyle);
	if ( !bRet )
		return FALSE;

	AddView(_T("Build"),    RUNTIME_CLASS(CScintillaView));
	AddView(_T("Debug"),    RUNTIME_CLASS(CScintillaView));
	AddView(_T("Find in Files"),    RUNTIME_CLASS(CScintillaView));

	return TRUE;
}

int COutputWnd::OnSci(CScintillaView* pView, SCNotification* pNotify)
{
	if ( pView==GetView(outputBuild) )
		return OnBuildSci(pNotify);
	else if ( pView==GetView(outputDebug) )
		return OnDebugSci(pNotify);

	return 0;
}

int COutputWnd::OnBuildSci(SCNotification* pNotify)
{
	CLuaEditor* pEditor = GetOutput(outputBuild)->GetEditor();

	CPoint pt;
	int nLine;
	CString strLine;
	switch (pNotify->nmhdr.code)
	{
		case SCN_DOUBLECLICK:
			GetCursorPos(&pt);
			pEditor->ScreenToClient(&pt);
			nLine = pEditor->LineFromPoint(pt);
			strLine = pEditor->GetLine(nLine);
			GotoLine(strLine);
		break;
	};

	return 0;
}

int COutputWnd::OnDebugSci(SCNotification* pNotify)
{
	CLuaEditor* pEditor = GetOutput(outputDebug)->GetEditor();

	CPoint pt;
	int nLine;
	CString strLine;
	switch (pNotify->nmhdr.code)
	{
		case SCN_DOUBLECLICK:
			GetCursorPos(&pt);
			pEditor->ScreenToClient(&pt);
			nLine = pEditor->LineFromPoint(pt);
			strLine = pEditor->GetLine(nLine);
			GotoLine(strLine);
		break;
	};

	return 0;
}

void COutputWnd::GotoLine(CString strLine)
{
	CString strPathName;
	int nLine;

	if ( !CLuaHelper::ErrorStringToFileLine(strLine, strPathName, nLine) )
		return;

	((CMainFrame*)AfxGetMainWnd())->GotoFileLine(strPathName, nLine);	
}
