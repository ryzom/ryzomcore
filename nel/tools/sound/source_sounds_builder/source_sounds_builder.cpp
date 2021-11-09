// source_sounds_builder.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "source_sounds_builder.h"
#include "source_sounds_builderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSource_sounds_builderApp

BEGIN_MESSAGE_MAP(CSource_sounds_builderApp, CWinApp)
	//{{AFX_MSG_MAP(CSource_sounds_builderApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSource_sounds_builderApp construction

CSource_sounds_builderApp::CSource_sounds_builderApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSource_sounds_builderApp object

CSource_sounds_builderApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSource_sounds_builderApp initialization

BOOL CSource_sounds_builderApp::InitInstance()
{
	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CSource_sounds_builderDlg dlg;
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
