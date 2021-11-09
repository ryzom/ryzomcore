#if !defined(AFX_STATESVIEW_H__9EDCC26C_73CE_4523_AAD5_91F07F9110E7__INCLUDED_)
#define AFX_STATESVIEW_H__9EDCC26C_73CE_4523_AAD5_91F07F9110E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StatesView.h : header file
//

#include <afxcview.h> // CTreeView base class
#include "Logic_editorDoc.h"


typedef CArray<HTREEITEM,HTREEITEM&> HTreeItemArray;
typedef CMap<HTREEITEM,HTREEITEM&,CEvent *, CEvent *&> CMapHTreeItemToEvent;





/////////////////////////////////////////////////////////////////////////////
// CStatesView view

class CStatesView : public CTreeView
{
protected:
	CStatesView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CStatesView)

// Attributes
protected:
	// map HTREEITEM to the adress of the associated CEvent object
	CMapHTreeItemToEvent	m_mapItemToEvent;
	CEvent	*				m_pSelectedEvent;
	CState	*				m_pSelectedState;


// Operations
public:

	// expand/collapse the tree ctrl
	void expand(UINT = TVE_EXPAND, HTREEITEM = NULL);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStatesView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CStatesView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CStatesView)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATESVIEW_H__9EDCC26C_73CE_4523_AAD5_91F07F9110E7__INCLUDED_)
