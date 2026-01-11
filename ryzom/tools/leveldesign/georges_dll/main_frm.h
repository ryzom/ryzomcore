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

#if !defined(AFX_MAINFRM_H__EF1B6999_B132_462C_A105_98AFE714DB42__INCLUDED_)
#define AFX_MAINFRM_H__EF1B6999_B132_462C_A105_98AFE714DB42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "cool_dialog_bar.h"
#include "file_browser_dialog.h"
#include "output_console_dlg.h"
#include "displayer.h"

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:
	bool	dataToClipboard (UINT format, void *data, uint len);
	bool	dataFromClipboard (UINT format, void *data);
	bool	clipboardSize (UINT format, uint &size);

	sint	createX, createY, createCX, createCY;

	void	outputConsoleString (const char *message);

	void	showOutputConsole (bool show = true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  
	// control bar embedded members
	CStatusBar			m_wndStatusBar;
	CToolBar			m_wndToolBar;
	CReBar				m_wndReBar;
	CDialogBar			m_wndDlgBar;
	CCoolDialogBar		FileBrowser;
	CFileBrowserDialog	FileBrowserDlg;
	CCoolDialogBar		OutputConsole;
	COutputConsoleDlg	OutputConsoleDlg;
	CGeorgesDisplayer	Displayer;
public:
	bool		m_bDontClose;
	bool		BrowserSortedByType;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSettings();
	afx_msg void OnClose();
	afx_msg void OnViewDockingdialogbar();
	afx_msg void OnUpdateViewDockingdialogbar(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewOutputConsole();
	afx_msg void OnUpdateViewOutputConsole(CCmdUI* pCmdUI);
	afx_msg void OnViewGotoFileBrowser();
	afx_msg void OnViewGotoOutputConsole();
	afx_msg void OnModules0();
	afx_msg void OnUpdateModules0(CCmdUI* pCmdUI);
	afx_msg void OnModules1();
	afx_msg void OnUpdateModules1(CCmdUI* pCmdUI);
	afx_msg void OnModules2();
	afx_msg void OnUpdateModules2(CCmdUI* pCmdUI);
	afx_msg void OnModules3();
	afx_msg void OnUpdateModules3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSettings(CCmdUI* pCmdUI);
	afx_msg void OnBrowserSortedByType();
	afx_msg void OnUpdateBrowserSortedByType(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__EF1B6999_B132_462C_A105_98AFE714DB42__INCLUDED_)
