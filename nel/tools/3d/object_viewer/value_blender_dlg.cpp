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



// ValueBlenderDlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "value_blender_dlg.h"
#include "edit_attrib_dlg.h"
#include "popup_notify.h"


/////////////////////////////////////////////////////////////////////////////
// CValueBlenderDlg dialog


CValueBlenderDlg::CValueBlenderDlg(IValueBlenderDlgClient *creationInterface,
								   bool destroyInterface,
								   CWnd* pParent,
								   IPopupNotify *pn,
								   CParticleWorkspace::CNode *ownerNode
								  )
	: _CreateInterface(creationInterface),
	  CDialog(CValueBlenderDlg::IDD, pParent),
	  _PN(pn),
	  _DestroyInterface(destroyInterface),
	  _Node(ownerNode)	  
{
	//{{AFX_DATA_INIT(CValueBlenderDlg)
	//}}AFX_DATA_INIT
}

CValueBlenderDlg::~CValueBlenderDlg()
{
	if (_DestroyInterface) delete _CreateInterface;
	delete _Dlg1 ;
	delete _Dlg2 ;	
}


void CValueBlenderDlg::init(CWnd *pParent)
{	
	CDialog::Create(IDD_VALUE_BLENDER, pParent);	
	ShowWindow(SW_SHOW);
}


void CValueBlenderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CValueBlenderDlg)
	DDX_Control(pDX, IDC_VALUE2, m_Value2);
	DDX_Control(pDX, IDC_VALUE1, m_Value1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CValueBlenderDlg, CDialog)
	//{{AFX_MSG_MAP(CValueBlenderDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CValueBlenderDlg message handlers

BOOL CValueBlenderDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateData() ;
	nlassert(_CreateInterface) ;	
	_Dlg1 = _CreateInterface->createDialog(0, _Node) ;
	_Dlg2 = _CreateInterface->createDialog(1, _Node) ;
	RECT r, or ;	
	GetWindowRect(&or) ;
	m_Value1.GetWindowRect(&r) ;
	_Dlg1->init(r.left - or.left, r.top - or.top, this) ;
	m_Value2.GetWindowRect(&r) ;
	_Dlg2->init(r.left - or.left, r.top - or.top, this) ;	
	
	UpdateData(FALSE) ;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}





void CValueBlenderDlg::OnClose() 
{	
	if (_PN) _PN->childPopupClosed(this);
	//CDialog::OnClose();
}
