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

// AddPathDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mission_compiler_fe.h"
#include "AddPathDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CAddPathDlg dialog


CAddPathDlg::CAddPathDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddPathDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddPathDlg)
	m_addPathLog = _T("");
	//}}AFX_DATA_INIT
}


void CAddPathDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddPathDlg)
	DDX_Text(pDX, IDC_ADD_PATH, m_addPathLog);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddPathDlg, CDialog)
	//{{AFX_MSG_MAP(CAddPathDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddPathDlg message handlers
