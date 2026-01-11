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

#if !defined(AFX_EMITTER_DLG_H__7D6DB229_8E72_4A60_BD03_8A3EF3F506CF__INCLUDED_)
#define AFX_EMITTER_DLG_H__7D6DB229_8E72_4A60_BD03_8A3EF3F506CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_emitter.h"
#include "start_stop_particle_system.h"

namespace NL3D
{
	class NL3D::CPSEmitter;
}

#include "attrib_dlg.h"
#include "dialog_stack.h"
#include "particle_workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CEmitterDlg dialog

class CEmitterDlg : public CDialog, public CDialogStack
{
public:
	// this enum match the option in the combo box that allow to choose how the direction of emission is computed.
	enum TDirectionMode { Default = 0, AlignOnEmitterDirection, InWorld, LocalToSystem, LocalToFatherSkeleton };
public:
	CEmitterDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSEmitter *emitter, CParticleDlg *particleDlg);   // standard constructor

	~CEmitterDlg();


	void init(CWnd* pParent = NULL);


// Dialog Data
	//{{AFX_DATA(CEmitterDlg)
	enum { IDD = IDD_EMITTER_DIALOG };
	CComboBox	m_DirectionModeCtrl;
	CComboBox	m_EmissionTypeCtrl;
	CComboBox	m_EmittedTypeCtrl;	
	BOOL		m_ConsistentEmission;
	BOOL		m_BypassAutoLOD;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmitterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// the emitter being edited
	NL3D::CPSEmitter	 *_Emitter;	
	CParticleWorkspace::CNode *_Node;
	CAttribDlgFloat		 *_PeriodDlg;
	CAttribDlgUInt		 *_GenNbDlg;
	CAttribDlgFloat		 *_StrenghtModulateDlg;
	CEditableRangeFloat  *_SpeedInheritanceFactorDlg;
	CEditableRangeFloat  *_DelayedEmissionDlg;
	CEditableRangeUInt   *_MaxEmissionCountDlg;
	//
	CParticleDlg		 *_ParticleDlg;

	// Generated message map functions
	//{{AFX_MSG(CEmitterDlg)
	afx_msg void OnSelchangeEmittedType();
	afx_msg void OnSelchangeTypeOfEmission();
	virtual BOOL OnInitDialog();	
	afx_msg void OnConvertSpeedVectorFromEmitterBasis();
	afx_msg void OnConsistentEmission();
	afx_msg void OnBypassAutoLOD();
	afx_msg void OnSelchangeDirectionMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	/// disable / enable the period edition dialog if it is needed
	void updatePeriodDlg(void);
	// Choose the right entry for the list box that display the emitted type
	void initEmittedType();

	//////////////////////////////////////////////
	//	WRAPPERS TO EDIT THE EMITTER PROPERTIES //
	//////////////////////////////////////////////



		////////////////////////
		// period of emission //
		////////////////////////

			struct CPeriodWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   CParticleWorkspace::CNode *Node;
			   NL3D::CPSEmitter *E;
			   CStartStopParticleSystem *SSPS;
			   float get(void) const { return E->getPeriod(); }
			   void set(const float &v) { E->setPeriod(v); SSPS->resetAutoCount(Node); }
			   scheme_type *getScheme(void) const { return E->getPeriodScheme(); }
			   void setScheme(scheme_type *s) { E->setPeriodScheme(s); SSPS->resetAutoCount(Node); }
			} _PeriodWrapper;

		//////////////////////////////////////////////
		// number of particle to generate each time //
		//////////////////////////////////////////////

			struct CGenNbWrapper : public IPSWrapperUInt, IPSSchemeWrapperUInt 
			{
			   NL3D::CPSEmitter *E;
			   CStartStopParticleSystem *SSPS;
			   CParticleWorkspace::CNode *Node;
			   uint32 get(void) const { return E->getGenNb(); }
			   void set(const uint32 &v) { E->setGenNb(v); SSPS->resetAutoCount(Node); }
			   scheme_type *getScheme(void) const { return E->getGenNbScheme(); }
			   void setScheme(scheme_type *s) { E->setGenNbScheme(s); SSPS->resetAutoCount(Node); }
			} _GenNbWrapper;

		////////////////////////////////////////////////////////
		// wrappers to emitters that have strenght modulation //
		////////////////////////////////////////////////////////

			struct CModulateStrenghtWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   NL3D::CPSModulatedEmitter *E;
			   float get(void) const { return E->getEmitteeSpeed(); }
			   void set(const float &v) { E->setEmitteeSpeed(v); }
			   scheme_type *getScheme(void) const { return E->getEmitteeSpeedScheme(); }
			   void setScheme(scheme_type *s) { E->setEmitteeSpeedScheme(s); }
			} _ModulatedStrenghtWrapper;

		//////////////////////////////////////////////////
		// wrappers to set the speed inheritance factor //
		//////////////////////////////////////////////////

			struct CSpeedInheritanceFactorWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSEmitter *E;
			   float get(void) const { return E->getSpeedInheritanceFactor(); }
			   void set(const float &f) { E->setSpeedInheritanceFactor(f); }	
			} _SpeedInheritanceFactorWrapper;

		////////////////////////////////////////////////
		// wrappers to tune the direction of emitters //
		////////////////////////////////////////////////

			struct CDirectionWrapper : public IPSWrapper<NLMISC::CVector>
			{
			   NL3D::CPSDirection *E;
			   NLMISC::CVector get(void) const { return E->getDir(); }
			   void set(const NLMISC::CVector &d){ E->setDir(d); }	
			} _DirectionWrapper;

		//////////////////////////////////////////////
		// wrapper to tune the radius of an emitter //
		//////////////////////////////////////////////
		
			struct CConicEmitterRadiusWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSEmitterConic *E;
			   float get(void) const { return E->getRadius(); }
			   void set(const float &f) { E->setRadius(f); }	
			} _ConicEmitterRadiusWrapper;


		//////////////////////////////////////////////
		// wrapper to tune delayed emission		    //
		//////////////////////////////////////////////
		
			struct CDelayedEmissionWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSEmitter *E;
			   CStartStopParticleSystem *SSPS;
			   CParticleWorkspace::CNode *Node;
			   float get(void) const { return E->getEmitDelay(); }
			   void set(const float &f) { E->setEmitDelay(f); SSPS->resetAutoCount(Node); }	
			} _DelayedEmissionWrapper;


		////////////////////////////////////////////////
		// wrapper to tune max number of emissions	  //
		////////////////////////////////////////////////
		
			struct CMaxEmissionCountWrapper : public IPSWrapperUInt
			{
			   CEditableRangeUInt   *MaxEmissionCountDlg;
			   NL3D::CPSEmitter *E;
			   CStartStopParticleSystem *SSPS;
			   CParticleWorkspace::CNode *Node;
			   HWND	HWnd;
			   uint32 get(void) const { return E->getMaxEmissionCount(); }
			   void set(const uint32 &count);
			} _MaxEmissionCountWrapper;


	// contains pointers to the located
	std::vector<NL3D::CPSLocated *> _LocatedList;

	void updateModifiedFlag() { if (_Node) _Node->setModified(true); }
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTER_DLG_H__7D6DB229_8E72_4A60_BD03_8A3EF3F506CF__INCLUDED_)
