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

// Generate.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "resource.h"
#include "generate_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CGenerateDlg dialog


CGenerateDlg::CGenerateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGenerateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGenerateDlg)
	MinX = 0;
	MinY = 0;
	MaxY = 0;
	MaxX = 0;
	ZoneBaseX = 0;
	ZoneBaseY = 0;
	ComboMaterialString = _T("");
	//}}AFX_DATA_INIT
}


void CGenerateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGenerateDlg)
	DDX_Control(pDX, IDC_COMBOMATERIAL, ComboMaterial);
	DDX_Text(pDX, IDC_EDITMINX, MinX);
	DDX_Text(pDX, IDC_EDITMINY, MinY);
	DDX_Text(pDX, IDC_EDITMAXY, MaxY);
	DDX_Text(pDX, IDC_EDITMAXX, MaxX);
	DDX_Text(pDX, IDC_EDITZONEBASEX, ZoneBaseX);
	DDX_Text(pDX, IDC_EDITZONEBASEY, ZoneBaseY);
	DDX_CBString(pDX, IDC_COMBOMATERIAL, ComboMaterialString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGenerateDlg, CDialog)
	//{{AFX_MSG_MAP(CGenerateDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenerateDlg message handlers

BOOL CGenerateDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	for (uint32 i = 0; i < AllMaterials.size(); ++i)
		ComboMaterial.InsertString(-1, AllMaterials[i].c_str());

	ComboMaterial.SetCurSel (0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGenerateDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData (TRUE);

	if (MinX > MaxX)
	{
		MessageBox ("MinX > MaxX", "Error", MB_OK|MB_ICONSTOP);
		return;
	}
	if (MinY > MaxY)
	{
		MessageBox ("MinY > MaxY", "Error", MB_OK|MB_ICONSTOP);
		return;
	}
	CDialog::OnOK();
}
