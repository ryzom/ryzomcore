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

// enter_name.cpp : implementation file
//
#ifdef _WINDOWS

#include "stdafx.h"
#include "enter_name.h"
#include "nel/misc/sstring.h"
#include "nel/misc/variable.h"
#include "nel/misc/path.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnterName dialog


CEnterName::CEnterName(CWnd* pParent /*=NULL*/)
	: CDialog(CEnterName::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnterName)
	Text = _T("");
	ComboValue = _T("");
	//}}AFX_DATA_INIT
}


void CEnterName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnterName)
	DDX_Control(pDX, IDC_COMBO1, Combo);
	DDX_Text(pDX, IDC_EDIT1, Text);
	DDX_CBString(pDX, IDC_COMBO1, ComboValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnterName, CDialog)
	//{{AFX_MSG_MAP(CEnterName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnterName message handlers

extern NLMISC::CVariable<std::string>	ToolsDirectory;

BOOL CEnterName::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	std::vector<std::string> directories;
	NLMISC::CPath::getPathContent(ToolsDirectory,false,true,false,directories);

	for (uint32 i=0;i<directories.size();++i)
	{
		NLMISC::CSString continentName= NLMISC::CFile::getFilename(NLMISC::CSString(directories[i]).rightCrop(1));
		nldebug("considering: '%s'",continentName.c_str());
		if (!continentName.empty() && continentName[0]!='.' && continentName!="common")
			Combo.InsertString (-1, continentName.c_str());
	}
	Combo.SetCurSel (0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#endif
