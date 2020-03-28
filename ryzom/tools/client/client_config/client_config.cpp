// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// client_config.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "client_config.h"
#include "client_configDlg.h"
#include "cfg_file.h"

#include <nel/misc/system_utils.h>

using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************

// CClientConfigApp

BEGIN_MESSAGE_MAP(CClientConfigApp, CWinApp)
	//{{AFX_MSG_MAP(CClientConfigApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// ***************************************************************************

// CClientConfigApp construction

CClientConfigApp::CClientConfigApp()
{
	// Place all significant initialization in InitInstance
	Localized = false;
}

// ***************************************************************************

// The one and only CClientConfigApp object

CClientConfigApp theApp;

// ***************************************************************************

// CClientConfigApp initialization

class CMyCommandLineInfo : public CCommandLineInfo
{
	virtual void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
	{
		/* Yoyo: Disable AutoConfig cause don't work on every config
		if ((strnicmp (lpszParam, "auto_config", 3) == 0) && bFlag)
		{
			AutoConfig = true;
		}
		if ((strnicmp (lpszParam, "gpu", 3) == 0) && bFlag)
			GLRenderer = lpszParam+3;
		if ((strnicmp (lpszParam, "cpu", 3) == 0) && bFlag)
			CPUFrequency = atoi (lpszParam+3);
		if ((strnicmp (lpszParam, "ram", 3) == 0) && bFlag)
			SystemMemory = 1024*1024*atoi (lpszParam+3);
		if ((strnicmp (lpszParam, "vram", 4) == 0) && bFlag)
			VideoMemory = 1024*1024*atoi (lpszParam+4);
		if ((strnicmp (lpszParam, "hwsb", 4) == 0) && bFlag)
			HardwareSoundBuffer = atoi (lpszParam+4);
		if ((strcmp (lpszParam, "?") == 0) && bFlag)
				MessageBoxW(NULL, 
			"\t/gpuGPUNAME\t\tSet GPU name to GPUNAME\n"
			"\t/cpuCPUFREQUENCY\t\tSet CPU frequency to CPUFREQENCY\n"
			"\t/ramAMOUNTRAM\t\tSet the amount of ram to AMOUNTRAM\n"
			"\t/vramAMOUNTVRAM\t\tSet the amount of vram to AMOUNTVRAM\n"
			"\t/hwsbHARDWARESOUNDBUFFER\t\tSet the number of hardware sound buffer\n"
			"\t/auto_config\t\tAuto configuration mode\n"
			"\t/?\t\tHelp\n"
			"\nExemple : ryzom_configuration_r \"/gpu geforce4 ti 4600\" /cpu1500 /ram512 /vram64"
			, CI18N::get ("uiConfigTitle"), MB_ICONEXCLAMATION|MB_OK);
			*/
	}
};

void pump ()
{
	// Display the window
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

INT_PTR CALLBACK MyDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	return FALSE;
}

