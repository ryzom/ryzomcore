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

// TypeSelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "type_sel_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CTypeSelDlg dialog


CTypeSelDlg::CTypeSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTypeSelDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTypeSelDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTypeSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTypeSelDlg)
	DDX_Control(pDX, IDC_LIST, TypeList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTypeSelDlg, CDialog)
	//{{AFX_MSG_MAP(CTypeSelDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTypeSelDlg message handlers

void CTypeSelDlg::OnOK() 
{
	// TODO: Add extra validation here

	if (TypeList.GetCurSel() == -1)
		return;

	CString sTmp;
	TypeList.GetText(TypeList.GetCurSel(), sTmp);
	_TypeSelected = (LPCSTR)sTmp;

	CDialog::OnOK();
}

BOOL CTypeSelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	for (uint32 i = 0; i < _TypesInit->size(); ++i)
	{
		TypeList.InsertString(-1, _TypesInit->operator[](i).Name.c_str());
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
