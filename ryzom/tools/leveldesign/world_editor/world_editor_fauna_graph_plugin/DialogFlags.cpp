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

// DialogFlags.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor_fauna_graph_plugin.h"
#include "DialogFlags.h"
#include "plugin.h"


/////////////////////////////////////////////////////////////////////////////
// CDialogFlags dialog


CDialogFlags::CDialogFlags(CPlugin *plugin)
	: DisplayCondition(DisplayAll), _Plugin(plugin)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//{{AFX_DATA_INIT(CDialogFlags)
	m_DisplayFlags = TRUE;
	m_DisplayIndices = TRUE;
	m_DisplayTargetIndices = FALSE;		
	//}}AFX_DATA_INIT
}


void CDialogFlags::DoDataExchange(CDataExchange* pDX)
{	
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogFlags)
	DDX_Check(pDX, IDC_DISPLAY_FLAGS, m_DisplayFlags);
	DDX_Check(pDX, IDC_DISPLAY_INDICES, m_DisplayIndices);
	DDX_Check(pDX, IDC_DISPLAY_TARGET_INDICES, m_DisplayTargetIndices);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogFlags, CDialog)
	//{{AFX_MSG_MAP(CDialogFlags)
	ON_CBN_SELCHANGE(IDC_DISPLAY_CONDITION, OnSelchangeDisplayCondition)
	ON_BN_CLICKED(IDC_DISPLAY_FLAGS, OnDisplayFlags)
	ON_BN_CLICKED(IDC_DISPLAY_INDICES, OnDisplayIndices)
	ON_BN_CLICKED(IDC_DISPLAY_TARGET_INDICES, OnDisplayTargetIndices)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogFlags message handlers

void CDialogFlags::OnSelchangeDisplayCondition() 
{	
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());	
	UpdateData(TRUE);	
	CComboBox *lb = (CComboBox *) GetDlgItem(IDC_DISPLAY_CONDITION);
	DisplayCondition = (TDisplayCondition) lb->GetCurSel();
	BOOL enabled = DisplayCondition == DisplayOff ? FALSE : TRUE;
	GetDlgItem(IDC_DISPLAY_FLAGS)->EnableWindow(enabled);
	GetDlgItem(IDC_DISPLAY_INDICES)->EnableWindow(enabled);
	GetDlgItem(IDC_DISPLAY_INDICES)->EnableWindow(enabled);
	GetDlgItem(IDC_DISPLAY_TARGET_INDICES)->EnableWindow(enabled);
	_Plugin->getPluginAccess()->invalidateLeftView();
}

void CDialogFlags::OnDisplayFlags() 
{	
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());	
	UpdateData(TRUE);	
	_Plugin->getPluginAccess()->invalidateLeftView();
}

void CDialogFlags::OnDisplayIndices() 
{	
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());	
	UpdateData(TRUE);
	_Plugin->getPluginAccess()->invalidateLeftView();
}


void CDialogFlags::OnDisplayTargetIndices() 
{	
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());	
	UpdateData(TRUE);
	_Plugin->getPluginAccess()->invalidateLeftView();
}

BOOL CDialogFlags::OnInitDialog() 
{	
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDialog::OnInitDialog();		
	//
	CComboBox *lb = (CComboBox *) GetDlgItem(IDC_DISPLAY_CONDITION);
	CString str;	
	lb->SetCurSel(0);
	//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CDialogFlags::OnClose() 
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_Plugin->closePlugin();	
	CDialog::OnClose();
}
