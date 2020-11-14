// logic_editorView.h : interface of the CLogic_editorView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGIC_EDITORVIEW_H__2EA2F60A_E538_4CCD_8AD1_9F42BC3611F1__INCLUDED_)
#define AFX_LOGIC_EDITORVIEW_H__2EA2F60A_E538_4CCD_8AD1_9F42BC3611F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CLogic_editorView : public CView
{
protected: // create from serialization only
	CLogic_editorView();
	DECLARE_DYNCREATE(CLogic_editorView)

// Attributes
public:
	CLogic_editorDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogic_editorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLogic_editorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CLogic_editorView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in logic_editorView.cpp
inline CLogic_editorDoc* CLogic_editorView::GetDocument()
   { return (CLogic_editorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGIC_EDITORVIEW_H__2EA2F60A_E538_4CCD_8AD1_9F42BC3611F1__INCLUDED_)
