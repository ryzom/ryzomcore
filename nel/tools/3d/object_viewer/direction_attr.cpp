// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "direction_attr.h"
#include "direction_edit.h"
#include "choose_name.h"
#include "nel/3d/ps_direction.h"
#include "nel/3d/particle_system.h"



/////////////////////////////////////////////////////////////////////////////
// CDirectionAttr dialog
CDirectionAttr::CDirectionAttr(const std::string &id): _DirectionDlg(NULL), _Wrapper(NULL), _DirectionWrapper(NULL)
{
	//{{AFX_DATA_INIT(CDirectionAttr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//=======================================================================================
void CDirectionAttr::init(uint32 x, uint32 y, CWnd *pParent)
{
	Create(IDD_DIRECTION_ATTR, pParent);
	RECT r;
	GetClientRect(&r);
	CRect wr;
	wr.left = r.left + x;
	wr.top = r.top + y;
	wr.bottom = r.bottom + y;
	wr.right = r.right + x;
	MoveWindow(wr);
	if (_DirectionWrapper && _DirectionWrapper->supportGlobalVectorValue())
	{
		GetDlgItem(IDC_GLOBAL_DIRECTION)->ShowWindow(TRUE);
	}
	EnableWindow(TRUE);
	ShowWindow(SW_SHOW);	
}


//=======================================================================================
void CDirectionAttr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDirectionAttr)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDirectionAttr, CDialog)
	//{{AFX_MSG_MAP(CDirectionAttr)
	ON_BN_CLICKED(IDC_VECT_I, OnVectI)
	ON_BN_CLICKED(IDC_VECT_J, OnVectJ)
	ON_BN_CLICKED(IDC_VECT_K, OnVectK)
	ON_BN_CLICKED(IDC_VECT_MINUS_I, OnVectMinusI)
	ON_BN_CLICKED(IDC_VECT_MINUS_J, OnVectMinusJ)
	ON_BN_CLICKED(IDC_VECT_MINUS_K, OnVectMinusK)
	ON_BN_CLICKED(IDC_CUSTOM_DIRECTION, OnCustomDirection)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_GLOBAL_DIRECTION, OnGlobalDirection)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirectionAttr message handlers


//=======================================================================================
void CDirectionAttr::OnVectI() 
{
	nlassert(_Wrapper);	
	_Wrapper->setAndUpdateModifiedFlag(NLMISC::CVector::I);
}

//=======================================================================================
void CDirectionAttr::OnVectJ() 
{
	nlassert(_Wrapper);	
	_Wrapper->setAndUpdateModifiedFlag(NLMISC::CVector::J);
}

//=======================================================================================
void CDirectionAttr::OnVectK() 
{
	_Wrapper->setAndUpdateModifiedFlag(NLMISC::CVector::K);
}

//=======================================================================================
void CDirectionAttr::OnVectMinusI() 
{
	_Wrapper->setAndUpdateModifiedFlag( - NLMISC::CVector::I);	
}

//=======================================================================================
void CDirectionAttr::OnVectMinusJ() 
{
	_Wrapper->setAndUpdateModifiedFlag(- NLMISC::CVector::J);
}

//=======================================================================================
void CDirectionAttr::OnVectMinusK() 
{
	_Wrapper->setAndUpdateModifiedFlag(- NLMISC::CVector::K);	
}	

//=======================================================================================
BOOL CDirectionAttr::EnableWindow(BOOL bEnable)
{
	BOOL enableUserDirection = TRUE;
	if (_DirectionWrapper && _DirectionWrapper->supportGlobalVectorValue() && !_DirectionWrapper->getGlobalVectorValueName().empty())
	{
		enableUserDirection = FALSE;
	}	
	GetDlgItem(IDC_VECT_I)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_J)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_K)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_MINUS_I)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_MINUS_J)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_MINUS_K)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_CUSTOM_DIRECTION)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_DIRECTION_TEXT)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_GLOBAL_DIRECTION)->EnableWindow(bEnable);
	return CEditAttribDlg::EnableWindow(bEnable);
}

//=======================================================================================
void CDirectionAttr::OnCustomDirection() 
{
	_DirectionDlg = new CDirectionEdit(_Wrapper);
	_DirectionDlg->init(this, this);
	EnableWindow(FALSE);	
}

//=======================================================================================
void CDirectionAttr::childPopupClosed(CWnd *)
{
	_DirectionDlg->DestroyWindow();
	delete _DirectionDlg;
	_DirectionDlg = NULL;
	EnableWindow(TRUE);

}

//=======================================================================================
void CDirectionAttr::OnDestroy() 
{
	CDialog::OnDestroy();	
	if (_DirectionDlg)
	{
		_DirectionDlg->DestroyWindow();
	}
	delete _DirectionDlg;
}

//=======================================================================================
void CDirectionAttr::OnGlobalDirection() 
{
	nlassert(_DirectionWrapper);
	CChooseName chooseName(nlUtf8ToTStr(_DirectionWrapper->getGlobalVectorValueName()));

	if (chooseName.DoModal() == IDOK)
	{
		_DirectionWrapper->enableGlobalVectorValue(chooseName.getName());
		if (!chooseName.getName().empty())
		{
			NL3D::CParticleSystem::setGlobalVectorValue(chooseName.getName(), NLMISC::CVector::I); // take a non NULL value for the direction
		}
	}
	EnableWindow(TRUE);
}
