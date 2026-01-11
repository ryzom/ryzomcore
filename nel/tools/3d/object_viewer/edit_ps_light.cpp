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

// edit_ps_light.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "edit_ps_light.h"
#include "attrib_dlg.h"


/////////////////////////////////////////////////////////////////////////////
// CEditPSLight dialog


//***************************************************************************************
CEditPSLight::CEditPSLight(CParticleWorkspace::CNode *ownerNode, NL3D::CPSLight *light) 
						  :	_Node(ownerNode),
							_ColorDlg(NULL),
							_AttenStartDlg(NULL),
							_AttenEndDlg(NULL)
{
	//{{AFX_DATA_INIT(CEditPSLight)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	nlassert(light);
	_Light = light;
}

//***************************************************************************************
CEditPSLight::~CEditPSLight()
{
	if (_ColorDlg)
	{
		_ColorDlg->DestroyWindow();
		delete _ColorDlg;
	}
	if (_AttenStartDlg)
	{
		_AttenStartDlg->DestroyWindow();
		delete _AttenStartDlg;
	}
	if (_AttenEndDlg)
	{
		_AttenEndDlg->DestroyWindow();
		delete _AttenEndDlg;
	}
}


//***************************************************************************************
void CEditPSLight::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditPSLight)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

//***************************************************************************************
void CEditPSLight::init(CWnd* pParent /*=NULL*/)
{
	Create(IDD_PS_LIGHT, pParent);	
	nlassert(_Light);
	//
	RECT r;
	HBITMAP bmh;
	//
	_ColorDlg = new CAttribDlgRGBA("LIGHT_COLOR", _Node);
	_ColorWrapper.L = _Light;
	_ColorDlg->setWrapper(&_ColorWrapper);	
	_ColorDlg->setSchemeWrapper(&_ColorWrapper);
	GetDlgItem(IDC_COLOR_PLACEHOLDER)->GetWindowRect(&r);
	ScreenToClient(&r);
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_COLOR));
	_ColorDlg->init(bmh, r.left, r.top, this);
	//
	_AttenStartDlg = new CAttribDlgFloat("LIGHT_ATTEN_START", _Node, 0.01f, 5.f);
	_AttenStartDlg->enableLowerBound(0.01f, true);
	_AttenStartWrapper.L = _Light;	
	_AttenStartDlg->setWrapper(&_AttenStartWrapper);	
	_AttenStartDlg->setSchemeWrapper(&_AttenStartWrapper);
	GetDlgItem(IDC_ATTEN_START_PLACEHOLDER)->GetWindowRect(&r);
	ScreenToClient(&r);
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIGHT_ATTEN_START));
	_AttenStartDlg->init(bmh, r.left, r.top, this);
	//
	_AttenEndDlg = new CAttribDlgFloat("LIGHT_ATTEN_END", _Node, 0.01f, 5.f);
	_AttenEndDlg->enableLowerBound(0.01f, true);
	_AttenEndWrapper.L = _Light;
	_AttenEndDlg->setWrapper(&_AttenEndWrapper);	
	_AttenEndDlg->setSchemeWrapper(&_AttenEndWrapper);
	GetDlgItem(IDC_ATTEN_END_PLACEHOLDER)->GetWindowRect(&r);
	ScreenToClient(&r);
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIGHT_ATTEN_END));
	_AttenEndDlg->init(bmh, r.left, r.top, this);
	//	
	ShowWindow(SW_SHOW); 
	UpdateData(FALSE);
}



BEGIN_MESSAGE_MAP(CEditPSLight, CDialog)
	//{{AFX_MSG_MAP(CEditPSLight)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditPSLight message handlers
