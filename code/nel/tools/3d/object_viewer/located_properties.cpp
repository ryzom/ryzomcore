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


// located_properties.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "located_properties.h"
#include "particle_dlg.h"
#include "particle_tree_ctrl.h"
#include "attrib_dlg.h"
#include "lb_extern_id_dlg.h"
#include "object_viewer.h"
//
#include "nel/3d/ps_located.h"
#include "nel/3d/particle_system.h"

using NL3D::CPSLocated;

//****************************************************************************************************************
/////////////////////////////////////////////////////////////////////////////
// CLocatedProperties dialog
CLocatedProperties::CLocatedProperties(CParticleWorkspace::CNode *node, NL3D::CPSLocated *loc,  CParticleDlg *pdlg)
	: CDialog(CLocatedProperties::IDD, pdlg), _Node(node), _Located(loc), _ParticleDlg(pdlg), _MassDialog(NULL), _LifeDialog(NULL)
{
	//{{AFX_DATA_INIT(CLocatedProperties)
	m_LimitedLifeTime = FALSE;
	m_MatrixMode = 0;
	m_DisgradeWithLOD = FALSE;
	m_ParametricIntegration = FALSE;
	m_ParametricMotion = FALSE;
	m_TriggerOnDeath = FALSE;
	m_MatrixMode = -1;
	//}}AFX_DATA_INIT

	nlassert(pdlg->getObjectViewer());
	pdlg->getObjectViewer()->registerMainLoopCallBack(this);
	

	_MaxNbParticles = new CEditableRangeUInt("MAX_NB_PARTICLES", _Node, 1, 501);
	_MaxNbParticles->enableUpperBound(30000, false);
	
}

//****************************************************************************************************************
CLocatedProperties::~CLocatedProperties()
{
	nlassert(_ParticleDlg && _ParticleDlg->getObjectViewer());
	_ParticleDlg->getObjectViewer()->removeMainLoopCallBack(this);
	
	_MassDialog->DestroyWindow();
	_LifeDialog->DestroyWindow();
	_MaxNbParticles->DestroyWindow();	

	delete _LifeDialog;
	delete _MassDialog;
	delete _MaxNbParticles;	
	
}

