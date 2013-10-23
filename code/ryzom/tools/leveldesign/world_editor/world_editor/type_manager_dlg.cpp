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

// TypeManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "type_manager_dlg.h"

#include "type_dlg.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CTypeManagerDlg dialog

// ---------------------------------------------------------------------------
CTypeManagerDlg::CTypeManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTypeManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTypeManagerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ---------------------------------------------------------------------------
void CTypeManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTypeManagerDlg)
	DDX_Control(pDX, IDC_LISTTYPE, ListType);
	//}}AFX_DATA_MAP
}

// ---------------------------------------------------------------------------
void CTypeManagerDlg::set (const vector<SType> &types)
{
	LocalTypes = types;
}

// ---------------------------------------------------------------------------
const vector<SType> CTypeManagerDlg::get ()
{
	return LocalTypes;
}

// ---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CTypeManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CTypeManagerDlg)
	ON_BN_CLICKED(IDC_ADDTYPE, OnAddtype)
	ON_BN_CLICKED(IDC_EDITTYPE, OnEdittype)
	ON_BN_CLICKED(IDC_REMOVETYPE, OnRemovetype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTypeManagerDlg message handlers

void CTypeManagerDlg::OnAddtype() 
{
	// TODO: Add your control notification handler code here
	CTypeDlg typeDlg(this);
	if (typeDlg.DoModal() == IDOK)
	{
		SType typeTmp;
		typeTmp.Name = (LPCSTR)typeDlg.EditName;
		typeTmp.Color = typeDlg.ButtonColorValue;
		// Check if the name of the new type is the same as an existing one
		bool bFound = false;
		for (uint32 i = 0; i < LocalTypes.size(); ++i)
		if (LocalTypes[i].Name == typeTmp.Name)
		{
			bFound = true;
			break;
		}
		// If not Add it to the 2 lists (win and internal)
		if (!bFound)
		{
			LocalTypes.push_back (typeTmp);
			ListType.InsertString(-1, typeTmp.Name.c_str());
		}
	}
}

void CTypeManagerDlg::OnEdittype() 
{
	// TODO: Add your control notification handler code here
	CTypeDlg typeDlg(this);

	int cursel = ListType.GetCurSel();
	if (cursel == -1)
		return;

	typeDlg.EditName = LocalTypes[cursel].Name.c_str();
	typeDlg.ButtonColorValue = LocalTypes[cursel].Color;

	if (typeDlg.DoModal() == IDOK)
	{
		SType typeTmp;
		typeTmp.Name = (LPCSTR)typeDlg.EditName;
		typeTmp.Color = typeDlg.ButtonColorValue;
		LocalTypes[cursel] = typeTmp;
		ListType.DeleteString (ListType.GetCurSel());
		ListType.InsertString (cursel, typeTmp.Name.c_str());
	}
}

void CTypeManagerDlg::OnRemovetype() 
{
	// TODO: Add your control notification handler code here
	int cursel = ListType.GetCurSel();
	if (cursel == -1)
		return;
	for (uint32 i = cursel; i < (LocalTypes.size()-1); ++i)
		LocalTypes[i] = LocalTypes[i+1];
	LocalTypes.resize(LocalTypes.size()-1);
	ListType.DeleteString (ListType.GetCurSel());
}

BOOL CTypeManagerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	for (uint32 i = 0; i < LocalTypes.size(); ++i)
	{
		ListType.InsertString(-1, LocalTypes[i].Name.c_str());
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
