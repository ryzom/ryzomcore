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

#if !defined(AFX_LOCATED_TARGET_DLG_H__FA197835_AE71_4057_88A4_48F28A01E367__INCLUDED_)
#define AFX_LOCATED_TARGET_DLG_H__FA197835_AE71_4057_88A4_48F28A01E367__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 



#include "nel/3d/ps_located.h"
#include "nel/3d/ps_force.h"
#include "dialog_stack.h"
#include "ps_wrapper.h"
#include "particle_workspace.h"


class CParticleDlg;


/////////////////////////////////////////////////////////////////////////////
// CLocatedTargetDlg dialog

class CLocatedTargetDlg : public CDialog, public CDialogStack
{
// Construction
public:
	CLocatedTargetDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSTargetLocatedBindable *blTarget, CParticleDlg *particleDlg);   // standard constructor



	// init the dialog with the given parent
	void init(CWnd* pParent);

// Dialog Data
	//{{AFX_DATA(CLocatedTargetDlg)
	enum { IDD = IDD_LOCATED_TARGET_DLG };
	CListBox	m_AvailableTargets;
	CListBox	m_Targets;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocatedTargetDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// the target we're focusing on
	NL3D::CPSTargetLocatedBindable *_LBTarget;
	CParticleDlg				   *_ParticleDlg;
	CParticleWorkspace::CNode	   *_Node;

	// Generated message map functions
	//{{AFX_MSG(CLocatedTargetDlg)
	afx_msg void OnAddTarget();
	afx_msg void OnRemoveTarget();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//////////////////////////////////////////////
	// wrapper to tune the intensity of a force //
	//////////////////////////////////////////////
	struct CForceIntensityWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSForceIntensity *F;
		float get(void) const { return F->getIntensity(); }
		void set(const float &value) {F->setIntensity(value); }
		scheme_type *getScheme(void) const { return F->getIntensityScheme(); }
		void setScheme(scheme_type *s) {F->setIntensityScheme(s); }
	} _ForceIntensityWrapper;

	///////////////////////////////////////////////////////
	// wrapper to tune the radial viscosity for vortices //
	///////////////////////////////////////////////////////


	struct CRadialViscosityWrapper : public IPSWrapperFloat
	{
		NL3D::CPSCylindricVortex *V;
		float get(void) const { return V->getRadialViscosity(); }
		void set(const float &value) { V->setRadialViscosity(value); }		
	} _RadialViscosityWrapper;

	///////////////////////////////////////////////////////
	// wrapper to tune the tangential viscosity for vortices //
	///////////////////////////////////////////////////////


	struct CTangentialViscosityWrapper : public IPSWrapperFloat
	{
		NL3D::CPSCylindricVortex *V;
		float get(void) const { return V->getTangentialViscosity(); }
		void set(const float &value) { V->setTangentialViscosity(value); }		
	} _TangentialViscosityWrapper;

	////////////////////////////////////
	// wrappers to tune the direction //
	////////////////////////////////////

	struct CDirectionWrapper : public IPSWrapper<NLMISC::CVector>
	{
	   NL3D::CPSDirection *E;
	   NLMISC::CVector get(void) const { return E->getDir(); }
	   void set(const NLMISC::CVector &d){ E->setDir(d); }	
	} _DirectionWrapper;

	//////////////////////////////////////////////////////////////
	// wrappers to tune the parametric factor of brownian force //
	//////////////////////////////////////////////////////////////

	struct CParamFactorWrapper : public IPSWrapperFloat
	{
	   NL3D::CPSBrownianForce *F;
	   float get(void) const { return F->getParametricFactor(); }
	   void set(const float &f){ F->setParametricFactor(f); }	
	} _ParamFactorWrapper;


	void updateModifiedFlag() { if (_Node) _Node->setModified(true); }
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATED_TARGET_DLG_H__FA197835_AE71_4057_88A4_48F28A01E367__INCLUDED_)
