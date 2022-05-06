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

// MainFrame.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ide2.h"

#include "MainFrame.h"
#include "ScintillaView.h"
#include "LuaDoc.h"
#include "LuaView.h"
#include "BreakPointWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_COMMAND_EX(ID_NEXT_DOC,   OnNextDoc)
	ON_COMMAND_EX(ID_PREV_DOC,   OnPrevDoc)
	ON_COMMAND_EX(ID_CONTROL_DOWN,   OnControlDown)
	ON_COMMAND_EX(ID_VIEW_OUTPUT, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_WORKSPACE, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORKSPACE, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_CALLSTACK, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CALLSTACK, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_LOCALS, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOCALS, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_WATCHES, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WATCHES, OnUpdateControlBarMenu)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_OPENPROJECT, OnFileOpenproject)
	ON_COMMAND(ID_FILE_SAVEPROJECT, OnFileSaveproject)
	ON_COMMAND(ID_FILE_SAVEPROJECTAS, OnFileSaveprojectas)
	ON_COMMAND(ID_PROJECT_ADD_FILES, OnProjectAddFiles)
	ON_COMMAND(ID_PROJECT_PROPERTIES, OnProjectProperties)
	ON_WM_CLOSE()
	ON_COMMAND(ID_BUILD_BUILD, OnBuildBuild)
	ON_UPDATE_COMMAND_UI(ID_BUILD_COMPILE, OnUpdateBuildCompile)
	ON_COMMAND(ID_BUILD_COMPILE, OnBuildCompile)
	ON_UPDATE_COMMAND_UI(ID_BUILD_BUILD, OnUpdateBuildBuild)
	ON_COMMAND(ID_BUILD_REBUILDALL, OnBuildRebuildall)
	ON_COMMAND(ID_BUILD_CLEAN, OnBuildClean)
	ON_COMMAND(ID_BUILD_GO, OnBuildGo)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_GO, OnUpdateDebugBreak)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_BREAK, OnUpdateDebug)
	ON_COMMAND(ID_DEBUG_GO, OnDebugGo)
	ON_COMMAND(ID_DEBUG_STEPINTO, OnDebugStepinto)
	ON_COMMAND(ID_DEBUG_STEPOVER, OnDebugStepover)
	ON_COMMAND(ID_DEBUG_STEPOUT, OnDebugStepout)
	ON_COMMAND(ID_DEBUG_RUNTOCURSOR, OnDebugRuntocursor)
	ON_COMMAND(ID_DEBUG_BREAK, OnDebugBreak)
	ON_COMMAND(ID_DEBUG_STOPDEBUGGING, OnDebugStopdebugging)
	ON_COMMAND(ID_DEBUG_RESTART, OnDebugRestart)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEPROJECT, OnUpdateCmdForProject)
	ON_COMMAND(ID_FILE_NEWPROJECT, OnFileNewproject)
	ON_COMMAND(ID_FILE_CLOSEPROJECT, OnFileCloseproject)
	ON_COMMAND(ID_BUILD_EXECUTE, OnBuildExecute)
	ON_COMMAND(ID_HELP_CONTACTAUTHOR, OnHelpContactauthor)
	ON_COMMAND(ID_HELP_LUAHELPPDF, OnHelpLuahelppdf)
	ON_COMMAND(ID_HELP_VISITHOMEPAGE, OnHelpVisithomepage)
	ON_COMMAND(ID_HELP_LUAHOMEPAGE,      OnHelpLuahomepage)	
	ON_WM_ACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_BUILD_GO, OnUpdateBuildBuild)
	ON_UPDATE_COMMAND_UI(ID_BUILD_REBUILDALL, OnUpdateBuildBuild)
	ON_UPDATE_COMMAND_UI(ID_BUILD_CLEAN, OnUpdateBuildBuild)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_STEPINTO, OnUpdateDebugBreak)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_STEPOVER, OnUpdateDebugBreak)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_STEPOUT, OnUpdateDebugBreak)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_RUNTOCURSOR, OnUpdateDebugBreak)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEPROJECTAS, OnUpdateCmdForProject)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSEPROJECT, OnUpdateCmdForProject)
	ON_UPDATE_COMMAND_UI(ID_BUILD_EXECUTE, OnUpdateBuildBuild)
	ON_COMMAND(ID_BREAKPOINTS, OnBreakPoints)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_hAccelNoProject = ::LoadAccelerators(theApp.m_hInstance, MAKEINTRESOURCE(IDR_ACCEL_NO_PROJECT));
	m_hAccelBuild = ::LoadAccelerators(theApp.m_hInstance, MAKEINTRESOURCE(IDR_ACCEL_BUILD));
	m_hAccelDebug = ::LoadAccelerators(theApp.m_hInstance, MAKEINTRESOURCE(IDR_ACCEL_DEBUG));
	m_hAccelDebugBreak = ::LoadAccelerators(theApp.m_hInstance, MAKEINTRESOURCE(IDR_ACCEL_DEBUG_BREAK));
	m_ExternalDebug = false;
	m_LastCycledDoc = NULL;
}

