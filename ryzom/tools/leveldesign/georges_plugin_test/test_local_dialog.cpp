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

// test_local_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "georges_plugin_test.h"
#include "test_local_dialog.h"

#include "nel/misc/file.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"

using namespace NLMISC;
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CTestLocalDialog dialog


CTestLocalDialog::CTestLocalDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTestLocalDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestLocalDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTestLocalDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestLocalDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTestLocalDialog, CDialog)
	//{{AFX_MSG_MAP(CTestLocalDialog)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestLocalDialog message handlers

void CTestLocalDialog::OnRefresh() 
{
	Plugin->Document->refreshView ();	
}

void CTestLocalDialog::OnSet() 
{
	Plugin->Document->setValue ("test set value", Plugin->LastValue.c_str ());
	// Plugin->Document->refreshView ();
}

void CTestLocalDialog::OnSave() 
{
	CFileDialog dialog (FALSE);
	if (dialog.DoModal () == IDOK)
	{
		string name = (const char*)dialog.GetPathName ();
		COFile file (name.c_str ());
		Plugin->Document->getForm ()->write (file, true);
	}
}
