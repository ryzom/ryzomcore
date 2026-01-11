#if !defined(AFX_CONDITIONPAGE_H__34B1DBCA_B747_4A2C_80F0_5DDC65B71D39__INCLUDED_)
#define AFX_CONDITIONPAGE_H__34B1DBCA_B747_4A2C_80F0_5DDC65B71D39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConditionPage.h : header file
//

//#include "ResizablePage.h"

#include "NumEdit.h"
#include "Condition.h"



class CLogic_editorDoc;


/////////////////////////////////////////////////////////////////////////////
// CConditionPage dialog

class CConditionPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CConditionPage)

// Construction
public:
	CConditionPage();
	~CConditionPage();

	void Update();

	BOOL checkNodeValidity();

	void addCondition( CLogic_editorDoc *pDoc, CCondition * condition );

// Dialog Data
	//{{AFX_DATA(CConditionPage)
	enum { IDD = IDD_PAGE_CONDITIONS };
	CNumEdit	m_ctrlComparand;
	CString	m_sType;
	CString	m_sOperator;
	CString	m_sVarName;
	CString	m_sSubCondName;
	CString	m_sConditionName;
	double	m_dComparand;
	//}}AFX_DATA

public:
	CCondition *		m_pSelectedCondition;
	CConditionNode *	m_pSelectedConditionNode;


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CConditionPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CConditionPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboNodeType();
	afx_msg void OnButtonAddCondition();
	afx_msg void OnButtonAddNode();
	afx_msg void OnButtonCondApply();
	afx_msg void OnButtonDeleteCondition();
	afx_msg void OnButtonNodeApply();
	afx_msg void OnButtonDeleteNode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONDITIONPAGE_H__34B1DBCA_B747_4A2C_80F0_5DDC65B71D39__INCLUDED_)
