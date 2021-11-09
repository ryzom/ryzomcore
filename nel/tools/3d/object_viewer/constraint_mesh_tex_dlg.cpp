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


#include "std_afx.h"
#include "object_viewer.h"
#include "constraint_mesh_tex_dlg.h"
#include "constraint_mesh_global_tex_anim_dlg.h"
#include "nel/3d/ps_mesh.h"



CConstraintMeshTexDlg::CConstraintMeshTexDlg(NL3D::CPSConstraintMesh *cm, CWnd* pParent /*=NULL*/)
	: CDialog(CConstraintMeshTexDlg::IDD, pParent), _CurrDlg(NULL), _CM(cm)
{
	nlassert(_CM);
	//{{AFX_DATA_INIT(CConstraintMeshTexDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CConstraintMeshTexDlg::init(uint x, uint y, CWnd *pParent)
{
	Create(IDD_CONSTRAINT_MESH_TEX_DLG, pParent);
	RECT r;
	GetClientRect(&r);		
	MoveWindow(x, y, r.right, r.bottom);
	ShowWindow(SW_SHOW);
}

static inline void RemoveDlg(CDialog *&dlg) 
{
	if (dlg) dlg->DestroyWindow();
	delete dlg;
	dlg = NULL;
}


CConstraintMeshTexDlg::~CConstraintMeshTexDlg()
{
	RemoveDlg(_CurrDlg);
}


void CConstraintMeshTexDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConstraintMeshTexDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


void	CConstraintMeshTexDlg::createGlobalAnimationDlg(uint stage)
{
	RemoveDlg(_CurrDlg);
	CConstraintMeshGlobalTexAnimDlg *gDlg 
		= new CConstraintMeshGlobalTexAnimDlg(_CM, stage, this);
	RECT r;
	GetDlgItem(IDC_CURR_STAGE)->GetWindowRect(&r);
	ScreenToClient(&r);	
	gDlg->init(r.left, r.top, this);
	_CurrDlg = gDlg;
}


BEGIN_MESSAGE_MAP(CConstraintMeshTexDlg, CDialog)
	//{{AFX_MSG_MAP(CConstraintMeshTexDlg)
	ON_CBN_SELCHANGE(IDC_CURRENT_STAGE, OnSelchangeCurrentStage)
	ON_CBN_SELCHANGE(IDC_TEX_ANIM_TYPE, OnSelchangeTexAnimType)
	ON_BN_CLICKED(IDC_REINIT_WHEN_NEW_ELEMENT_IS_CREATED, OnReinitWhenNewElementIsCreated)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConstraintMeshTexDlg message handlers

BOOL CConstraintMeshTexDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	setupDlg();		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void   CConstraintMeshTexDlg::setupDlg()
{
	if (_CM->getTexAnimType() == NL3D::CPSConstraintMesh::GlobalAnim)
	{
		((CComboBox *) GetDlgItem(IDC_TEX_ANIM_TYPE))->SetCurSel(1);
		((CComboBox *) GetDlgItem(IDC_CURRENT_STAGE))->SetCurSel(0);
		createGlobalAnimationDlg(0);
		((CComboBox *) GetDlgItem(IDC_CURRENT_STAGE))->EnableWindow(TRUE);
		GetDlgItem(IDC_REINIT_WHEN_NEW_ELEMENT_IS_CREATED)->ShowWindow(SW_SHOW);
		((CButton *) GetDlgItem(IDC_REINIT_WHEN_NEW_ELEMENT_IS_CREATED))->SetCheck(_CM->isGlobalAnimTimeResetOnNewElementForced() ?
																					1 : 0);
	}
	else
	{
		((CComboBox *) GetDlgItem(IDC_TEX_ANIM_TYPE))->SetCurSel(0);
		((CComboBox *) GetDlgItem(IDC_CURRENT_STAGE))->SetCurSel(-1);
		RemoveDlg(_CurrDlg);
		((CComboBox *) GetDlgItem(IDC_CURRENT_STAGE))->EnableWindow(FALSE);
		GetDlgItem(IDC_REINIT_WHEN_NEW_ELEMENT_IS_CREATED)->ShowWindow(SW_HIDE);
	}
}

void CConstraintMeshTexDlg::OnSelchangeCurrentStage() 
{
	UpdateData();
	RemoveDlg(_CurrDlg);
	createGlobalAnimationDlg((uint) ((CComboBox *) GetDlgItem(IDC_CURRENT_STAGE))->GetCurSel());	
}

void CConstraintMeshTexDlg::OnSelchangeTexAnimType() 
{
	UpdateData();
	int selItem = ((CComboBox *) GetDlgItem(IDC_TEX_ANIM_TYPE))->GetCurSel();
	switch(selItem)
	{
		case 0: // no anim
			_CM->setTexAnimType(NL3D::CPSConstraintMesh::NoAnim);
		break;
		case 1: // global anim
			_CM->setTexAnimType(NL3D::CPSConstraintMesh::GlobalAnim);
		break;
		default:
			nlstop;
		break;
	}

	setupDlg();
}

void CConstraintMeshTexDlg::OnReinitWhenNewElementIsCreated() 
{
	_CM->forceGlobalAnimTimeResetOnNewElement(((CButton *) GetDlgItem(IDC_REINIT_WHEN_NEW_ELEMENT_IS_CREATED))->GetCheck() ? true : false);
}
