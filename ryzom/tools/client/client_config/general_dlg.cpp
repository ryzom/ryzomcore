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

// general_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "client_config.h"
#include "general_dlg.h"
#include "cfg_file.h"
#include "client_configdlg.h"

// ***************************************************************************
// CGeneralDlg dialog
// ***************************************************************************

CGeneralDlg::CGeneralDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CGeneralDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGeneralDlg)
	Sleep = FALSE;
	SaveConfig = FALSE;
	Language = -1;
	//}}AFX_DATA_INIT
}

// ***************************************************************************

void CGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneralDlg)
	DDX_Check(pDX, IDC_SLEEP, Sleep);
	DDX_Check(pDX, IDC_SAVE_CONFIG, SaveConfig);
	DDX_CBIndex(pDX, IDC_LANGUAGE, Language);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CGeneralDlg, CDialog)
	//{{AFX_MSG_MAP(CGeneralDlg)
	ON_BN_CLICKED(IDC_SLEEP, OnSleep)
	ON_BN_CLICKED(IDC_SAVE_CONFIG, OnSaveConfig)
	ON_CBN_SELENDOK(IDC_LANGUAGE, OnSelendokLanguage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CGeneralDlg message handlers
// ***************************************************************************

void CGeneralDlg::OnSleep() 
{
	UpdateData (TRUE);
	updateState ();
	UpdateData (FALSE);
	InvalidateConfig ();
}

// ***************************************************************************

void CGeneralDlg::updateState ()
{
}

// ***************************************************************************

void CGeneralDlg::OnSaveConfig() 
{
	InvalidateConfig ();
}

// ***************************************************************************

BOOL CGeneralDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	updateState ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CGeneralDlg::OnSelendokLanguage() 
{
	InvalidateConfig ();

	// Localize the windows
	CClient_configDlg *dlg = (CClient_configDlg*)theApp.m_pMainWnd;
	dlg->UpdateData (TRUE);
	dlg->changeLanguage (::GetIntForceLanguage()?"en" : (dlg->GeneralDlg.Language==2)?"de":(dlg->GeneralDlg.Language==1)?"fr":"en");
}

// ***************************************************************************
