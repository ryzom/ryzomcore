// nel_launcher.h : main header file for the NEL_LAUNCHER application
//

#if !defined(AFX_NEL_LAUNCHER_H__C45E038E_0266_4168_98C0_57A9FD49B3B7__INCLUDED_)
#define AFX_NEL_LAUNCHER_H__C45E038E_0266_4168_98C0_57A9FD49B3B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include <afxinet.h>

#include "Configuration.h"

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherApp:
// See nel_launcher.cpp for the implementation of this class
//
#define APP	(*((CNel_launcherApp*)AfxGetApp()))
#define MAGIC_KEY_MD5	"#-#ryzom#-#"
#define RYZOM_HOST		"ryzom-europe.goa.com"
#define URL_LOGIN_WEB	"http://" RYZOM_HOST "/betatest/login.php"
#define CONFIG_FILE		"nel_launcher.cfg"
#define PATCH_BAT_FILE	"update_nel_launcher.bat"
#define MSGBOX(sTitle, sMsg)	{	CMsgDlg dlgMsg(sTitle, sMsg); dlgMsg.DoModal();	}
#define IS_WINNT		(APP.m_bWinNT)

class CNel_launcherApp : public CWinApp
{
public:
	CNel_launcherApp();
	void SetRegKey(char* lpszValueName, CString csValue);
	CString GetRegKeyValue(const char* lpszEntry);
	void	ResetConnection();
	BOOL	GetWindowsVersion();

public:	
	CString	m_csLogin;
	CString	m_csPassword;
	HCURSOR	m_hcPointer;
	BOOL	m_bWinNT;
	BOOL	m_bAuthWeb;
	BOOL	m_bAuthGame;
	CConfiguration	m_config;
	double	m_dVersion;
	BOOL	m_bLog;

private:
	CString ReadInfoFromRegistry(const char* lpszEntry, HKEY hkey);
	void	LoadVersion();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNel_launcherApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNel_launcherApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEL_LAUNCHER_H__C45E038E_0266_4168_98C0_57A9FD49B3B7__INCLUDED_)