CMainFrame::~CMainFrame()
{
	::DestroyAcceleratorTable(m_hAccelNoProject);
	::DestroyAcceleratorTable(m_hAccelBuild);
	::DestroyAcceleratorTable(m_hAccelDebug);
	::DestroyAcceleratorTable(m_hAccelDebugBreak);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndMDIClient.SubclassWindow (m_hWndMDIClient)) 
	{
              TRACE ("Failed to subclass MDI client window\n");
              return (-1);
    }            

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	InitDockingWindows();

	LoadBarState(_T("Build"));

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL CMainFrame::InitDockingWindows()
{
	SetInitialSize(125,125,225,225);

	if (!m_wndWorkspace.Create(this, ID_VIEW_WORKSPACE,
		_T("Workspace"), CSize(200,100), CBRS_LEFT))
	{
		TRACE0("Failed to create dialog bar m_wndWorkspace\n");
		return -1;		// fail to create
	}
	
	if (!m_wndOutput.Create(this, ID_VIEW_OUTPUT,
		_T("Output"), CSize(300,100), CBRS_BOTTOM))
	{
		TRACE0("Failed to create dialog bar m_wndOutput\n");
		return -1;		// fail to create
	}

	if (!m_wndCallStack.Create(this, ID_VIEW_CALLSTACK,
		_T("Call Stack"), CSize(300,100), CBRS_RIGHT))
	{
		TRACE0("Failed to create dialog bar m_wndCallStack\n");
		return -1;		// fail to create
	}

	if (!m_wndLocals.Create(this, ID_VIEW_LOCALS,
		_T("Local Variables"), CSize(200,100), CBRS_BOTTOM))
	{
		TRACE0("Failed to create dialog bar m_wndLocals\n");
		return -1;		// fail to create
	}

	if (!m_wndWatches.Create(this, ID_VIEW_WATCHES,
		_T("Local Variables"), CSize(200,100), CBRS_BOTTOM))
	{
		TRACE0("Failed to create dialog bar m_wndWatches\n");
		return -1;		// fail to create
	}

	m_wndWorkspace.EnableDockingOnSizeBar(CBRS_ALIGN_ANY);
	m_wndOutput.EnableDockingOnSizeBar(CBRS_ALIGN_ANY);
	m_wndCallStack.EnableDockingOnSizeBar(CBRS_ALIGN_ANY);
	m_wndLocals.EnableDockingOnSizeBar(CBRS_ALIGN_ANY);
	m_wndWatches.EnableDockingOnSizeBar(CBRS_ALIGN_ANY);

	EnableDockingSizeBar(CBRS_ALIGN_ANY);
	DockSizeBar(&m_wndWorkspace);
	DockSizeBar(&m_wndOutput);
	DockSizeBar(&m_wndCallStack);
	DockSizeBar(&m_wndLocals);
	DockSizeBar(&m_wndWatches);

	return TRUE;
}

