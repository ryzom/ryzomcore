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

// ide2.h : main header file for the IDE2 application
//

#if !defined(AFX_IDE2_H__A3FA6447_9B87_4B84_A3F1_D73F185A1AAC__INCLUDED_)
#define AFX_IDE2_H__A3FA6447_9B87_4B84_A3F1_D73F185A1AAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CIdeApp:
// See ide2.cpp for the implementation of this class
//

class CLuaView;
class CLuaDoc;
class CProjectFile;

class CMainFrame;
struct IDebuggedAppMainLoop;

class CMultiDocTemplateEx : public CMultiDocTemplate
{
public:
	CMultiDocTemplateEx(UINT nIDResource, CRuntimeClass* pDocClass,
					  CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
	// helper for cycling bewteen documents
	void MoveDocAfter(CDocument *doc, CDocument *target);
};

class CIdeApp : public CWinApp
{
public:
	void MainLoop();
	CMainFrame *GetMainFrame();
	BOOL FirstFileIsNewer(CString strPathName1, CString strPathName2);
	void DeleteAllFilesInCurrentDir();
	BOOL SaveModifiedDocuments();
	CLuaView* OpenProjectFilesView(CProjectFile* pPF, int nLine=-1);
	CLuaView* LoadProjectFilesView(CProjectFile* pPF);
	CLuaView* FindProjectFilesView(CProjectFile* pPF);
	void CheckExternallyModifiedFiles();
	CString GetModuleDir();
	void FormatMessage(char* pszAPI);
	CIdeApp();

	CDocument *GetActiveDoc();
	CMultiDocTemplateEx* m_pLuaTemplate;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIdeApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CIdeApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:		
	HWND				  m_EmbeddingAppWnd;
	IDebuggedAppMainLoop *m_DebuggedAppMainLoop;

protected:
	HMODULE m_hScintilla;
};




extern CIdeApp theApp;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IDE2_H__A3FA6447_9B87_4B84_A3F1_D73F185A1AAC__INCLUDED_)
