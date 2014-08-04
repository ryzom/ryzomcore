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
#include "global.h"
#include "water_env_map_rdr.h"
#include "interface_v3/interface_manager.h"

using namespace NLMISC;

// ***************************************************************************
// Main System
NL3D::UDriver				*Driver = NULL;		// The main 3D Driver
NL3D::IStereoDisplay		*StereoDisplay = NULL; // Stereo display
NL3D::IStereoHMD			*StereoHMD = NULL;	// Head mount display
CSoundManager				*SoundMngr = NULL;		// the sound manager
NL3D::UMaterial				GenericMat;	// Generic Material
NL3D::UTextContext			*TextContext = NULL;	// Context for all the text in the client.

// Effects
NL3D::CFXAA					*FXAA = NULL;

// Main 3D Objects
NL3D::UScene						*Scene = NULL;
NL3D::UScene						*SceneRoot = NULL;
NL3D::UInstanceGroup				*BackgroundIG = NULL;
NL3D::ULandscape					*Landscape = NULL;
NL3D::UCloudScape				    *CloudScape = NULL;
NL3D::UCamera						MainCam;
NL3D::UVisualCollisionManager		*CollisionManager = NULL;
#ifdef USE_WATER_ENV_MAP
	CWaterEnvMapRdr						WaterEnvMapRdr;
	NL3D::UWaterEnvMap					*WaterEnvMap = NULL;
	uint								WaterEnvMapRefCount = 0;
	NL3D::UCamera						WaterEnvMapSkyCam;
	NL3D::UCamera						WaterEnvMapCanopyCam;
#endif

// Network Walls
bool						UserCharPosReceived = false;
bool						SabrinaPhraseBookLoaded = false;

// Main scene matrices.
NLMISC::CMatrix				MainSceneViewMatrix;		// Matrix to transform from world space to camera space
NLMISC::CMatrix				InvMainSceneViewMatrix;		// Matrix to transform from camera space to world space

// Misc
bool						InitCloudScape = true; // tells that the cloud scape must be reinitialized
CEntityAnimationManager		*EAM = NULL;
CProgress					ProgressBar;
TBackground					LoadingBackground = StartBackground;
string						LoadingMusic;
CContinent					*LoadingContinent = NULL;
bool						ConnectionReadySent= false;
bool						PermanentlyBanned = false;
bool						IgnoreEntityDbUpdates = false;
bool						FreeTrial = false;

std::vector<std::pair<
	std::string, std::string> >		VRDeviceCache;

/// Ring main page
string						RingMainURL;

void resetTextContext (const char *font, bool resetInterfaceManager)
{
	if (TextContext != NULL)
		Driver->deleteTextContext(TextContext);
	TextContext = Driver->createTextContext(CPath::lookup(font));
	if (TextContext != NULL)
		TextContext->setKeep800x600Ratio(false);
	else
		nlerror("Cannot create a TextContext with font %s.", font);

	// Reset the text indexes
	if (resetInterfaceManager)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CViewRenderer::setTextContext( TextContext );
		im->resetTextIndex();
	}
}

