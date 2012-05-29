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

// LuaView.h : interface of the CLuaView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_LUAVIEW_H__F7E3BD0D_1558_484D_9634_B138CC9D9B31__INCLUDED_)
#define AFX_LUAVIEW_H__F7E3BD0D_1558_484D_9634_B138CC9D9B31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LuaEditor.h"

class CLuaView : public CView
{
protected: // create from serialization only
	CLuaView();
	DECLARE_DYNCREATE(CLuaView)

// Attributes
public:
	CLuaDoc* GetDocument();
	CLuaEditor* GetEditor() { return &m_editor; };

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLuaView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	void ToggleBreakPoint(int nLine);
	void CloseFrame();
	void Activate();
	virtual ~CLuaView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CLuaEditor m_editor;
	int OnSci(SCNotification* pNotify);

// Generated message map functions
protected:
	//{{AFX_MSG(CLuaView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditClear();
	afx_msg void OnUpdateEditClear(CCmdUI* pCmdUI);
	afx_msg void OnToggleBreakpoint();
	afx_msg void OnUpdateToggleBreakpoint(CCmdUI* pCmdUI);
	afx_msg void OnGotoLine();
	afx_msg void OnFindText();
	afx_msg void OnFindNext();
	afx_msg void OnFindPrevious();
	afx_msg void OnSelectFindNext();	
	afx_msg void OnSelectFindPrevious();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnUpdateCheckSyntax(CCmdUI* pCmdUI);
	afx_msg void OnCheckSyntax();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void find(bool forward, bool next);
};

#ifndef _DEBUG  // debug version in LuaView.cpp
inline CLuaDoc* CLuaView::GetDocument()
   { return (CLuaDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUAVIEW_H__F7E3BD0D_1558_484D_9634_B138CC9D9B31__INCLUDED_)
