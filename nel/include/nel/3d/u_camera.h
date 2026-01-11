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

#ifndef NL_U_CAMERA_H
#define NL_U_CAMERA_H

#include "nel/misc/types_nl.h"
#include "u_transform.h"
#include "frustum.h"


namespace NL3D
{


// ***************************************************************************
/**
 * Game interface for manipulating Camera.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UCamera : public UTransform
{
public:
	/// \name Default Camera frustum (perspective).
	//@{
	static const float		DefLx;		//=0.26f;
	static const float		DefLy;		//=0.2f;
	static const float		DefLzNear;	//=0.15f;
	static const float		DefLzFar;	//=1000.0f;
	//@}


public:


	/// \name Frustum
	// @{
	/// Set the frustum of the camera.
	void		setFrustum(const CFrustum &f);
	/// Get the frustum of the camera.
	const		CFrustum&	getFrustum() const;
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true);
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float width, float height, float znear, float zfar, bool perspective= true);
	/// Get the camera frustum.
	void		getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const ;
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
	// @}

	/// \name Misc
	// @{
	void		buildCameraPyramid(std::vector<NLMISC::CPlane>	&pyramid, bool useWorldMatrix);
	void		buildCameraPyramidCorners(std::vector<NLMISC::CVector>	&pyramidCorners, bool useWorldMatrix);
	// @}

	/// Proxy interface

	/// Constructors
	UCamera() { _Object = NULL; }
	UCamera(class CCamera *object) { _Object = (ITransformable*)object; };
	/// Attach an object to this proxy
	void			attach(class CCamera *object) { _Object = (ITransformable*)object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CCamera	*getObjectPtr() const {return (CCamera*)_Object;}
};


} // NL3D


#endif // NL_U_CAMERA_H

/* End of u_camera.h */
