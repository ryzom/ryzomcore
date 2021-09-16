#if !defined(AFX_LOADINGPAGEDLG_H__15410098_7AF4_4CCF_9A1E_658B2DCA8D60__INCLUDED_)
#define AFX_LOADINGPAGEDLG_H__15410098_7AF4_4CCF_9A1E_658B2DCA8D60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoadingPageDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CLoadingPageDlg dialog

class CLoadingPageDlg : public CDialog
{
// Construction
public:
	CLoadingPageDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoadingPageDlg)
	enum { IDD = IDD_LOADING_PAGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadingPageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLoadingPageDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADINGPAGEDLG_H__15410098_7AF4_4CCF_9A1E_658B2DCA8D60__INCLUDED_)
