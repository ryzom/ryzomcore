/*
 * $Id:
 */

#if !defined(AFX_CLOSEWARNING_H__B55CE000_8E72_4AD1_8283_25D3A30336FD__INCLUDED_)
#define AFX_CLOSEWARNING_H__B55CE000_8E72_4AD1_8283_25D3A30336FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CloseWarning.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCloseWarning dialog

class CCloseWarning : public CDialog
{
// Construction
public:
	CCloseWarning(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCloseWarning)
	enum { IDD = IDD_CLOSE_WARNING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCloseWarning)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCloseWarning)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLOSEWARNING_H__B55CE000_8E72_4AD1_8283_25D3A30336FD__INCLUDED_)
