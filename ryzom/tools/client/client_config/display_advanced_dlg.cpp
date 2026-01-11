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

// display_advanced_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "client_config.h"
#include "display_advanced_dlg.h"
#include "cfg_file.h"

// ***************************************************************************
// CDisplayAdvancedDlg dialog
// ***************************************************************************

CDisplayAdvancedDlg::CDisplayAdvancedDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDisplayAdvancedDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDisplayAdvancedDlg)
	DisableAGPVertices = FALSE;
	DisableTextureShaders = FALSE;
	DisableVertexProgram = FALSE;
	DisableDXTC = FALSE;
	//}}AFX_DATA_INIT
}

// ***************************************************************************

void CDisplayAdvancedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisplayAdvancedDlg)
	DDX_Check(pDX, IDC_DISABLE_AGP_VERTICES, DisableAGPVertices);
	DDX_Check(pDX, IDC_DISABLE_TEXTURE_SHADERS, DisableTextureShaders);
	DDX_Check(pDX, IDC_DISABLE_VERTEX_PROGRAM, DisableVertexProgram);
	DDX_Check(pDX, IDC_FORCE_DXTC, DisableDXTC);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CDisplayAdvancedDlg, CDialog)
	//{{AFX_MSG_MAP(CDisplayAdvancedDlg)
	ON_BN_CLICKED(IDC_DISABLE_AGP_VERTICES, OnDisableAgpVertices)
	ON_BN_CLICKED(IDC_DISABLE_TEXTURE_SHADERS, OnDisableTextureShaders)
	ON_BN_CLICKED(IDC_DISABLE_VERTEX_PROGRAM, OnDisableVertexProgram)
	ON_BN_CLICKED(IDC_FORCE_DXTC, OnForceDxtc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CDisplayAdvancedDlg message handlers
// ***************************************************************************

void CDisplayAdvancedDlg::OnDisableAgpVertices() 
{
	InvalidateConfig ();	
}

// ***************************************************************************

void CDisplayAdvancedDlg::OnDisableTextureShaders() 
{
	InvalidateConfig ();	
}

// ***************************************************************************

void CDisplayAdvancedDlg::OnDisableVertexProgram() 
{
	InvalidateConfig ();	
}

// ***************************************************************************

void CDisplayAdvancedDlg::OnForceDxtc() 
{
	InvalidateConfig ();	
}

// ***************************************************************************

