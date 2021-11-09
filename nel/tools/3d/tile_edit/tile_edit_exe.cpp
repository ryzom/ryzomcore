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

#include "stdafx.h"
#include "tile_edit_exe.h"
#include "SelectionTerritoire.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTile_edit_exeApp

BEGIN_MESSAGE_MAP(CTile_edit_exeApp, CWinApp)
	//{{AFX_MSG_MAP(CTile_edit_exeApp)
	ON_BN_CLICKED(ID_SAVE_AS, OnSaveAs)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTile_edit_exeApp construction

CTile_edit_exeApp::CTile_edit_exeApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTile_edit_exeApp object

CTile_edit_exeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTile_edit_exeApp initialization

BOOL CTile_edit_exeApp::InitInstance()
{
//	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	Start();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CTile_edit_exeApp::OnSaveAs() 
{
	// TODO: Add your control notification handler code here
	
}
