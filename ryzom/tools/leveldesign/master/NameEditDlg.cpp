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

// NewRegion.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "NameEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRegionPropertiesDlg dialog


CNameEditDlg::CNameEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNameEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRegionPropertiesDlg)
	Title = _T("");
	Comment = _T("");
	Name = _T("");
	//}}AFX_DATA_INIT
}

void CNameEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegionPropertiesDlg)
	DDX_Text(pDX, IDC_EDIT_NAME, Name);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNameEditDlg, CDialog)
	//{{AFX_MSG_MAP(CRegionPropertiesDlg)
	ON_WM_SHOWWINDOW ()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegionPropertiesDlg message handlers

BOOL CNameEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetWindowText (Title);
	CStatic *pS = (CStatic*)GetDlgItem (IDC_COMMENT);
	pS->SetWindowText (Comment);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNameEditDlg::OnShowWindow (BOOL bShow, UINT nStatus)
{
	CEdit *pEd = (CEdit*)GetDlgItem(IDC_EDIT_NAME);
	pEd->SetSel (0,-1);
	pEd->SetFocus ();
}
