// nel_launcher.cpp : Defines the class behaviors for the application.
//







/**	TODO:

  - find a way to remove the right-click-popup-window in the IE
  - find a way to remove the right scroll bar when it s not needed
  - if load page error, trap it and display an error, retry, quit...

  */



#include "stdafx.h"
#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include "nel_launcher.h"
#include "nel_launcherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

NLMISC::CFileDisplayer FileDisplayer;

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherApp

BEGIN_MESSAGE_MAP(CNel_launcherApp, CWinApp)
	//{{AFX_MSG_MAP(CNel_launcherApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherApp construction

CNel_launcherApp::CNel_launcherApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNel_launcherApp object

CNel_launcherApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherApp initialization

BOOL CNel_launcherApp::InitInstance()
{
	// use log.log if NEL_LOG_IN_FILE defined as 1
	NLMISC::createDebug(NULL, true, true);

	// filedisplayer only deletes the 001 etc
	if (NLMISC::CFile::isExists("nel_launcher.log"))
		NLMISC::CFile::deleteFile("nel_launcher.log");
	// initialize the log file
	FileDisplayer.setParam("nel_launcher.log", true);
	NLMISC::DebugLog->addDisplayer(&FileDisplayer);
	NLMISC::InfoLog->addDisplayer(&FileDisplayer);
	NLMISC::WarningLog->addDisplayer(&FileDisplayer);
	NLMISC::AssertLog->addDisplayer(&FileDisplayer);
	NLMISC::ErrorLog->addDisplayer(&FileDisplayer);

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CNel_launcherDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