void CMainFrame::OnFileOpenproject() 
{
	if ( GetProject()->Load() )
	{
		SetMode(modeBuild);
		m_wndWorkspace.Enable(TRUE);
		OnUpdateFrameTitle(TRUE);
	}
}

void CMainFrame::OnFileSaveproject() 
{
	GetProject()->Save();
}

void CMainFrame::OnFileSaveprojectas() 
{
	GetProject()->SaveAs();
}

void CMainFrame::OnProjectAddFiles() 
{
	GetProject()->AddFiles();
}

void CMainFrame::OnProjectProperties() 
{
	GetProject()->Properties();
}

void CMainFrame::OnClose() 
{
	if ( m_nAppMode==modeDebug || m_nAppMode==modeDebugBreak)
	{
		if ( AfxMessageBox("This command will stop debugger", MB_OKCANCEL)==IDOK )
			OnDebugStopdebugging();
		else
			return;
	}

	SaveBarState(_T("Build"));

	if ( m_nAppMode!=modeNoProject )
		GetProject()->SaveModified();
	
	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnUpdateBuildBuild(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_project.NofFiles());
}

void CMainFrame::OnBuildBuild() 
{
	theApp.SaveModifiedDocuments();

	m_project.Build();
}

void CMainFrame::OnUpdateBuildCompile(CCmdUI* pCmdUI) 
{
	CLuaView *pView = GetActiveView();
	if ( pView )
		pCmdUI->Enable(pView->GetDocument()->IsInProject());
	else
		pCmdUI->Enable(FALSE);
}

void CMainFrame::OnBuildCompile() 
{
	CLuaDoc *pDoc = GetActiveView()->GetDocument();

	if ( pDoc->IsModified() )
		pDoc->DoFileSave();

	m_project.Compile(pDoc->GetProjectFile());
}

CLuaView* CMainFrame::GetActiveView()
{
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	CMDIChildWnd *pChild = (CMDIChildWnd *)pFrame->GetActiveFrame();
	if ( !pChild )
		return NULL;

	CView *pView = pChild->GetActiveView();
	if ( pView && pView->IsKindOf(RUNTIME_CLASS(CLuaView)) )
		return (CLuaView*)pView;

	return NULL;
}

void CMainFrame::OnBuildRebuildall()
{
	OnBuildClean();
	OnBuildBuild();
}

void CMainFrame::OnBuildClean() 
{
	m_project.CleanIntermediateAndOutputFiles();
}

void CMainFrame::OnBuildGo() 
{
	// TODO: Add your command handler code here
	theApp.SaveModifiedDocuments();

	if ( !m_project.Build() )
		return;

	m_debug.Prepare(NULL, NULL);

	m_debug.Start();
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if ( message >= _DMSG_FIRST_MSG && message <= _DMSG_LAST_MSG )
		return DebugMessage(message, wParam, lParam);
	
	return CMDIFrameWnd::WindowProc(message, wParam, lParam);
}

