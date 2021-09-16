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

#if !defined(AFX_LEFTVIEW_H__5D7BB0E1_ADB3_42FF_AD3D_4ACDF282BA21__INCLUDED_)
#define AFX_LEFTVIEW_H__5D7BB0E1_ADB3_42FF_AD3D_4ACDF282BA21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGeorgesEditDoc;
class CGeorgesEditView;
class CGeorgesEditDocSub;

class CLeftView : public CView
{
protected: // create from serialization only
	CLeftView ();
	DECLARE_DYNCREATE(CLeftView)

// Attributes
public:
	CGeorgesEditDoc* GetDocument();

	// Update the tree structure
	void	getFromDocument ();

	// Get the current selection Id. If no selection, return 0xffffffff.
	uint	getCurrentSelectionId (uint *count=NULL, HTREEITEM item = NULL);

	// Set the current selection Id. Return true if the node has been found.
	bool	setCurrentSelectionId (uint	id, HTREEITEM item = NULL);

	// Return the seledcted sub object. Called by CGeorgesEditDoc::getSelectedObject ().
	CGeorgesEditDocSub *getSelectedObject ();

	// Change the sub seletion. Called by CGeorgesEditDoc::changeSubSelection ().
	bool	changeSubSelection (CGeorgesEditDocSub *sub, HTREEITEM item = NULL);

	// Change the sub seletion. Called by CGeorgesEditDoc::changeSubSelection ().
	bool	changeSubSelection (uint &sub, HTREEITEM item = NULL);

	// Expand // Collapse
	void	expand ();
	void	expand (HTREEITEM item);
	void	collapse (HTREEITEM item);

private:

	// Recurse sub object tree called by getFromDocument
	void	getSubObject (CGeorgesEditDocSub *subObject, HTREEITEM parent, HTREEITEM item);

	CTreeCtrl	TreeCtrl;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLeftView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CLeftView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditCut();
	afx_msg void OnInsert();
	afx_msg void OnDelete();
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRename();
	afx_msg void OnEditFetch1();
	afx_msg void OnEditFetch2();
	afx_msg void OnEditFetch3();
	afx_msg void OnEditFetch4();
	afx_msg void OnEditHold1();
	afx_msg void OnEditHold2();
	afx_msg void OnEditHold3();
	afx_msg void OnEditHold4();
	afx_msg void OnEditCollapseall();
	afx_msg void OnEditExpandall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in LeftView.cpp
inline CGeorgesEditDoc* CLeftView::GetDocument()
   { return (CGeorgesEditDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEFTVIEW_H__5D7BB0E1_ADB3_42FF_AD3D_4ACDF282BA21__INCLUDED_)