//****************************************************************************************************************
void CLocatedProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedProperties)
	DDX_Control(pDX, IDC_EDIT_TRIGGER_ON_DEATH, m_EditTriggerOnDeath);
	DDX_Control(pDX, IDC_TRIGGER_ON_DEATH, m_TriggerOnDeathCtrl);
	DDX_Control(pDX, IDC_PARAMETRIC_MOTION, m_ParametricMotionCtrl);
	DDX_Control(pDX, IDC_PARTICLE_NUMBER_POS, m_MaxNbParticles);
	DDX_Check(pDX, IDC_LIMITED_LIFE_TIME, m_LimitedLifeTime);	
	DDX_Check(pDX, IDC_DISGRADE_WITH_LOD, m_DisgradeWithLOD);	
	DDX_Check(pDX, IDC_PARAMETRIC_MOTION, m_ParametricMotion);
	DDX_Check(pDX, IDC_TRIGGER_ON_DEATH, m_TriggerOnDeath);
	DDX_CBIndex(pDX, IDC_MATRIX_MODE, m_MatrixMode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedProperties, CDialog)
	//{{AFX_MSG_MAP(CLocatedProperties)
	ON_BN_CLICKED(IDC_LIMITED_LIFE_TIME, OnLimitedLifeTime)	
	ON_BN_CLICKED(IDC_DISGRADE_WITH_LOD, OnDisgradeWithLod)
	ON_BN_CLICKED(IDC_PARAMETRIC_MOTION, OnParametricMotion)
	ON_BN_CLICKED(IDC_EDIT_TRIGGER_ON_DEATH, OnEditTriggerOnDeath)
	ON_BN_CLICKED(IDC_TRIGGER_ON_DEATH, OnTriggerOnDeath)
	ON_BN_CLICKED(IDC_ASSIGN_COUNT, OnAssignCount)
	ON_CBN_SELCHANGE(IDC_MATRIX_MODE, OnSelchangeMatrixMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//****************************************************************************************************************
/////////////////////////////////////////////////////////////////////////////
// CLocatedProperties message handlers
BOOL CLocatedProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

///////////////////////////////////////////


//****************************************************************************************************************
void CLocatedProperties::init(uint32 x, uint32 y)
{

	Create(IDD_LOCATED_PROPERTIES, (CWnd *) _ParticleDlg);
	RECT r, pr;
	GetClientRect(&r);
	MoveWindow(x, y, r.right, r.bottom);	

	GetWindowRect(&pr);


	const sint xPos = 0;
	sint yPos = 100;

	_LifeDialog = new CAttribDlgFloat("LIFETIME", _Node);
	_LifeDialog->enableMemoryScheme(false);

	_LifeWrapper.Located = _Located;
	_LifeWrapper.Node = _Node;
	_LifeWrapper.SSPS = _ParticleDlg->StartStopDlg;

	_LifeDialog->setWrapper(&_LifeWrapper);			

	_LifeDialog->setSchemeWrapper(&_LifeWrapper);

	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIFE_TIME));

	_LifeDialog->init(bmh, xPos, yPos, this);


	_LifeDialog->GetClientRect(&r);	
	yPos += r.bottom + 3;

	if (_Located->getLastForever())
	{
		_LifeDialog->EnableWindow(FALSE);
	}	


	_MassDialog = new CAttribDlgFloat("PARTICLE_MASS", _Node, 0.001f, 10);	
	_MassDialog->enableLowerBound(0, true); // 0 is disallowed
	_MassDialog->enableMemoryScheme(false);
	_MassWrapper.Located = _Located;
	_MassDialog->setWrapper(&_MassWrapper);			
	_MassDialog->setSchemeWrapper(&_MassWrapper);
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_WEIGHT));
	_MassDialog->init(bmh, xPos, yPos, this);
	_MassDialog->GetClientRect(&r);	
	yPos += r.bottom + 3;
	


	m_MaxNbParticles.GetWindowRect(&r);
	_MaxNbParticlesWrapper.TreeCtrl = _ParticleDlg->ParticleTreeCtrl;
	_MaxNbParticlesWrapper.Located = _Located;
	_MaxNbParticlesWrapper.Node = _Node;
	_MaxNbParticles->setWrapper(&_MaxNbParticlesWrapper);	
	_MaxNbParticles->init(r.left - pr.left, r.top - pr.top, this);

	if (_Located->getOwner())
	{
		_MaxNbParticles->EnableWindow(_Located->getOwner()->getAutoCountFlag() ? FALSE : TRUE);
	}
	
	
	m_MatrixMode				= (int) _Located->getMatrixMode();
	m_LimitedLifeTime			= _Located->getLastForever() ? FALSE : TRUE;
	m_TriggerOnDeath			= _Located->isTriggerOnDeathEnabled();
	updateTriggerOnDeath();
	

	m_DisgradeWithLOD = _Located->hasLODDegradation();
	updateIntegrable();
	updateTriggerOnDeath();
	UpdateData(FALSE);
	ShowWindow(SW_SHOW);
}

//****************************************************************************************************************
void CLocatedProperties::updateTriggerOnDeath(void)
{
	nlassert(_Located);
	BOOL enable = !_Located->getLastForever();
	m_TriggerOnDeathCtrl.EnableWindow(enable);
	m_EditTriggerOnDeath.EnableWindow(( (enable ? true : false /* MSVC6 warning */ )
										&& _Located->isTriggerOnDeathEnabled())
									  );
}

//****************************************************************************************************************
void CLocatedProperties::updateIntegrable(void) 
{
	m_ParametricMotion			= _Located->isParametricMotionEnabled();
	m_ParametricMotionCtrl.EnableWindow(_Located->supportParametricMotion());
}

