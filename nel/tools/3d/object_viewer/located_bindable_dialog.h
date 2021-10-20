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

#if !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
#define AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	class CPSLocatedBindable;
}



#include "nel/misc/rgba.h"
//
#include "nel/3d/texture.h"
#include "nel/3d/ps_plane_basis.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_particle2.h"
//
#include "ps_wrapper.h"
#include "dialog_stack.h"

using NLMISC::CRGBA;

class CParticleDlg;



/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog dialog

class CLocatedBindableDialog : public CDialog, CDialogStack
{
// Construction
public:
	// create this dialog to edit the given bindable
	CLocatedBindableDialog(CParticleWorkspace::CNode *ownerNode,  NL3D::CPSLocatedBindable *bindable);   // standard constructor

	/// dtor
	~CLocatedBindableDialog();


	// init the dialog as a child of the given wnd
	void init(CParticleDlg* pParent);
// Dialog Data
	//{{AFX_DATA(CLocatedBindableDialog)
	enum { IDD = IDD_LOCATED_BINDABLE };
	CEdit	m_ZBias;
	CComboBox	m_BlendingMode;
	BOOL	m_IndependantSizes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocatedBindableDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL




// Implementation
protected:

	// enables or disabled controls for independant sizes
	void updateIndependantSizes(void);
	//  update zbias from edit box & display error mb if needed
	void updateZBias();
	/// create the size control, or update it if it has been created. It returns the heivht of the control
	uint updateSizeControl();

	CParticleWorkspace::CNode		*_Node;
	NL3D::CPSLocatedBindable		*_Bindable;		// the bindable being edited
	CParticleDlg					*_ParticleDlg; // the dialog that owns us
	class CAttribDlgFloat			*_SizeCtrl;	// the control used for size
	sint							_SizeCtrlX;	// x position of the control used for size
	sint							_SizeCtrlY;	// x position of the control used for size	
	
	// look at specific : pointer on windows to edit motion blur params
	std::vector<CWnd *>				_MotionBlurWnd;


	// Generated message map functions
	//{{AFX_MSG(CLocatedBindableDialog)
	afx_msg void OnSelchangeBlendingMode();
	afx_msg void OnIndeSizes();
	afx_msg void OnSizeWidth();
	afx_msg void OnSizeHeight();
	afx_msg void OnNoAutoLod();
	afx_msg void OnGlobalColorLighting();
	afx_msg void OnAlignOnMotion();
	afx_msg void OnZtest();
	afx_msg void OnChangeZbias();
	afx_msg void OnKillfocusZbias();
	afx_msg void OnZalign();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	//////////////////////////////////////////////
	// wrappers to various element of bindables //
	//////////////////////////////////////////////

