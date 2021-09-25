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

#if !defined(AFX_GEORGES_EDITVIEW_H__BA694E6B_7DC1_40A4_BD65_2A4272862E8A__INCLUDED_)
#define AFX_GEORGES_EDITVIEW_H__BA694E6B_7DC1_40A4_BD65_2A4272862E8A__INCLUDED_

#include "type_dialog.h"
#include "header_dialog.h"
#include "dfn_dialog.h"
#include "form_dialog.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGeorgesEditDoc;

#define WM_UPDATE_ALL_VIEWS (WM_APP+0x20)

class CGeorgesEditView : public CScrollView
{
protected: // create from serialization only
	CGeorgesEditView();
	DECLARE_DYNCREATE(CGeorgesEditView)

	enum
	{
		WidgetLeftMargin = 20,
		WidgetTopMargin = 40,
		WidgetRightMargin = 20,
		WidgetBottomMargin = 20,
	};

// Attributes
public:
	CGeorgesEditDoc* GetDocument();

	// The top tab control
	CTabCtrl		TabCtrl;

	// The header dialog
	CHeaderDialog	HeaderDialog;

	// The type dialog
	CTypeDialog		TypeDialog;

	// The dfn dialog
	CDfnDialog		DfnDialog;

	// The form dialog
	CFormDialog		FormDialog;

	// Set the virtual view size (call by rightview dialog is resizeWidget callback
	void			setViewSize (uint width, uint height);

	void			updateTab ();
	void			updateDocument ();
	bool			isFocusable () const;
	void			setFocusLeftView ();
	void			setFocusLastWidget ();

	uint			VirtualWidth, VirtualHeight;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesEditView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGeorgesEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesEditView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnOpenSelected();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditFetch1();
	afx_msg void OnEditFetch2();
	afx_msg void OnEditFetch3();
	afx_msg void OnEditFetch4();
	afx_msg void OnEditHold1();
	afx_msg void OnEditHold2();
	afx_msg void OnEditHold3();
	afx_msg void OnEditHold4();
	//}}AFX_MSG
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in georges_editView.cpp
inline CGeorgesEditDoc* CGeorgesEditView::GetDocument()
   { return (CGeorgesEditDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEORGES_EDITVIEW_H__BA694E6B_7DC1_40A4_BD65_2A4272862E8A__INCLUDED_)
