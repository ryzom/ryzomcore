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

#include "nel/3d/camera.h"
#include "nel/3d/scene.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace	NL3D
{


// ***************************************************************************
void	CCamera::registerBasic()
{
	CScene::registerModel(CameraId, TransformId, CCamera::creator);
}


// ***************************************************************************
CCamera::CCamera()
{
	setFrustum(1.0f, 1.0f, 0.01f, 1.0f);

	// IAnimatable.
	IAnimatable::resize(AnimValueLast);

	_FovAnimationEnabled= false;
	_TargetAnimationEnabled= false;
	_FovAnimationAspectRatio= 4.0f/3.0f;

	// Default Anims.
	_Fov.Value= (float)NLMISC::Pi/2;
	_Target.Value= CVector::Null;
	_Roll.Value= 0;
}
// ***************************************************************************
void		CCamera::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	_Frustum.init( left, right,bottom, top, znear, zfar, perspective);
}
// ***************************************************************************
void		CCamera::setFrustum(float width, float height, float znear, float zfar, bool perspective)
{
	_Frustum.init(width, height, znear, zfar, perspective);
}
// ***************************************************************************
void		CCamera::setPerspective(float fov, float aspectRatio, float znear, float zfar)
{
	_Frustum.initPerspective(fov, aspectRatio, znear, zfar);
}
// ***************************************************************************
void		CCamera::getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const
{
	left= _Frustum.Left;
	right= _Frustum.Right;
	bottom=	_Frustum.Bottom;
	top= _Frustum.Top;
	znear= _Frustum.Near;
	zfar= _Frustum.Far;
}
// ***************************************************************************
bool		CCamera::isOrtho() const
{
	return !_Frustum.Perspective;
}
// ***************************************************************************
bool		CCamera::isPerspective() const
{
	return _Frustum.Perspective;
}


// ***************************************************************************
// ***************************************************************************
// Anims.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
IAnimatedValue* CCamera::getValue (uint valueId)
{
	// what value ?
	switch (valueId)
	{
	case FovValue:			return &_Fov;
	case TargetValue:		return &_Target;
	case RollValue:			return &_Roll;
	}

	return CTransform::getValue(valueId);
}
// ***************************************************************************
const char *CCamera::getValueName (uint valueId) const
{
	// what value ?
	switch (valueId)
	{
	case FovValue:			return getFovValueName();
	case TargetValue:		return getTargetValueName();
	case RollValue:			return getRollValueName();
	}

	return CTransform::getValueName(valueId);
}

// ***************************************************************************
CTrackDefaultFloat		CCamera::DefaultFov( (float)NLMISC::Pi/2 );
CTrackDefaultFloat		CCamera::DefaultRoll( 0 );


ITrack* CCamera::getDefaultTrack (uint valueId)
{
	// what value ?
	switch (valueId)
	{
	case PosValue:			return &_DefaultPos;
	case FovValue:			return &DefaultFov;
	case TargetValue:		return &_DefaultTargetPos;
	case RollValue:			return &DefaultRoll;
	}

	return CTransform::getDefaultTrack(valueId);
}
// ***************************************************************************
void	CCamera::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	// For CCamera, channels are not detailled.
	addValue(chanMixer, FovValue, OwnerBit, prefix, false);
	addValue(chanMixer, TargetValue, OwnerBit, prefix, false);
	addValue(chanMixer, RollValue, OwnerBit, prefix, false);

	CTransform::registerToChannelMixer(chanMixer, prefix);
}



// ***************************************************************************
void	CCamera::update()
{
	// test animations
	if(IAnimatable::isTouched(OwnerBit) || IAnimatable::isTouched(ITransformable::OwnerBit))
	{
		// FOV.
		if( _FovAnimationEnabled && IAnimatable::isTouched(FovValue))
		{
			// keep the same near/far.
			setPerspective(_Fov.Value, _FovAnimationAspectRatio, _Frustum.Near, _Frustum.Far);
			IAnimatable::clearFlag(FovValue);
		}

		// Target / Roll.
		// If target/Roll is animated, compute our own quaternion.
		if( _TargetAnimationEnabled && (IAnimatable::isTouched(TargetValue) || IAnimatable::isTouched(RollValue) || IAnimatable::isTouched(PosValue)) )
		{
			lookAt (getPos (), _Target.Value, -_Roll.Value);

			IAnimatable::clearFlag(TargetValue);
			IAnimatable::clearFlag(RollValue);
		}
	}

	CTransform::update();

	// We are OK!
	IAnimatable::clearFlag(OwnerBit);
}


