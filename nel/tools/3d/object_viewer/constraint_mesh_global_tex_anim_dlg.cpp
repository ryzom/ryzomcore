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
#include "constraint_mesh_global_tex_anim_dlg.h"
#include "nel/3d/ps_mesh.h"

/////////////////////////////////////////////////////////////////////////////
// CConstraintMeshGlobalTexAnimDlg dialog


CConstraintMeshGlobalTexAnimDlg::CConstraintMeshGlobalTexAnimDlg(NL3D::CPSConstraintMesh *cm, uint stage, CWnd* pParent /*= NULL*/)
	: _CM(cm), _Stage(stage), CDialog(CConstraintMeshGlobalTexAnimDlg::IDD, pParent)
{
	nlassert(_CM);
	//{{AFX_DATA_INIT(CConstraintMeshGlobalTexAnimDlg)
	//}}AFX_DATA_INIT
}


void CConstraintMeshGlobalTexAnimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConstraintMeshGlobalTexAnimDlg)
	DDX_Control(pDX, IDC_V_START, m_VStartCtrl);
	DDX_Control(pDX, IDC_U_START, m_UStartCtrl);
	DDX_Control(pDX, IDC_U_SCALE_START, m_UScaleStartCtrl);
	DDX_Control(pDX, IDC_WROT_SPEED, m_WRotSpeedCtrl);
	DDX_Control(pDX, IDC_WROT_ACCEL, m_WRotAccelCtrl);
	DDX_Control(pDX, IDC_V_SPEED, m_VSpeedCtrl);
	DDX_Control(pDX, IDC_V_SCALE_START, m_VScaleStartCtrl);
	DDX_Control(pDX, IDC_V_SCALE_SPEED, m_VScaleSpeedCtrl);
	DDX_Control(pDX, IDC_V_SCALE_ACCEL, m_VScaleAccelCtrl);
	DDX_Control(pDX, IDC_U_SCALE_SPEED, m_UScaleSpeedCtrl);
	DDX_Control(pDX, IDC_V_ACCEL, m_VAccelCtrl);
	DDX_Control(pDX, IDC_U_SCALE_ACCEL, m_UScaleAccel);
	DDX_Control(pDX, IDC_U_ACCEL, m_UAccelCtrl);
	DDX_Control(pDX, IDC_U_SPEED, m_USpeedCtrl);
	//}}AFX_DATA_MAP
}


void CConstraintMeshGlobalTexAnimDlg::init(uint x, uint y, CWnd *pParent)
{
	Create(IDD_CONSTRAINT_MESH_GLOBAL_TEX_ANIM_DLG, pParent);
	RECT r;
	GetClientRect(&r);		
	MoveWindow(x, y, r.right, r.bottom);
	ShowWindow(SW_SHOW);
}



BEGIN_MESSAGE_MAP(CConstraintMeshGlobalTexAnimDlg, CDialog)
	//{{AFX_MSG_MAP(CConstraintMeshGlobalTexAnimDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConstraintMeshGlobalTexAnimDlg message handlers

BOOL CConstraintMeshGlobalTexAnimDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_WRotSpeedCtrl.setType(CEditEx::FloatType);
	m_WRotAccelCtrl.setType(CEditEx::FloatType);
	m_VSpeedCtrl.setType(CEditEx::FloatType);
	m_VStartCtrl.setType(CEditEx::FloatType);
	m_VScaleStartCtrl.setType(CEditEx::FloatType);
	m_VScaleSpeedCtrl.setType(CEditEx::FloatType);
	m_VScaleAccelCtrl.setType(CEditEx::FloatType);
	m_UScaleSpeedCtrl.setType(CEditEx::FloatType);
	m_VAccelCtrl.setType(CEditEx::FloatType);
	m_UScaleAccel.setType(CEditEx::FloatType);
	m_UAccelCtrl.setType(CEditEx::FloatType);
	m_USpeedCtrl.setType(CEditEx::FloatType);
	m_UStartCtrl.setType(CEditEx::FloatType);
	m_UScaleStartCtrl.setType(CEditEx::FloatType);

	m_WRotSpeedCtrl.setListener(this);
	m_WRotAccelCtrl.setListener(this);
	m_VSpeedCtrl.setListener(this);
	m_VStartCtrl.setListener(this);
	m_VScaleStartCtrl.setListener(this);
	m_VScaleSpeedCtrl.setListener(this);
	m_VScaleAccelCtrl.setListener(this);
	m_UScaleSpeedCtrl.setListener(this);
	m_VAccelCtrl.setListener(this);
	m_UScaleAccel.setListener(this);
	m_UAccelCtrl.setListener(this);
	m_USpeedCtrl.setListener(this);
	m_UStartCtrl.setListener(this);
	m_UScaleStartCtrl.setListener(this);

	const NL3D::CPSConstraintMesh::CGlobalTexAnim &gta = _CM->getGlobalTexAnim(_Stage);

	m_WRotSpeedCtrl.setFloat(gta.WRotSpeed);
	m_WRotAccelCtrl.setFloat(gta.WRotAccel);
	m_VSpeedCtrl.setFloat(gta.TransSpeed.y);
	m_VStartCtrl.setFloat(gta.TransOffset.y);
	m_VScaleStartCtrl.setFloat(gta.ScaleStart.y);
	m_VScaleSpeedCtrl.setFloat(gta.ScaleSpeed.y);
	m_VScaleAccelCtrl.setFloat(gta.ScaleAccel.y);
	m_UScaleSpeedCtrl.setFloat(gta.ScaleSpeed.x);
	m_VAccelCtrl.setFloat(gta.TransAccel.y);
	m_UScaleAccel.setFloat(gta.ScaleAccel.x);
	m_UAccelCtrl.setFloat(gta.TransAccel.x);
	m_USpeedCtrl.setFloat(gta.TransSpeed.x);
	m_UStartCtrl.setFloat(gta.TransOffset.x);
	m_UScaleStartCtrl.setFloat(gta.ScaleStart.x);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CConstraintMeshGlobalTexAnimDlg::editExValueChanged(CEditEx *ctrl)
{
	NL3D::CPSConstraintMesh::CGlobalTexAnim gta;

	gta.WRotSpeed    = m_WRotSpeedCtrl.getFloat();
	gta.WRotAccel    = m_WRotAccelCtrl.getFloat();
	gta.TransSpeed.y = m_VSpeedCtrl.getFloat();
	gta.TransOffset.y = m_VStartCtrl.getFloat();
	gta.ScaleStart.y = m_VScaleStartCtrl.getFloat();
	gta.ScaleSpeed.y = m_VScaleSpeedCtrl.getFloat();
	gta.ScaleAccel.y = m_VScaleAccelCtrl.getFloat();
	gta.ScaleSpeed.x = m_UScaleSpeedCtrl.getFloat();
	gta.TransAccel.y = m_VAccelCtrl.getFloat();
	gta.ScaleAccel.x = m_UScaleAccel.getFloat();
	gta.TransAccel.x = m_UAccelCtrl.getFloat();
	gta.TransSpeed.x = m_USpeedCtrl.getFloat();
	gta.TransOffset.x = m_UStartCtrl.getFloat();
	gta.ScaleStart.x = m_UScaleStartCtrl.getFloat();

	_CM->setGlobalTexAnim(_Stage, gta);

}
