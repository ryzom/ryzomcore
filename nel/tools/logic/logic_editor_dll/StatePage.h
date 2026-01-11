#if !defined(AFX_STATEPAGE_H__6FCFCFD9_9FCA_4790_9B43_9BF802EC332C__INCLUDED_)
#define AFX_STATEPAGE_H__6FCFCFD9_9FCA_4790_9B43_9BF802EC332C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StatePage.h : header file
//

#include "ResizablePage.h"
#include "State.h"

class CLogic_editorDoc;

/////////////////////////////////////////////////////////////////////////////
// CStatePage dialog

class CStatePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CStatePage)

// Construction
public:
	CStatePage();
	~CStatePage();


	void Update();

	void addState( CLogic_editorDoc *pDoc, CState * state);

public:
	CState	*	m_pSelectedState;
	CEvent	*	m_pSelectedEvent;

// Dialog Data
	//{{AFX_DATA(CStatePage)
	enum { IDD = IDD_PAGE_STATES };
	int		m_nEventMessage;
	CString	m_sConditionName;
	CString	m_sNextStateName;
	CString	m_sStateName;
	CString	m_sMessageID;
	CString	m_sArgument;
	CString	m_sDestination;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CStatePage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CStatePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioStateChange();
	afx_msg void OnRadioStateEventMsg();
	afx_msg void OnButtonAddState();
	afx_msg void OnButtonAddEvent();
	afx_msg void OnButtonStateRemove();
	afx_msg void OnButtonStateApply();
	afx_msg void OnButtonEventApply();
	afx_msg void OnButtonEventRemove();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATEPAGE_H__6FCFCFD9_9FCA_4790_9B43_9BF802EC332C__INCLUDED_)
