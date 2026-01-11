/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef LOCATED_BINDABLE_PAGE_H
#define LOCATED_BINDABLE_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_located_bindable_form.h"

// STL includes

// NeL includes
#include "nel/misc/rgba.h"
#include "nel/3d/texture.h"
#include "nel/3d/ps_plane_basis.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_particle2.h"

// Project includes
#include "ps_wrapper.h"
#include "particle_node.h"

namespace NLQT
{

/**
@class CLocatedBindablePage
@brief Page for QStackWidget, to edit located bindables in a particle system
*/
class CLocatedBindablePage: public QWidget
{
	Q_OBJECT

public:
	CLocatedBindablePage(QWidget *parent = 0);
	~CLocatedBindablePage();

	/// Set the located bindable to edit.
	void setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable);

private Q_SLOTS:
	void setAutoLOD(bool state);
	void setBlendMode(int index);
	void setGlobalColorLight(bool state);
	void setIndependantSize(bool state);
	void setAlignOnMotion(bool state);
	void setZTest(bool state);
	void setZAlign(bool state);
	void setZBias(double value);
	void setSizeWidth();
	void setHint(bool state);
	void setRotSpeedMax(double value);
	void setRotSpeedMin(double value);
	void setNumModels(int value);
	void setUseHermitteInterpolation(bool state);
	void setConstantLength(bool state);
	void setTrailCoordSystem(int index);

private:

	/// Wrappers to various element of bindables

