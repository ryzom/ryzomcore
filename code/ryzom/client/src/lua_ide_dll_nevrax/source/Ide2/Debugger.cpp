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

// Debugger.cpp: implementation of the CDebugger class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ide2.h"
#include "Debugger.h"

#include "MainFrame.h"
#include "ScintillaView.h"
#include "LuaDoc.h"
#include "LuaView.h"
#include "../../include/lua_ide_dll/ide_interface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebugger* CDebugger::m_pDebugger = NULL;

CDebugger::CDebugger()
{
	m_pDebugger = this;
	m_pThread = NULL;
	m_ExternalDebugging = false;
	m_monoThreadEvent = false;
	m_EvaluatingCondition = false;
}

CDebugger::~CDebugger()
{
	if(m_pThread!=NULL)
 		delete m_pThread;
}

void CDebugger::ResetEvent()
{
	if (m_ExternalDebugging)
	{
		m_monoThreadEvent = false;
	}
	else
	{
		m_event.ResetEvent();
	}
}

void CDebugger::SetEvent()
{
	if (m_ExternalDebugging)
	{
		m_monoThreadEvent = true;
	}
	else
	{
		m_event.SetEvent();
	}
}


HWND CDebugger::GetMainWnd()
{
	return theApp.GetMainFrame()->m_hWnd;
}


void CDebugger::SetExternalDebugging()
{
	m_ExternalDebugging = true;
	m_lua.SetExternalDebugging();
}


BOOL CDebugger::Prepare(lua_realloc_t reallocfunc, lua_free_t freefunc)
{
	m_hWndMainFrame = AfxGetMainWnd()->GetSafeHwnd();

	COutputWnd* pBar = ((CMainFrame*)AfxGetMainWnd())->GetOutputWnd();
	pBar->SetActiveOutput(COutputWnd::outputDebug);

	CScintillaView* pTab = pBar->GetOutput(COutputWnd::outputDebug);
	pTab->Clear();

	CProject *pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();
	if ( pProject->PositionBreakPoints() )
		AfxMessageBox("One or more breakpoints are not positioned on valid lines. These breakpoints have been moved to the next valid line.", MB_OK);

	m_lua.PrepareDebugger(reallocfunc, freefunc);

	m_nMode = DMOD_NONE;
	
	return TRUE;
}

