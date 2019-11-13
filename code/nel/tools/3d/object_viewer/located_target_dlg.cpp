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


#include "nel/3d/particle_system.h"
#include "object_viewer.h"
#include "located_target_dlg.h"
#include "collision_zone_dlg.h"
#include "editable_range.h"
#include "attrib_dlg.h"
#include "direction_attr.h"

/////////////////////////////////////////////////////////////////////////////
// CLocatedTargetDlg dialog


CLocatedTargetDlg::CLocatedTargetDlg(CParticleWorkspace::CNode *ownerNode,
									 NL3D::CPSTargetLocatedBindable *lbTarget,
									 CParticleDlg *particleDlg) : _Node(ownerNode), _LBTarget(lbTarget), _ParticleDlg(particleDlg)
{
	nlassert(particleDlg);
	//{{AFX_DATA_INIT(CLocatedTargetDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLocatedTargetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedTargetDlg)
	DDX_Control(pDX, IDC_AVAILABLE_TARGET, m_AvailableTargets);
	DDX_Control(pDX, IDC_TARGET, m_Targets);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedTargetDlg, CDialog)
	//{{AFX_MSG_MAP(CLocatedTargetDlg)
	ON_BN_CLICKED(IDC_ADD_TARGET, OnAddTarget)
	ON_BN_CLICKED(IDC_REMOVE_TARGET, OnRemoveTarget)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocatedTargetDlg message handlers


void CLocatedTargetDlg::init(CWnd* pParent)
{
	Create(IDD_LOCATED_TARGET_DLG, pParent);
	ShowWindow(SW_SHOW);
}


void CLocatedTargetDlg::OnAddTarget() 
{
	UpdateData();
	int totalCount = m_AvailableTargets.GetCount();
	nlassert(totalCount);
	std::vector<int> indexs;
	indexs.resize(totalCount);
	int selCount = m_AvailableTargets.GetSelItems(totalCount, &indexs[0]);

	std::sort(indexs.begin(), indexs.begin() + selCount); // we never know ...

	// check that force isn't applied on a forever lasting object
	if (dynamic_cast<NL3D::CPSForce *>(_LBTarget))
	{	
		bool forEverLastingTarget = false;
		for (int k = 0; k < selCount; ++k)
		{
			NL3D::CPSLocated *loc = (NL3D::CPSLocated *) m_AvailableTargets.GetItemData(indexs[k] - k);
			nlassert(loc);
			if (loc->getLastForever())
			{
				forEverLastingTarget = true;
				break;
			}
		}
		if (forEverLastingTarget)
		{	
			CString warningStr;
			CString messStr;
			warningStr.LoadString(IDS_WARNING);
			messStr.LoadString(IDS_FORCE_APPLIED_ON_OBJECT_THAT_LAST_FOREVER);
			if (MessageBox((LPCTSTR) messStr, (LPCTSTR) warningStr, MB_OKCANCEL) != IDOK)
			{
				return;
			}
		}
	}
	//
	for (int k = 0; k < selCount; ++k)
	{
		NL3D::CPSLocated *loc = (NL3D::CPSLocated *) m_AvailableTargets.GetItemData(indexs[k] - k);
		nlassert(loc);
		_LBTarget->attachTarget(loc);
		m_AvailableTargets.DeleteString(indexs[k] - k);
		int l = m_Targets.AddString(nlUtf8ToTStr(loc->getName()));
		m_Targets.SetItemData(l, (DWORD_PTR) loc);
	}	
	UpdateData(FALSE);
	//
	updateModifiedFlag();
}

void CLocatedTargetDlg::OnRemoveTarget() 
{
	UpdateData();	
	int totalCount = m_Targets.GetCount();
	nlassert(totalCount);
	std::vector<int> indexs;
	indexs.resize(totalCount);
	int selCount = m_Targets.GetSelItems(totalCount, &indexs[0]);
	std::sort(indexs.begin(), indexs.begin() + selCount); // we never know ...
	for (int k = 0; k < selCount; ++k)
	{
		NL3D::CPSLocated *loc = (NL3D::CPSLocated *) m_Targets.GetItemData(indexs[k] - k);
		nlassert(loc);
		_LBTarget->detachTarget(loc);
		m_Targets.DeleteString(indexs[k] - k);
		int l = m_AvailableTargets.AddString(nlUtf8ToTStr(loc->getName()));
	
		m_AvailableTargets.SetItemData(l, (DWORD_PTR) loc);
	}
	UpdateData(FALSE);	
	updateModifiedFlag();
}

