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

// test_global_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "georges_plugin_test.h"
#include "test_global_dialog.h"

/////////////////////////////////////////////////////////////////////////////
// CTestGlobalDialog dialog


CTestGlobalDialog::CTestGlobalDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTestGlobalDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestGlobalDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTestGlobalDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestGlobalDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTestGlobalDialog, CDialog)
	//{{AFX_MSG_MAP(CTestGlobalDialog)
	ON_BN_CLICKED(IDC_CREATE_DOC, OnCreateDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestGlobalDialog message handlers

void CTestGlobalDialog::OnCreateDoc() 
{
	Plugin->GlobalInterface->createDocument ("item.dfn", "test_new_document_plugin.item");
}
