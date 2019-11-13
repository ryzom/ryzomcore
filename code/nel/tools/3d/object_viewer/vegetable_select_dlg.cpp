// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// vegetable_select_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_select_dlg.h"
#include "vegetable_dlg.h"


/////////////////////////////////////////////////////////////////////////////
// CVegetableSelectDlg dialog


CVegetableSelectDlg::CVegetableSelectDlg(CVegetableDlg *vegetableDlg, CWnd* pParent /*=NULL*/)
	: CDialog(CVegetableSelectDlg::IDD, pParent), _VegetableDlg(vegetableDlg)
{
	//{{AFX_DATA_INIT(CVegetableSelectDlg)
	VegetableSelected = -1;
	//}}AFX_DATA_INIT
}


void CVegetableSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetableSelectDlg)
	DDX_Control(pDX, IDC_LIST1, VegetableList);
	DDX_LBIndex(pDX, IDC_LIST1, VegetableSelected);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVegetableSelectDlg, CDialog)
	//{{AFX_MSG_MAP(CVegetableSelectDlg)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVegetableSelectDlg message handlers

BOOL CVegetableSelectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Init the control list.
	uint	num= _VegetableDlg->getNumVegetables();
	for(uint i=0; i<num; i++)
	{
		VegetableList.AddString(nlUtf8ToTStr(_VegetableDlg->getVegetableName(i)));
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVegetableSelectDlg::OnDblclkList1() 
{
	UpdateData();
	// DblClck select the name.
	EndDialog(IDOK);
}
