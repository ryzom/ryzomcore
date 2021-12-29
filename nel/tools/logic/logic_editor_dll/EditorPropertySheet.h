#if !defined(AFX_EDITORPROPERTYSHEET_H__8A0DFA3B_C96E_4D64_A63F_2EAC39871677__INCLUDED_)
#define AFX_EDITORPROPERTYSHEET_H__8A0DFA3B_C96E_4D64_A63F_2EAC39871677__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorPropertySheet.h : header file
//

#include "ResizableSheet.h"

#include "CounterPage.h"
#include "ConditionPage.h"
#include "VariablePage.h"
#include "StatePage.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorPropertySheet

class CEditorPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CEditorPropertySheet)

// Construction
public:
	CEditorPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CEditorPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Operations
public:
	
// Attributes
public:

//protected:
	CCounterPage	m_counterPage;
	CConditionPage	m_conditionPage;
	CVariablePage	m_variablePage;
	CStatePage		m_statePage;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorPropertySheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditorPropertySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorPropertySheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORPROPERTYSHEET_H__8A0DFA3B_C96E_4D64_A63F_2EAC39871677__INCLUDED_)
