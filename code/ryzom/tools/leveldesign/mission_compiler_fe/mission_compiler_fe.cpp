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

// mission_compiler_fe.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "mission_compiler_fe.h"
#include "mission_compiler_feDlg.h"
#include "dialog_mode.h"
#include <string>
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_utils.h"
#include "nel/misc/config_file.h"
#include "../mission_compiler_lib/mission_compiler.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CMissionCompilerFeApp

BEGIN_MESSAGE_MAP(CMissionCompilerFeApp, CWinApp)
	//{{AFX_MSG_MAP(CMissionCompilerFeApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissionCompilerFeApp construction

CMissionCompilerFeApp::CMissionCompilerFeApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The nel context
NLMISC::CApplicationContext AppContext;

/////////////////////////////////////////////////////////////////////////////
// The one and only CMissionCompilerFeApp object

CMissionCompilerFeApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CMissionCompilerFeApp initialization

BOOL CMissionCompilerFeApp::InitInstance()
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

	// look at the command line parameters for command line mode
	string cmdLine = m_lpCmdLine;
	if (cmdLine.find("-c") != string::npos)
	{
		NLMISC::createDebug();
		NLMISC::CStdDisplayer disp;
		NLMISC::DebugLog->addDisplayer(&disp);
		NLMISC::InfoLog->addDisplayer(&disp);
		NLMISC::WarningLog->addDisplayer(&disp);
		NLMISC::ErrorLog->addDisplayer(&disp);

		CMissionCompilerFeDlg dlg;

		// init the config file and other params
		if (!dlg.readConfigFile())
			exit(-1);

		// get the primitive file to compile
		string::size_type pos = cmdLine.find("-c");
		string fileName = cmdLine.substr(pos+2);
		fileName = fileName.substr(0, fileName.find(" "));

		if (!NLMISC::CFile::isExists(NLMISC::CPath::lookup(fileName, false, true)))
		{
			nlwarning("Can't find file '%s'", fileName.c_str());
			return -1;
		}

		uint32 nbMission = 0;

		nlinfo("Compiling primitive file '%s' in '%s'", fileName.c_str(), NLMISC::CPath::lookup(fileName, false, true).c_str());
		NLLIGO::CPrimitives primDoc;
		NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
		NLLIGO::loadXmlPrimitiveFile(primDoc, fileName, dlg.LigoConfig);
		NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;
		try
		{
			CMissionCompiler mc;
			mc.compileMissions(primDoc.RootNode, fileName);
			nlinfo("Found %u valid missions",mc.getMissionsCount());
			mc.installCompiledMission(dlg.LigoConfig, fileName);
			nbMission += mc.getMissionsCount();
		}
		catch(EParseException e)
		{
			string msg;
			msg + "\r\n";
			if (e.Primitive != NULL)
			{
				msg += "In '"+NLLIGO::buildPrimPath(e.Primitive)+"'\r\n";
			}
			msg += "Error while compiling '"+fileName+"' :\r\n"+e.Why+"\r\n";
			nlwarning(msg.c_str());
			exit(-1);
		}
		nlinfo("Compiled and installed %u missions", nbMission);

		NLMISC::DebugLog->removeDisplayer(&disp);
		NLMISC::InfoLog->removeDisplayer(&disp);
		NLMISC::WarningLog->removeDisplayer(&disp);
		NLMISC::ErrorLog->removeDisplayer(&disp);

		return FALSE;
	}

	//CDialogMode		dlgMode;
	//dlgMode.DoModal();

	CMissionCompilerFeDlg dlg;
	//dlg.Mode = dlgMode.Mode;
	dlg.Mode = mode_compile;
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
