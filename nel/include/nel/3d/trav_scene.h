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

#ifndef NL_TRAV_SCENE_H
#define NL_TRAV_SCENE_H


#include "nel/misc/matrix.h"
#include "nel/misc/smart_ptr.h"



namespace	NL3D
{

class CScene;

using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


// ***************************************************************************
/**
 * A Traversal which may be renderable in a CScene.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTraversal
{
public:
	CScene	*Scene; // the scene owning this traversal (if any)
public:
	/// ctor
	CTraversal() : Scene(NULL) {}
};


// ***************************************************************************
/**
 * A ITravScene traversal, with camera setup  (common to CRenderTrav and CClipTrav).
 *
 */
class CTravCameraScene : public CTraversal
{
public:
	/** \name FOR MODEL TRAVERSING ONLY.  (Read only)
	 * Those variables are valid only in traverse().
	 */
	//@{
	// NB: znear and zfar are >0 (if perspective).
	float				Left, Right, Bottom, Top, Near, Far;
	bool				Perspective;
	NLMISC::CMatrix		CamMatrix;		// The camera matrix.
	NLMISC::CMatrix		ViewMatrix;		// ViewMatrix= CamMatrix.inverted();
	NLMISC::CVector		CamPos;			// The camera position in world space.
	NLMISC::CVector		CamLook;		// The Y direction of the camera in world space.
	//@}


public:
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true)
	{
		Left= left;
		Right= right;
		Bottom=	bottom;
		Top= top;
		Near= znear;
		Far= zfar;
		Perspective= perspective;
	}
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float width, float height, float znear, float zfar, bool perspective= true)
	{
		setFrustum(-width/2, width/2, -height/2, height/2, znear, zfar, perspective);
	}
	/// Setup the camera matrix (a translation/rotation matrix).
	void		setCamMatrix(const NLMISC::CMatrix	&camMatrix)
	{
		CamMatrix= camMatrix;
	}


	/// Constructor.
	CTravCameraScene()
	{
		setFrustum(1.0f, 1.0f, 0.01f, 1.0f);
		CamMatrix.identity();
		ViewMatrix.identity();
		CamPos= NLMISC::CVector::Null;
		CamLook= NLMISC::CVector::Null;
	}


protected:

	/// update the dependent information.
	void	update()
	{
		ViewMatrix= CamMatrix.inverted();
		CamPos= CamMatrix.getPos();
		CamLook= CamMatrix.mulVector(NLMISC::CVector::J);
	}


};


}


#endif // NL_TRAV_SCENE_H

/* End of trav_scene.h */
