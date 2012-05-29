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

// MoveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "resource.h"
#include "move_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CMoveDlg dialog


CMoveDlg::CMoveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMoveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMoveDlg)
	XOffset = 0;
	YOffset = 0;
	//}}AFX_DATA_INIT
}


void CMoveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMoveDlg)
	DDX_Text(pDX, IDC_EDITXOFFSET, XOffset);
	DDX_Text(pDX, IDC_EDITYOFFSET, YOffset);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMoveDlg, CDialog)
	//{{AFX_MSG_MAP(CMoveDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMoveDlg message handlers