BOOL CDebugger::Start()
{
	if (m_ExternalDebugging)
	{
		StartDebugger();
		return TRUE;
	}
	if(m_pThread!=NULL)
 		delete m_pThread;	
 	m_pThread = AfxBeginThread(StartDebugger, this,0,0,CREATE_SUSPENDED);
 	if(m_pThread!=NULL)
 	{
 		m_pThread->m_bAutoDelete=false;
 		m_pThread->ResumeThread();
 	}

	return m_pThread!=NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// thread functions
//////////////////////////////////////////////////////////////////////////////////////////////

UINT CDebugger::StartDebugger()
{
	m_nLine = 0;
	m_nLevel = 0;

	::SendMessage(m_hWndMainFrame, DMSG_DEBUG_START, 0, 0);
	
	int nRet = m_lua.StartDebugger();

	if (!m_ExternalDebugging)
	{
		EndThread();
	}
	
	return nRet;
}

UINT CDebugger::StartDebugger( LPVOID pParam )
{	
	return ((CDebugger*)pParam)->StartDebugger();
}

void CDebugger::Write(const char* szMsg)
{
	::SendMessage(m_hWndMainFrame, DMSG_WRITE_DEBUG, (WPARAM)szMsg, 0);
}

void CDebugger::LineHook(const char *szFile, int nLine)
{
	if ( m_nMode == DMOD_STOP && !m_ExternalDebugging)
	{
		EndThread();
	}	
	BOOL hasBreakPoint = false;
	if (m_ExternalDebugging)
	{
		if (theApp.GetMainFrame()->GetProject()->IsBreakPointPossibleAtLine(nLine))
		{
			hasBreakPoint = theApp.GetMainFrame()->GetProject()->HasBreakPoint(szFile, nLine);	
		}
	}
	else
	{
		hasBreakPoint = ::SendMessage(m_hWndMainFrame, DMSG_HAS_BREAKPOINT, (WPARAM)szFile, (LPARAM)nLine);
	}

	if (m_nMode==DMOD_STEP_INTO || 
		m_nMode==DMOD_BREAK ||
		(m_nMode==DMOD_STEP_OVER && m_nLevel<=0) || 
		(m_nMode==DMOD_STEP_OUT && m_nLevel<0) ||
		(m_nMode==DMOD_RUN_TO_CURSOR && m_strPathName.CompareNoCase(szFile)==0 && m_nLine==nLine) )
	{
		if (m_EvaluatingCondition) return;
		DebugBreak(szFile, nLine);
	}
	else if (hasBreakPoint)
	{
		if (m_EvaluatingCondition) return;
		// check that the condition for this breakpoint is met
		CProjectFile::CBreakPoint bp;
		if (theApp.GetMainFrame()->GetProject()->GetBreakPoint(szFile, nLine, bp))
		{
			if (bp.m_Enabled)
			{
				if (bp.m_Condition.IsEmpty())
				{
					DebugBreak(szFile, nLine);
				}
				else
				{
					CString evalCond = Eval(bp.m_Condition);
					if (evalCond != "nil" && evalCond != "false")
					{
						DebugBreak(szFile, nLine);
					}
				}
			}
		}
		else
		{
			DebugBreak(szFile, nLine);
		}
	}
}

void CDebugger::FunctionHook(const char *szFile, int nLine, BOOL bCall)
{
	if ( m_nMode == DMOD_STOP && !m_ExternalDebugging)
		EndThread();

	m_nLevel += (bCall?1:-1);
}

void CDebugger::DebugBreak(const char *szFile, int nLine)
{
	
	m_nMode = DMOD_NONE;

	::SendMessage(m_hWndMainFrame, DMSG_GOTO_FILELINE, (WPARAM)szFile, (LPARAM)nLine);

	m_lua.DrawStackTrace();
	m_lua.DrawGlobalVariables();
	::SendMessage(m_hWndMainFrame, DMSG_GOTO_STACKTRACE_LEVEL, 0, 0);
	::SendMessage(m_hWndMainFrame, DMSG_REDRAW_WATCHES, 0, 0);	

	ResetEvent();
	
	::SendMessage(m_hWndMainFrame, DMSG_DEBUG_BREAK, 0, 0);

	if (m_ExternalDebugging)
	{	
		theApp.GetMainFrame()->ShowWindow(SW_SHOW);
		SetForegroundWindow(theApp.GetMainFrame()->m_hWnd);
		// pump event until main loop returns to "Debug" instead of "Debug break"
		WCHAR tmpTitle[1024];
		GetWindowTextW(theApp.m_EmbeddingAppWnd, tmpTitle, 1024);
		SetWindowText(theApp.m_EmbeddingAppWnd, "Lua breaked");
		while (!m_monoThreadEvent)
		{
			theApp.MainLoop();				
			if (theApp.m_DebuggedAppMainLoop)
			{
				theApp.m_DebuggedAppMainLoop->breakEventLoop();
			}
		}
		SetWindowTextW(theApp.m_EmbeddingAppWnd, tmpTitle);
	}
	else
	{
		CSingleLock lock(&m_event, TRUE);
	}

	if ( m_nMode == DMOD_STOP && !m_ExternalDebugging)
		EndThread();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void CDebugger::Go()
{
	SetEvent();
	::SendMessage(m_hWndMainFrame, DMSG_DEBUG_START, 0, 0);
	if (m_ExternalDebugging)
	{
		// switch to main window
		SetForegroundWindow(theApp.m_EmbeddingAppWnd);
	}
}

void CDebugger::StepInto()
{
	m_nMode = DMOD_STEP_INTO;
	Go();
}

void CDebugger::StepOver()
{
	m_nMode = DMOD_STEP_OVER;
	m_strPathName = m_lua.GetSource();
	m_nLevel = 0;
	Go();
}

void CDebugger::StepOut()
{
	m_nMode = DMOD_STEP_OUT;
	m_strPathName = m_lua.GetSource();
	m_nLevel = 0;
	Go();
}

void CDebugger::RunToCursor()
{
	CLuaView* pView = ((CMainFrame*)AfxGetMainWnd())->GetActiveView();
	CLuaDoc* pDoc = pView->GetDocument();

	if ( !pDoc->IsInProject() )
		return;

	m_nMode = DMOD_RUN_TO_CURSOR;
	
	CProjectFile* pPF = pDoc->GetProjectFile();
	m_strPathName = pPF->GetPathName();

	int nLine = pView->GetEditor()->GetCurrentLine();
	m_nLine = pPF->GetNearestDebugLine(nLine);

	Go();
}

void CDebugger::Stop()
{
	m_nMode = DMOD_STOP;
	Go();

	if (!m_ExternalDebugging)
	{
		MSG msg;
		while ( WaitForSingleObject (m_pThread->m_hThread, 1)==WAIT_TIMEOUT )
			if ( ::PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE) )
				AfxGetThread()->PumpMessage ();

		delete m_pThread;
 		m_pThread=NULL;
	}
}

void CDebugger::Break()
{
	m_nMode = DMOD_BREAK;
}

void CDebugger::ClearStackTrace()
{
	::SendMessage(m_hWndMainFrame, DMSG_CLEAR_STACKTRACE, 0, 0);
}

void CDebugger::AddStackTrace(const char* szDesc, const char* szFile, int nLine)
{
	StackTrace st;
	st.szDesc = szDesc;
	st.szFile = szFile;
	st.nLine = nLine;

	::SendMessage(m_hWndMainFrame, DMSG_ADD_STACKTRACE, (WPARAM)&st, 0);
}

int CDebugger::GetStackTraceLevel()
{
	return ::SendMessage(m_hWndMainFrame, DMSG_GET_STACKTRACE_LEVEL, 0, 0);
}

void CDebugger::StackLevelChanged()
{
	m_lua.DrawLocalVariables();
}

void CDebugger::ClearLocalVariables()
{
	::SendMessage(m_hWndMainFrame, DMSG_CLEAR_LOCALVARIABLES, 0, 0);
}

void CDebugger::AddLocalVariable(const char *name, const char *type, const char *value)
{
	Variable var;
	var.szName = name;
	var.szType = type;
	var.szValue = value;

	::SendMessage(m_hWndMainFrame, DMSG_ADD_LOCALVARIABLE, (WPARAM)&var, 0);
}

void CDebugger::ClearGlobalVariables()
{
	::SendMessage(m_hWndMainFrame, DMSG_CLEAR_GLOBALVARIABLES, 0, 0);
}

void CDebugger::AddGlobalVariable(const char *name, const char *type, const char *value)
{
	Variable var;
	var.szName = name;
	var.szType = type;
	var.szValue = value;

	::SendMessage(m_hWndMainFrame, DMSG_ADD_GLOBALVARIABLE, (WPARAM)&var, 0);
}

BOOL CDebugger::GetCalltip(const char *szWord, char *szCalltip)
{
	return m_lua.GetCalltip(szWord, szCalltip);
}

void CDebugger::EndThread()
{
	if (!m_pDebugger->m_ExternalDebugging)
	{
		FreeConsole();
	}
	::SendMessage(m_pDebugger->m_hWndMainFrame, DMSG_DEBUG_END, 0, 0);
	m_pDebugger->Write("The program has exited...\n");
	m_pDebugger->m_lua.StopDebugger();
	if (!m_pDebugger->m_ExternalDebugging)
	{
		AfxEndThread(0);
	}
}

CString CDebugger::Eval(CString strCode)
{
	m_EvaluatingCondition = true;
	strCode = "return " + strCode;
	CString strRet;
	m_lua.Eval(strCode.GetBuffer(0), strRet.GetBuffer(256));
	strRet.ReleaseBuffer();
	m_EvaluatingCondition = false;
	return strRet;
}

void CDebugger::Execute()
{
	CString strInterpreter = /*"F:\\a ja tak\\lubie cie bardzo\\lua.exe";*/ theApp.GetModuleDir() + "\\" + "lua.exe";
	CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();

	_spawnl( _P_NOWAIT, strInterpreter, strInterpreter, "\"" + pProject->GetDebugPathNameExt() + "\"", NULL );
}
