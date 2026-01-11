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

// EmailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "bug_report.h"
#include "EmailDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmailDlg dialog


CEmailDlg::CEmailDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEmailDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmailDlg)
	m_email = _T("");
	//}}AFX_DATA_INIT
}


void CEmailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmailDlg)
	DDX_Text(pDX, IDC_EMAIL, m_email);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmailDlg, CDialog)
	//{{AFX_MSG_MAP(CEmailDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmailDlg message handlers

BOOL CEmailDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWnd *wnd = GetDlgItem (IDC_EMAIL);
	if (wnd != NULL)
		wnd->SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
