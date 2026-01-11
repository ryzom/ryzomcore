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


// auto_lod_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "auto_lod_dlg.h"
#include "editable_range.h"
#include "popup_notify.h"


/////////////////////////////////////////////////////////////////////////////
// CAutoLODDlg dialog


CAutoLODDlg::CAutoLODDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CParticleSystem *ps, IPopupNotify *pn, CWnd* pParent /* = NULL*/)
	:   _Node(ownerNode),
	    _PS(ps),
		_PN(pn),
		CDialog(CAutoLODDlg::IDD, pParent)
{
	nlassert(ps);
	//{{AFX_DATA_INIT(CAutoLODDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
}

void CAutoLODDlg::init(CWnd *pParent)
{
	Create(IDD_AUTO_LOD, pParent);
	ShowWindow(SW_SHOW);
}

void CAutoLODDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAutoLODDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutoLODDlg, CDialog)
	//{{AFX_MSG_MAP(CAutoLODDlg)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_DEGRADATION_EXPONENT, OnSelchangeDegradationExponent)
	ON_BN_CLICKED(IDC_SKIP_PARTICLES, OnSkipParticles)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoLODDlg message handlers

void CAutoLODDlg::OnClose() 
{
	CDialog::OnClose();
	if (_PN)
	_PN->childPopupClosed(this);		
}

BOOL CAutoLODDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	_DistRatioWrapper.PS = _PS;
	_MaxDistLODBiasWrapper.PS = _PS;

	RECT r;
	
	// Edit the distance at which LOD starts	
	CEditableRangeFloat *erf = new CEditableRangeFloat("AUTO_LOD_DIST_RATIO", _Node, 0.f, 0.99f);
	erf->enableUpperBound(1.f, true);	
	erf->enableLowerBound(0.f, false);
	erf->setWrapper(&_DistRatioWrapper);
	GetDlgItem(IDC_START_PERCENT_DIST)->GetWindowRect(&r);
	ScreenToClient(&r);
	erf->init(r.left, r.top, this);
	pushWnd(erf);

	// For non-shared systems only : Set the LOD bias at the max distance, so that some particles are still displayed
	erf = new CEditableRangeFloat("MAX_DIST_LOD_BIAS", _Node, 0.f, 1.0f);
	erf->enableUpperBound(1.f, false);	
	erf->enableLowerBound(0.f, false);
	erf->setWrapper(&_MaxDistLODBiasWrapper);
	GetDlgItem(IDC_MAX_DIST_LOD_BIAS)->GetWindowRect(&r);
	ScreenToClient(&r);
	erf->init(r.left, r.top, this);
	pushWnd(erf);

	if (_PS->isSharingEnabled())
	{
		erf->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_DIST_BIAS_TEXT)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_SKIP_PARTICLES)->ShowWindow(FALSE);
	}



	((CComboBox *) GetDlgItem(IDC_DEGRADATION_EXPONENT))->SetCurSel(std::min(4, (sint) _PS->getAutoLODDegradationExponent()) - 1);	
	((CButton *) GetDlgItem(IDC_SKIP_PARTICLES))->SetCheck(_PS->getAutoLODMode() ? 1 : 0);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAutoLODDlg::OnSelchangeDegradationExponent() 
{
	_PS->setupAutoLOD(_PS->getAutoLODStartDistPercent(),
					  ((CComboBox *) GetDlgItem(IDC_DEGRADATION_EXPONENT))->GetCurSel() + 1);
}

void CAutoLODDlg::OnSkipParticles() 
{
	_PS->setAutoLODMode(((CButton *) GetDlgItem(IDC_SKIP_PARTICLES))->GetCheck() != 0);
}
