// nel_launcherDlg.h : header file
//
#if !defined(AFX_NEL_LAUNCHERDLG_H__A68D28F3_A36F_4D34_99D9_716C870E9099__INCLUDED_)
#define AFX_NEL_LAUNCHERDLG_H__A68D28F3_A36F_4D34_99D9_716C870E9099__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ProgressDlg.h"
#include "WebDlg.h"
#include "BarTabsWnd.h"
#include "PictureHlp.h"
#include "LoginDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherDlg dialog
#define TITLEBAR_H		20
#define BROWSER_X		50-2
#define BROWSER_Y		(90+20-2)
#define BROWSER_W		400
#define BROWSER_H		338
#define PROGRESS_H		50
#define TABS_H			25
#define CLOSE_BTN_X		460
#define CLOSE_BTN_Y		28
#define CLOSE_BTN_W		36
#define CLOSE_BTN_H		36
#define BG_W			500
#define BG_H			500
#define MAIN_DLG_TITLE	"Nel Launcher"

class CNel_launcherDlg : public CDialog, CTabsObserver
{
// Construction
public:
	CNel_launcherDlg(CWnd* pParent = NULL);	// standard constructor

public:
	CBarTabsWnd		m_wndTabs;

private:
	CProgressDlg	m_dlgProgress;
	CWebDlg			m_dlgWeb;
	int				m_iX;
	int				m_iY;
	BOOL			m_bMoving;
	CPictureHlp		m_pictBG;
	CLoginDlg*		m_pdlgLogin;
	
// Dialog Data
	//{{AFX_DATA(CNel_launcherDlg)
	enum { IDD = IDD_NEL_LAUNCHER_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNel_launcherDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

public:
	void launch(CString cs);
	void patch(CString cs);
	virtual void OnTab(int iTab);
	BOOL Login();

private:
	void launch(const std::string &str);
	void patch(const std::string &str);
	void MoveBy(int x, int y);
	void CreateTabs();
	void CreateWebZone();
	void CreateProgressBar();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNel_launcherDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEL_LAUNCHERDLG_H__A68D28F3_A36F_4D34_99D9_716C870E9099__INCLUDED_)
