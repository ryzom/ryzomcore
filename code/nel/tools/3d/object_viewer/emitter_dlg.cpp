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
#include "emitter_dlg.h"
#include "direction_attr.h"
#include "particle_tree_ctrl.h"
#include "particle_dlg.h"

#include "nel/3d/particle_system.h"



/////////////////////////////////////////////////////////////////////////////
// CEmitterDlg dialog


CEmitterDlg::CEmitterDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSEmitter *emitter, CParticleDlg *particleDlg)
	  : _Node(ownerNode),
		_Emitter(emitter),
	    _PeriodDlg(NULL),
		_GenNbDlg(NULL),
	    _StrenghtModulateDlg(NULL),
	    _SpeedInheritanceFactorDlg(NULL),
	    _ParticleDlg(particleDlg)
{
	nlassert(_Emitter);
	nlassert(_ParticleDlg);
	//{{AFX_DATA_INIT(CEmitterDlg)
	m_ConsistentEmission = _Emitter->isConsistentEmissionEnabled();
	m_BypassAutoLOD = FALSE;
	//}}AFX_DATA_INIT
}

CEmitterDlg::~CEmitterDlg()
{
	_PeriodDlg->DestroyWindow();
	_GenNbDlg->DestroyWindow();
	_StrenghtModulateDlg->DestroyWindow();
	_SpeedInheritanceFactorDlg->DestroyWindow();
	_DelayedEmissionDlg->DestroyWindow();
	_MaxEmissionCountDlg->DestroyWindow();

	delete _PeriodDlg;
	delete _GenNbDlg;
	delete _StrenghtModulateDlg;
	delete _SpeedInheritanceFactorDlg;
	delete _DelayedEmissionDlg;
	delete _MaxEmissionCountDlg;
}


void CEmitterDlg::init(CWnd* pParent)
{
	Create(IDD_EMITTER_DIALOG, pParent);	
	// fill the emitted type combo box with all the types of located	
	initEmittedType();	
	m_EmissionTypeCtrl.SetCurSel((int) _Emitter->getEmissionType() );
	ShowWindow(SW_SHOW); 
	UpdateData(FALSE);
}

void CEmitterDlg::initEmittedType()
{	
	m_EmittedTypeCtrl.ResetContent();
	NL3D::CParticleSystem *ps = _Emitter->getOwner()->getOwner();
	uint nbLocated = ps->getNbProcess(); 
	m_EmittedTypeCtrl.InitStorage(nbLocated, 16);	
	for (uint k = 0; k < nbLocated; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k));
		if (loc) // is this a located
		{
			m_EmittedTypeCtrl.AddString(nlUtf8ToTStr(loc->getName()));
			_LocatedList.push_back(loc);			
			if (loc == _Emitter->getEmittedType())
			{
				m_EmittedTypeCtrl.SetCurSel(k);
			}
		}
	}
}

void CEmitterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmitterDlg)
	DDX_Control(pDX, IDC_DIRECTION_MODE, m_DirectionModeCtrl);
	DDX_Control(pDX, IDC_TYPE_OF_EMISSION, m_EmissionTypeCtrl);
	DDX_Control(pDX, IDC_EMITTED_TYPE, m_EmittedTypeCtrl);	
	DDX_Check(pDX, IDC_CONSISTENT_EMISSION, m_ConsistentEmission);
	DDX_Check(pDX, IDC_BYPASS_AUTOLOD, m_BypassAutoLOD);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmitterDlg, CDialog)
	//{{AFX_MSG_MAP(CEmitterDlg)
	ON_CBN_SELCHANGE(IDC_EMITTED_TYPE, OnSelchangeEmittedType)
	ON_CBN_SELCHANGE(IDC_TYPE_OF_EMISSION, OnSelchangeTypeOfEmission)	
	ON_BN_CLICKED(IDC_CONSISTENT_EMISSION, OnConsistentEmission)
	ON_BN_CLICKED(IDC_BYPASS_AUTOLOD, OnBypassAutoLOD)
	ON_CBN_SELCHANGE(IDC_DIRECTION_MODE, OnSelchangeDirectionMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmitterDlg message handlers

void CEmitterDlg::OnSelchangeEmittedType() 
{
	UpdateData();
	uint k = m_EmittedTypeCtrl.GetCurSel();
	if (!_Emitter->setEmittedType(_LocatedList[k]))
	{
		if (_Emitter->getOwner()->getOwner()->getBehaviourType() == NL3D::CParticleSystem::SpellFX || _Emitter->getOwner()->getOwner()->getBypassMaxNumIntegrationSteps())
		{		
			MessageBox(_T("Can't perform operation : the system is flagged with 'No max nb steps' or uses the preset 'Spell FX', and thus, should have a finite duration. This operation create a loop in the system, and so is forbidden."), _T("Error"), MB_ICONEXCLAMATION);
		}
		else
		{
			MessageBox(_T("Loops with emitters are forbidden."), _T("Error"), MB_ICONEXCLAMATION);
		}
		initEmittedType();
	}	
	_ParticleDlg->StartStopDlg->resetAutoCount(_Node);
	updateModifiedFlag();
}

void CEmitterDlg::OnSelchangeTypeOfEmission() 
{
	UpdateData();
	if (!_Emitter->setEmissionType((NL3D::CPSEmitter::TEmissionType) m_EmissionTypeCtrl.GetCurSel()))
	{
		CString mess;
		mess.LoadString(IDS_PS_NO_FINITE_DURATION);
		CString errorStr;
		errorStr.LoadString(IDS_ERROR);
		MessageBox((LPCTSTR) mess, (LPCTSTR) errorStr, MB_ICONEXCLAMATION);
		m_EmissionTypeCtrl.SetCurSel((int) _Emitter->getEmissionType());		
	}

	updatePeriodDlg();
	_ParticleDlg->StartStopDlg->resetAutoCount(_Node);
	updateModifiedFlag();
}


void CEmitterDlg::updatePeriodDlg(void)
{
	BOOL bEnable = _Emitter->getEmissionType() == NL3D::CPSEmitter::regular;
	_PeriodDlg->EnableWindow(bEnable);
	_DelayedEmissionDlg->EnableWindow(bEnable);
	_MaxEmissionCountDlg->EnableWindow(bEnable);
}

BOOL CEmitterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	RECT r;

	 GetDlgItem(IDC_SPEED_INHERITANCE_FACTOR_FRAME)->GetWindowRect(&r);
	 ScreenToClient(&r);
	_SpeedInheritanceFactorDlg = new CEditableRangeFloat("SPEED_INHERITANCE_FACTOR", _Node, -1.f, 1.f);
	_SpeedInheritanceFactorWrapper.E = _Emitter;
	_SpeedInheritanceFactorDlg->setWrapper(&_SpeedInheritanceFactorWrapper);
	_SpeedInheritanceFactorDlg->init(r.left, r.top, this);

	 GetDlgItem(IDC_DELAYED_EMISSION_FRAME)->GetWindowRect(&r);
	 ScreenToClient(&r);
	_DelayedEmissionDlg = new CEditableRangeFloat("DELAYED_EMISSION", _Node, 0.f, 10.f);
	_DelayedEmissionDlg->enableLowerBound(0.f, false);
	_DelayedEmissionWrapper.E = _Emitter;
	_DelayedEmissionWrapper.Node = _Node;
	_DelayedEmissionWrapper.SSPS = _ParticleDlg->StartStopDlg;
	_DelayedEmissionDlg->setWrapper(&_DelayedEmissionWrapper);
	_DelayedEmissionDlg->init(r.left, r.top, this);

	GetDlgItem(IDC_MAX_EMISSION_COUNT_FRAME)->GetWindowRect(&r);
	 ScreenToClient(&r);
	_MaxEmissionCountDlg = new CEditableRangeUInt("MAX_EMISSION_COUNT", _Node, 0, 100);
	_MaxEmissionCountDlg->enableUpperBound(256, false);
	_MaxEmissionCountWrapper.E = _Emitter;
	_MaxEmissionCountWrapper.Node = _Node;
	_MaxEmissionCountWrapper.SSPS = _ParticleDlg->StartStopDlg;
	_MaxEmissionCountWrapper.HWnd = (HWND) (*this);
	_MaxEmissionCountDlg->setWrapper(&_MaxEmissionCountWrapper);
	_MaxEmissionCountDlg->init(r.left, r.top, this);
	_MaxEmissionCountWrapper.MaxEmissionCountDlg = _MaxEmissionCountDlg;



	uint posX = 13;
	uint posY = r.bottom + 5;	

	// setup the dialog for the period of emission edition

	_PeriodDlg = new CAttribDlgFloat("EMISSION_PERIOD", _Node, 0.f, 2.f);	
	_PeriodWrapper.E = _Emitter;
	_PeriodWrapper.Node = _Node;
	_PeriodWrapper.SSPS = _ParticleDlg->StartStopDlg;
	_PeriodDlg->setWrapper(&_PeriodWrapper);
	_PeriodDlg->setSchemeWrapper(&_PeriodWrapper);
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_EMISSION_PERIOD));
	_PeriodDlg->init(bmh, posX, posY, this);
	posY += 120;

	// setup the dialog that helps tuning the number of particle being emitted at a time

	_GenNbDlg = new CAttribDlgUInt("EMISSION_GEN_NB",  _Node, 1, 11);
	_GenNbWrapper.E = _Emitter;
	_GenNbWrapper.Node = _Node;
	_GenNbWrapper.SSPS = _ParticleDlg->StartStopDlg;
	_GenNbDlg->setWrapper(&_GenNbWrapper);
	_GenNbDlg->setSchemeWrapper(&_GenNbWrapper);
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_EMISSION_QUANTITY));
	_GenNbDlg->init(bmh, posX, posY, this);
	posY += 120;

	if (dynamic_cast<NL3D::CPSModulatedEmitter *>(_Emitter))
	{
		_StrenghtModulateDlg = new CAttribDlgFloat("EMISSION_GEN_NB",  _Node, 1, 11);
		_ModulatedStrenghtWrapper.E = dynamic_cast<NL3D::CPSModulatedEmitter *>(_Emitter);
		_StrenghtModulateDlg->setWrapper(&_ModulatedStrenghtWrapper);
		_StrenghtModulateDlg->setSchemeWrapper(&_ModulatedStrenghtWrapper);
		bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_MODULATE_STRENGHT));
		_StrenghtModulateDlg->init(bmh, posX, posY, this);
		posY += 120;
	}

	// deals with emitters that have a direction
	if (dynamic_cast<NL3D::CPSDirection *>(_Emitter))
	{
		CDirectionAttr *da = new CDirectionAttr(std::string("DIRECTION"));
		pushWnd(da);		
		_DirectionWrapper.E = dynamic_cast<NL3D::CPSDirection *>(_Emitter);
		da->setWrapper(&_DirectionWrapper);
		da->setDirectionWrapper(dynamic_cast<NL3D::CPSDirection *>(_Emitter));
		da->init(posX, posY, this);
		da->GetClientRect(&r);
		posY += r.bottom;
	}

	// radius  for conic emitter
	if (dynamic_cast<NL3D::CPSEmitterConic *>(_Emitter))
	{
		CEditableRangeFloat *ecr = new CEditableRangeFloat(std::string("CONIC EMITTER RADIUS"), _Node, 0.1f, 2.1f);
		pushWnd(ecr);
		_ConicEmitterRadiusWrapper.E = dynamic_cast<NL3D::CPSEmitterConic *>(_Emitter);
		ecr->setWrapper(&_ConicEmitterRadiusWrapper);
		ecr->init(posX + 80, posY, this);

		CStatic *s = new CStatic;
		pushWnd(s);
		s->Create(_T("Radius :"), SS_LEFT, CRect(posX, posY + 10 , posX + 70, posY + 32), this);
		s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
		s->ShowWindow(SW_SHOW);

		ecr->GetClientRect(&r);
		posY += r.bottom;
	}
	
	if (_Emitter->isSpeedBasisEmissionEnabled())
	{
		m_DirectionModeCtrl.SetCurSel((int)AlignOnEmitterDirection);
	}
	else if (!_Emitter->isUserMatrixModeForEmissionDirectionEnabled())
	{
		m_DirectionModeCtrl.SetCurSel((int)Default);
	}
	else if (_Emitter->getUserMatrixModeForEmissionDirection() == NL3D::PSFXWorldMatrix)
	{
		m_DirectionModeCtrl.SetCurSel((int)LocalToSystem);
	}
	else if (_Emitter->getUserMatrixModeForEmissionDirection() == NL3D::PSIdentityMatrix)
	{
		m_DirectionModeCtrl.SetCurSel((int)InWorld);
	}
	else if (_Emitter->getUserMatrixModeForEmissionDirection() == NL3D::PSUserMatrix)
	{
		m_DirectionModeCtrl.SetCurSel((int)LocalToFatherSkeleton);
	}
	else
	{
		nlassert(0);
	}	

	updatePeriodDlg();

	// bypass auto LOD
	nlassert(_Emitter->getOwner() && _Emitter->getOwner()->getOwner());
	NL3D::CParticleSystem &ps = *_Emitter->getOwner()->getOwner();
	CButton *button = (CButton *) GetDlgItem(IDC_BYPASS_AUTOLOD);
	if (ps.isAutoLODEnabled() && !ps.isSharingEnabled())
	{		
		button->EnableWindow(TRUE);
		m_BypassAutoLOD = _Emitter->getBypassAutoLOD();
	}
	else
	{
		button->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmitterDlg::OnConsistentEmission() 
{
	UpdateData();
	_Emitter->enableConsistenEmission(m_ConsistentEmission != 0 ? true : false /* VC6 warning */);
	updateModifiedFlag();
	UpdateData(TRUE);
}

void CEmitterDlg::OnBypassAutoLOD() 
{
	UpdateData();
	_Emitter->setBypassAutoLOD(m_BypassAutoLOD ? true : false);
	UpdateData(TRUE);	
	updateModifiedFlag();
}

void CEmitterDlg::CMaxEmissionCountWrapper::set(const uint32 &count)
{
   if (!E->setMaxEmissionCount((uint8) count))
   {
	   CString mess;
	   mess.LoadString(IDS_PS_NO_FINITE_DURATION);
	   CString errorStr;
	   errorStr.LoadString(IDS_ERROR);
	   ::MessageBox(HWnd, (LPCTSTR) mess, (LPCTSTR) errorStr, MB_ICONEXCLAMATION);	   
	   MaxEmissionCountDlg->updateValueFromReader();
   }
   SSPS->resetAutoCount(Node);
}

void CEmitterDlg::OnSelchangeDirectionMode() 
{
	UpdateData();
	nlassert(_Emitter);
	switch(m_DirectionModeCtrl.GetCurSel())
	{
		case Default:
			_Emitter->enableSpeedBasisEmission(false);
			_Emitter->enableUserMatrixModeForEmissionDirection(false);
		break;
		case AlignOnEmitterDirection:
			_Emitter->enableSpeedBasisEmission(true);
			_Emitter->enableUserMatrixModeForEmissionDirection(false);
		break;
		case InWorld:
			_Emitter->enableSpeedBasisEmission(false);
			_Emitter->enableUserMatrixModeForEmissionDirection(true);
			_Emitter->setUserMatrixModeForEmissionDirection(NL3D::PSIdentityMatrix);
		break;
		case LocalToSystem:
			_Emitter->enableSpeedBasisEmission(false);
			_Emitter->enableUserMatrixModeForEmissionDirection(true);
			_Emitter->setUserMatrixModeForEmissionDirection(NL3D::PSFXWorldMatrix);
		break;
		case LocalToFatherSkeleton:
			_Emitter->enableSpeedBasisEmission(false);
			_Emitter->enableUserMatrixModeForEmissionDirection(true);
			_Emitter->setUserMatrixModeForEmissionDirection(NL3D::PSUserMatrix);
		break;
	}
	updateModifiedFlag();
	UpdateData(FALSE);
}
