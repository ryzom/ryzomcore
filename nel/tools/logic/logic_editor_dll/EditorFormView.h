#if !defined(AFX_EDITORFORMVIEW_H__E6809F0B_872A_499C_8A94_A24C38BC0756__INCLUDED_)
#define AFX_EDITORFORMVIEW_H__E6809F0B_872A_499C_8A94_A24C38BC0756__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorFormView.h : header file
//

#include "EditorPropertySheet.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorFormView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CEditorFormView : public CFormView
{
protected:
	CEditorFormView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditorFormView)

// Form Data
public:
	//{{AFX_DATA(CEditorFormView)
	enum { IDD = IDD_FORM_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:
	CEditorPropertySheet *	m_pPropertySheet;
	BOOL					m_bInitDone;
protected:	
	CRect					m_rectInitialSheet;

// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorFormView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEditorFormView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CEditorFormView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORFORMVIEW_H__E6809F0B_872A_499C_8A94_A24C38BC0756__INCLUDED_)
