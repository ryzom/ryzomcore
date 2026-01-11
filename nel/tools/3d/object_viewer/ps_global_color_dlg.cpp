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
#include "ps_global_color_dlg.h"
#include "popup_notify.h"
#include "attrib_dlg.h"
#include "nel/3d/particle_system.h"


/////////////////////////////////////////////////////////////////////////////
// CPSGlobalColorDlg dialog


CPSGlobalColorDlg::CPSGlobalColorDlg(CParticleWorkspace::CNode *ownerNode, IPopupNotify *pn, CWnd* pParent /* = NULL */)
	: _Node(ownerNode),
	  _PN(pn),
	  CDialog(CPSGlobalColorDlg::IDD, pParent)
{
	nlassert(_Node);
	//{{AFX_DATA_INIT(CPSGlobalColorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPSGlobalColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPSGlobalColorDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPSGlobalColorDlg, CDialog)
	//{{AFX_MSG_MAP(CPSGlobalColorDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///===========================================================
void CPSGlobalColorDlg::init(CWnd *pParent)
{
	Create(IDD_GLOBAL_COLOR, pParent);
	ShowWindow(SW_SHOW);
}

///===========================================================
CPSGlobalColorDlg::CGlobalColorWrapper::scheme_type *CPSGlobalColorDlg::CGlobalColorWrapper::getScheme(void) const
{
	nlassert(PS);
	return PS->getColorAttenuationScheme();
}

///===========================================================
void CPSGlobalColorDlg::CGlobalColorWrapper::setScheme(CPSGlobalColorDlg::CGlobalColorWrapper::scheme_type *s)
{
	PS->setColorAttenuationScheme(s);
}


/////////////////////////////////////////////////////////////////////////////
// CPSGlobalColorDlg message handlers

void CPSGlobalColorDlg::OnClose() 
{		
	CDialog::OnClose();
	if (_PN)
		_PN->childPopupClosed(this);
}

///===========================================================
BOOL CPSGlobalColorDlg::OnInitDialog() 
{		
	CDialog::OnInitDialog();
	_GlobalColorWrapper.PS = _Node->getPSPointer();
	RECT r;	
	CAttribDlgRGBA *adr = new CAttribDlgRGBA("GLOBAL_PS_COLOR", _Node);	
	adr->setSchemeWrapper(&_GlobalColorWrapper);
	adr->enableMemoryScheme(false);
	adr->enableNbCycles(false);
	adr->enableSrcInput(false);
	adr->disableConstantValue();
	GetDlgItem(IDC_COLOR_ATTRIB)->GetWindowRect(&r);
	ScreenToClient(&r);
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_COLOR));
	adr->init(bmh, r.left, r.top, this);
	pushWnd(adr);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
