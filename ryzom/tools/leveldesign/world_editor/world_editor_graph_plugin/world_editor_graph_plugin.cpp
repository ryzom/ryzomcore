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

// world_editor_graph_plugin.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "world_editor_graph_plugin.h"
#include "world_editor_graph_plugin_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorGraphPluginApp

BEGIN_MESSAGE_MAP(CWorldEditorGraphPluginApp, CWinApp)
	//{{AFX_MSG_MAP(CWorldEditorGraphPluginApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorGraphPluginApp construction

CWorldEditorGraphPluginApp::CWorldEditorGraphPluginApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWorldEditorGraphPluginApp object

// if we put that, it failed to load the dll :
// "A dynamic link library (DLL) initialization routine failed."
//CWorldEditorGraphPluginApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorGraphPluginApp initialization

BOOL CWorldEditorGraphPluginApp::InitInstance()
{
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


	CWorldEditorGraphPluginDlg dlg;
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

int CWorldEditorGraphPluginApp::ExitInstance() 
{
	return CWinApp::ExitInstance();
}
