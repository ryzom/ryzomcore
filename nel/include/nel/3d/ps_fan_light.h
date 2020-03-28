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

#ifndef NL_PS_FAN_LIGHT_H
#define NL_PS_FAN_LIGHT_H

#include "nel/3d/ps_particle_basic.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"

#include <vector>


namespace NL3D
{


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * A fan light particle
 */
class CPSFanLight : public CPSParticle, public CPSColoredParticle
				  , public CPSSizedParticle, public CPSRotated2DParticle
				  , public CPSMaterial, public CPSTexturedParticleNoAnim
{
public:
	NLMISC_DECLARE_CLASS(CPSFanLight);
	virtual bool		completeBBox(NLMISC::CAABBox &box) const;
	///\name Object
	//@{
	/// Ctor, with the numbers of fans to draw (minimum is 3, maximum is 128)
	CPSFanLight(uint32 nbFans = 7);
	/// Dtor
	~CPSFanLight();
	void				serial(NLMISC::IStream &f);
	//@}

	// Set the number of fans used for drawing (minimum is 3, maximum is 128)
	void				setNbFans(uint32 nbFans);

	/** Set the smoothness of phases. The default is 0 which means no smoothness.
	  * n mean that the phase will be linearly interpolated between each n + 1 fans
	  * It ranges from 0 to 31
	  */
	void				setPhaseSmoothness(uint32 smoothNess)
	{
		nlassert(smoothNess < 32);
		_PhaseSmoothness = smoothNess;
	}

	/// retrieve the phase smoothness
	uint32				getPhaseSmoothness(void) const { return _PhaseSmoothness;}

	/// set the intensity of fan movement. Default is 1.5
	void				setMoveIntensity(float intensity) { _MoveIntensity = intensity; }

	/// get the intensity of fans movement
	float				getMoveIntensity(void) const      { return _MoveIntensity; }

	// Get the number of fans used for drawing
	uint32				getNbFans(void) const
	{
		return _NbFans;
	}

	/** Set the speed for phase
	 *	If the located holding this particle as a limited lifetime, it gives how many 0-2Pi cycle it'll do during its life
	 *  Otherwise it gives how many cycle there are in a second
	 */
	void				setPhaseSpeed(float multiplier);

	/// get the speed for phase
	float				getPhaseSpeed(void) const { return _PhaseSpeed / 256.0f; }

	// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void		updateMatAndVbForColor(void);


	/// must call this at least if you intend to use fanlight
	static void			initFanLightPrecalc(void);

	/// return true if there are transparent faces in the object
	virtual bool		hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool		hasOpaqueFaces(void);


	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32		getNumWantedTris() const;

	/// Set a texture. NULL remove it
	void setTexture(CSmartPtr<ITexture> tex)
	{
		_Tex = tex;
	}

	/// get the texture used
	ITexture *getTexture(void)
	{
		return _Tex;
	}
	const ITexture *getTexture(void) const
	{
		return _Tex;
	}

	/// from CPSParticle : return true if there are lightable faces in the object
	virtual bool hasLightableFaces() { 	return false; }

	// from CPSParticle
	virtual bool supportGlobalColorLighting() const { return true; }

	virtual	void			enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv);

	// from CPSParticle
	virtual void			setZBias(float value) { CPSMaterial::setZBias(value); }
	virtual float			getZBias() const { return CPSMaterial::getZBias(); }

protected:
	void				newElement(const CPSEmitterInfo &info);
	void				deleteElement(uint32);
	virtual void resize(uint32 size);
	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	virtual CPSLocated *getSizeOwner(void) { return _Owner; }
	virtual CPSLocated *getAngle2DOwner(void) { return _Owner; }
private:
	friend class CPSFanLightHelper;
	typedef CHashMap<uint, CVertexBuffer>  TVBMap;
	typedef CHashMap<uint, CIndexBuffer >  TIBMap;
private:
	uint32						_NbFans;
	uint32						_PhaseSmoothness;
	float						_MoveIntensity;
	NLMISC::CSmartPtr<ITexture> _Tex;
	float						_PhaseSpeed;
	//
	static uint8				_RandomPhaseTab[32][128];

	static TVBMap				_VBMap; // fanlight, no texture
	static TVBMap				_TexVBMap; // fanlight, textured
	static TVBMap				_ColoredVBMap; // fanlight, no texture, varying color
	static TVBMap				_ColoredTexVBMap; // fanlight, textured, varying color
	static TIBMap				_IBMap;

	static bool _RandomPhaseTabInitialized;
private:
	/// initialisations
	virtual void init(void);
	virtual void draw(bool opaque);
	// setup and get the needed vb for display
	void getVBnIB(CVertexBuffer *&vb, CIndexBuffer *&ib);
	uint getNumFanlightsInVB() const;
	void setupMaterial();

};


} // NL3D


#endif // NL_PS_FAN_LIGHT_H

/* End of ps_fan_light.h */
