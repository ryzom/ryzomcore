#if !defined(AFX_COUNTERPAGE_H__C0CC8238_CBB5_441E_A7CD_B0C4F161BDB2__INCLUDED_)
#define AFX_COUNTERPAGE_H__C0CC8238_CBB5_441E_A7CD_B0C4F161BDB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CounterPage.h : header file
//


#include "NumEdit.h"

class CLogic_editorDoc;
class CCounter;


/////////////////////////////////////////////////////////////////////////////
// CCounterPage dialog

class CCounterPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CCounterPage)

// Construction
public:
	CCounterPage();
	~CCounterPage();

	void addCounter( CLogic_editorDoc *pDoc, CCounter * pCounter );

// Dialog Data
	//{{AFX_DATA(CCounterPage)
	enum { IDD = IDD_PAGE_COUNTERS };
	CNumEdit	m_neLowerLimit;
	CListBox	m_counters;
	CString	m_sMode;
	CString	m_sWay;
	CString	m_sCounterName;
	long	m_nUpperLimit;
	long	m_nLowerLimit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CCounterPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCounterPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAddCounter();
	afx_msg void OnSelchangeListCounters();
	afx_msg void OnButtonCounterRemove();
	afx_msg void OnButtonCounterApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COUNTERPAGE_H__C0CC8238_CBB5_441E_A7CD_B0C4F161BDB2__INCLUDED_)
