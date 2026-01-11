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

// ribbon_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "ribbon_dlg.h"
#include "editable_range.h"

#include "nel/3d/ps_particle2.h"


/////////////////////////////////////////////////////////////////////////////
// CRibbonDlg dialog


CRibbonDlg::CRibbonDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSRibbonBase *ribbon, CWnd* pParent /* = NULL*/)
					 : CDialog(CRibbonDlg::IDD, pParent),
					   _Node(ownerNode),
					   _Ribbon(ribbon),
					   _RibbonLengthDlg(NULL)
{
	nlassert(ribbon);
	//{{AFX_DATA_INIT(CRibbonDlg)
	m_UseHermitteInterpolation = _Ribbon->getInterpolationMode() == NL3D::CPSRibbonBase::Hermitte;
	m_ConstantLength = _Ribbon->getRibbonMode() == NL3D::CPSRibbonBase::FixedSize;
	//}}AFX_DATA_INIT
}

///==================================
CRibbonDlg::~CRibbonDlg()
{
	if (_RibbonLengthDlg)
	{
		_RibbonLengthDlg->DestroyWindow();
		delete _RibbonLengthDlg;
	}
	if (_LODDegradationDlg)
	{
		_LODDegradationDlg->DestroyWindow();
		delete _LODDegradationDlg;
	}
}

///==================================
void CRibbonDlg::init(CWnd *pParent, sint x, sint y)
{
	Create(IDD_RIBBON_DLG, pParent);
	RECT r;
	GetClientRect(&r);
	r.top += y; r.bottom += y;
	r.right += x; r.left += x;
	MoveWindow(&r);	
	ShowWindow(SW_SHOW);
}


///==================================
void CRibbonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRibbonDlg)
	DDX_Check(pDX, IDC_USE_HERMITTE_INTERPOLATION, m_UseHermitteInterpolation);
	DDX_Check(pDX, IDC_CONSTANT_LENGTH, m_ConstantLength);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRibbonDlg, CDialog)
	//{{AFX_MSG_MAP(CRibbonDlg)
	ON_BN_CLICKED(IDC_USE_HERMITTE_INTERPOLATION, OnUseHermitteInterpolation)
	ON_BN_CLICKED(IDC_CONSTANT_LENGTH, OnConstantLength)
	ON_CBN_SELCHANGE(IDC_TRAIL_COORD_SYSTEM, OnSelchangeTrailCoordSystem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRibbonDlg message handlers

///=========================================================
void CRibbonDlg::OnUseHermitteInterpolation() 
{
	UpdateData();	
	_Ribbon->setInterpolationMode(m_UseHermitteInterpolation ? 
									NL3D::CPSRibbonBase::Hermitte :
									NL3D::CPSRibbonBase::Linear);
	updateModifiedFlag();
}

///=========================================================
void CRibbonDlg::OnConstantLength() 
{
	UpdateData();
	_Ribbon->setRibbonMode(m_ConstantLength ? 
						   NL3D::CPSRibbonBase::FixedSize :
						   NL3D::CPSRibbonBase::VariableSize);
	updateState();
	updateModifiedFlag();
}


///=========================================================
BOOL CRibbonDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// Length
	CEditableRangeFloat *erf = new CEditableRangeFloat("RIBBON_LENGTH", _Node, 0.1f, 10.1f);
	_RibbonLengthDlg = erf;
	_RibbonLengthWrapper.R = _Ribbon;
	erf->setWrapper(&_RibbonLengthWrapper);
	erf->enableLowerBound(0.f, true);
	RECT r;
	GetDlgItem(IDC_RIBBON_LENGTH)->GetWindowRect(&r);
	ScreenToClient(&r);	
	erf->init(r.left, r.top, this);


	// Lod degradation
	erf = new CEditableRangeFloat("LOD_DEGRADATION", _Node, 0.f, 1.f);
	_LODDegradationDlg = erf;
	_LODDegradationWrapper.R = _Ribbon;
	erf->setWrapper(&_LODDegradationWrapper);
	erf->enableLowerBound(0.f, false);
	erf->enableUpperBound(0.f, false);	
	GetDlgItem(IDC_LOD_DEGRADATION)->GetWindowRect(&r);
	ScreenToClient(&r);	
	erf->init(r.left, r.top, this);

	// Coord system
	((CComboBox *) GetDlgItem(IDC_TRAIL_COORD_SYSTEM))->SetCurSel((int) _Ribbon->getMatrixMode());

	updateState();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

///=========================================================
void CRibbonDlg::updateState()
{
	_RibbonLengthDlg->EnableWindow(_Ribbon->getRibbonMode() == NL3D::CPSRibbonBase::FixedSize);
}

///=========================================================
float CRibbonDlg::CRibbonLengthWrapper::get() const 
{ 
	return R->getRibbonLength(); 
}

///=========================================================
void  CRibbonDlg::CRibbonLengthWrapper::set(const float &v)
{ 
	R->setRibbonLength(v); 
}


///=========================================================
float CRibbonDlg::CLODDegradationWrapper::get() const 
{ 
	return R->getLODDegradation(); 
}

///=========================================================
void  CRibbonDlg::CLODDegradationWrapper::set(const float &v)
{ 
	R->setLODDegradation(v); 
}


///=========================================================
void CRibbonDlg::OnSelchangeTrailCoordSystem() 
{
	_Ribbon->setMatrixMode((NL3D::CPSRibbonBase::TMatrixMode) ((CComboBox *) GetDlgItem(IDC_TRAIL_COORD_SYSTEM))->GetCurSel());
	_Node->setModified(true);
}