		//////////
		// size //
		//////////
			struct CSizeWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   NL3D::CPSSizedParticle *S;
			   float get(void) const { return S->getSize(); }
			   void set(const float &v) { S->setSize(v); }
			   scheme_type *getScheme(void) const { return S->getSizeScheme(); }
			   void setScheme(scheme_type *s) { S->setSizeScheme(s); }
			} _SizeWrapper;
			
		///////////
		// color //
		///////////
			struct CColorWrapper : public IPSWrapperRGBA, IPSSchemeWrapperRGBA
			{
			   NL3D::CPSColoredParticle *S;
			   CRGBA get(void) const { return S->getColor(); }
			   void set(const CRGBA &v) { S->setColor(v); }
			   scheme_type *getScheme(void) const { return S->getColorScheme(); }
			   void setScheme(scheme_type *s) { S->setColorScheme(s); }
			} _ColorWrapper;
			
		//////////////
		// angle 2D //
		//////////////
			struct CAngle2DWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
			{
			   NL3D::CPSRotated2DParticle *S;
			   float get(void) const { return S->getAngle2D(); }
			   void set(const float &v) { S->setAngle2D(v); }
			   scheme_type *getScheme(void) const { return S->getAngle2DScheme(); }
			   void setScheme(scheme_type *s) { S->setAngle2DScheme(s); }
			} _Angle2DWrapper;

		/////////////////
		// plane basis //
		/////////////////
			struct CPlaneBasisWrapper : public IPSWrapper<NL3D::CPlaneBasis>, IPSSchemeWrapper<NL3D::CPlaneBasis>
			{
			   NL3D::CPSRotated3DPlaneParticle *S;
			   NL3D::CPlaneBasis get(void) const { return S->getPlaneBasis(); }
			   void set(const NL3D::CPlaneBasis &p) { S->setPlaneBasis(p); }
			   scheme_type *getScheme(void) const { return S->getPlaneBasisScheme(); }
			   void setScheme(scheme_type *s) { S->setPlaneBasisScheme(s); }
			} _PlaneBasisWrapper;


	

		///////////////////////
		// motion blur coeff //
		///////////////////////

			struct CMotionBlurCoeffWrapper : public IPSWrapperFloat
			{
				NL3D::CPSFaceLookAt *P;
			   float get(void) const { return P->getMotionBlurCoeff(); }
			   void set(const float &v) { P->setMotionBlurCoeff(v); }
			}  _MotionBlurCoeffWrapper;

			struct CMotionBlurThresholdWrapper : public IPSWrapperFloat
			{
				NL3D::CPSFaceLookAt *P;
			   float get(void) const { return P->getMotionBlurThreshold(); }
			   void set(const float &v) { P->setMotionBlurThreshold(v); }
			}  _MotionBlurThresholdWrapper;

		///////////////
		// fanlight  //
		///////////////
			struct CFanLightWrapper : public IPSWrapperUInt
			{
				NL3D::CPSFanLight *P;
			  uint32 get(void) const { return P->getNbFans(); }
			   void set(const uint32 &v) { P->setNbFans(v); }
			}  _FanLightWrapper;

			struct CFanLightSmoothnessWrapper : public IPSWrapperUInt
			{
				NL3D::CPSFanLight *P;
			  uint32 get(void) const { return P->getPhaseSmoothness(); }
			   void set(const uint32 &v) { P->setPhaseSmoothness(v); }
			}  _FanLightSmoothnessWrapper;

			struct CFanLightPhase : public IPSWrapperFloat
			{
				NL3D::CPSFanLight *P;
				float get(void) const { return P->getPhaseSpeed(); }
			    void set(const float &v) { P->setPhaseSpeed(v); }
			}  _FanLightPhaseWrapper;
			struct CFanLightIntensityWrapper : public IPSWrapperFloat
			{
				NL3D::CPSFanLight *P;
				float get(void) const { return P->getMoveIntensity(); }
			    void set(const float &v) { P->setMoveIntensity(v); }
			}  _FanLightIntensityWrapper;

		///////////////////////
		// ribbon / tail dot //
		///////////////////////
			
			struct CTailParticleWrapper : public IPSWrapperUInt
			{
				NL3D::CPSTailParticle *P;
			    uint32 get(void) const { return P->getTailNbSeg(); }
			    void set(const uint32 &v) { P->setTailNbSeg(v); }
			}  _TailParticleWrapper;

		//////////////////////////////////////
		// duration of segment for a ribbon //
		//////////////////////////////////////		
			struct CSegDurationWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSRibbonBase *R;
			   float get(void) const { return R->getSegDuration(); }
			   void set(const float &v) { R->setSegDuration(v); }
			} _SegDurationWrapper;
		
		/////////////////////////////
		//		shockwave          //
		/////////////////////////////
	
			struct CRadiusCutWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSShockWave *S;
			   float get(void) const { return S->getRadiusCut(); }
			   void set(const float &v) { S->setRadiusCut(v); }
			} _RadiusCutWrapper;

			struct CShockWaveNbSegWrapper : public IPSWrapperUInt
			{
			   NL3D::CPSShockWave *S;
			   uint32 get(void) const { return S->getNbSegs(); }
			   void set(const uint32 &v) { S->setNbSegs(v); }
			} _ShockWaveNbSegWrapper;

			struct CShockWaveUFactorWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSShockWave *S;
			   float get(void) const { return S->getUFactor(); }
			   void set(const float &v) { S->setUFactor(v); }
			} _ShockWaveUFactorWrapper;


		////////////////////////
		// unanimated texture //
		////////////////////////

			struct CTextureNoAnimWrapper : public IPSWrapperTexture
			{
				NL3D::CPSTexturedParticleNoAnim *TP;
				virtual NL3D::ITexture *get(void) { return TP->getTexture(); }
				virtual void set(NL3D::ITexture *t) { TP->setTexture(t); }
			} _TextureNoAnimWrapper;	

		//////////////////////////////
		// u / v factors for ribbon //
		//////////////////////////////

			struct CRibbonUFactorWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSRibbon *R;
			   float get(void) const { return R->getUFactor(); }
			   void set(const float &u) { R->setTexFactor(u, R->getVFactor()); }
			} _RibbonUFactorWrapper;

			struct CRibbonVFactorWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSRibbon *R;
			   float get(void) const { return R->getVFactor(); }
			   void set(const float &v) { R->setTexFactor(R->getUFactor(), v); }
			} _RibbonVFactorWrapper;

	void updateModifiedFlag() { if (_Node) _Node->setModified(true); }
	void touchPSState();
	void updateValidWndForAlignOnMotion(bool align);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
