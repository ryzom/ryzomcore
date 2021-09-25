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

#include "stdafx.h"
#include "tile_edit_exe.h"
#include "select_rotation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SelectRotation dialog


SelectRotation::SelectRotation(CWnd* pParent /*=NULL*/)
	: CDialog(SelectRotation::IDD, pParent)
{
	//{{AFX_DATA_INIT(SelectRotation)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SelectRotation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SelectRotation)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SelectRotation, CDialog)
	//{{AFX_MSG_MAP(SelectRotation)
	ON_BN_CLICKED(ID_ROT0, OnRot0)
	ON_BN_CLICKED(ID_ROT1, OnRot1)
	ON_BN_CLICKED(ID_ROT2, OnRot2)
	ON_BN_CLICKED(ID_ROT3, OnRot3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SelectRotation message handlers

void SelectRotation::OnRot0() 
{
	// TODO: Add your control notification handler code here
	RotSelected=0;
	OnOK ();
}

void SelectRotation::OnRot1() 
{
	// TODO: Add your control notification handler code here
	RotSelected=1;
	OnOK ();
}

void SelectRotation::OnRot2() 
{
	// TODO: Add your control notification handler code here
	RotSelected=2;
	OnOK ();
}

void SelectRotation::OnRot3() 
{
	// TODO: Add your control notification handler code here
	RotSelected=3;
	OnOK ();
}
