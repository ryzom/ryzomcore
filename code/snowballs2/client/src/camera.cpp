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

//
// Includes
//

#include <nel/misc/types_nl.h>

#include <cmath>
#include <sstream>
#include <nel/misc/vectord.h>
#include <nel/misc/config_file.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_visual_collision_entity.h>
#include <nel/3d/u_visual_collision_manager.h>
#include <nel/3d/u_cloud_scape.h>
#include <nel/3d/viewport.h>

#include <nel/3d/stereo_hmd.h>

#include "snowballs_client.h"
#include "entities.h"
#include "mouse_listener.h"
#include "pacs.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace SBCLIENT {

//
// Variables
//

// The camera for the whole scene
UCamera					Camera = NULL;
// The collision entity use to snap the camera on the ground
UVisualCollisionEntity	*CamCollisionEntity = NULL;

// The particle system for the snowing effect
static UInstance			Snow = NULL;

// The sky 3D objects
UScene				*SkyScene = NULL;
UCamera				SkyCamera = NULL;
static UInstance			Sky = NULL;

static UCloudScape			*Clouds = NULL;

IStereoDisplay *StereoDisplay = NULL;
IStereoHMD *StereoHMD = NULL;

//
// Functions
//

void	initCamera()
{
	if (ConfigFile->getVar("HMDEnable").asBool())
	{
		std::vector<NL3D::CStereoDeviceInfo> devices;
		IStereoDisplay::listDevices(devices);
		for (std::vector<NL3D::CStereoDeviceInfo>::iterator it(devices.begin()), end(devices.end()); it != end; ++it)
		{
			std::stringstream name;
			name << std::string("[") << it->Serial << "] [" << IStereoDisplay::getLibraryName(it->Library) << " - " << it->Manufacturer << " - " << it->ProductName << "]";
			nlinfo("Stereo Display: %s", name.str().c_str());
		}
		CStereoDeviceInfo *deviceInfo = NULL;
		std::string hmdDeviceCfg = ConfigFile->getVar("HMDDevice").asString();
		if (hmdDeviceCfg == std::string("Auto")
			&& devices.begin() != devices.end())
		{
			for (std::vector<NL3D::CStereoDeviceInfo>::iterator it(devices.begin()), end(devices.end()); it != end; ++it)
			{
				if (it->AllowAuto)
				{
					deviceInfo = &devices[0];
				}
			}
		}
		else
		{
			std::string hmdDeviceId = ConfigFile->getVar("HMDDeviceId").asString();
			for (std::vector<NL3D::CStereoDeviceInfo>::iterator it(devices.begin()), end(devices.end()); it != end; ++it)
			{
				std::stringstream name;
				name << IStereoDisplay::getLibraryName(it->Library) << " - " << it->Manufacturer << " - " << it->ProductName;
				if (name.str() == hmdDeviceCfg)
					deviceInfo = &(*it);
				if (hmdDeviceId == it->Serial)
					break;
			}
		}
		if (deviceInfo)
		{
			nlinfo("Create VR stereo display device");
			StereoDisplay = IStereoDisplay::createDevice(*deviceInfo);
			if (StereoDisplay)
			{
				if (deviceInfo->Class == CStereoDeviceInfo::StereoHMD)
				{
					nlinfo("Stereo display device is a HMD");
					StereoHMD = static_cast<IStereoHMD *>(StereoDisplay);
					StereoHMD->setScale(3.0f); // snowballs is about 4 units per meter
				}
				StereoDisplay->setDriver(Driver); // move after driver creation, move stereodisplay before driver creation
			}
		}
	}
	IStereoDisplay::releaseUnusedLibraries();

	// Set up directly the camera
	Camera = Scene->getCam();
	Camera.setTransformMode (UTransformable::DirectMatrix);
	Camera.setPerspective((float)Pi/2.f, 
		ConfigFile->getVar("ScreenWidth").asFloat() / ConfigFile->getVar("ScreenHeight").asFloat(), 
		0.1f, 1000.f);
	Camera.lookAt (CVector(ConfigFile->getVar("StartPoint").asFloat(0),
							ConfigFile->getVar("StartPoint").asFloat(1),
							ConfigFile->getVar("StartPoint").asFloat(2)),
							CVectorD (0,0,0));

	CamCollisionEntity = VisualCollisionManager->createEntity();
	CamCollisionEntity->setCeilMode(true);

	// Create the snowing particle system
	Snow = Scene->createInstance("snow.ps");
	// And setup it
	Snow.setTransformMode (UTransformable::DirectMatrix);

	//
	// Setup the sky scene
	//

	// -- -- not sure what the sky has to do with the camera

	SkyScene = Driver->createScene(false);

	SkyCamera = SkyScene->getCam ();
	SkyCamera.setTransformMode (UTransformable::DirectMatrix);
	// Set the very same frustum as the main camera
	SkyCamera.setFrustum (Camera.getFrustum ());

	Sky = SkyScene->createInstance("sky.shape");
	Sky.setTransformMode (UTransformable::DirectMatrix);
	Sky.setMatrix(CMatrix::Identity);
}

void releaseCamera()
{
	SkyScene->deleteInstance(Sky);
	Driver->deleteScene(SkyScene);
	Scene->deleteInstance(Snow);
	VisualCollisionManager->deleteEntity(CamCollisionEntity);

	if (StereoHMD)
	{
		delete StereoHMD;
		StereoHMD = NULL;
		StereoDisplay = NULL;
	}
	delete StereoDisplay;
	StereoDisplay = NULL;
	IStereoDisplay::releaseAllLibraries();
}

void updateCamera()
{
	if (StereoHMD)
	{
		NLMISC::CQuat hmdOrient = StereoHMD->getOrientation();
		NLMISC::CMatrix camMatrix = Camera.getMatrix();
		NLMISC::CMatrix hmdMatrix;
		hmdMatrix.setRot(hmdOrient);
		NLMISC::CMatrix posMatrix; // minimal head modeling, will be changed in the future
		posMatrix.translate(StereoHMD->getEyePosition());
		Camera.setMatrix((camMatrix * hmdMatrix) * posMatrix);
	}
	// Set the new position of the snow emitter
	CMatrix	mat = CMatrix::Identity;
	mat.setPos (Camera.getMatrix().getPos()/*+CVector (0.0f, 0.0f, -10.0f)*/);
	Snow.setMatrix(mat);
}

void initSky()
{
	// -- -- or what the clouds have to do with the sky

	SCloudScapeSetup css;
	Clouds = Scene->createCloudScape ();
	Clouds->init (&css);
	Clouds->setQuality (160);
	Clouds->setNbCloudToUpdateIn80ms (1);
}

void releaseSky()
{
	Scene->deleteCloudScape(Clouds);
}

// -- -- random note: update and render makes more sense than animate and update
void animateSky(double dt)
{
	if (!StereoHMD) Clouds->anim(dt);
	SkyScene->animate(AnimationTime);
}

// this is actually render
void updateSky()
{
	CMatrix skyCameraMatrix;
	skyCameraMatrix.identity();
	// 
	skyCameraMatrix= Camera.getMatrix();
	skyCameraMatrix.setPos(CVector::Null);
	SkyCamera.setMatrix(skyCameraMatrix);

	SkyScene->render();
	// Must clear ZBuffer For incoming rendering.
	Driver->clearZBuffer();

	if (!StereoHMD) // Cloudscape not supported (fix Viewport please)
		Clouds->render();
}

} /* namespace SBCLIENT */

/* end of file */
