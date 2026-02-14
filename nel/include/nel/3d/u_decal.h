/** \file u_decal.h
 * User interface for projected texture decals.
 */

/* Copyright, 2007 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_U_DECAL_H
#define NL_U_DECAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/uv.h"
#include "nel/misc/matrix.h"
#include "nel/misc/vector_2f.h"
#include "nel/3d/u_transform.h"


namespace NL3D
{

class CDecal;

class UDecal : public UTransform
{
public:

	/// Constructors
	UDecal() { _Object = NULL; }
	UDecal(class CDecal *object) { _Object = (ITransformable*)object; }
	/// Attach an object to this proxy
	void			attach(class CDecal *object) { _Object = (ITransformable*)object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const { return _Object == NULL; }
	/// For advanced usage, get the internal object ptr
	class CDecal	*getObjectPtr() const { return (CDecal*)_Object; }

	/// Set the decal texture from a filename
	void			setTexture(const std::string &filename);

	/// Set the material ID for batching (from CDecalManager::registerMaterial)
	void			setMaterialId(uint32 id);

	/// Set the UV sub-region within a texture atlas
	void			setUVCoord(const NLMISC::CUV &uv1, const NLMISC::CUV &uv2);

	/// Set the clipping mode (0=None, 1=Mask, 2=Geometry)
	void			setClippingMode(uint mode);

	/// Mark this decal as static (only recomputed once)
	void			setStatic(bool isStatic);

	/// Set the diffuse color (RGB tints texture, A is base opacity)
	void			setDiffuse(NLMISC::CRGBA diffuse);

	/// Set the emissive color (added to texture × diffuse)
	void			setEmissive(NLMISC::CRGBA emissive);

	/// Set the bottom Z-blend region (fade from 0 at zMin to 1 at zMax)
	void			setBottomBlend(float zMin, float zMax);

	/// Set the top Z-blend region (fade from 1 at zMin to 0 at zMax)
	void			setTopBlend(float zMin, float zMax);

	/// Set the render priority (0 = highest/first, 7 = lowest/last)
	void			setPriority(uint8 priority);

	/// Set whether downward-facing surfaces should be clipped
	void			setClipDownFacing(bool clipDownFacing);

	/// Set a custom UV matrix (world → UV transform), replacing default UV generation
	void			setCustomUVMatrix(bool on, const NLMISC::CMatrix &matrix = NLMISC::CMatrix::Identity);

	/// Set the texture coordinate transform matrix
	void			setTextureMatrix(const NLMISC::CMatrix &matrix);

	/// Set the world matrix for an arrow-shaped decal
	void			setWorldMatrixForArrow(const NLMISC::CVector2f &start, const NLMISC::CVector2f &end, float halfWidth);

	/// Set the world matrix for a spot-shaped decal
	void			setWorldMatrixForSpot(const NLMISC::CVector2f &pos, float radius, float angleInRadians = 0.f);

	/// Test if a 2D point is contained within this decal's projection area (for hit-testing)
	bool			contains(const NLMISC::CVector2f &pos) const;
};


} //NL3D

#endif