BOOL CLocatedTargetDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	RECT r;

	uint k;
	uint nbTarg = _LBTarget->getNbTargets();

	m_Targets.InitStorage(nbTarg, 128);

	std::set<NL3D::CPSLocated *> targetSet;

	// fill the box thta tells us what the target are
	for(k = 0; k < nbTarg; ++k)
	{
		m_Targets.AddString(nlUtf8ToTStr(_LBTarget->getTarget(k)->getName()));
		m_Targets.SetItemData(k, (DWORD_PTR) _LBTarget->getTarget(k));
		targetSet.insert(_LBTarget->getTarget(k));
	};

	// fill abox with the available targets
	NL3D::CParticleSystem  *ps = _LBTarget->getOwner()->getOwner();

	uint nbLocated = ps->getNbProcess();


	
	m_AvailableTargets.InitStorage(nbTarg, 128);
	for (k = 0; k < nbLocated; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k));
		if (loc)
		{
			if (targetSet.find(loc) == targetSet.end())
			{
				int l = m_AvailableTargets.AddString(nlUtf8ToTStr(loc->getName()));
				m_AvailableTargets.SetItemData(l, (DWORD_PTR) loc);
			}
		}
	}

	const sint posX = 5;
	sint posY = 180;

	// collision zone case

	if (dynamic_cast<NL3D::CPSZone *>(_LBTarget))
	{
		CCollisionZoneDlg *czd = new CCollisionZoneDlg(_Node, dynamic_cast<NL3D::CPSZone *>(_LBTarget), _ParticleDlg);
		pushWnd(czd);
		czd->init(posX, posY, this);
	}


	// force with intensity case

	if (dynamic_cast<NL3D::CPSForceIntensity *>(_LBTarget))
	{
		_ForceIntensityWrapper.F = dynamic_cast<NL3D::CPSForceIntensity *>(_LBTarget);
		CAttribDlgFloat *fi = new CAttribDlgFloat(std::string("FORCE INTENSITY"), _Node, 0, 100);
		pushWnd(fi);			
		fi->setWrapper(&_ForceIntensityWrapper);
		fi->setSchemeWrapper(&_ForceIntensityWrapper);

		HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_FORCE_INTENSITY));
		fi->init(bmh, posX, posY, this);
		

		fi->GetClientRect(&r);
		posY += r.bottom + 3;			
	}

	// vortex (to tune viscosity)
	if (dynamic_cast<NL3D::CPSCylindricVortex *>(_LBTarget))
	{
		CEditableRangeFloat *rv = new CEditableRangeFloat(std::string("RADIAL_VISCOSITY"), _Node, 0, 1);
		pushWnd(rv);
		_RadialViscosityWrapper.V = dynamic_cast<NL3D::CPSCylindricVortex *>(_LBTarget);
		rv->setWrapper(&_RadialViscosityWrapper);
		rv->init(posX + 140, posY, this);
		CStatic *s = new CStatic;
		pushWnd(s);
		s->Create(_T("Radial viscosity : "), SS_LEFT, CRect(posX, posY, posX + 139, posY + 32), this);
		s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));		
		s->ShowWindow(SW_SHOW);


		rv->GetClientRect(&r);
		posY += r.bottom + 3;

		CEditableRangeFloat *tv = new CEditableRangeFloat(std::string("TANGENTIAL_VISCOSITY"), _Node, 0, 1);
		pushWnd(tv);
		_TangentialViscosityWrapper.V = dynamic_cast<NL3D::CPSCylindricVortex *>(_LBTarget);
		tv->setWrapper(&_TangentialViscosityWrapper);
		tv->init(posX + 140, posY, this);

		s = new CStatic;			
		pushWnd(s);
		s->Create(_T("Tangential Viscosity : "), SS_LEFT, CRect(posX, posY, posX + 139, posY + 32), this);
		s->ShowWindow(SW_SHOW);

		tv->GetClientRect(&r);
		posY += r.bottom + 3;
	}

	// deals with emitters that have a direction
	if (dynamic_cast<NL3D::CPSDirection *>(_LBTarget))
	{
		CDirectionAttr *da = new CDirectionAttr(std::string("DIRECTION"));
		pushWnd(da);		
		_DirectionWrapper.E = dynamic_cast<NL3D::CPSDirection *>(_LBTarget);
		da->setWrapper(&_DirectionWrapper);
		da->setDirectionWrapper(dynamic_cast<NL3D::CPSDirection *>(_LBTarget));
		da->init(posX, posY, this);
		da->GetClientRect(&r);
		posY += r.bottom;
	}

	// Brownian (to tune parametric factor)
	if (dynamic_cast<NL3D::CPSBrownianForce *>(_LBTarget))
	{
		CEditableRangeFloat *rv = new CEditableRangeFloat(std::string("PARAMETRIC_FACTOR"), _Node, 0, 64);
		pushWnd(rv);
		_ParamFactorWrapper.F = static_cast<NL3D::CPSBrownianForce *>(_LBTarget);
		rv->setWrapper(&_ParamFactorWrapper);
		rv->init(posX + 140, posY, this);
		CStatic *s = new CStatic;
		pushWnd(s);
		s->Create(_T("Parametric factor : "), SS_LEFT, CRect(posX, posY, posX + 139, posY + 40), this);
		s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));		
		s->ShowWindow(SW_SHOW);

		rv->GetClientRect(&r);
		posY += r.bottom + 3;
	
	}



	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
