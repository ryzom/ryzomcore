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

#ifndef NL_PS_FACE_H
#define NL_PS_FACE_H

#include "nel/3d/ps_quad.h"
//
#include "nel/misc/traits_nl.h"

namespace NL3D
{

struct CPlaneBasisPair
{
	CPlaneBasis Basis;
	CVector		Axis; // an axis for rotation
	float		AngularVelocity; // an angular velocity
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * A face particle
 * Unlike FaceLookAt, these particle can have an orientation in space.
 * They are drawn with an angle bias of 45deg in their local basis (for optimisation purpose)
 *
 *          ^ y
 *          |
 *          0
 *         / \
 *        /   \
 *       3     1--> x
 *        \   /
 *         \ /
 *          2
 * If all particle must rotate the same, but with a rotattionnal bias, a hint can be provided, so that
 * there are batch of particle that share the same orientation. The users must give the number of various phase
 * This is the fastest.
 * Other cases need an attribute maker that produce a couple of vectors
 * , giving the x & y direction of the local basis (plane particle)
 *
 */


class CPSFace       : public CPSQuad
					, public CPSRotated3DPlaneParticle
					, public CPSHintParticleRotateTheSame
{
public:
	/** Create the face
	 *  you can give a non-animated texture here
	 */
	CPSFace(CSmartPtr<ITexture> tex = NULL);

	void serial(NLMISC::IStream &f);

	NLMISC_DECLARE_CLASS(CPSFace);
	/** Tells that all faces are turning in the same manner, and only have a rotationnal bias
	 *  This is faster then other method. Any previous set scheme for 3d rotation is kept.
	 *	\param: the number of rotation configuration we have. The more high it is, the slower it'll be
	 *          If this is too low, a lot of particles will have the same orientation
	 *          If it is 0, then the hint is disabled
 	 *  \param  minAngularVelocity : the maximum angular velocity for particle rotation
	 *  \param  maxAngularVelocity : the maximum angular velocity for particle rotation
	 *  \see    CPSRotated3dPlaneParticle
	 */
	void hintRotateTheSame(uint32 nbConfiguration
							, float minAngularVelocity = NLMISC::Pi
							, float maxAngularVelocity = NLMISC::Pi
						  );

	/** disable the hint 'hintRotateTheSame'
	 *  The previous set scheme for roation is used
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */
	void disableHintRotateTheSame(void)
	{
		hintRotateTheSame(0);
	}

	/** check whether a call to hintRotateTheSame was performed
	 *  \return 0 if the hint is disabled, the number of configurations else
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */

	uint32 checkHintRotateTheSame(float &min, float &max) const
	{
		min = _MinAngularVelocity;
		max = _MaxAngularVelocity;
		return (uint32)_PrecompBasis.size();
	}

	/// from CPSParticle : return true if there are lightable faces in the object
	virtual bool hasLightableFaces() { 	return false; }


protected:

	friend class CPSFaceHelper; /// for private use only

	virtual void	step(TPSProcessPass pass);
	virtual void	newElement(const CPSEmitterInfo &info);
	virtual void	deleteElement(uint32 index);
	virtual void	resize(uint32 size);
	/// fill _IndexInPrecompBasis with index in the range [0.. nb configurations[
	void fillIndexesInPrecompBasis(void);
	virtual CPSLocated *getPlaneBasisOwner(void) { return _Owner; }

	// we must store them for serialization
	float			_MinAngularVelocity;
	float			_MaxAngularVelocity;

	/// a set of precomp basis, before and after transfomation in world space, used if the hint 'RotateTheSame' has been called
	CPSVector<CPlaneBasisPair>::V _PrecompBasis;

	/// this contain an index in _PrecompBasis for each particle
	CPSVector<uint32>::V _IndexInPrecompBasis;
};





} // NL3D

// special traits
namespace NLMISC
{
	NL_TRIVIAL_TYPE_TRAITS(NL3D::CPlaneBasisPair)
}


#endif // NL_PS_FACE_H

/* End of ps_face.h */