LRESULT CMainFrame::DebugMessage(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch( nMsg )
	{
	case DMSG_WRITE_DEBUG:
		GetOutputWnd()->GetOutput(COutputWnd::outputDebug)->Write((const char*)wParam);
		break;
	case DMSG_HAS_BREAKPOINT:
		return GetProject()->HasBreakPoint((const char*)wParam, (int)lParam);
	case DMSG_GOTO_FILELINE:
		GotoFileLine((const char*)wParam, (int)lParam);
		break;
	case DMSG_DEBUG_START:
		SetMode(modeDebug);
		break;
	case DMSG_DEBUG_BREAK:
		SetMode(modeDebugBreak);
		break;
	case DMSG_DEBUG_END:
		SetMode(modeBuild);
		break;
	case DMSG_CLEAR_STACKTRACE:
		m_wndCallStack.Clear();
		break;
	case DMSG_ADD_STACKTRACE:
		m_wndCallStack.Add(((StackTrace*)wParam)->szDesc, 
			((StackTrace*)wParam)->szFile, 
			((StackTrace*)wParam)->nLine);
		break;
	case DMSG_GOTO_STACKTRACE_LEVEL:
		m_wndCallStack.GotoStackTraceLevel(wParam);
		break;
	case DMSG_GET_STACKTRACE_LEVEL:
		return m_wndCallStack.GetLevel();
	case DMSG_CLEAR_LOCALVARIABLES:
		m_wndLocals.RemoveAll();
		break;
	case DMSG_ADD_LOCALVARIABLE:
		m_wndLocals.AddVariable(((Variable*)wParam)->szName, 
			((Variable*)wParam)->szType, 
			((Variable*)wParam)->szValue);
		break;
	case DMSG_REDRAW_WATCHES:
		m_wndWatches.Redraw();
	}

	return 0;
}

void CMainFrame::GotoFileLine(const char *szFile, int nLine)
{
	CProjectFile* pPF = GetProject()->GetProjectFile(szFile);

	if ( pPF )
		theApp.OpenProjectFilesView(pPF, nLine);
}

void CMainFrame::SetMode(int nMode)
{
	m_nAppMode = nMode;
	switch ( nMode )
	{
	case modeNoProject:
		m_hAccelTable = m_hAccelNoProject;
		m_wndCallStack.Clear();
		break;
	case modeBuild:
		m_hAccelTable = m_hAccelBuild;
		m_wndCallStack.Clear();
		break;
	case modeDebug:
		m_hAccelTable = m_hAccelDebug;
		m_wndCallStack.Clear();
		break;
	case modeDebugBreak:
		m_hAccelTable = m_hAccelDebugBreak;
		break;
	}

	m_wndMDIClient.ResetMenu();
	DrawMenuBar();

	OnUpdateFrameTitle(TRUE);
}

void CMainFrame::OnDebugGo() 
{
	m_debug.Go();
}

void CMainFrame::OnUpdateDebugBreak(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetMode()==modeDebugBreak);	
}

void CMainFrame::OnUpdateDebug(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetMode()==modeDebug);	
}

void CMainFrame::OnDebugStepinto() 
{
	m_debug.StepInto();
}

void CMainFrame::OnDebugStepover() 
{
	m_debug.StepOver();
}

void CMainFrame::OnDebugStepout() 
{
	m_debug.StepOut();
}

void CMainFrame::OnDebugRuntocursor() 
{
	m_debug.RunToCursor();
}

void CMainFrame::OnDebugBreak() 
{
	m_debug.Break();
}

void CMainFrame::OnDebugStopdebugging() 
{
	m_debug.Stop();
}

void CMainFrame::OnDebugRestart() 
{
	m_debug.Stop();

	OnBuildGo();
	
}


BOOL CMainFrame::GetCalltip(const char *szWord, CString &strCalltip)
{
	BOOL bFound = m_debug.GetCalltip(szWord, strCalltip.GetBuffer(1000));
	strCalltip.ReleaseBuffer();

	return bFound;
}

void CMainFrame::OnUpdateCmdForProject(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nAppMode != modeNoProject);
}


void CMainFrame::OnFileNewproject() 
{
	if ( GetProject()->New() )
	{
		SetMode(modeBuild);
		m_wndWorkspace.Enable(TRUE);
		OnUpdateFrameTitle(TRUE);
	}
}

void CMainFrame::InitExternalDebug(const char *tmpProjectFile, lua_realloc_t reallocfunc, lua_free_t freefunc)
{
	m_ExternalDebug = true;
	GetProject()->InitForExternalDebug(tmpProjectFile);
	m_debug.SetExternalDebugging();
	m_debug.Prepare(reallocfunc, freefunc);
	m_debug.Start();
	{
		SetMode(modeDebug);
		m_wndWorkspace.Enable(TRUE);
		OnUpdateFrameTitle(TRUE);
	}
}


