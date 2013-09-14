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

#include <nel/misc/types_nl.h>
#include "main_loop_utilities.h"

#include <nel/3d/u_driver.h>
#include <nel/3d/u_cloud_scape.h>

#include "game_share/scenario_entry_points.h"

#include "client_cfg.h"
#include "misc.h"
#include "global.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "user_entity.h"
#include "view.h"
#include "ig_client.h"
#include "entities.h"
#include "input.h"
#include "sound_manager.h"
#include "camera.h"

using namespace NLMISC;
using namespace NL3D;

void updateVRDevicesComboUI(); // from action_handler_game.cpp
void initStereoDisplayDevice(); // from init.cpp
void releaseStereoDisplayDevice(); // from release.cpp

//---------------------------------------------------
// Compare ClientCfg and LastClientCfg to know what we must update
//---------------------------------------------------
void updateFromClientCfg()
{
	CClientConfig::setValues();
	ClientCfg.IsInvalidated = false;

	// GRAPHICS - GENERAL
	//---------------------------------------------------
	if ((ClientCfg.Windowed != LastClientCfg.Windowed)	||
		(ClientCfg.Width != LastClientCfg.Width)		||
		(ClientCfg.Height != LastClientCfg.Height)		||
		(ClientCfg.Depth != LastClientCfg.Depth)		||
		(ClientCfg.Frequency != LastClientCfg.Frequency))
	{
		setVideoMode(UDriver::CMode(ClientCfg.Width, ClientCfg.Height, (uint8)ClientCfg.Depth,
									ClientCfg.Windowed, ClientCfg.Frequency));
	}

	if (ClientCfg.DivideTextureSizeBy2 != LastClientCfg.DivideTextureSizeBy2)
	{
		if (ClientCfg.DivideTextureSizeBy2)
			Driver->forceTextureResize(2);
		else
			Driver->forceTextureResize(1);
	}

	//---------------------------------------------------
	if (ClientCfg.WaitVBL != LastClientCfg.WaitVBL)
	{
		if(ClientCfg.WaitVBL)
			Driver->setSwapVBLInterval(1);
		else
			Driver->setSwapVBLInterval(0);
	}

	if (ClientCfg.VREnable != LastClientCfg.VREnable)
	{
		// VR_CONFIG

		// switch visibility of list & reload
		updateVRDevicesComboUI();
	}
	else
	{
		if (((StereoDisplay != NULL) != ClientCfg.VREnable)
			|| (ClientCfg.VREnable && (
				ClientCfg.VRDisplayDevice != LastClientCfg.VRDisplayDevice
				|| ClientCfg.VRDisplayDeviceId != LastClientCfg.VRDisplayDeviceId
				)))
		{
			nldebug("Apply VR device change");
			releaseStereoDisplayDevice();
			initStereoDisplayDevice();
		}
	}

	// GRAPHICS - LANDSCAPE
	//---------------------------------------------------
	if (ClientCfg.LandscapeThreshold != LastClientCfg.LandscapeThreshold)
	{
		if (Landscape) Landscape->setThreshold(ClientCfg.getActualLandscapeThreshold());
	}

	//---------------------------------------------------
	if (ClientCfg.LandscapeTileNear != LastClientCfg.LandscapeTileNear)
	{
		if (Landscape) Landscape->setTileNear(ClientCfg.LandscapeTileNear);
	}

	//---------------------------------------------------
	if (Landscape)
	{
		if (ClientCfg.Vision != LastClientCfg.Vision)
		{
			if (!ClientCfg.Light)
			{
				// Not in an indoor ?
				if (ContinentMngr.cur() && !ContinentMngr.cur()->Indoor)
				{
					// Refresh All Zone in streaming according to the refine position
					std::vector<string>		zonesAdded;
					std::vector<string>		zonesRemoved;
					const R2::CScenarioEntryPoints::CCompleteIsland *ci = R2::CScenarioEntryPoints::getInstance().getCompleteIslandFromCoords(CVector2f((float) UserEntity->pos().x, (float) UserEntity->pos().y));
					Landscape->refreshAllZonesAround(View.refinePos(), ClientCfg.Vision + ExtraZoneLoadingVision, zonesAdded, zonesRemoved, ProgressBar, ci ? &(ci->ZoneIDs) : NULL);
					LandscapeIGManager.unloadArrayZoneIG(zonesRemoved);
					LandscapeIGManager.loadArrayZoneIG(zonesAdded);
				}
			}
		}
	}

	//---------------------------------------------------
	if (ClientCfg.Vision != LastClientCfg.Vision || ClientCfg.FoV!=LastClientCfg.FoV ||
		ClientCfg.Windowed != LastClientCfg.Windowed || ClientCfg.ScreenAspectRatio != LastClientCfg.ScreenAspectRatio )
	{
		updateCameraPerspective();
	}

	//---------------------------------------------------
	if (Landscape)
	{
		if (ClientCfg.MicroVeget != LastClientCfg.MicroVeget)
		{
			if(ClientCfg.MicroVeget)
			{
				// if configured, enable the vegetable and load the texture.
				Landscape->enableVegetable(true);
				// Default setup. TODO later by gameDev.
				Landscape->setVegetableWind(CVector(0.5, 0.5, 0).normed(), 0.5, 1, 0);
				// Default setup. should work well for night/day transition in 30 minutes.
				// Because all vegetables will be updated every 20 seconds => 90 steps.
				Landscape->setVegetableUpdateLightingFrequency(1/20.f);
				// Density (percentage to ratio)
				Landscape->setVegetableDensity(ClientCfg.MicroVegetDensity/100.f);
			}
			else
			{
				Landscape->enableVegetable(false);
			}
		}
	}

	//---------------------------------------------------
	if (ClientCfg.MicroVegetDensity != LastClientCfg.MicroVegetDensity)
	{
		// Density (percentage to ratio)
		if (Landscape) Landscape->setVegetableDensity(ClientCfg.MicroVegetDensity/100.f);
	}

	// GRAPHICS - SPECIAL EFFECTS
	//---------------------------------------------------
	if (ClientCfg.FxNbMaxPoly != LastClientCfg.FxNbMaxPoly)
	{
		if (Scene->getGroupLoadMaxPolygon("Fx") != ClientCfg.FxNbMaxPoly)
			Scene->setGroupLoadMaxPolygon("Fx", ClientCfg.FxNbMaxPoly);
	}

	//---------------------------------------------------
	if (ClientCfg.Cloud != LastClientCfg.Cloud)
	{
		if (ClientCfg.Cloud)
		{
			InitCloudScape = true;
			CloudScape = Scene->createCloudScape();
		}
		else
		{
			if (CloudScape != NULL)
				Scene->deleteCloudScape(CloudScape);
			CloudScape = NULL;
		}
	}

	//---------------------------------------------------
	if (ClientCfg.CloudQuality != LastClientCfg.CloudQuality)
	{
		if (CloudScape != NULL)
			CloudScape->setQuality(ClientCfg.CloudQuality);
	}

	//---------------------------------------------------
	if (ClientCfg.CloudUpdate != LastClientCfg.CloudUpdate)
	{
		if (CloudScape != NULL)
			CloudScape->setNbCloudToUpdateIn80ms(ClientCfg.CloudUpdate);
	}

	//---------------------------------------------------
	if (ClientCfg.Shadows != LastClientCfg.Shadows)
	{
		// Enable/Disable Receive on Landscape
		if(Landscape)
		{
			Landscape->enableReceiveShadowMap(ClientCfg.Shadows);
		}
		// Enable/Disable Cast for all entities
		for(uint i=0;i<EntitiesMngr.entities().size();i++)
		{
			CEntityCL	*ent= EntitiesMngr.entities()[i];
			if(ent)
				ent->updateCastShadowMap();
		}
	}

	// GRAPHICS - CHARACTERS
	//---------------------------------------------------
	if (ClientCfg.SkinNbMaxPoly != LastClientCfg.SkinNbMaxPoly)
	{
		if (Scene->getGroupLoadMaxPolygon("Skin") != ClientCfg.SkinNbMaxPoly)
			Scene->setGroupLoadMaxPolygon("Skin", ClientCfg.SkinNbMaxPoly);
	}

	//---------------------------------------------------
	if (ClientCfg.NbMaxSkeletonNotCLod != LastClientCfg.NbMaxSkeletonNotCLod )
	{
		Scene->setMaxSkeletonsInNotCLodForm(ClientCfg.NbMaxSkeletonNotCLod);
	}

	//---------------------------------------------------
	if (ClientCfg.CharacterFarClip != LastClientCfg.CharacterFarClip)
	{
		// Nothing to do
	}

	//---------------------------------------------------
	if (ClientCfg.HDEntityTexture != LastClientCfg.HDEntityTexture)
	{
		// Don't reload Texture, will be done at next Game Start
	}

	// INTERFACE works


	// INPUTS
	//---------------------------------------------------
	if (ClientCfg.CursorSpeed != LastClientCfg.CursorSpeed)
		SetMouseSpeed (ClientCfg.CursorSpeed);

	if (ClientCfg.CursorAcceleration != LastClientCfg.CursorAcceleration)
		SetMouseAcceleration (ClientCfg.CursorAcceleration);

	if (ClientCfg.HardwareCursor != LastClientCfg.HardwareCursor)
	{
		if (ClientCfg.HardwareCursor != IsMouseCursorHardware())
		{
			InitMouseWithCursor (ClientCfg.HardwareCursor);
		}
	}


	// SOUND
	//---------------------------------------------------
	bool	mustReloadSoundMngrContinent= false;

	// disable/enable sound?
	if (ClientCfg.SoundOn != LastClientCfg.SoundOn)
	{
		if (SoundMngr && !ClientCfg.SoundOn)
		{
			nlwarning("Deleting sound manager...");
			delete SoundMngr;
			SoundMngr = NULL;
		}
		else if (SoundMngr == NULL && ClientCfg.SoundOn)
		{
			nlwarning("Creating sound manager...");
			SoundMngr = new CSoundManager();
			try
			{
				SoundMngr->init(NULL);
			}
			catch(const Exception &e)
			{
				nlwarning("init : Error when creating 'SoundMngr' : %s", e.what());
				SoundMngr = 0;
			}

			// re-init with good SFX/Music Volume
			if(SoundMngr)
			{
				SoundMngr->setSFXVolume(ClientCfg.SoundSFXVolume);
				SoundMngr->setGameMusicVolume(ClientCfg.SoundGameMusicVolume);
			}
		}
		else
		{
			nlwarning("Sound config error !");
		}

		mustReloadSoundMngrContinent= true;
	}

	// change EAX?
	if ( SoundMngr && LastClientCfg.SoundOn &&
		(ClientCfg.UseEax != LastClientCfg.UseEax) )
	{
		SoundMngr->reset();

		mustReloadSoundMngrContinent= true;
	}

	// change SoundForceSoftwareBuffer?
	if ( SoundMngr && LastClientCfg.SoundOn &&
		(ClientCfg.SoundForceSoftwareBuffer != LastClientCfg.SoundForceSoftwareBuffer) )
	{
		SoundMngr->reset();

		mustReloadSoundMngrContinent= true;
	}

	// change MaxTrack? don't reset
	if ( SoundMngr && LastClientCfg.SoundOn &&
		(ClientCfg.MaxTrack != LastClientCfg.MaxTrack))
	{
		SoundMngr->getMixer()->changeMaxTrack(ClientCfg.MaxTrack);
	}

	// change SoundFX Volume? don't reset
	if (SoundMngr && ClientCfg.SoundSFXVolume != LastClientCfg.SoundSFXVolume)
	{
		SoundMngr->setSFXVolume(ClientCfg.SoundSFXVolume);
	}

	// change Game Music Volume? don't reset
	if (SoundMngr && ClientCfg.SoundGameMusicVolume != LastClientCfg.SoundGameMusicVolume)
	{
		SoundMngr->setGameMusicVolume(ClientCfg.SoundGameMusicVolume);
	}

	// reload only if active and reseted
	if (mustReloadSoundMngrContinent && SoundMngr && ContinentMngr.cur() && !ContinentMngr.cur()->Indoor && UserEntity)
	{
		SoundMngr->loadContinent(ContinentMngr.getCurrentContinentSelectName(), UserEntity->pos());
	}

	// Ok backup the new clientcfg
	LastClientCfg = ClientCfg;
}

/* end of file */