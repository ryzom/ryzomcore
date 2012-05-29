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

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherApp:
// See nel_launcher.cpp for the implementation of this class
//

class CNel_launcherApp : public CWinApp
{
public:
	CNel_launcherApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNel_launcherApp)
	public:
	virtual BOOL InitInstance();
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
