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

// LoadingDialog.cpp : implementation file
//

#include "std_sound_plugin.h"
#include "georges_plugin_sound.h"
#include "loading_dialog.h"


/////////////////////////////////////////////////////////////////////////////
// CLoadingDialog dialog


CLoadingDialog::CLoadingDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadingDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadingDialog)
	Message = _T("");
	//}}AFX_DATA_INIT
}


void CLoadingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadingDialog)
	DDX_Text(pDX, IDC_STATIC_MESSAGE, Message);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadingDialog, CDialog)
	//{{AFX_MSG_MAP(CLoadingDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadingDialog message handlers
