#if !defined(AFX_PROGRESS_DIALOG_H__BAFC0013_447F_4C68_B7F4_AE148ED1FA22__INCLUDED_)
#define AFX_PROGRESS_DIALOG_H__BAFC0013_447F_4C68_B7F4_AE148ED1FA22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// progress_dialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressDialog dialog

class CProgressDialog : public CDialog, public NLMISC::IProgressCallback
{
// Construction
public:
	CProgressDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProgressDialog)
	enum { IDD = IDD_PROGRESS };
	CProgressCtrl	Bar;
	CString	Text;
	//}}AFX_DATA

	/// \from IProgressCallback
	virtual void progress (float progressValue);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProgressDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESS_DIALOG_H__BAFC0013_447F_4C68_B7F4_AE148ED1FA22__INCLUDED_)
