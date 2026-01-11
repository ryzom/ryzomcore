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

// GotoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "goto_dialog.h"
#include "display.h"

using namespace NLMISC;

/////////////////////////////////////////////////////////////////////////////
// CGotoDialog dialog


CGotoDialog::CGotoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGotoDialog)
	m_gotoX = 0.0f;
	m_gotoY = 0.0f;
	m_ZoomAtPosition = FALSE;
	//}}AFX_DATA_INIT
}


void CGotoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGotoDialog)
	DDX_Control(pDX, IDC_GOTO_POS_X, m_posXCtrl);
	DDX_Text(pDX, IDC_GOTO_POS_X, m_gotoX);
	DDX_Text(pDX, IDC_GOTO_POS_Y, m_gotoY);
	DDX_Check(pDX, IDC_CHECK_POS_ZOMM, m_ZoomAtPosition);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGotoDialog, CDialog)
	//{{AFX_MSG_MAP(CGotoDialog)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGotoDialog message handlers

void CGotoDialog::OnOK() 
{
	// center the 3D view on the coord
	
	UpdateData();
	CVector	center(m_gotoX, m_gotoY, 0.0f);

	getDisplay()->setDisplayCenter(center, m_ZoomAtPosition == TRUE);
	
	CDialog::OnOK();
}

void CGotoDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		m_posXCtrl.SetFocus();
		m_posXCtrl.SetSel(0, -1);
	}
}
