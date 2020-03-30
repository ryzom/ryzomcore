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

// display_information_d3d_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "client_config.h"
#include "display_information_d3d_dlg.h"
#include "display_dlg.h"

// ***************************************************************************
// CDisplayInformationD3DDlg dialog
// ***************************************************************************

CDisplayInformationD3DDlg::CDisplayInformationD3DDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDisplayInformationD3DDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDisplayInformationD3DDlg)
	//}}AFX_DATA_INIT
}

// ***************************************************************************

void CDisplayInformationD3DDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisplayInformationD3DDlg)
	DDX_Control(pDX, IDC_D3D_DRIVER_VERSION, DriverVersion);
	DDX_Control(pDX, IDC_D3D_DRIVER, Driver);
	DDX_Control(pDX, IDC_D3D_DESCRIPTION, Description);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CDisplayInformationD3DDlg, CDialog)
	//{{AFX_MSG_MAP(CDisplayInformationD3DDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CDisplayInformationD3DDlg message handlers
// ***************************************************************************

BOOL CDisplayInformationD3DDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	Description.SetWindowText (D3DDescription.c_str ());
	Driver.SetWindowText (D3DDriver.c_str ());
	DriverVersion.SetWindowText (D3DDriverVersion.c_str ());
	
	UpdateData (TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************
