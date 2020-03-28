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

// georges_plugin_test.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "georges_plugin_test.h"


using namespace NLGEORGES;
using namespace std;

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CGeorges_plugin_testApp

BEGIN_MESSAGE_MAP(CGeorges_plugin_testApp, CWinApp)
	//{{AFX_MSG_MAP(CGeorges_plugin_testApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeorges_plugin_testApp construction

CGeorges_plugin_testApp::CGeorges_plugin_testApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGeorges_plugin_testApp object

CGeorges_plugin_testApp theApp;

// ***************************************************************************
// MyDocumentPlugin
// ***************************************************************************

MyDocumentPlugin::MyDocumentPlugin (IEditDocument *doc)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	Document = doc;
	MyLocalDialog.Plugin = this;
}

// ***************************************************************************

MyDocumentPlugin::~MyDocumentPlugin ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (IsWindow (MyLocalDialog))
		MyLocalDialog.DestroyWindow ();
}

// ***************************************************************************

void MyDocumentPlugin::dialogInit (HWND documentView)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	MyLocalDialog.Create (IDD_TEST_LOCAL, CWnd::FromHandle( documentView ));
}

// ***************************************************************************

bool MyDocumentPlugin::pretranslateMessage (MSG *pMsg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_F3))
	{
		MessageBox (NULL, "LOCAL: F3", "Test georges plugin", MB_OK);
	}

	return false;
}

// ***************************************************************************

void MyDocumentPlugin::activate (bool activated)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (IsWindow (MyLocalDialog))
		MyLocalDialog.ShowWindow (activated?SW_SHOW:SW_HIDE);
}

// ***************************************************************************

void MyDocumentPlugin::onValueChanged (const char *formName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	MyLocalDialog.GetDlgItem (IDC_MESSAGE)->SetWindowText (formName);
	LastValue = formName;
}

// ***************************************************************************

void MyDocumentPlugin::onNodeChanged ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	string str;
	if (Document->getActiveNode (str))
		MyLocalDialog.GetDlgItem (IDC_MESSAGE2)->SetWindowText (str.c_str ());
	else
		MyLocalDialog.GetDlgItem (IDC_MESSAGE2)->SetWindowText ("NILL");
}

// ***************************************************************************
// MyPlugin
// ***************************************************************************

MyPlugin::MyPlugin (NLGEORGES::IEdit	*globalInterface) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	GlobalInterface = globalInterface;
	MyGlobalDialog.Plugin = this;
}

// ***************************************************************************

MyPlugin::~MyPlugin ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (IsWindow (MyGlobalDialog))
		MyGlobalDialog.DestroyWindow ();
}

// ***************************************************************************

void MyPlugin::dialogInit (HWND mainFrm)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	MyGlobalDialog.Create (IDD_TEST_GLOBAL, CWnd::FromHandle( mainFrm ));
}

// ***************************************************************************

bool MyPlugin::pretranslateMessage (MSG *pMsg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_F1))
	{
		MessageBox (NULL, "GLOBAL: F1", "Test georges plugin", MB_OK);
	}

	return false;
}

// ***************************************************************************

void MyPlugin::onCreateDocument (IEditDocument *document)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Set text in the global dialog
	string toto;
	document->getDfnFilename (toto);
	if (IsWindow (MyGlobalDialog))
		MyGlobalDialog.GetDlgItem (IDC_MESSAGE)->SetWindowText (toto.c_str());

	// Bind an interface on the document
	document->bind (this, new MyDocumentPlugin (document));
}

// ***************************************************************************

void MyPlugin::activate (bool activate)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (IsWindow (MyGlobalDialog))
		MyGlobalDialog.ShowWindow (activate?SW_SHOW:SW_HIDE);
}

// ***************************************************************************

void MyPlugin::getPluginName (std::string &name)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	name = "Test georges plugin";
}

// ***************************************************************************

__declspec( dllexport ) IEditPlugin *IGeorgesEditGetInterface (int version, NLGEORGES::IEdit *globalInterface)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Same version ?
	if (version == NLGEORGES_PLUGIN_INTERFACE_VERSION)
	{
		return new MyPlugin (globalInterface);
	}
	else
	{
		MessageBox (NULL, "Plugin version invalid.", "Test plugin for georges editor", MB_OK|MB_ICONEXCLAMATION);
		return NULL;
	}

}
