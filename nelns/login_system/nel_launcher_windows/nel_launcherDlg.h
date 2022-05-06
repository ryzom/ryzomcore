// nel_launcherDlg.h : header file
//
//{{AFX_INCLUDES()
#include "webbrowser2.h"
//}}AFX_INCLUDES

#if !defined(AFX_NEL_LAUNCHERDLG_H__A68D28F3_A36F_4D34_99D9_716C870E9099__INCLUDED_)
#define AFX_NEL_LAUNCHERDLG_H__A68D28F3_A36F_4D34_99D9_716C870E9099__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherDlg dialog

class CNel_launcherDlg : public CDialog
{
// Construction
public:
	CNel_launcherDlg(CWnd* pParent = NULL);	// standard constructor

	CWebBrowser2	m_explore;

// Dialog Data
	//{{AFX_DATA(CNel_launcherDlg)
	enum { IDD = IDD_NEL_LAUNCHER_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNel_launcherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	void launch (const std::string &str);
	void patch (const std::string &str);
	void openUrl (const std::string &url);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNel_launcherDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBeforeNavigate2Explorer1(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT FAR* URL);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEL_LAUNCHERDLG_H__A68D28F3_A36F_4D34_99D9_716C870E9099__INCLUDED_)
