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

#ifndef NL_CAMERA_H
#define NL_CAMERA_H

#include "nel/3d/frustum.h"
#include "nel/3d/transform.h"


namespace	NL3D
{


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		CameraId=NLMISC::CClassId(0x5752634c, 0x6abe76f5);

// ***************************************************************************

/**
  * A camera descriptor
  *
  * Used to export or build a CCamera.
  */
class CCameraInfo
{
public:
	CCameraInfo ();
	NLMISC::CVector			Pos;
	NLMISC::CVector			TargetPos;
	float					Roll;
	float					Fov;
	bool					TargetMode;
	bool					UseFov;

	void serial (NLMISC::IStream &s);
};


// ***************************************************************************
/**
 * A Camera node, based on a CTransform node.
 * The camera looks in his local Y direction (see CScene).
 *
 * No special traverse*()
 *	- has default behavior of a transform.
 *	- can't be clipped (well...  :) ).
 *	- is not lightable
 *	- is not renderable
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CCamera : public CTransform
{
public:
	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();


public:

	/// Build a camera
	void		build (const CCameraInfo &info);

	/// Set the frustum of the camera.
	void		setFrustum(const CFrustum &f) {_Frustum= f;}
	/// Get the frustum of the camera.
	const CFrustum&	getFrustum() const {return _Frustum;}
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true);
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float width, float height, float znear, float zfar, bool perspective= true);
	/// Get the camera frustum.
	void		getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const;
	/// Is a ortho camera?
	bool		isOrtho() const;
	/// Is a perspective camera?
	bool		isPerspective() const;
	/** Setup a perspective camera, giving a fov in radians.
	 * \param fov the horizontal angle of view, in radians. (Pi/2 as example)
	 * \param aspectRatio the ratio horizontal/vertical (1.33 as example).
	 * \param znear the front clipping plane distance.
	 * \param zfar the back clipping plane distance.
	 */
	void		setPerspective(float fov, float aspectRatio, float znear, float zfar);


	/** enable FOV animation. (default is false). see setPerspective(). znear and zfar are kept from previous setup.
	 * NB: as setPerspective(), fov is the full horizontal angle of camera (in radians).
	 */
	bool		enableFovAnimation(bool en, float aspectRatio= 4.0f / 3.0f)
	{
		_FovAnimationEnabled= en;
		if(en)
			_FovAnimationAspectRatio= aspectRatio;
		return true;
	}
	/** enable Target/Roll animation. (default is false). TTransform mode is forced to RotQuatMode. Roll is forced to 0.
	 * The camera builds the rot matrix from the animated target/roll.
	 */
	bool		enableTargetAnimation(bool en)
	{
		setTransformMode(ITransformable::RotQuat);
		_TargetAnimationEnabled= en;
		_Roll.Value= 0;
		return true;
	}

	/// \name Get / Set some values
	/// Works only if enableTargetAnimation.
	void	setTargetPos(const CVector &pos)
	{
		nlassert(_TargetAnimationEnabled);
		_Target.Value= pos;
		touch(TargetValue, OwnerBit);
	}
	/// Works only if enableTargetAnimation.
	void	setTargetPos(float x, float y, float z)
	{
		setTargetPos(CVector(x,y,z));
	}
	/// Works only if enableTargetAnimation.
	void	setRoll(float roll)
	{
		nlassert(_TargetAnimationEnabled);
		_Roll.Value = roll;
		touch(RollValue, OwnerBit);
	}
	/// Works only if enableFovAnimation.
	void	setFov(float fov)
	{
		nlassert(_FovAnimationEnabled);
		_Fov.Value = fov;
		touch(FovValue, OwnerBit);
	}
	/// Works only if enableTargetAnimation.
	void	getTargetPos(CVector &pos) const
	{
		nlassert(_TargetAnimationEnabled);
		pos=_Target.Value;
	}
	/// Works only if enableTargetAnimation.
	float	getRoll() const
	{
		nlassert(_TargetAnimationEnabled);
		return _Roll.Value;
	}
	/// Works only if enableFovAnimation.
	float	getFov() const
	{
		nlassert(_FovAnimationEnabled);
		return _Fov.Value;
	}

	/// \name Get some track name
	// @{
	/** Return the name of the fov track.
	 * NB: as setPerspective(), fov is the full horizontal angle of camera (in radians).
	 */
	static const char *getFovValueName() {return "fov";}
	/// Return the name of the target track. This target is in the parent space of camera.
	static const char *getTargetValueName() {return "target";}
	/// Return the name of the roll track
	static const char *getRollValueName() {return "roll";}
	// @}


	/// \name From Ianimatable.
	// @{
	enum	TAnimValues
	{
		OwnerBit= CTransform::AnimValueLast,
		FovValue ,
		TargetValue,
		RollValue,			// Roll is the roll angle in radians.

		AnimValueLast,
	};

	/// From IAnimatable
	virtual IAnimatedValue* getValue (uint valueId);
	/// From IAnimatable
	virtual const char *getValueName (uint valueId) const;
	/// Default Track Values for are identity (roll= 0, target= CVector::Null, fov=Pi/2).
	virtual ITrack* getDefaultTrack (uint valueId);
	/// register camera channels (in global anim mode).
	virtual void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);

	// @}

	/// \name access default tracks.
	// @{
	CTrackDefaultVector*	getDefaultPos ()			{return &_DefaultPos;}
	CTrackDefaultVector*	getDefaultTargetPos ()		{return &_DefaultTargetPos;}
	// @}

	/// Build the camera Pyramid from current worldMatrix, and frustum
	void			buildCameraPyramid(std::vector<NLMISC::CPlane>	&pyramid, bool useWorldMatrix);

	/// Compute corners of the camera Pyramid from current worldMatrix, and frustum
	void			buildCameraPyramidCorners(std::vector<NLMISC::CVector>	&pyramidCorners, bool useWorldMatrix);

protected:
	/// Constructor
	CCamera();
	/// Destructor
	virtual ~CCamera() {}

	// NB: znear and zfar are be >0 (if perspective).
	CFrustum	_Frustum;


	/// Implement the update method.
	virtual void	update();


private:
	static CTransform	*creator() {return new CCamera;}


private:
	bool					_FovAnimationEnabled;
	bool					_TargetAnimationEnabled;
	float					_FovAnimationAspectRatio;

	// AnimValues.
	CAnimatedValueFloat		_Fov;
	CAnimatedValueVector	_Target;
	CAnimatedValueFloat		_Roll;

	CTrackDefaultVector		_DefaultPos;
	CTrackDefaultVector		_DefaultTargetPos;

	// Default tracks.
	static CTrackDefaultFloat		DefaultFov;		//( NLMISC::Pi/2 );
	static CTrackDefaultFloat		DefaultRoll;	//( 0 );


};



}


#endif // NL_CAMERA_H

/* End of camera.h */
