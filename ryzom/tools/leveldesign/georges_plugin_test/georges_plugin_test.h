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

// georges_plugin_test.h : main header file for the GEORGES_PLUGIN_TEST DLL
//

#if !defined(AFX_GEORGES_PLUGIN_TEST_H__17C63138_6057_4288_99B4_D1158EE798CA__INCLUDED_)
#define AFX_GEORGES_PLUGIN_TEST_H__17C63138_6057_4288_99B4_D1158EE798CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include "../georges_dll/plugin_interface.h"
#include "test_local_dialog.h"
#include "test_global_dialog.h"

/////////////////////////////////////////////////////////////////////////////
// CGeorges_plugin_testApp
// See georges_plugin_test.cpp for the implementation of this class
//

class CGeorges_plugin_testApp : public CWinApp
{
public:
	CGeorges_plugin_testApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorges_plugin_testApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGeorges_plugin_testApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class MyDocumentPlugin : public NLGEORGES::IEditDocumentPlugin
{
public:
	MyDocumentPlugin (NLGEORGES::IEditDocument *doc);
	~MyDocumentPlugin ();
	virtual void dialogInit (HWND documentView);
	virtual bool pretranslateMessage (MSG *pMsg);
	virtual void activate (bool activated);
	virtual void onValueChanged (const char *formName);
	virtual void onNodeChanged ();

	CTestLocalDialog MyLocalDialog;
	NLGEORGES::IEditDocument *Document;
	std::string				LastValue;
};

class MyPlugin : public NLGEORGES::IEditPlugin
{
public:
	// From IEditPlugin
	MyPlugin (NLGEORGES::IEdit	*globalInterface);
	virtual ~MyPlugin ();
	virtual void dialogInit (HWND mainFrm);
	virtual bool pretranslateMessage (MSG *pMsg);
	virtual void onCreateDocument (NLGEORGES::IEditDocument *document);
	virtual void activate (bool activate);
	virtual void getPluginName (std::string &name); 

	CTestGlobalDialog	MyGlobalDialog;
	NLGEORGES::IEdit	*GlobalInterface;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEORGES_PLUGIN_TEST_H__17C63138_6057_4288_99B4_D1158EE798CA__INCLUDED_)
