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

// login_window.cpp : implementation file
//

#ifdef _WINDOWS

#include "stdafx.h"
#include "resource.h"
#include "login_window.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoginWindow dialog


CLoginWindow::CLoginWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoginWindow)
	Shard = _T("");
	Login = _T("");
	Password = _T("");
	//}}AFX_DATA_INIT
}


void CLoginWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginWindow)
	DDX_Text(pDX, IDC_EDIT1, Shard);
	DDX_Text(pDX, IDC_EDIT2, Login);
	DDX_Text(pDX, IDC_EDIT3, Password);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoginWindow, CDialog)
	//{{AFX_MSG_MAP(CLoginWindow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoginWindow message handlers

BOOL CLoginWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#endif
