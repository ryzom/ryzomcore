#if !defined(AFX_INTRO_SCREEN_DLG_H__B8233ADB_85E0_4CF7_B6FA_678A609E6A00__INCLUDED_)
#define AFX_INTRO_SCREEN_DLG_H__B8233ADB_85E0_4CF7_B6FA_678A609E6A00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// intro_screen_dlg.h : header file
//


#include "rich_edit_ctrl_ex.h"

/////////////////////////////////////////////////////////////////////////////
// CIntroScreenDlg dialog

class CIntroScreenDlg : public CDialog
{
// Construction
public:
	CIntroScreenDlg(CWnd* pParent = NULL);   // standard constructor	
// Dialog Data
	//{{AFX_DATA(CIntroScreenDlg)
	enum { IDD = IDD_INSTALL_STEP };
	CRichEditCtrlEx	m_IntroText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIntroScreenDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIntroScreenDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCancel();
	afx_msg void OnNewinstall();
	afx_msg void OnRepair();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTRO_SCREEN_DLG_H__B8233ADB_85E0_4CF7_B6FA_678A609E6A00__INCLUDED_)
