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
#include "value_gradient_dlg.h"

#include "edit_attrib_dlg.h"
#include "editable_range.h"

#include "popup_notify.h"


/////////////////////////////////////////////////////////////////////////////
// CValueGradientDlg dialog


CValueGradientDlg::CValueGradientDlg(IValueGradientDlgClient *clientInterface,
									 CParticleWorkspace::CNode *ownerNode,
									 bool destroyClientInterface,
									 CWnd* pParent,
									 IPopupNotify *pn,
									 bool canTuneNbStages /* = true*/,
									 uint minSize /*= 2*/)
	: CDialog(CValueGradientDlg::IDD, pParent),
	  _ClientInterface(clientInterface),
	  _Node(ownerNode),
	  _DestroyClientInterface(destroyClientInterface),
	  _EditValueDlg(NULL),
	  _CanTuneNbStages(canTuneNbStages),
	  _MinSize(minSize),
	  _PN(pn)
{
	//{{AFX_DATA_INIT(CValueGradientDlg)
	//}}AFX_DATA_INIT	
	_NbStepDlg = new CEditableRangeUInt(std::string("GRADIENT NB STEP"), ownerNode, 1, 255);
}


CValueGradientDlg::~CValueGradientDlg()
{	
	if (_DestroyClientInterface) delete _ClientInterface;
	delete _EditValueDlg;
	_NbStepDlg->DestroyWindow();
	delete _NbStepDlg;
}

void CValueGradientDlg::init(CWnd *pParent)
{	
	CDialog::Create(IDD_GRADIENT_DLG, pParent);	
	ShowWindow(SW_SHOW);
}



void CValueGradientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CValueGradientDlg)
	DDX_Control(pDX, IDC_NO_SAMPLES, m_NoSamples);
	DDX_Control(pDX, IDC_GRADIENT_LIST, m_GradientList);
	DDX_Control(pDX, IDC_EDITED_VALUE, m_Value);
	DDX_Control(pDX, IDC_REMOVE_VALUE, m_RemoveCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CValueGradientDlg, CDialog)
	//{{AFX_MSG_MAP(CValueGradientDlg)
	ON_BN_CLICKED(IDC_ADD_VALUE, OnAddValue)
	ON_BN_CLICKED(IDC_INSERT_VALUE, OnInsertValue)
	ON_BN_CLICKED(IDC_REMOVE_VALUE, OnRemoveValue)
	ON_BN_CLICKED(IDC_VALUE_DOWN, OnValueDown)
	ON_BN_CLICKED(IDC_VALUE_UP, OnValueUp)
	ON_LBN_SELCHANGE(IDC_GRADIENT_LIST, OnSelchangeGradientList)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CValueGradientDlg message handlers

void CValueGradientDlg::OnAddValue() 
{
	nlassert(_ClientInterface);
	UpdateData();
	++_Size;
	_ClientInterface->modifyGradient(IValueGradientDlgClient::Add, 0);
	m_GradientList.AddString("value");		
	m_RemoveCtrl.EnableWindow(TRUE);

	m_GradientList.SetCurSel(_Size - 1);
	m_GradientList.Invalidate();
	OnSelchangeGradientList();
	UpdateData(FALSE);
}

void CValueGradientDlg::OnInsertValue() 
{
	nlassert(_ClientInterface);
	UpdateData();
	uint oldIndex = m_GradientList.GetCurSel();
	++_Size;
	_ClientInterface->modifyGradient(IValueGradientDlgClient::Insert, m_GradientList.GetCurSel());
	m_GradientList.InsertString(m_GradientList.GetCurSel(), "value");		
	m_GradientList.Invalidate();
	m_GradientList.SetCurSel(oldIndex);
	OnSelchangeGradientList();
	UpdateData(FALSE);	
}

void CValueGradientDlg::OnRemoveValue() 
{
	nlassert(_ClientInterface);
	UpdateData();

	uint oldIndex = m_GradientList.GetCurSel();
	--_Size;
	_ClientInterface->modifyGradient(IValueGradientDlgClient::Delete, m_GradientList.GetCurSel());
	m_GradientList.DeleteString(m_GradientList.GetCurSel());				

	if (_Size <= _MinSize)
	{
		m_RemoveCtrl.EnableWindow(FALSE);
	}
	if (oldIndex < _Size)
	{
		m_GradientList.SetCurSel(oldIndex);
	}
	else
	{
		m_GradientList.SetCurSel(oldIndex - 1);
	}
	m_GradientList.Invalidate();	
	OnSelchangeGradientList(); /// this won't be called automatically ...
	UpdateData(FALSE);		
}

void CValueGradientDlg::OnValueDown() 
{
	nlassert(_ClientInterface);
	UpdateData();	
	uint currIndex = m_GradientList.GetCurSel();
	if (currIndex == (uint) (m_GradientList.GetCount() - 1)) return;
	_ClientInterface->modifyGradient(IValueGradientDlgClient::Down, currIndex);	
	m_GradientList.SetCurSel(currIndex + 1);
	m_GradientList.Invalidate();
	OnSelchangeGradientList();
	UpdateData(FALSE);	
}

void CValueGradientDlg::OnValueUp() 
{
	nlassert(_ClientInterface);
	UpdateData();	
	uint currIndex = m_GradientList.GetCurSel();
	if (currIndex == 0) return;
	_ClientInterface->modifyGradient(IValueGradientDlgClient::Up, currIndex);	
	m_GradientList.SetCurSel(currIndex - 1);
	m_GradientList.Invalidate();
	OnSelchangeGradientList();
	UpdateData(FALSE);	
}

BOOL CValueGradientDlg::OnInitDialog() 
{
	nlassert(_ClientInterface);
	_Size = _ClientInterface->getSchemeSize();
	CDialog::OnInitDialog();
	UpdateData();
	m_GradientList.InitStorage(_Size, 16);
	for (uint k = 0; k < _Size; ++k)
	{
		m_GradientList.AddString("value");
	}
	m_RemoveCtrl.EnableWindow(_Size > _MinSize ? TRUE : FALSE);
	m_GradientList.SetCurSel(0);
	m_GradientList.setCtrlID(IDC_GRADIENT_LIST);
	m_GradientList.setDrawer(_ClientInterface);

	if (_CanTuneNbStages)
	{
		_NbStepWrapper.I = _ClientInterface;
		_NbStepDlg->setWrapper(&_NbStepWrapper);
		_NbStepDlg->init(181, 45, this);
		_NbStepDlg->enableLowerBound(0, true);
	}
	else
	{
		m_NoSamples.ShowWindow(SW_HIDE);
	}
	UpdateData(FALSE);		
	OnSelchangeGradientList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CValueGradientDlg::OnSelchangeGradientList() 
{
	nlassert(_ClientInterface);
	UpdateData(TRUE);
	delete _EditValueDlg;	
	_EditValueDlg =	_ClientInterface->createDialog(m_GradientList.GetCurSel(), this, _Node);

	RECT r, or;
	GetWindowRect(&or);
	m_Value.GetWindowRect(&r);
	_EditValueDlg->init(r.left - or.left, r.top - or.top, this);
	UpdateData(FALSE);
	
}


void CValueGradientDlg::invalidateGrad(void)
{
	UpdateData(TRUE);
		m_GradientList.Invalidate();
	UpdateData(FALSE);
}

void CValueGradientDlg::OnClose() 
{
	CDialog::OnClose();
	if (_PN) _PN->childPopupClosed(this);		
}