BOOL CClientConfigApp::InitInstance()
{
	HANDLE mutex = CreateMutex (NULL, false, "RyzomConfig");
	if (mutex && GetLastError() == ERROR_ALREADY_EXISTS) 
		exit (0);

	// Get the bitmap size
	HRSRC hrsrc = FindResource(m_hInstance, MAKEINTRESOURCE(IDB_SLASH_SCREEN), RT_BITMAP);
	nlassert (hrsrc);
	HGLOBAL hBitmap = LoadResource (m_hInstance, hrsrc);
	nlassert (hBitmap);
	BITMAP *bitmap = (BITMAP*)LockResource(hBitmap);
	nlassert (bitmap);
	int width = bitmap->bmWidth;
	int height = bitmap->bmHeight;

	// Look the command line to see if we have a cookie and a addr
	HWND SlashScreen = CreateDialog (m_hInstance, MAKEINTRESOURCE(IDD_SLASH_SCREEN), NULL, MyDialogProc);
	RECT rect;
	RECT rectDesktop;
	GetWindowRect (SlashScreen, &rect);
	GetWindowRect (GetDesktopWindow (), &rectDesktop);
	SetWindowPos (SlashScreen, HWND_TOP, (rectDesktop.right-rectDesktop.left-width)/2, (rectDesktop.bottom-rectDesktop.top-height)/2, width, height, 0);
	ShowWindow (SlashScreen, SW_SHOW);

	pump ();

	try
	{
		AfxEnableControlContainer();

		// Create drivers
		IDriver *glDriver = CDRU::createGlDriver();
		IDriver *d3dDriver = CDRU::createD3DDriver();

		// Get some information about the system
		RegisterVideoModes (0, glDriver);
		RegisterVideoModes (1, d3dDriver);
		GetSystemInformation (d3dDriver);

		// Load the config file
		if (!LoadConfigFile ())
			return FALSE;

		// Add search pathes for *.uxt
		// Local search path ?
		if (::GetIntTestConfig())
			CPath::addSearchPath ("translation/work", true, false);
		CPath::addSearchPath ("patch", true, false);
		CPath::addSearchPath ("data", true, false);

		// Standard initialization
		// If you are not using these features and wish to reduce the size
		//  of your final executable, you should remove from the following
		//  the specific initialization routines you do not need.

#ifdef _AFXDLL
		Enable3dControls();			// Call this when using MFC in a shared DLL
#else
		Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

		// Init the data
		LoadConfigFileDefault ();

		// Init the database
		CreateDataBase ();

		// Parse command line for standard shell commands, DDE, file open
		CMyCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);

		CClient_configDlg dlg;
		m_pMainWnd = &dlg;

		DestroyWindow (SlashScreen);

		sint nResponse = (sint)dlg.DoModal();
		if (nResponse == IDOK)
		{
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			//  dismissed with Cancel
		}
	}
	catch (Exception &e)
	{
		error ((ucstring)e.what());
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


// ***************************************************************************

void CClientConfigApp::error (const ucstring &message)
{
	if (m_pMainWnd)
	{
		if (Localized)
		{
			MessageBoxW(*m_pMainWnd, (WCHAR*)message.c_str(), (WCHAR*)CI18N::get ("uiConfigTitle").c_str(), MB_OK|MB_ICONEXCLAMATION);
		}
		else
		{
			MessageBoxW(*m_pMainWnd, (WCHAR*)message.c_str(), (WCHAR*)ucstring("Ryzom Configuration").c_str(), MB_OK|MB_ICONEXCLAMATION);
		}
	}
	else
		MessageBoxW(NULL, (WCHAR*)message.c_str(), (WCHAR*)ucstring("Ryzom Configuration").c_str(), MB_OK|MB_ICONEXCLAMATION);
}

// ***************************************************************************

bool CClientConfigApp::yesNo (const ucstring &question)
{
	if (m_pMainWnd)
		return MessageBoxW(*m_pMainWnd, (WCHAR*)question.c_str(), (WCHAR*)CI18N::get ("uiConfigTitle").c_str(), MB_YESNO|MB_ICONQUESTION) == IDYES;
	else
		return MessageBoxW(NULL, (WCHAR*)question.c_str(), (WCHAR*)CI18N::get ("uiConfigTitle").c_str(), MB_YESNO|MB_ICONQUESTION) == IDYES;
}

// ***************************************************************************

std::string GetNeLString (uint res)
{
	CString str;
	str.LoadString (res);
	return (const char*)str;
}

// ***************************************************************************

CString GetString (uint res)
{
	CString str;
	str.LoadString (res);
	return str;
}

// ***************************************************************************
void setWindowText(HWND hwnd, LPCWSTR lpText)
{		
	if (CSystemUtils::supportUnicode())
	{
		SetWindowTextW(hwnd, lpText);
	}
	else
	{
		ucstring text((const ucchar *) lpText);		
		SetWindowTextA(hwnd, (LPCTSTR) text.toString().c_str());
	}	
}

