#if !defined(AFX_CHOOSE_PACKAGE_DLG_H__4C2CF3C5_2FAD_497D_BBB0_85475EDADBFA__INCLUDED_)
#define AFX_CHOOSE_PACKAGE_DLG_H__4C2CF3C5_2FAD_497D_BBB0_85475EDADBFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// choose_package_dlg.h : header file
//


#include "rich_edit_ctrl_ex.h"

/////////////////////////////////////////////////////////////////////////////
// CChoosePackageDlg dialog

class CChoosePackageDlg : public CDialog
{
// Construction
public:
	CChoosePackageDlg(CWnd* pParent = NULL);   // standard constructor
	// change the content of the text displayed in the windows (change the title or the content)
	void setInstallText(const ucstring& title, const ucstring& content);
// Dialog Data
	//{{AFX_DATA(CChoosePackageDlg)
	enum { IDD = IDD_CHOOSE_PACKAGE };
	CRichEditCtrlEx	m_PackageText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChoosePackageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChoosePackageDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCancel();
	afx_msg void OnTorrent();
	afx_msg void OnFullInstall();
	afx_msg void OnMinimalInstall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSE_PACKAGE_DLG_H__4C2CF3C5_2FAD_497D_BBB0_85475EDADBFA__INCLUDED_)
