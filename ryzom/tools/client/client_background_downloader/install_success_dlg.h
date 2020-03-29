#if !defined(AFX_INSTALLSUCCESSDLG_H__99D7912B_475B_45E1_889F_87FFAF541DAA__INCLUDED_)
#define AFX_INSTALLSUCCESSDLG_H__99D7912B_475B_45E1_889F_87FFAF541DAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InstallSuccessDlg.h : header file
//


#include "rich_edit_ctrl_ex.h"

/////////////////////////////////////////////////////////////////////////////
// CInstallSuccessDlg dialog

class CInstallSuccessDlg : public CDialog
{
// Construction
public:
	CInstallSuccessDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInstallSuccessDlg)
	enum { IDD = IDD_INSTALL_SUCCESS };
	CRichEditCtrlEx	m_CongratText;
	BOOL	m_StartRyzomNow;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInstallSuccessDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInstallSuccessDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnStartRyzomNow();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSTALLSUCCESSDLG_H__99D7912B_475B_45E1_889F_87FFAF541DAA__INCLUDED_)
