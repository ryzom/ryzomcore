/** \file u_decal.cpp
 * User interface implementation for projected texture decals.
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

#include "std3d.h"

#include "nel/3d/u_decal.h"
#include "nel/3d/decal.h"

namespace NL3D
{

// ***************************************************************************
void UDecal::setTexture(const std::string &filename)
{
	CDecal *object = getObjectPtr();
	object->setTexture(filename);
}

// ***************************************************************************
void UDecal::setMaterialId(uint32 id)
{
	CDecal *object = getObjectPtr();
	object->setMaterialId(id);
}

// ***************************************************************************
void UDecal::setUVCoord(const NLMISC::CUV &uv1, const NLMISC::CUV &uv2)
{
	CDecal *object = getObjectPtr();
	object->setUVCoord(uv1, uv2);
}

// ***************************************************************************
void UDecal::setClippingMode(uint mode)
{
	CDecal *object = getObjectPtr();
	TDecalClipMode clipMode;
	switch (mode)
	{
	case 0: clipMode = DecalClipNone; break;
	case 1: clipMode = DecalClipMask; break;
	case 2: clipMode = DecalClipGeometry; break;
	default: clipMode = DecalClipGeometry; break;
	}
	object->setClippingMode(clipMode);
}

// ***************************************************************************
void UDecal::setStatic(bool isStatic)
{
	CDecal *object = getObjectPtr();
	object->setStatic(isStatic);
}

// ***************************************************************************
void UDecal::setDiffuse(NLMISC::CRGBA diffuse)
{
	CDecal *object = getObjectPtr();
	object->setDiffuse(diffuse);
}

// ***************************************************************************
void UDecal::setEmissive(NLMISC::CRGBA emissive)
{
	CDecal *object = getObjectPtr();
	object->setEmissive(emissive);
}

// ***************************************************************************
void UDecal::setBottomBlend(float zMin, float zMax)
{
	CDecal *object = getObjectPtr();
	object->setBottomBlend(zMin, zMax);
}

// ***************************************************************************
void UDecal::setTopBlend(float zMin, float zMax)
{
	CDecal *object = getObjectPtr();
	object->setTopBlend(zMin, zMax);
}

// ***************************************************************************
void UDecal::setPriority(uint8 priority)
{
	CDecal *object = getObjectPtr();
	object->setPriority(priority);
}

// ***************************************************************************
void UDecal::setClipDownFacing(bool clipDownFacing)
{
	CDecal *object = getObjectPtr();
	object->setClipDownFacing(clipDownFacing);
}

// ***************************************************************************
void UDecal::setCustomUVMatrix(bool on, const NLMISC::CMatrix &matrix)
{
	CDecal *object = getObjectPtr();
	object->setCustomUVMatrix(on, matrix);
}

// ***************************************************************************
void UDecal::setTextureMatrix(const NLMISC::CMatrix &matrix)
{
	CDecal *object = getObjectPtr();
	object->setTextureMatrix(matrix);
}

// ***************************************************************************
void UDecal::setWorldMatrixForArrow(const NLMISC::CVector2f &start, const NLMISC::CVector2f &end, float halfWidth)
{
	CDecal *object = getObjectPtr();
	object->setWorldMatrixForArrow(start, end, halfWidth);
}

// ***************************************************************************
void UDecal::setWorldMatrixForSpot(const NLMISC::CVector2f &pos, float radius, float angleInRadians)
{
	CDecal *object = getObjectPtr();
	object->setWorldMatrixForSpot(pos, radius, angleInRadians);
}

// ***************************************************************************
bool UDecal::contains(const NLMISC::CVector2f &pos) const
{
	CDecal *object = (CDecal*)_Object;
	return object->contains(pos);
}

} // NL3D
