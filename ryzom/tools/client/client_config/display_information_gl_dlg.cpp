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

// display_information_gl_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "client_config.h"
#include "display_information_gl_dlg.h"
#include "display_dlg.h"

// ***************************************************************************
// CDisplayInformationGlDlg dialog
// ***************************************************************************

CDisplayInformationGlDlg::CDisplayInformationGlDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDisplayInformationGlDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDisplayInformationGlDlg)
	//}}AFX_DATA_INIT
}

// ***************************************************************************

void CDisplayInformationGlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisplayInformationGlDlg)
	DDX_Control(pDX, IDC_GL_VERSION, VersionCtrl);
	DDX_Control(pDX, IDC_GL_VENDOR, VendorCtrl);
	DDX_Control(pDX, IDC_GL_RENDERER, RendererCtrl);
	DDX_Control(pDX, IDC_EXTENSION, Extensions);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CDisplayInformationGlDlg, CDialog)
	//{{AFX_MSG_MAP(CDisplayInformationGlDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CDisplayInformationGlDlg message handlers
// ***************************************************************************

BOOL CDisplayInformationGlDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	RendererCtrl.SetWindowText (GLRenderer.c_str ());
	VendorCtrl.SetWindowText (GLVendor.c_str ());
	VersionCtrl.SetWindowText (GLVersion.c_str ());
	uint i;
	for (i=0; i<GLExtensions.size (); i++)
		Extensions.InsertString (-1, GLExtensions[i].c_str ());
	
	UpdateData (TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************
