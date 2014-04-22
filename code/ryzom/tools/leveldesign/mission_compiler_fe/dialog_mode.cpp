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

// dialog_mode.cpp : implementation file
//

#include "stdafx.h"
#include "mission_compiler_fe.h"
#include "dialog_mode.h"

/////////////////////////////////////////////////////////////////////////////
// CDialogMode dialog


CDialogMode::CDialogMode(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogMode::IDD, pParent)
{
	Mode = mode_compile;
	//{{AFX_DATA_INIT(CDialogMode)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDialogMode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogMode)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogMode, CDialog)
	//{{AFX_MSG_MAP(CDialogMode)
	ON_BN_CLICKED(ID_MODE_COMPILE, OnModeCompile)
	ON_BN_CLICKED(ID_MODE_PUBLISH, OnModePublish)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogMode message handlers

void CDialogMode::OnModeCompile() 
{
	Mode = mode_compile;
	
	this->EndDialog(IDOK);
}

void CDialogMode::OnModePublish() 
{
	Mode = mode_publish;	

	this->EndDialog(IDOK);
}
