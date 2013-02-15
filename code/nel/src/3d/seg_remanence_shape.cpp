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

#include "std3d.h"

#include "nel/3d/seg_remanence_shape.h"
#include "nel/3d/seg_remanence.h"
#include "nel/3d/driver.h"
#include "nel/3d/scene.h"
//
#include "nel/misc/bsphere.h"



namespace NL3D
{


//===========================================================
CSegRemanenceShape::CSegRemanenceShape() : _GeomTouched(true),
										   _MatTouched(true),
										   _TextureShifting(true),
										   _NumSlices(8),
										   _SliceTime(0.05f),
										   _RollUpRatio(1.f),
										   _AnimatedMat(NULL)
{
	_BBox.setCenter(NLMISC::CVector::Null);
	_BBox.setHalfSize(NLMISC::CVector(3, 3, 3));
	setNumCorners(2);
}

//===========================================================
void CSegRemanenceShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// version 2 : added default tracks
	// version 1 : rollup ratio
	// version 0 : base version

	sint ver = f.serialVersion(2);
	f.serial(_NumSlices);
	f.serial(_SliceTime);
	f.serialCont(_Corners);
	f.serial(_Mat);
	f.serial(_BBox);
	f.serial(_TextureShifting);
	f.serialPtr(_AnimatedMat);
	if (f.isReading())
	{
		_GeomTouched = true;
		_MatTouched  = true;
	}
	if (ver >= 1)
	{
		f.serial(_RollUpRatio);
	}
	if (ver >= 2)
	{
		f.serial(_DefaultPos);
		f.serial(_DefaultRotQuat);
		f.serial(_DefaultScale);
	}
}

//===========================================================
void CSegRemanenceShape::setSliceTime(float sliceTime)
{
	nlassert(sliceTime > 0);
	_SliceTime = sliceTime;
}

//===========================================================
void CSegRemanenceShape::setCorner(uint corner, const NLMISC::CVector &value)
{
	nlassert(corner < _Corners.size());
	_Corners[corner] = value;
}

//===========================================================
void CSegRemanenceShape::setNumSlices(uint32 numSlices)
{
	nlassert(numSlices >= 2);
	_NumSlices = numSlices;
	_GeomTouched = true;
}

//===========================================================
NLMISC::CVector CSegRemanenceShape::getCorner(uint corner) const
{
	nlassert(corner < _Corners.size());
	return _Corners[corner];
}

//===========================================================
void CSegRemanenceShape::setNumCorners(uint numCorners)
{
	nlassert(numCorners >= 2);
	_Corners.resize(numCorners);
	std::fill(_Corners.begin(), _Corners.end(), NLMISC::CVector::Null);
	_GeomTouched = true;
}

//===========================================================
void CSegRemanenceShape::render(IDriver *drv, CTransformShape *trans, bool opaquePass)
{
	if ((!opaquePass && _Mat.getBlend())
	    || (opaquePass && !_Mat.getBlend())
	   )
	{
		CSegRemanence *sr = NLMISC::safe_cast<CSegRemanence *>(trans);
		#ifndef DEBUG_SEG_REMANENCE_DISPLAY
		if (!sr->isStarted()) return;
		#endif
		setupMaterial();
		//
		sr->render(drv, _Mat);
	}
}


//===========================================================
void CSegRemanenceShape::flushTextures(IDriver &driver, uint selectedTexture)
{
	_Mat.flushTextures(driver, selectedTexture);
}

//===========================================================
CTransformShape *CSegRemanenceShape::createInstance(CScene &scene)
{
	CSegRemanence *sr = NLMISC::safe_cast<CSegRemanence *>(scene.createModel(NL3D::SegRemanenceShapeId) );
	sr->Shape = this;
	CAnimatedMaterial *aniMat = NULL;
	if (_AnimatedMat)
	{
		aniMat = new CAnimatedMaterial(_AnimatedMat);
		aniMat->setMaterial(&_Mat);
	}
	sr->setAnimatedMaterial(aniMat);
	sr->setupFromShape();
	// SegRemanence are added to the "Fx" Load Balancing Group.
	sr->setLoadBalancingGroup("Fx");

	sr->ITransformable::setPos( _DefaultPos.getDefaultValue() );
	sr->ITransformable::setRotQuat( _DefaultRotQuat.getDefaultValue() );
	sr->ITransformable::setScale( _DefaultScale.getDefaultValue() );

	sr->setSliceTime(_SliceTime);

	return sr;
}


