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

// GotoLine.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "GotoLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGotoLine dialog


CGotoLine::CGotoLine(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoLine::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGotoLine)
	m_Line = 0;
	//}}AFX_DATA_INIT
}


void CGotoLine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGotoLine)
	DDX_Control(pDX, IDC_LINE, m_LineCtrl);
	DDX_Text(pDX, IDC_LINE, m_Line);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGotoLine, CDialog)
	//{{AFX_MSG_MAP(CGotoLine)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGotoLine message handlers

void CGotoLine::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
		m_LineCtrl.SetSel(0, -1);
		m_LineCtrl.SetFocus();
	}
}
