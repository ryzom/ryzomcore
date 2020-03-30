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

// TypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "type_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CTypeDlg dialog


CTypeDlg::CTypeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTypeDlg)
	EditName = _T("");
	ButtonColor.setColor(CRGBA(255,255,255,255));
	//}}AFX_DATA_INIT
}


void CTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTypeDlg)
	DDX_Control(pDX, IDC_BUTTONCOLOR, ButtonColor);
	DDX_Text(pDX, IDC_EDITNAME, EditName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CTypeDlg)
	ON_BN_CLICKED(IDC_BUTTONCOLOR, OnButtoncolor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTypeDlg message handlers

void CTypeDlg::OnButtoncolor() 
{
	// TODO: Add your control notification handler code here
	CColorDialog coldlg;
	if (coldlg.DoModal() == IDOK)
	{
		int r = GetRValue(coldlg.GetColor());
		int g = GetGValue(coldlg.GetColor());
		int b = GetBValue(coldlg.GetColor());
		ButtonColor.setColor(CRGBA(r,g,b,255));
		Invalidate();
	}
}

BOOL CTypeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	ButtonColor.setColor (ButtonColorValue);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTypeDlg::OnOK() 
{
	// TODO: Add extra validation here
	ButtonColorValue = ButtonColor.getColor();
	CDialog::OnOK();
}
