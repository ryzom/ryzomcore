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
#include "choose_veget_set.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseVegetSet dialog


CChooseVegetSet::CChooseVegetSet(SelectionTerritoire* pParent, const std::string &oldFile)
	: CDialog((UINT)CChooseVegetSet::IDD, (CWnd*)pParent)
{
	//{{AFX_DATA_INIT(CChooseVegetSet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	FileName = oldFile;
	Parent = pParent;
}


void CChooseVegetSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseVegetSet)
	DDX_Control(pDX, IDC_BROWSE, Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseVegetSet, CDialog)
	//{{AFX_MSG_MAP(CChooseVegetSet)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDRESET, OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseVegetSet message handlers

void CChooseVegetSet::OnBrowse() 
{
	// Select a veget set
	static char BASED_CODE szFilter[] = "NeL VegetSet Files (*.vegetset)|*.vegetset|All Files (*.*)|*.*||";

	// Create a file dialog
 	CFileDialog dialog ( TRUE, "*.vegetset", "*.vegetset", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, (CWnd*)Parent);
	if (dialog.DoModal() == IDOK)
	{
		// Get the file name
		FileName = dialog.GetFileName ();
		Name.SetWindowText (FileName.c_str());
	}
}

BOOL CChooseVegetSet::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (FileName != "")
		Name.SetWindowText (FileName.c_str());
	else
		Name.SetWindowText ("Browse...");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseVegetSet::OnReset() 
{
	FileName = "";
	Name.SetWindowText ("Browse...");
}
