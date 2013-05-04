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

// DialogLogin.cpp : implementation file
//

#include "stdafx.h"
#include "DialogLogin.h"


/////////////////////////////////////////////////////////////////////////////
// CDialogLogin dialog


CDialogLogin::CDialogLogin(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogLogin::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogLogin)
	m_Login = _T("");
	m_Password = _T("");	
	//}}AFX_DATA_INIT
}


void CDialogLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogLogin)
	DDX_Text(pDX, IDC_LOGIN, m_Login);
	DDX_Text(pDX, IDC_PASSWORD, m_Password);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogLogin, CDialog)
	//{{AFX_MSG_MAP(CDialogLogin)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogLogin message handlers
