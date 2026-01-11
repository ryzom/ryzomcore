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

// scene_rot_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "scene_rot_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSceneRotDlg dialog


CSceneRotDlg::CSceneRotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSceneRotDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSceneRotDlg)
	RotX = _T("");
	RotY = _T("");
	RotZ = _T("");
	//}}AFX_DATA_INIT
}


void CSceneRotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSceneRotDlg)
	DDX_Text(pDX, IDC_ROTX, RotX);
	DDX_Text(pDX, IDC_ROTY, RotY);
	DDX_Text(pDX, IDC_ROTZ, RotZ);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSceneRotDlg, CDialog)
	//{{AFX_MSG_MAP(CSceneRotDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneRotDlg message handlers
