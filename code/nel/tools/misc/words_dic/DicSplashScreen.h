#if !defined(AFX_DICSPLASHSCREEN_H__7DF0565F_D1BE_4FBA_B8E9_6DDDEEBBAF0E__INCLUDED_)
#define AFX_DICSPLASHSCREEN_H__7DF0565F_D1BE_4FBA_B8E9_6DDDEEBBAF0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DicSplashScreen.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDicSplashScreen dialog

class CDicSplashScreen : public CDialog
{
// Construction
public:
	CDicSplashScreen(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDicSplashScreen)
	enum { IDD = IDD_SplashScreen };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDicSplashScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDicSplashScreen)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DICSPLASHSCREEN_H__7DF0565F_D1BE_4FBA_B8E9_6DDDEEBBAF0E__INCLUDED_)
