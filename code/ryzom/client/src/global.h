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


#ifndef CL_GLOBAL_H
#define CL_GLOBAL_H

#include "nel/misc/types_nl.h"
#include "3d_notes.h"
#include "progress.h"


// tmp
#define USE_WATER_ENV_MAP


// ***************************************************************************
namespace NL3D
{
	class UDriver;
	class USCene;
	class UInstanceGroup;
	class ULandscape;
	class UCloudScape;
	class UCamera;
	class UVisualCollisionManager;
	class UMaterial;
	class UTextContext;
	class UWaterEnvMap;
	class IStereoDisplay;
	class IStereoHMD;
	class CFXAA;
}

class CEntityAnimationManager;
class CSoundManager;
class CContinent;
class CWaterEnvMapRdr;


// ***************************************************************************

enum TBackground
{
	StartBackground = 0,
	ElevatorBackground,
	TeleportKamiBackground,
	TeleportKaravanBackground,
	ResurectKamiBackground,
	ResurectKaravanBackground,
	EndBackground,
	IntroNevrax,
	IntroNVidia,
	LoadBackground,
};

// Camera Setup
const float		SceneRootCameraZNear	= 20.0f;
const float		SceneRootCameraZFar		= 20000.0f;
const float		SkyCameraZFar			= 500.0f;
const float		CameraSetupZNear		= 0.15f;

// Misc
const float		ExtraZoneLoadingVision	= 100.f;


// ***************************************************************************
// Main System
extern NL3D::UDriver				*Driver;		// The main 3D Driver
extern NL3D::IStereoDisplay			*StereoDisplay; // Stereo display
extern NL3D::IStereoHMD				*StereoHMD;		// Head mount display
extern CSoundManager				*SoundMngr;		// the sound manager
extern NL3D::UMaterial				GenericMat;	// Generic Material
extern NL3D::UTextContext			*TextContext;	// Context for all the text in the client.

// Effects
extern NL3D::CFXAA					*FXAA;

// Main 3D Objects
extern NL3D::UScene						*Scene;
extern NL3D::UScene						*SceneRoot;
extern NL3D::UInstanceGroup				*BackgroundIG;
extern NL3D::ULandscape					*Landscape;
extern NL3D::UCloudScape			    *CloudScape;
extern NL3D::UCamera					MainCam;
extern NL3D::UVisualCollisionManager	*CollisionManager;
#ifdef USE_WATER_ENV_MAP
	extern CWaterEnvMapRdr				WaterEnvMapRdr;
	extern NL3D::UWaterEnvMap			*WaterEnvMap;
	extern uint							WaterEnvMapRefCount;
	extern NL3D::UCamera				WaterEnvMapSkyCam;
	extern NL3D::UCamera				WaterEnvMapCanopyCam;
#endif

// Network Walls
extern bool							UserCharPosReceived;
extern bool							SabrinaPhraseBookLoaded;

// Main scene matrices.
extern NLMISC::CMatrix				MainSceneViewMatrix;		// Matrix to transform from world space to camera space
extern NLMISC::CMatrix				InvMainSceneViewMatrix;		// Matrix to transform from camera space to world space

// Misc
extern bool							InitCloudScape; // tells that the cloud scape must be reinitialized
extern CEntityAnimationManager		*EAM;
extern CProgress					ProgressBar;
extern TBackground					LoadingBackground;
extern std::string					LoadingMusic;
extern CContinent					*LoadingContinent;
extern bool							ConnectionReadySent;
extern bool							PermanentlyBanned;
extern bool							IgnoreEntityDbUpdates;

// VR
extern std::vector<std::pair<
	std::string, std::string> >		VRDeviceCache;

extern std::string					Cookie, FSAddr;
extern std::string					RingMainURL;
extern bool							FreeTrial;

void resetTextContext (const char *font, bool resetInterfaceManager);

#endif // CL_GLOBAL_H

/* End of global.h */
