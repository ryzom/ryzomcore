// branch_patcher.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "branch_patcher.h"
#include "branch_patcherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBranch_patcherApp

BEGIN_MESSAGE_MAP(CBranch_patcherApp, CWinApp)
	//{{AFX_MSG_MAP(CBranch_patcherApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBranch_patcherApp construction

CBranch_patcherApp::CBranch_patcherApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBranch_patcherApp object

CBranch_patcherApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CBranch_patcherApp initialization

BOOL CBranch_patcherApp::InitInstance()
{
	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CBranch_patcherDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