//****************************************************************************************************************
void CLocatedProperties::OnLimitedLifeTime() 
{	
	UpdateData();	
	if (!m_LimitedLifeTime)
	{
		bool forceApplied = false;
		// check that no force are applied on the located
		std::vector<NL3D::CPSTargetLocatedBindable *> targeters;
		_Located->getOwner()->getTargeters(_Located, targeters);
		for(uint k = 0; k < targeters.size(); ++k)
		{
			if (targeters[k]->getType() == NL3D::PSForce)
			{
				forceApplied = true;
				break;
			}
		}		
		if (forceApplied)
		{
			CString caption;
			CString mess;
			caption.LoadString(IDS_WARNING);
			mess.LoadString(IDS_HAS_FORCE_APPLIED);
			if (MessageBox((LPCTSTR) mess, (LPCTSTR) caption, MB_OKCANCEL) != IDOK)
			{
				m_LimitedLifeTime = true;
				UpdateData(FALSE);
				return;
			}
		}
		if (_Located->setLastForever())
		{		
			_LifeDialog->EnableWindow(FALSE);
		}
		else
		{
			CString mess;
			mess.LoadString(IDS_PS_NO_FINITE_DURATION);
			CString errorStr;
			errorStr.LoadString(IDS_ERROR);
			MessageBox((LPCTSTR) mess, (LPCTSTR) errorStr, MB_ICONEXCLAMATION);
			m_LimitedLifeTime = TRUE;
			UpdateData(FALSE);
		}
	}
	else
	{
		_Located->setInitialLife(_Located->getInitialLife());
		_LifeDialog->EnableWindow(TRUE);
	}
	updateTriggerOnDeath();
	_ParticleDlg->StartStopDlg->resetAutoCount(_Node);
	touchPSState();
}

//****************************************************************************************************************
void CLocatedProperties::OnDisgradeWithLod() 
{
	UpdateData();
	_Located->forceLODDegradation(m_DisgradeWithLOD ? true : false /* to avoid warning from MSVC */);
	touchPSState();
}

//****************************************************************************************************************
void CLocatedProperties::OnParametricMotion() 
{
	UpdateData();
	_Located->enableParametricMotion(m_ParametricMotion ? true : false);
	touchPSState();
}

//****************************************************************************************************************
void CLocatedProperties::OnEditTriggerOnDeath() 
{
	UpdateData();
	CLBExternIDDlg 	dlg(_Located->getTriggerEmitterID());
	INT_PTR res = dlg.DoModal();
	if ( res == IDOK )
	{
		if (dlg.getNewID() != _Located->getTriggerEmitterID())
		{		
			_Located->setTriggerEmitterID( dlg.getNewID() );
			touchPSState();
		}
	}
}

//****************************************************************************************************************
void CLocatedProperties::OnTriggerOnDeath() 
{
	UpdateData();
	_Located->enableTriggerOnDeath(m_TriggerOnDeath ? true : false /* MSVC6 wraning */);
	updateTriggerOnDeath();
	touchPSState();
}

//****************************************************************************************************************
void CLocatedProperties::goPostRender()
{
	nlassert(_ParticleDlg);
	if (_Located->getOwner()->getAutoCountFlag()) 
	{
		// update number of particle from ps
		_MaxNbParticles->update();
	}
	// in all cases, show the current number of particles being used
	GetDlgItem(IDC_CURR_NUM_PARTS)->SetWindowText(NLMISC::toString(_Located->getSize()).c_str());
}

//****************************************************************************************************************
void CLocatedProperties::OnAssignCount() 
{	
	_Located->resize(_Located->getSize()); // set new max size
	_MaxNbParticles->update();
	touchPSState();
}

//****************************************************************************************************************
void CLocatedProperties::OnSelchangeMatrixMode() 
{
	UpdateData();
	nlassert(_Located);
	_Located->setMatrixMode((NL3D::TPSMatrixMode) m_MatrixMode);
	updateIntegrable();
	UpdateData(FALSE);
	touchPSState();
}

//****************************************************************************************************************
void CLocatedProperties::touchPSState()
{
	if (_Node) _Node->setModified(true);
}


