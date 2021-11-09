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

// set_value_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "set_value_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSetValueDlg dialog


CSetValueDlg::CSetValueDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetValueDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetValueDlg)
	Value = _T("");
	//}}AFX_DATA_INIT
}


void CSetValueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetValueDlg)
	DDX_Text(pDX, IDC_VALUE, Value);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetValueDlg, CDialog)
	//{{AFX_MSG_MAP(CSetValueDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetValueDlg message handlers

BOOL CSetValueDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetWindowText (Title);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
