#if !defined(AFX_VARIABLEPAGE_H__DC1D7AB2_6CC1_492C_AC43_3D39F53180F4__INCLUDED_)
#define AFX_VARIABLEPAGE_H__DC1D7AB2_6CC1_492C_AC43_3D39F53180F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VariablePage.h : header file
//

#include "ResizablePage.h"

class CLogic_editorDoc;


/////////////////////////////////////////////////////////////////////////////
// CVariablePage dialog

class CVariablePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CVariablePage)

// Construction
public:
	CVariablePage();
	~CVariablePage();

	void addVariable( CLogic_editorDoc *pDoc, CString varName );

// Dialog Data
	//{{AFX_DATA(CVariablePage)
	enum { IDD = IDD_PAGE_VARIABLES };
	CListBox	m_listVariables;
	CString	m_sVarName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CVariablePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CVariablePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAdd();
	afx_msg void OnSelchangeListVariables();
	afx_msg void OnButtonVarDelete();
	afx_msg void OnButtonVarApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VARIABLEPAGE_H__DC1D7AB2_6CC1_492C_AC43_3D39F53180F4__INCLUDED_)
