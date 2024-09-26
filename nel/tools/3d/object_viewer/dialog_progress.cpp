// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// dialog_progress.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "dialog_progress.h"

/////////////////////////////////////////////////////////////////////////////
// CDialogProgress dialog


CDialogProgress::CDialogProgress(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogProgress::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogProgress)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDialogProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogProgress)
	DDX_Control(pDX, IDC_STATIC_PROGRESS_DESC, ProgressText);
	DDX_Control(pDX, IDC_PROGRESS1, ProgressBar);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogProgress, CDialog)
	//{{AFX_MSG_MAP(CDialogProgress)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogProgress message handlers
