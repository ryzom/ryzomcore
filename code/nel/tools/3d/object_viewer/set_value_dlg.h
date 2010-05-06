#if !defined(AFX_SET_VALUE_DLG_H__7120C592_1A40_4A48_B718_714AC36356A1__INCLUDED_)
#define AFX_SET_VALUE_DLG_H__7120C592_1A40_4A48_B718_714AC36356A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// set_value_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetValueDlg dialog

class CSetValueDlg : public CDialog
{
// Construction
public:
	CSetValueDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetValueDlg)
	enum { IDD = IDD_SET_VALUE };
	CString	Value;
	//}}AFX_DATA

	CString	Title;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetValueDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetValueDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SET_VALUE_DLG_H__7120C592_1A40_4A48_B718_714AC36356A1__INCLUDED_)