//===========================================================
float CSegRemanenceShape::getNumTriangles(float distance)
{
	return (float) (_NumSlices * 2);
}


//===========================================================
void CSegRemanenceShape::setBBox(const NLMISC::CAABBox &bbox)
{
	_BBox = bbox;
}

//===========================================================
void CSegRemanenceShape::setMaterial(const CMaterial &mat)
{
	_Mat = mat;
	_MatTouched = true;
}

//===========================================================
void CSegRemanenceShape::setTextureShifting(bool on /*=true*/)
{
	_TextureShifting = on;
	_MatTouched = true;
}

//===========================================================
void CSegRemanenceShape::setRollupRatio(float ratio)
{
	nlassert(ratio > 0);
	_RollUpRatio = ratio;
}

//===========================================================
void CSegRemanenceShape::setupMaterial()
{
	if (!_MatTouched) return;
	_Mat.enableUserTexMat(0);
	if (_Mat.getTexture(0))
	{
		_Mat.getTexture(0)->setWrapS(ITexture::Clamp);
		_Mat.getTexture(0)->setWrapT(ITexture::Clamp);
	}
	_Mat.setDoubleSided(true);
	_Mat.setLighting(false); // lighting not supported (the vb has no normals anyway..)
	_MatTouched = false;
}

//===========================================================
void CSegRemanenceShape::setAnimatedMaterial(const std::string &name)
{
	nlassert(!name.empty());
	nlassert(_AnimatedMat == NULL);
	_AnimatedMat  = new CMaterialBase;
	_AnimatedMat->Name = name;
}



//===========================================================
CSegRemanenceShape::CSegRemanenceShape(const CSegRemanenceShape &other) : IShape(other), _AnimatedMat(NULL)
{
	copyFromOther(other);
}

//===========================================================
CSegRemanenceShape &CSegRemanenceShape::operator = (const CSegRemanenceShape &other)
{
	if (&other != this)
	{
		copyFromOther(other);
		(IShape &) *this = (IShape &) other; // copy base part
	}
	return *this;
}

//===========================================================
CSegRemanenceShape::~CSegRemanenceShape()
{
	delete _AnimatedMat;
}

//===========================================================
void CSegRemanenceShape::copyFromOther(const CSegRemanenceShape &other)
{
	if (&other == this) return;
	CMaterialBase *otherAnimatedMat = other._AnimatedMat != NULL ? new CMaterialBase(*other._AnimatedMat)
																 : NULL;
	delete _AnimatedMat;
	_AnimatedMat = otherAnimatedMat;

	_GeomTouched	 = other._GeomTouched;
	_MatTouched      = other._MatTouched;
	_TextureShifting = other._TextureShifting;
	_NumSlices       = other._NumSlices;
	_SliceTime       = other._SliceTime;
	_Corners		 = other._Corners;
	_Mat             = other._Mat;
	_BBox			 = other._BBox;
	_RollUpRatio     = other._RollUpRatio;
}



//===========================================================
bool CSegRemanenceShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// Speed Clip: clip just the sphere.
	NLMISC::CBSphere	localSphere(_BBox.getCenter(), _BBox.getRadius());
	NLMISC::CBSphere	worldSphere;

	// transform the sphere in WorldMatrix (with nearly good scale info).
	localSphere.applyTransform(worldMatrix, worldSphere);

	// if out of only plane, entirely out.
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		// if SpherMax OUT return false.
		float	d= pyramid[i]*worldSphere.Center;
		if(d>worldSphere.Radius)
			return false;
	}
	return true;
}

}
