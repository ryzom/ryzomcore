#if !defined(AFX_ABOUT_DIALOG_H__7AC72A72_F42F_4947_8630_1FE077BF66F4__INCLUDED_)
#define AFX_ABOUT_DIALOG_H__7AC72A72_F42F_4947_8630_1FE077BF66F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// about_dialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAboutDialog dialog

class CAboutDialog : public CDialog
{
// Construction
public:
	CAboutDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAboutDialog)
	enum { IDD = IDD_ABOUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAboutDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUT_DIALOG_H__7AC72A72_F42F_4947_8630_1FE077BF66F4__INCLUDED_)