// ***************************************************************************
void CCamera::build (const CCameraInfo &cameraInfo)
{
	// Target ?
	enableTargetAnimation(cameraInfo.TargetMode);
	enableFovAnimation(cameraInfo.UseFov);
	if (cameraInfo.TargetMode)
	{
		// Set the rot model
		setTransformMode (ITransformable::RotQuat);
		setTargetPos (cameraInfo.TargetPos);
		_DefaultTargetPos.setDefaultValue (cameraInfo.TargetPos);
		setRoll (cameraInfo.Roll);
	}
	if (cameraInfo.UseFov)
	{
		setFov (cameraInfo.Fov);
	}
	setPos (cameraInfo.Pos);
	_DefaultPos.setDefaultValue (cameraInfo.Pos);
}


// ***************************************************************************
CCameraInfo::CCameraInfo ()
{
	Roll = 0;
	Fov = 0;
	TargetMode = false;
	UseFov = false;
}


// ***************************************************************************
void CCameraInfo::serial (NLMISC::IStream &s)
{
	s.serialVersion (0);

	s.serial (Pos);
	s.serial (TargetPos);
	s.serial (Roll);
	s.serial (Fov);
	s.serial (TargetMode);
	s.serial (UseFov);
}


// ***************************************************************************
void CCamera::buildCameraPyramid(std::vector<CPlane>	&pyramid, bool useWorldMatrix)
{
	pyramid.resize(6);

	// Compute pyramid in view basis.
	CVector		pfoc(0,0,0);
	CVector		lb(_Frustum.Left,  _Frustum.Near, _Frustum.Bottom );
	CVector		lt(_Frustum.Left,  _Frustum.Near, _Frustum.Top    );
	CVector		rb(_Frustum.Right, _Frustum.Near, _Frustum.Bottom );
	CVector		rt(_Frustum.Right, _Frustum.Near, _Frustum.Top    );

	CVector		lbFar(_Frustum.Left,  _Frustum.Far, _Frustum.Bottom);
	CVector		ltFar(_Frustum.Left,  _Frustum.Far, _Frustum.Top   );
	CVector		rbFar(_Frustum.Right, _Frustum.Far, _Frustum.Bottom);
	CVector		rtFar(_Frustum.Right, _Frustum.Far, _Frustum.Top   );

	// near
	pyramid[0].make(lt, lb, rt);
	// far
	pyramid[1].make(lbFar, ltFar, rtFar);

	if(_Frustum.Perspective)
	{
		// left
		pyramid[2].make(pfoc, lt, lb);
		// top
		pyramid[3].make(pfoc, rt, lt);
		// right
		pyramid[4].make(pfoc, rb, rt);
		// bottom
		pyramid[5].make(pfoc, lb, rb);
	}
	else
	{
		// left
		pyramid[2].make(lt, ltFar, lbFar);
		// top
		pyramid[3].make(lt, rtFar, ltFar);
		// right
		pyramid[4].make(rt, rbFar, rtFar);
		// bottom
		pyramid[5].make(lb, lbFar, rbFar);
	}

	// get invCamMatrix
	CMatrix		invCamMatrix;
	if(useWorldMatrix)
		invCamMatrix= getWorldMatrix();
	else
		invCamMatrix= getMatrix();
	invCamMatrix.invert();

	// Compute pyramid in World basis.
	// The vector transformation M of a plane p is computed as p*M-1.
	for (uint i = 0; i < 6; i++)
	{
		pyramid[i]= pyramid[i]*invCamMatrix;
	}
}

// ***************************************************************************
void CCamera::buildCameraPyramidCorners(std::vector<NLMISC::CVector>	&pyramidCorners, bool useWorldMatrix)
{
	pyramidCorners.resize(8);
	pyramidCorners[0].set(_Frustum.Left,  _Frustum.Near, _Frustum.Bottom );
	pyramidCorners[1].set(_Frustum.Left,  _Frustum.Near, _Frustum.Top    );
	pyramidCorners[2].set(_Frustum.Right, _Frustum.Near, _Frustum.Bottom );
	pyramidCorners[3].set(_Frustum.Right, _Frustum.Near, _Frustum.Top    );
	float f = _Frustum.Perspective ? (_Frustum.Far / _Frustum.Near) : 1.f;
	pyramidCorners[4].set(f * _Frustum.Left,  _Frustum.Far, f * _Frustum.Bottom);
	pyramidCorners[5].set(f * _Frustum.Left,  _Frustum.Far, f * _Frustum.Top   );
	pyramidCorners[6].set(f * _Frustum.Right, _Frustum.Far, f * _Frustum.Bottom);
	pyramidCorners[7].set(f * _Frustum.Right, _Frustum.Far, f * _Frustum.Top   );

	const CMatrix &camMatrix = useWorldMatrix ? getWorldMatrix() : getMatrix();
	for(uint k = 0; k < 8; ++k)
	{
		pyramidCorners[k] = camMatrix * pyramidCorners[k];
	}
}



}



