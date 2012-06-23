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

// LoadDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "LoadDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CLoadDialog dialog


CLoadDialog::CLoadDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadDialog)
	Message = _T("");
	//}}AFX_DATA_INIT
}


void CLoadDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadDialog)
	DDX_Text(pDX, IDC_STATIC_MESSAGE, Message);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadDialog, CDialog)
	//{{AFX_MSG_MAP(CLoadDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadDialog message handlers
