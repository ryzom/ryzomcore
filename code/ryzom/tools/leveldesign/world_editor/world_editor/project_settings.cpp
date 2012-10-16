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

	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;
	bi.lpszTitle = "Choose the data directory for this project";
	bi.ulFlags = 0;
	bi.lpfn = dataDirBrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, DataDirectory);
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

	CString str;
	getWindowTextUTF8 (Context, str);
	doc->setContext ((const char*)str);
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
		Context.InsertString (-1, contexts[i].c_str());
	}

	// Select the string 
	Context.SelectString (-1, doc->getContext ().c_str ());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

