// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// project_settings.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "world_editor_doc.h"
#include "main_frm.h"
#include "project_settings.h"

using namespace std;

// ***************************************************************************
// CProjectSettings dialog


CProjectSettings::CProjectSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CProjectSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProjectSettings)
	DataDirectory = getDocument ()->getDataDir ().c_str ();
	//}}AFX_DATA_INIT
}


void CProjectSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProjectSettings)
	DDX_Control(pDX, IDC_CONTEXT, Context);
	DDX_Text(pDX, IDC_DATA_DIRECTORY, DataDirectory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProjectSettings, CDialog)
	//{{AFX_MSG_MAP(CProjectSettings)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
int CALLBACK dataDirBrowseCallbackProc (HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
	switch (uMsg) 
	{
		case BFFM_INITIALIZED: 
			SendMessage (hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
		default:
		break;
	}
	return 0;
}

// ***************************************************************************
// CProjectSettings message handlers

void CProjectSettings::OnBrowse() 
{
	UpdateData ();

	BROWSEINFO	 bi;
	TCHAR		 str[MAX_PATH];
	LPITEMIDLIST pidl;
	TCHAR sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;
	bi.lpszTitle = _T("Choose the data directory for this project");
	bi.ulFlags = 0;
	bi.lpfn = dataDirBrowseCallbackProc;

	TCHAR sDir[512];
	_tcscpy(sDir, DataDirectory);
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (SHGetPathFromIDList(pidl, str)) 
	{
		DataDirectory = str;

		// Refresh data
		UpdateData (FALSE);
	}
}

// ***************************************************************************

void CProjectSettings::OnOK() 
{
	CDialog::OnOK();

	UpdateData ();

	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	std::string str;
	getWindowTextUTF8 (Context, str);
	doc->setContext (str);
}

// ***************************************************************************

BOOL CProjectSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Init the combo box
	const vector<string> &contexts = theApp.Config.getContextString();
	for (uint i=0; i<contexts.size (); i++)
	{
		// Add the string
		Context.InsertString(-1, nlUtf8ToTStr(contexts[i]));
	}

	// Select the string 
	Context.SelectString(-1, nlUtf8ToTStr(doc->getContext()));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

