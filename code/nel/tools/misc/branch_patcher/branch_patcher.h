// branch_patcher.h : main header file for the BRANCH_PATCHER application
//

#if !defined(AFX_BRANCH_PATCHER_H__7221F056_C6DB_48EB_9FF4_CD5208425BB2__INCLUDED_)
#define AFX_BRANCH_PATCHER_H__7221F056_C6DB_48EB_9FF4_CD5208425BB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBranch_patcherApp:
// See branch_patcher.cpp for the implementation of this class
//

class CBranch_patcherApp : public CWinApp
{
public:
	CBranch_patcherApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBranch_patcherApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBranch_patcherApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BRANCH_PATCHER_H__7221F056_C6DB_48EB_9FF4_CD5208425BB2__INCLUDED_)
