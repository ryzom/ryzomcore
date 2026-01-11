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

// edit_follow_path.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "edit_follow_path.h"
#include "popup_notify.h"
#include "nel/3d/ps_plane_basis_maker.h"



/////////////////////////////////////////////////////////////////////////////
// CEditFollowPath dialog


CEditFollowPath::CEditFollowPath(NL3D::CPSPlaneBasisFollowSpeed *pbfs, CParticleWorkspace::CNode *ownerNode, CWnd* pParent, IPopupNotify *pn)
	: CDialog(CEditFollowPath::IDD, pParent), _Node(ownerNode), _PN(pn)
{
	nlassert(pbfs);
	_FollowPath = pbfs;
	//{{AFX_DATA_INIT(CEditFollowPath)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CEditFollowPath::init(CWnd *pParent)
{
	CDialog::Create(CEditFollowPath::IDD, pParent);	
	ShowWindow(SW_SHOW);
}

void CEditFollowPath::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditFollowPath)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditFollowPath, CDialog)
	//{{AFX_MSG_MAP(CEditFollowPath)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_PROJECTION_MODE, OnSelchangeProjectionMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditFollowPath message handlers

BOOL CEditFollowPath::OnInitDialog() 
{
	CDialog::OnInitDialog();
	nlassert(_FollowPath);
	((CComboBox *) GetDlgItem(IDC_PROJECTION_MODE))->SetCurSel((int) _FollowPath->getProjectionPlane());
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEditFollowPath::OnClose() 
{	
	CDialog::OnClose();
	if (_PN) _PN->childPopupClosed(this);
}

void CEditFollowPath::OnSelchangeProjectionMode() 
{
	nlassert(_FollowPath);
	int index= ((CComboBox *) GetDlgItem(IDC_PROJECTION_MODE))->GetCurSel();
	_FollowPath->setProjectionPlane((NL3D::CPSPlaneBasisFollowSpeed::TProjectionPlane) index);	
	if (_Node) _Node->setModified(true);
}
