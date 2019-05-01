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

#include "std_afx.h"
#include "object_viewer.h"
#include "select_string.h"


/////////////////////////////////////////////////////////////////////////////
// CSelectString dialog


CSelectString::CSelectString(const std::vector<std::string>& vectString, const std::string &title, CWnd* pParent, bool empty)
	: CDialog(CSelectString::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectString)
	//}}AFX_DATA_INIT
	Title=title;
	Strings=vectString;
	Selection=-1;
	Empty=empty;
}


void CSelectString::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectString)
	DDX_Control(pDX, IDEMPTY, EmptyCtrl);
	DDX_Control(pDX, IDC_LIST, ListCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectString, CDialog)
	//{{AFX_MSG_MAP(CSelectString)
	ON_LBN_DBLCLK(IDC_LIST, OnDblclkList)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	ON_BN_CLICKED(IDEMPTY, OnEmpty)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectString message handlers

void CSelectString::OnOK() 
{
	// TODO: Add extra validation here
	if (Selection!=-1)
		CDialog::OnOK();
	else
		CDialog::OnCancel();
}

void CSelectString::OnDblclkList() 
{
	// TODO: Add your control notification handler code here
	OnSelchangeList();
	OnOK();
}

BOOL CSelectString::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Change title
	SetWindowText(nlUtf8ToTStr(Title));

	// Empty button ?
	EmptyCtrl.ShowWindow (Empty?SW_SHOW:SW_HIDE);

	// Add string
	for (uint s=0; s<Strings.size(); s++)
		ListCtrl.InsertString(-1, nlUtf8ToTStr(Strings[s]));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectString::OnSelchangeList() 
{
	// Get selection
	UpdateData ();
	Selection=ListCtrl.GetCurSel ();
	if (Selection==LB_ERR)
		Selection=-1;
}

void CSelectString::OnEmpty() 
{
	// TODO: Add your control notification handler code here
	Selection=-1;
	CDialog::OnOK();
}
