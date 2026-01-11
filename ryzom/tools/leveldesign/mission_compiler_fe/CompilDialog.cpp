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

// CompilDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mission_compiler_fe.h"
#include "CompilDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CCompilDialog dialog


CCompilDialog::CCompilDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCompilDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCompilDialog)
	m_compileLog = _T("");
	//}}AFX_DATA_INIT
}


void CCompilDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCompilDialog)
	DDX_Control(pDX, IDC_COMPIL_LOG, m_compileLogCtrl);
	DDX_Control(pDX, IDOK, m_okBtn);
	DDX_Text(pDX, IDC_COMPIL_LOG, m_compileLog);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCompilDialog, CDialog)
	//{{AFX_MSG_MAP(CCompilDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompilDialog message handlers

void CCompilDialog::OnOK() 
{
	CDialog::OnOK();
}
