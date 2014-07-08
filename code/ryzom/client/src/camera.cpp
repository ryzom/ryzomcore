// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "camera.h"

#include <nel/3d/stereo_display.h>

#include "global.h"
#include "misc.h"

using namespace NLMISC;
using namespace NL3D;

//---------------------------------------------------
// update the camera perspective setup
//---------------------------------------------------
void updateCameraPerspective()
{
	float	fov, aspectRatio;
	computeCurrentFovAspectRatio(fov, aspectRatio);

	// change the perspective of the scene
	if(!MainCam.empty())
		MainCam.setPerspective(fov, aspectRatio, CameraSetupZNear, ClientCfg.Vision);
	// change the perspective of the root scene
	if(SceneRoot)
	{
		UCamera cam= SceneRoot->getCam();
		cam.setPerspective(fov, aspectRatio, SceneRootCameraZNear, SceneRootCameraZFar);
	}
}

void buildCameraClippingPyramid(std::vector<CPlane> &planes)
{
	if (StereoDisplay) StereoDisplay->getClippingFrustum(0, &MainCam);

	// Compute pyramid in view basis.
	CVector		pfoc(0,0,0);
	const CFrustum &frustum  = MainCam.getFrustum();
	InvMainSceneViewMatrix = MainCam.getMatrix();
	MainSceneViewMatrix = InvMainSceneViewMatrix;
	MainSceneViewMatrix.invert();

	CVector		lb(frustum.Left,  frustum.Near, frustum.Bottom );
	CVector		lt(frustum.Left,  frustum.Near, frustum.Top    );
	CVector		rb(frustum.Right, frustum.Near, frustum.Bottom );
	CVector		rt(frustum.Right, frustum.Near, frustum.Top    );

	CVector		lbFar(frustum.Left,  ClientCfg.CharacterFarClip, frustum.Bottom);
	CVector		ltFar(frustum.Left,  ClientCfg.CharacterFarClip, frustum.Top   );
	CVector		rtFar(frustum.Right, ClientCfg.CharacterFarClip, frustum.Top   );

	planes.resize (4);
	// planes[0].make(lbFar, ltFar, rtFar);
	planes[0].make(pfoc, lt, lb);
	planes[1].make(pfoc, rt, lt);
	planes[2].make(pfoc, rb, rt);
	planes[3].make(pfoc, lb, rb);

	// Compute pyramid in World basis.
	// The vector transformation M of a plane p is computed as p*M-1.
	// Here, ViewMatrix== CamMatrix-1. Hence the following formula.
	uint i;

	for (i = 0; i < 4; i++)
	{
		planes[i] = planes[i]*MainSceneViewMatrix;
	}
}

/* end of file */