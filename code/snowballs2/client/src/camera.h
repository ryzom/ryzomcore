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

#ifndef CAMERA_H
#define CAMERA_H

//
// Includes
//

#include <nel/3d/u_camera.h>

//
// External definitions
//

namespace NL3D
{
	class UVisualCollisionEntity;
	class CStereoOVR;
};

namespace SBCLIENT {

//
// External variables
//

extern NL3D::UCamera				Camera;
extern NL3D::UCamera						SkyCamera;
extern NL3D::UVisualCollisionEntity	*CamCollisionEntity;
extern NL3D::UScene				*SkyScene;

//
// External functions
//

void	initCamera();
void	updateCamera();
void	releaseCamera();


void	initSky ();
void releaseSky();

// Update the sky for this frame, and render it.
// Must be called before ANY rendering
// this is actually render
void	updateSky ();

// this is update :x
void	animateSky (double dt);

} /* namespace SBCLIENT */


#endif // CAMERA_H

/* End of camera.h */