void CMainFrame::OnFileCloseproject() 
{
	if ( GetProject()->Close() )
	{
		SetMode(modeNoProject);
		m_wndWorkspace.Enable(FALSE);
		OnUpdateFrameTitle(TRUE);
	}
}

void CMainFrame::OnBuildExecute() 
{
	theApp.SaveModifiedDocuments();

	if ( !m_project.Build() )
		return;

	m_debug.Execute();
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	if ((GetStyle() & FWS_ADDTOTITLE) == 0)
		return;     // leave it alone!

	CLuaView* pView = GetActiveView();
	if ( pView && bAddToTitle )
		UpdateFrameTitleForDocument(pView->GetDocument()->GetTitle());
	else
		UpdateFrameTitleForDocument(NULL);
}

void CMainFrame::UpdateFrameTitleForDocument(LPCTSTR lpszDocName)
{
	// copy first part of title loaded at time of frame creation
	TCHAR szText[256+_MAX_PATH];

	if (GetStyle() & FWS_PREFIXTITLE)
	{
		szText[0] = '\0';   // start with nothing

		// get name of currently active view
		if (lpszDocName != NULL)
		{
			lstrcpy(szText, lpszDocName);
			// add current window # if needed
			if (m_nWindow > 0)
				wsprintf(szText + lstrlen(szText), _T(":%d"), m_nWindow);
			lstrcat(szText, _T(" - "));
		}
		lstrcat(szText, m_strTitle);
	}
	else
	{
		// get name of currently active view
		if ( m_nAppMode!=modeNoProject )
		{
			lstrcpy(szText, m_project.GetName());
			lstrcat(szText, _T(" - "));
			lstrcat(szText, m_strTitle);
		}
		else
			lstrcpy(szText, m_strTitle);

		if ( m_nAppMode==modeDebug )
			lstrcat(szText, _T(" [run] "));
		else if ( m_nAppMode==modeDebugBreak )
			lstrcat(szText, _T(" [break] "));

		if (lpszDocName != NULL)
		{
			BOOL maximized;
			CMDIChildWnd* child = theApp.GetMainFrame()->MDIGetActive(&maximized);
			if (!maximized)
			{				

				lstrcat(szText,      _T(" - ["));
				lstrcat(szText,      lpszDocName);
				// add current window # if needed
				if (m_nWindow > 0)
					wsprintf(szText + lstrlen(szText),      _T(":%d"),      m_nWindow);
				lstrcat(szText,      _T("]"));
			}					
		}		
	}	

	// set title if changed, but don't remove completely
	// Note: will be excessive for MDI Frame with maximized child
	AfxSetWindowText(m_hWnd, szText);
}


void CMainFrame::OnHelpContactauthor() 
{
	ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), "open", "mailto:rybak@gorlice.net.pl", NULL, NULL, SW_SHOWNA);	
}

void CMainFrame::OnHelpLuahelppdf() 
{
	ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), "open", theApp.GetModuleDir() + "\\docs\\refman-5.0.pdf", NULL, NULL, SW_SHOWNA);	
}

void CMainFrame::OnHelpVisithomepage() 
{
	ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), "open", "http://www.gorlice.net.pl/~rybak/luaide", NULL, NULL, SW_SHOWNA);	
}

void CMainFrame::OnHelpLuahomepage() 
{
	ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), "open", "http://www.lua.org", NULL, NULL, SW_SHOWNA);	
}

void CMainFrame::OnActivate(UINT nState,      CWnd* pWndOther,      BOOL bMinimized)
{
	if (!bMinimized)
	{
		theApp.CheckExternallyModifiedFiles();		
		CDocument *currDoc = theApp.GetActiveDoc();
		if (currDoc)
		{
			((CLuaDoc *) currDoc)->GetView()->SetFocus();
		}
	}
}


