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

#ifndef NL_PS_FACE_LOOK_AT_H
#define NL_PS_FACE_LOOK_AT_H

#include "nel/3d/ps_particle_basic.h"
#include "nel/3d/ps_quad.h"
#include "nel/3d/ps_iterator.h"

namespace NL3D
{


class CPSFaceLookAt;


/**
 * A FaceLookAt particle
 *  These particles can have 2 different size (width and height) when activated
 */
class CPSFaceLookAt :   public CPSQuad, public CPSRotated2DParticle
{
public:
	/** create the face look at by giving a texture. This can't be a CTextureGrouped (for animation)
     * animation must be set later by using setTextureScheme
	 */
	CPSFaceLookAt(CSmartPtr<ITexture> tex = NULL);

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	NLMISC_DECLARE_CLASS(CPSFaceLookAt);

	/** activate 'fake' motion blur (its done by deforming the quad)
	 *  This is slower, however. This has no effect with rotated particles.
	 *  \param coeff a coefficient for motion blur (too high value may give unrealistic result)
	 *         0 deactivate the motion blur
	 *  \param threshold : speed will be clamped below this value
	 */
	void activateMotionBlur(float coeff = 1.f, float threshold = 1.f)
	{
		_MotionBlurCoeff = coeff;
		_Threshold = threshold;
	}

	/// set the motion blur coeff (0 = none)
	void setMotionBlurCoeff(float coeff) { _MotionBlurCoeff = coeff; }

	/// set the motion blur threshold
	void setMotionBlurThreshold(float threshold) { _Threshold = threshold; }



	/** return the motion blur coeff (0.f means none)
	 *  \see  activateMotionBlur()
	 */
	float getMotionBlurCoeff(void) const { return _MotionBlurCoeff; }

	/// get the motion blur threshold
	float getMotionBlurThreshold(void) const { return _Threshold; }

	/** Setting this to true allows to have independant height and width for these particles.
	  * The interface to manage the second size can be obtained from getSecondSize(), which correspond to the height of particles.
	  * The default is to not have independant sizes
	  */
	void setIndependantSizes(bool enable  = true) { _IndependantSizes = enable; }

	/// test whether independant sizes are activated
	bool hasIndependantSizes(void) const { return _IndependantSizes; }

	/// retrieve an interface to set the second size
	CPSSizedParticle &getSecondSize(void)
	{
		nlassert(_IndependantSizes);
		return _SecondSize;
	}

	/// retrieve an interface to set the second size const version
	const CPSSizedParticle &getSecondSize(void) const
	{
		nlassert(_IndependantSizes);
		return _SecondSize;
	}

	/// from CPSParticle : return true if there are lightable faces in the object
	virtual bool			hasLightableFaces() { 	return false; }

	// Force faces to be aligned on motion. This bypass motion blur / align on z axis
	void					setAlignOnMotion(bool align) { _AlignOnMotion = align; }
	bool					getAlignOnMotion() const { return _AlignOnMotion; }
	// Force lookat to align with the z-axis
	void					setAlignOnZAxis(bool align) { _AlignOnZAxis = align; }
	bool					getAlignOnZAxis() const { return _AlignOnZAxis; }
protected:
	friend class			CPSFaceLookAtHelper;	/// used for implementation only
	float					_MotionBlurCoeff;
	float					_Threshold; // motion blur theshold
	// in this struct we defines the getSizeOwner method, which is abstract in the CPSSizedParticle clas
	struct					CSecondSize : public CPSSizedParticle
	{
		CPSFaceLookAt *Owner;
		virtual CPSLocated *getSizeOwner(void) { return Owner->getOwner(); }
	} _SecondSize;
	bool					_IndependantSizes;
	bool                    _AlignOnMotion;
	bool					_AlignOnZAxis;
	virtual void			draw(bool opaque);
	void					newElement(const CPSEmitterInfo &info);
	void					deleteElement(uint32);
	void					resize(uint32);
	virtual CPSLocated		*getAngle2DOwner(void) { return _Owner; }

};

extern uint64 PSLookAtRenderTime;


} // NL3D

#endif // NL_PS_FACE_LOOK_AT_H

/* End of ps_face_look_at.h */