	/// Size
	struct CSizeWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSSizedParticle *S;
		float get(void) const
		{
			return S->getSize();
		}
		void set(const float &v)
		{
			S->setSize(v);
		}
		scheme_type *getScheme(void) const
		{
			return S->getSizeScheme();
		}
		void setScheme(scheme_type *s)
		{
			S->setSizeScheme(s);
		}
	} _SizeWrapper;

	/// Color
	struct CColorWrapper : public IPSWrapperRGBA, IPSSchemeWrapperRGBA
	{
		NL3D::CPSColoredParticle *S;
		NLMISC::CRGBA get(void) const
		{
			return S->getColor();
		}
		void set(const NLMISC::CRGBA &v)
		{
			S->setColor(v);
		}
		scheme_type *getScheme(void) const
		{
			return S->getColorScheme();
		}
		void setScheme(scheme_type *s)
		{
			S->setColorScheme(s);
		}
	} _ColorWrapper;

	/// Angle 2D
	struct CAngle2DWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSRotated2DParticle *S;
		float get(void) const
		{
			return S->getAngle2D();
		}
		void set(const float &v)
		{
			S->setAngle2D(v);
		}
		scheme_type *getScheme(void) const
		{
			return S->getAngle2DScheme();
		}
		void setScheme(scheme_type *s)
		{
			S->setAngle2DScheme(s);
		}
	} _Angle2DWrapper;

	/// Plane basis
	struct CPlaneBasisWrapper : public IPSWrapper<NL3D::CPlaneBasis>, IPSSchemeWrapper<NL3D::CPlaneBasis>
	{
		NL3D::CPSRotated3DPlaneParticle *S;
		NL3D::CPlaneBasis get(void) const
		{
			return S->getPlaneBasis();
		}
		void set(const NL3D::CPlaneBasis &p)
		{
			S->setPlaneBasis(p);
		}
		scheme_type *getScheme(void) const
		{
			return S->getPlaneBasisScheme();
		}
		void setScheme(scheme_type *s)
		{
			S->setPlaneBasisScheme(s);
		}
	} _PlaneBasisWrapper;

	/// Motion blur coeff
	struct CMotionBlurCoeffWrapper : public IPSWrapperFloat
	{
		NL3D::CPSFaceLookAt *P;
		float get(void) const
		{
			return P->getMotionBlurCoeff();
		}
		void set(const float &v)
		{
			P->setMotionBlurCoeff(v);
		}
	}  _MotionBlurCoeffWrapper;
	struct CMotionBlurThresholdWrapper : public IPSWrapperFloat
	{
		NL3D::CPSFaceLookAt *P;
		float get(void) const
		{
			return P->getMotionBlurThreshold();
		}
		void set(const float &v)
		{
			P->setMotionBlurThreshold(v);
		}
	}  _MotionBlurThresholdWrapper;

	/// Fanlight
	struct CFanLightWrapper : public IPSWrapperUInt
	{
		NL3D::CPSFanLight *P;
		uint32 get(void) const
		{
			return P->getNbFans();
		}
		void set(const uint32 &v)
		{
			P->setNbFans(v);
		}
	}  _FanLightWrapper;

	struct CFanLightSmoothnessWrapper : public IPSWrapperUInt
	{
		NL3D::CPSFanLight *P;
		uint32 get(void) const
		{
			return P->getPhaseSmoothness();
		}
		void set(const uint32 &v)
		{
			P->setPhaseSmoothness(v);
		}
	}  _FanLightSmoothnessWrapper;

	struct CFanLightPhase : public IPSWrapperFloat
	{
		NL3D::CPSFanLight *P;
		float get(void) const
		{
			return P->getPhaseSpeed();
		}
		void set(const float &v)
		{
			P->setPhaseSpeed(v);
		}
	}  _FanLightPhaseWrapper;

	struct CFanLightIntensityWrapper : public IPSWrapperFloat
	{
		NL3D::CPSFanLight *P;
		float get(void) const
		{
			return P->getMoveIntensity();
		}
		void set(const float &v)
		{
			P->setMoveIntensity(v);
		}
	}  _FanLightIntensityWrapper;

	/// Ribbon / tail dot
	struct CTailParticleWrapper : public IPSWrapperUInt
	{
		NL3D::CPSTailParticle *P;
		uint32 get(void) const
		{
			return P->getTailNbSeg();
		}
		void set(const uint32 &v)
		{
			P->setTailNbSeg(v);
		}
	}  _TailParticleWrapper;

	/// Duration of segment for a ribbon
	struct CSegDurationWrapper : public IPSWrapperFloat
	{
		NL3D::CPSRibbonBase *R;
		float get(void) const
		{
			return R->getSegDuration();
		}
		void set(const float &v)
		{
			R->setSegDuration(v);
		}
	} _SegDurationWrapper;

	/// Shockwave
	struct CRadiusCutWrapper : public IPSWrapperFloat
	{
		NL3D::CPSShockWave *S;
		float get(void) const
		{
			return S->getRadiusCut();
		}
		void set(const float &v)
		{
			S->setRadiusCut(v);
		}
	} _RadiusCutWrapper;

	struct CShockWaveNbSegWrapper : public IPSWrapperUInt
	{
		NL3D::CPSShockWave *S;
		uint32 get(void) const
		{
			return S->getNbSegs();
		}
		void set(const uint32 &v)
		{
			S->setNbSegs(v);
		}
	} _ShockWaveNbSegWrapper;

	struct CShockWaveUFactorWrapper : public IPSWrapperFloat
	{
		NL3D::CPSShockWave *S;
		float get(void) const
		{
			return S->getUFactor();
		}
		void set(const float &v)
		{
			S->setUFactor(v);
		}
	} _ShockWaveUFactorWrapper;

	/// Unanimated texture
	struct CTextureNoAnimWrapper : public IPSWrapperTexture
	{
		NL3D::CPSTexturedParticleNoAnim *TP;
		virtual NL3D::ITexture *get(void)
		{
			return TP->getTexture();
		}
		virtual void set(NL3D::ITexture *t)
		{
			TP->setTexture(t);
		}
	} _TextureNoAnimWrapper;

	/// u / v factors for ribbon
	struct CRibbonUFactorWrapper : public IPSWrapperFloat
	{
		NL3D::CPSRibbon *R;
		float get(void) const
		{
			return R->getUFactor();
		}
		void set(const float &u)
		{
			R->setTexFactor(u, R->getVFactor());
		}
	} _RibbonUFactorWrapper;

	struct CRibbonVFactorWrapper : public IPSWrapperFloat
	{
		NL3D::CPSRibbon *R;
		float get(void) const
		{
			return R->getVFactor();
		}
		void set(const float &v)
		{
			R->setTexFactor(R->getUFactor(), v);
		}
	} _RibbonVFactorWrapper;

	struct CRibbonLengthWrapper : IPSWrapperFloat
	{
		NL3D::CPSRibbonBase *R;
		float get() const
		{
			return R->getRibbonLength();
		}
		void  set(const float &v)
		{
			R->setRibbonLength(v);
		}
	} _RibbonLengthWrapper;


	struct CLODDegradationWrapper : IPSWrapperFloat
	{
		NL3D::CPSRibbonBase *R;
		float get() const
		{
			return R->getLODDegradation();
		}
		void  set(const float &v)
		{
			R->setLODDegradation(v);
		}
	} _LODDegradationWrapper;

	void updateValidWidgetForAlignOnMotion(bool align);

	void updateSizeControl();

	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	void hideAllWidget();

	void touchPSState();

	CWorkspaceNode *_Node;

	/// The bindable being edited
	NL3D::CPSLocatedBindable *_Bindable;

	Ui::CLocatedBindablePage _ui;

}; /* class CLocatedBindablePage */

} /* namespace NLQT */


#endif // LOCATED_BINDABLE_PAGE_H