void CMainFrame::OnNextDoc()
{
	BOOL maximized;
	CMDIChildWnd* activeWnd = theApp.GetMainFrame()->MDIGetActive(&maximized);	
	if (!activeWnd) return;
	// start from active document and get next one
	POSITION pos = theApp.m_pLuaTemplate->GetFirstDocPosition();
	while (pos != NULL)
	{		
		CLuaDoc* pDoc = (CLuaDoc*)theApp.m_pLuaTemplate->GetNextDoc(pos);
		if (pDoc->GetView()->GetParentFrame() == activeWnd)
		{
				if (m_LastCycledDoc == NULL) m_LastCycledDoc = pDoc;
				if (pos == NULL)
				{
					pos = theApp.m_pLuaTemplate->GetFirstDocPosition();					
				}
				pDoc = (CLuaDoc*)theApp.m_pLuaTemplate->GetNextDoc(pos);				
				MDIActivate(pDoc->GetView()->GetParentFrame());				
				return;
		}		
	}	
}

void CMainFrame::OnPrevDoc()
{
	BOOL maximized;
	CMDIChildWnd* activeWnd = theApp.GetMainFrame()->MDIGetActive(&maximized);	
	if (!activeWnd) return;
	// start from active document and get next one
	POSITION pos = theApp.m_pLuaTemplate->GetFirstDocPosition();
	while (pos != NULL)
	{		
		CLuaDoc* pDoc = (CLuaDoc*)theApp.m_pLuaTemplate->GetNextDoc(pos);
		if (pDoc->GetView()->GetParentFrame() == activeWnd)
		{
				if (m_LastCycledDoc == NULL) m_LastCycledDoc = pDoc;
				CDocument *pPrevDoc;
				CDocument *pSearchedDoc = pDoc;
				for (;;)
				{					
					if (pos == NULL)
					{
						pos = theApp.m_pLuaTemplate->GetFirstDocPosition();					
					}
					pPrevDoc = pDoc;
					pDoc = (CLuaDoc*)theApp.m_pLuaTemplate->GetNextDoc(pos);
					if (pDoc == pSearchedDoc) break;
				}				
				((CLuaDoc*) pPrevDoc)->GetView()->SetFocus();
				return;
		}		
	}	
}

void CMainFrame::OnControlDown()
{
	if (!m_LastCycledDoc) return;
	CDocument *currDoc = theApp.GetActiveDoc();	
	theApp.m_pLuaTemplate->MoveDocAfter(m_LastCycledDoc, currDoc);
	m_LastCycledDoc = NULL;
}

void CMainFrame::OnBreakPoints()
{
	CBreakPointWnd bpw;
	// fill list af current breakpoints
	bpw.m_BP.clear();
	for(int k = 0; k < m_project.NofFiles(); ++k)
	{
		std::vector<CProjectFile::CBreakPoint> BPs;
		m_project.GetProjectFile(k)->GetBreakPoints(BPs);
		for(UINT l = 0; l < BPs.size(); ++l)
		{
			CBreakPointWnd::CFileBreakPoint fbp;
			fbp.m_BP = BPs[l];
			fbp.m_File = m_project.GetProjectFile(k);
			bpw.m_BP.push_back(fbp);
		}		
	}
	if (bpw.DoModal() == IDOK)
	{
		UINT k;
		// erase and reset all breakpoints
		for(k = 0; k < (UINT) m_project.NofFiles(); ++k)
		{
			m_project.GetProjectFile(k)->RemoveAllBreakPoints();
		}
		for(UINT l = 0; l < bpw.m_BP.size(); ++l)
		{
			bpw.m_BP[l].m_File->AddBreakPoint(bpw.m_BP[l].m_BP);
		}
		// update breakpoints display in all views
		for(k = 0; k < (UINT) m_project.NofFiles(); ++k)
		{
			CLuaView* view = theApp.FindProjectFilesView(m_project.GetProjectFile(k));
			if (view)
			{
				m_project.GetProjectFile(k)->SetBreakPointsIn(view->GetEditor());
			}
		}
	}
}
