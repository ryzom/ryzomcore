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

// MainFrame.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRAME_H__CCF97055_F1B3_48F3_B536_F23CF34360E9__INCLUDED_)
#define AFX_MAINFRAME_H__CCF97055_F1B3_48F3_B536_F23CF34360E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WorkspaceWnd.h"
#include "OutputWnd.h"
#include "CallStack.h"
#include "Project.h"
#include "MDIClientWnd.h"
#include "Debugger.h"
#include "VariablesBar.h"
#include "WatchBar.h"
#include "FindText.h"

class CMainFrame : public CCJMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
	enum {
		modeNoProject,
		modeBuild,
		modeDebug,
		modeDebugBreak
	} appMode;

protected:
	CWorkspaceWnd	m_wndWorkspace;
	COutputWnd		m_wndOutput;
	CCallStack		m_wndCallStack;
	CProject m_project;
	CDebugger m_debug;
	CMDIClientWnd	m_wndMDIClient;
	CVariablesBar	m_wndLocals;
	CWatchBar		m_wndWatches;
	HACCEL m_hAccelBuild, m_hAccelDebug, m_hAccelDebugBreak, m_hAccelNoProject;
	int m_nAppMode;
public:
	CFindText m_FindText;

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	//}}AFX_VIRTUAL

// Implementation
public:
	void InitExternalDebug(const char *tmpProjectFile, lua_realloc_t reallocfunc, lua_free_t freefunc);
	bool IsExternalDebug() const { return m_ExternalDebug; }
	BOOL GetCalltip(const char* szWord, CString& strCalltip);
	void SetMode(int nMode);
	void GotoFileLine(const char* szFile, int nLine);
	LRESULT DebugMessage(UINT nMsg, WPARAM wParam, LPARAM lParam);
	CLuaView* GetActiveView();
	int GetMode() { return m_nAppMode; };
	COutputWnd* GetOutputWnd() { return &m_wndOutput; };
	CProject* GetProject() { return &m_project; };
	CWorkspaceWnd* GetWorkspaceWnd() { return &m_wndWorkspace; };
	CCallStack* GetCallStack() { return &m_wndCallStack; };
	CDebugger* GetDebugger() { return & m_debug; };
	BOOL InitDockingWindows();
	void UpdateFrameTitleForDocument(LPCTSTR lpszDocName);
	virtual ~CMainFrame();
	void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	bool		m_ExternalDebug;
// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileOpenproject();
	afx_msg void OnFileSaveproject();
	afx_msg void OnFileSaveprojectas();
	afx_msg void OnProjectAddFiles();
	afx_msg void OnProjectProperties();
	afx_msg void OnClose();
	afx_msg void OnBuildBuild();
	afx_msg void OnUpdateBuildCompile(CCmdUI* pCmdUI);
	afx_msg void OnBuildCompile();
	afx_msg void OnUpdateBuildBuild(CCmdUI* pCmdUI);
	afx_msg void OnBuildRebuildall();
	afx_msg void OnBuildClean();
	afx_msg void OnBuildGo();
	afx_msg void OnUpdateDebugBreak(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDebug(CCmdUI* pCmdUI);
	afx_msg void OnDebugGo();
	afx_msg void OnDebugStepinto();
	afx_msg void OnDebugStepover();
	afx_msg void OnDebugStepout();
	afx_msg void OnDebugRuntocursor();
	afx_msg void OnDebugBreak();
	afx_msg void OnDebugStopdebugging();
	afx_msg void OnDebugRestart();
	afx_msg void OnUpdateCmdForProject(CCmdUI* pCmdUI);
	afx_msg void OnFileNewproject();
	afx_msg void OnFileCloseproject();
	afx_msg void OnBuildExecute();
	afx_msg void OnHelpContactauthor();
	afx_msg void OnHelpLuahelppdf();
	afx_msg void OnHelpVisithomepage();
	afx_msg void OnHelpLuahomepage();
	afx_msg void OnNextDoc();
	afx_msg void OnPrevDoc();
	afx_msg void OnControlDown();
	afx_msg void OnBreakPoints();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CDocument *m_LastCycledDoc;	
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRAME_H__CCF97055_F1B3_48F3_B536_F23CF34360E9__INCLUDED_)
