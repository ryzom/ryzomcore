#if !defined(AFX_LOGICTREEVIEW_H__EB843012_833A_4979_AECA_63302172A812__INCLUDED_)
#define AFX_LOGICTREEVIEW_H__EB843012_833A_4979_AECA_63302172A812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogicTreeView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogicTreeView view

class CLogicTreeView : public CView
{
protected:
	CLogicTreeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLogicTreeView)

// Attributes
protected:
public:
	/// the splitter window in the view
	CSplitterWnd m_wndSplitter;


// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogicTreeView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CLogicTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CLogicTreeView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGICTREEVIEW_H__EB843012_833A_4979_AECA_63302172A812__INCLUDED_)


	
