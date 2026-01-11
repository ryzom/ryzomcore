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

#if !defined(AFX_SCINTILLAVIEW_H__FE8FF881_86FA_4CCA_A16D_0E586E353DB4__INCLUDED_)
#define AFX_SCINTILLAVIEW_H__FE8FF881_86FA_4CCA_A16D_0E586E353DB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScintillaView.h : header file
//

#include "LuaEditor.h"

/////////////////////////////////////////////////////////////////////////////
// CScintillaView view

class CScintillaView : public CView
{
protected:
	CScintillaView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CScintillaView)

// Attributes
public:

protected:
	CLuaEditor m_view;

// Operations
public:
	void Clear();
	void Write(CString strMsg);

	CLuaEditor* GetEditor() { return &m_view; };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScintillaView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CScintillaView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CScintillaView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCINTILLAVIEW_H__FE8FF881_86FA_4CCA_A16D_0E586E353DB4__INCLUDED_)
