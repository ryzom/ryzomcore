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

// settings_dialog.cpp : implementation file
//

#include "stdafx.h"

#include "georges_edit.h"
#include "settings_dialog.h"

using namespace NLMISC;

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog dialog


CSettingsDialog::CSettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingsDialog)
	RootSearchPath = _T("");
	RememberListSize = 0;
	DefaultDfn = _T("");
	DefaultType = _T("");
	TypeDfnSubDirectory = _T("");
	MaxUndo = 0;
	StartExpanded = TRUE;
	Georges4CVS = TRUE;
	//}}AFX_DATA_INIT
}


void CSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDialog)
	DDX_Text(pDX, IDC_ROOT_SEARCH_PATH, RootSearchPath);
	DDX_Text(pDX, IDC_REMEMBER_LIST_SIZE, RememberListSize);
	DDX_Text(pDX, IDC_DEFAULT_DFN, DefaultDfn);
	DDX_Text(pDX, IDC_DEFAULT_TYPE, DefaultType);
	DDX_Text(pDX, IDC_TYP_DFN_PATH, TypeDfnSubDirectory);
	DDX_Text(pDX, IDC_MAX_UNDO, MaxUndo);
	DDX_Check(pDX, IDC_START_EXPANDED, StartExpanded);
	DDX_Check(pDX, IDC_GEORGES_4_CVS, Georges4CVS);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingsDialog, CDialog)
	//{{AFX_MSG_MAP(CSettingsDialog)
	ON_EN_CHANGE(IDC_ROOT_SEARCH_PATH, OnChangeRootSearchPath)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_EN_CHANGE(IDC_REMEMBER_LIST_SIZE, OnChangeRememberListSize)
	ON_EN_CHANGE(IDC_DEFAULT_DFN, OnChangeDefaultDfn)
	ON_EN_CHANGE(IDC_DEFAULT_TYPE, OnChangeDefaultType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog message handlers

void CSettingsDialog::OnChangeRootSearchPath() 
{
}

BOOL CSettingsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Reset flags
	RootSearchPath = "\\";

	// Get the config file path
	theApp.loadCfg ();
	RootSearchPath = theApp.RootSearchPath.c_str();
	TypeDfnSubDirectory = theApp.TypeDfnSubDirectory.c_str();
	RememberListSize = theApp.RememberListSize;
	MaxUndo = theApp.MaxUndo;
	DefaultDfn = theApp.DefaultDfn.c_str ();
	DefaultType = theApp.DefaultType.c_str ();
	StartExpanded = theApp.StartExpanded;
	Georges4CVS = theApp.Georges4CVS;

	UpdateData (FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDialog::OnOK() 
{
	UpdateData ();

	// Make a config file
	theApp.RootSearchPath = RootSearchPath;
	theApp.TypeDfnSubDirectory = TypeDfnSubDirectory;
	theApp.RememberListSize = RememberListSize;
	theApp.StartExpanded = StartExpanded ? TRUE : FALSE;
	theApp.Georges4CVS = Georges4CVS ? TRUE : FALSE;
	theApp.MaxUndo = MaxUndo;
	theApp.DefaultDfn = DefaultDfn;
	theApp.DefaultType = DefaultType;
	theApp.saveCfg ();
	theApp.initCfg ();

	CDialog::OnOK();
}

void CSettingsDialog::OnBrowse() 
{
	UpdateData ();
	
	// Select a directory.
	char path[MAX_PATH];

	// Build the struct
	BROWSEINFO info;
	memset (&info, 0, sizeof (BROWSEINFO));
	info.lpszTitle="Select the root search directory";
	info.ulFlags=BIF_RETURNONLYFSDIRS;

	// Select the path
	LPITEMIDLIST list;
	if (list=SHBrowseForFolder (&info))
	{
		// Convert item into path string
		BOOL bRet=SHGetPathFromIDList(list, path);
		if (bRet)
		{
			// Set the path
			RootSearchPath = path;
		}
	}	
	UpdateData (FALSE);
}

void CSettingsDialog::OnChangeRememberListSize() 
{
}

void CSettingsDialog::OnChangeDefaultDfn() 
{
}

void CSettingsDialog::OnChangeDefaultType() 
{
}
