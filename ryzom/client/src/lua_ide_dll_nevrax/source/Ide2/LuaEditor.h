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

#if !defined(AFX_LUAEDITOR_H__C002AF8D_DFEF_4577_A494_10F2B3DF2458__INCLUDED_)
#define AFX_LUAEDITOR_H__C002AF8D_DFEF_4577_A494_10F2B3DF2458__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LuaEditor.h : header file
//

#include "Platform.h"
#include "scintilla.h"

/////////////////////////////////////////////////////////////////////////////
// CLuaEditor window

class CProjectFile;

class CLuaEditor : public CWnd
{
// Construction
public:
	CLuaEditor();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLuaEditor)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetFocus(BOOL hasFocus);
	void SetLuaLexer();
	BOOL PreparePrint(CDC* pDC, CPrintInfo* pInfo);
	void PrintPage(CDC* pDC, CPrintInfo* pInfo);
	void EndPrint(CDC* pDC, CPrintInfo* pInfo);
	void SetStackTraceLevel(int nLevel);
	void SetCallStackMargins();
	int GetCurrentLine();
	int GetLineCount();
	int GetTextLength();
	int GetCursorPos();
	void GotoLastLine();
	void WordLeft();
	void WordRight();
	void WordLeftExtend();
	void WordRightExtend();
	void WordPartLeft();
	void WordPartRight();
	void WordPartLeftExtend();
	void WordPartRightExtend();
	void FindText(const char *txt, BOOL matchCase, BOOL wholeWord, BOOL regExp, BOOL forward, int startPos, int &first, int &last);
	CString GetLine(int nLine);
	CString GetSubString(int start, int length);
	void ClearAll();
	void AddText(const char* szText);
	void SetReadOnly(BOOL bReadOnly);
	void SetEditorMargins();
	void GrabFocus();
	void Copy();
	void Paste();
	BOOL CanPaste();
	void Clear();
	void Cut();
	BOOL CanCutOrClear();
	void SelectAll();
	void SetSelection(int start, int end);
	void GetSelection(int &start, int &end);
	void Redo();
	BOOL CanRedo();
	void Undo();
	BOOL CanUndo();
	void SetBreakPointsIn(CProjectFile* pPF);
	void GotoLine(int nLine);
	void SetCursorLine(int nLine);
	void SetCursorPos(int pos);
	void SetBreakpoint(int nLine);
	void ClearAllBreakpoints();
	BOOL Save(CFile* pFile);
	BOOL Load(CFile* pFile);
	BOOL ToggleBreakpoint(int nLine);
	int LineFromPoint(CPoint& pt);
	BOOL Create(CWnd* pParentWnd, UINT nCtrlID);
	virtual ~CLuaEditor();

// thread safe functions
	//void tsAddText(const char *szMsg) {	AddText(szMsg); };
	//void tsSetReadOnly(BOOL bReadOnly) { SetReadOnly(bReadOnly); };

	void Print(bool showDialog);
	CharacterRange GetSelection();
protected:
	int (*m_fnScintilla)(void*,int,int,int);
	void *m_ptrScintilla;
	CUIntArray	m_pages;
	BOOL m_bShowCalltips;
	CString m_strCallTip;

public:
	int Sci(int nCmd, int wParam=0, int lParam=0);

	// Generated message map functions
protected:
	//{{AFX_MSG(CLuaEditor)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUAEDITOR_H__C002AF8D_DFEF_4577_A494_10F2B3DF2458__INCLUDED_)
