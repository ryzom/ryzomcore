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
#include "main_loop_debug.h"

#include <nel/3d/u_text_context.h>
#include <nel/gui/lua_ihm.h>

#include "game_share/ryzom_version.h"

#include "global.h"
#include "client_cfg.h"
#include "user_entity.h"
#include "debug_client.h"
#include "entities.h"
#include "motion/user_controls.h"
#include "pacs_client.h"
#include "sound_manager.h"
#include "view.h"
#include "prim_file.h"
#include "weather.h"
#include "light_cycle_manager.h"
#include "net_manager.h"
#include "ping.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "client_sheets/weather_function_params_sheet.h"
#include "weather_manager_client.h"
#include "fog_map.h"
#include "misc.h"
#include "interface_v3/interface_manager.h"

using namespace NLMISC;
using namespace NL3D;
using namespace NLGUI;

// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************

extern std::set<std::string> LodCharactersNotFound;
extern uint32 NbDatabaseChanges;
extern CFogState MainFogState;
extern CPing Ping;
extern bool Filter3D[RYZOM_MAX_FILTER_3D];

//namespace /* anonymous */ {

NLMISC::CValueSmoother smoothFPS;
NLMISC::CValueSmoother moreSmoothFPS(64);

//} /* anonymous namespace */

//---------------------------------------------------
// displayDebug :
// Display some debug infos.
//---------------------------------------------------
void displayDebug()
{
	float lineStep = ClientCfg.DebugLineStep;
	float line;

	// Initialize Pen //
	//----------------//
	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(ClientCfg.DebugFontSize);
	// Set the text color
	TextContext->setColor(ClientCfg.DebugFontColor);

	// TOP LEFT //
	//----------//
	TextContext->setHotSpot(UTextContext::TopLeft);
	line = 0.9f;
	// FPS and Ms per frame
	{
		// smooth across frames.
		double deltaTime = smoothFPS.getSmoothValue ();
		// FPS and Ms per frame
		if(deltaTime != 0.f)
			TextContext->printfAt(0.f, line,"%.1f fps", 1.f/deltaTime);
		else
			TextContext->printfAt(0.f, line,"%.1f fps", 0.f);
		TextContext->printfAt(0.1f, line, "%d ms", (uint)(deltaTime*1000));
	}
	line -= lineStep;
	line -= lineStep;

	// USER
	// Front
	TextContext->printfAt(0.0f, line, "  %f (%f,%f,%f) front", atan2(UserEntity->front().y, UserEntity->front().x), UserEntity->front().x, UserEntity->front().y, UserEntity->front().z);
	line -= lineStep;
	// Dir
	TextContext->printfAt(0.0f, line, "  %f (%f,%f,%f) dir", atan2(UserEntity->dir().y, UserEntity->dir().x), UserEntity->dir().x, UserEntity->dir().y, UserEntity->dir().z);
	line -= lineStep;
	// NB Stage
	TextContext->printfAt(0.0f, line, "  NB Stage: %d", UserEntity->nbStage());
	line -= lineStep;
	// NB Animation FXs still remaining in the remove list.
	TextContext->printfAt(0.0f, line, "  NB FXs to remove: %d", UserEntity->nbAnimFXToRemove());
	line -= lineStep;
	// Mode.
	TextContext->printfAt(0.0f, line, "  Mode: %d (%s)", (sint)UserEntity->mode(), MBEHAV::modeToString(UserEntity->mode()).c_str());
	line -= lineStep;
	// Behaviour.
	TextContext->printfAt(0.0f, line, "  Behaviour: %d (%s)", (sint)UserEntity->behaviour(), MBEHAV::behaviourToString(UserEntity->behaviour()).c_str());
	line -= lineStep;
	// Display the target mount.
	TextContext->printfAt(0.0f, line, "  Mount: %d", UserEntity->mount());
	line -= lineStep;
	// Display the target rider.
	TextContext->printfAt(0.0f, line, "  Rider: %d", UserEntity->rider());
	line -= lineStep;
	// Display the current animation name.
	TextContext->printfAt(0.0f, line, "  Current Animation Name: %s", UserEntity->currentAnimationName().c_str());
	line -= lineStep;
	// Display the current move animation set name.
	TextContext->printfAt(0.0f, line, "  Current AnimationSet Name (MOVE): %s", UserEntity->currentAnimationSetName(MOVE).c_str());
	line -= lineStep;
	// Display Missing Animations
	if(::CAnimation::MissingAnim.empty() == false)
	{
		TextContext->printfAt(0.0f, line, "  '%u' Missing Animations, 1st: '%s'", ::CAnimation::MissingAnim.size(), (*(::CAnimation::MissingAnim.begin())).c_str());
		line -= lineStep;
	}
	// Display Missing LoD
	if(LodCharactersNotFound.empty() == false)
	{
		TextContext->printfAt(0.0f, line, "  '%u' Missing LoD, 1st: '%s'", LodCharactersNotFound.size(), (*(LodCharactersNotFound.begin())).c_str());
		line -= lineStep;
	}

	// Watched Entity
	line -= lineStep;
	// Now Displaying the selection.
	TextContext->printfAt(0.0f, line, "--*** Watched entity ***--");
	line -= lineStep;
	// Display information about the debug entity slot.
	if(WatchedEntitySlot != CLFECOMMON::INVALID_SLOT)
	{
		// Get a pointer on the target.
		CEntityCL *watchedEntity = EntitiesMngr.entity(WatchedEntitySlot);
		if(watchedEntity)
		{
			// Display Debug Information about the Selection.
			watchedEntity->displayDebug(0.0f, line, -lineStep);

			// Distance of the target
			CVectorD diffvector = UserEntity->pos() - watchedEntity->pos();
			TextContext->printfAt(0.0f, line, "  Distance: %10.2f (Manhattan: %.2f)", diffvector.norm(), fabs(diffvector.x) + fabs(diffvector.y) );
			line -= lineStep;
		}
		// Target not allocated
		else
		{
			TextContext->printfAt(0.0f, line, "Not allocated (%d)", WatchedEntitySlot);
			line -= lineStep;
		}
	}
	// No Target
	else
	{
		TextContext->printfAt(0.0f, line, "None");
		line -= lineStep;
	}

	/* Ca rame grave !

	  uint nMem = NLMEMORY::GetAllocatedMemory();
	line -= lineStep;
	TextContext->printfAt(0.0f, line, "Mem Used: %d",nMem);*/

	// 3D Filters information:
#ifdef _PROFILE_ON_
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "3D Filters:");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "MeshNoVP: %s", Filter3D[FilterMeshNoVP]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "MeshVP: %s", Filter3D[FilterMeshVP]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "FXs: %s", Filter3D[FilterFXs]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	if (Landscape)
	{
		TextContext->printfAt(0.0f, line, "Landscape: %s", Filter3D[FilterLandscape]?"Ok":"NOT RENDERED!");
		line-= lineStep;
	}
	else
	{
		TextContext->printfAt(0.0f, line, "Landscape not enabled");
	}
	TextContext->printfAt(0.0f, line, "Vegetable: %s", Filter3D[FilterVegetable]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Skeleton: %s", Filter3D[FilterSkeleton]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Water: %s", Filter3D[FilterWater]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Cloud: %s", Filter3D[FilterCloud]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "CoarseMesh: %s", Filter3D[FilterCoarseMesh]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Sky: %s", Filter3D[FilterSky]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	// Materials Infos
	TextContext->printfAt(0.0f, line, "SetupedMatrix: %d", Driver->profileSetupedModelMatrix() );
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "SetupedMaterials: %d", Driver->profileSetupedMaterials() );
	line-= lineStep;
	// Display camera cluster system
	TextContext->printfAt(0.0f, line, "ClusterSystem: %p", MainCam.getClusterSystem() );
	line-= 2 * lineStep;
	// Lua stuffs
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	TextContext->printfAt(0.0f, line, "Lua mem (kb) : %d / %d", CLuaManager::getInstance().getLuaState()->getGCCount(),  CLuaManager::getInstance().getLuaState()->getGCThreshold());
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Lua stack size = %d", CLuaManager::getInstance().getLuaState()->getTop());
	line-= lineStep;

#endif

	// TOP LEFT //
	//-----------//
	TextContext->setHotSpot(UTextContext::TopLeft);
	line = 1.f;
	string str;
#if FINAL_VERSION
	str = "FV";
#else
	str = "DEV";
#endif
	if(ClientCfg.ExtendedCommands)
		str += "_E";
	str += " "RYZOM_VERSION;
	TextContext->printfAt(0.f, line, "Version %s", str.c_str());

	// TOP MIDDLE //
	//------------//
	TextContext->setHotSpot(UTextContext::MiddleTop);
	line = 1.f;
	// Motion Mode
	TextContext->printfAt(0.5f, line, "%s", UserControls.modeStr().c_str());
	line -= lineStep;

	// TOP RIGHT //
	//-----------//
	TextContext->setHotSpot(UTextContext::TopRight);
	line = 1.f;
	//// 3D Infos
	// Video mem allocated.
	TextContext->printfAt(1.f, line, "Video mem. : %f", Driver->profileAllocatedTextureMemory()/(1024.f*1024.f));
	line -= lineStep;
	// Video mem used since last swapBuffers().
	TextContext->printfAt(1.f, line, "Video mem. since last swap buffer: %f", Driver->getUsedTextureMemory()/(1024.f*1024.f));
	line -= lineStep;
	// Get the last face count asked from the main scene before reduction.
	TextContext->printfAt(1.f, line, "Nb Skin Face Asked: %f", Scene->getGroupNbFaceAsked("Skin"));
	line -= lineStep;
	TextContext->printfAt(1.f, line, "Nb Fx Face Asked: %f", Scene->getGroupNbFaceAsked("Fx"));
	line -= lineStep;
	// All Triangles In
	CPrimitiveProfile pIn;
	CPrimitiveProfile pOut;
	Driver->profileRenderedPrimitives(pIn, pOut);
	TextContext->printfAt(1.f, line, "Tri In : %d", pIn.NTriangles+2*pIn.NQuads);
	line -= lineStep;
	// All Triangles Out
	TextContext->printfAt(1.f, line, "Tri Out : %d", pOut.NTriangles+2*pIn.NQuads);
	line -= lineStep;
	// Current Cluster
	string strPos;
	// Check there is a PACS Primitive before using it.
	if(UserEntity->getPrimitive() && GR)
	{
		UGlobalPosition gPos;
		UserEntity->getPrimitive()->getGlobalPosition(gPos, dynamicWI);
		string strPos = GR->getIdentifier(gPos);
	}
	else
		strPos = "No Primitive";
	TextContext->printfAt(1.f, line, "Cluster : %s", strPos.c_str());
	line -= lineStep;
	//// SOUND Infos
	line -= lineStep;
	if(SoundMngr)
	{
		TextContext->printfAt(1.f, line, "Sound source instance: %u", SoundMngr->getSourcesInstanceCount());
		line -= lineStep;
		TextContext->printfAt(1.f, line, "Logical playing SoundSource: %u", SoundMngr->getMixer()->getPlayingSourcesCount ());
		line -= lineStep;
		TextContext->printfAt(1.f, line, "Audio tracks: %u/%u", SoundMngr->getMixer()->getUsedTracksCount(), SoundMngr->getMixer()->getPolyphony());
		line -= lineStep;
		if (SoundMngr->getMixer()->getMutedPlayingSourcesCount() > 0)
		{
			TextContext->printfAt(1.f, line, "Source muted: %u !", SoundMngr->getMixer()->getMutedPlayingSourcesCount());
			line -= lineStep;
		}
		TextContext->printfAt(1.f, line, "Samples in memory: %g MB", SoundMngr->getLoadingSamplesSize() / (1024.0f*1024.0f));
		line -= lineStep;

	}

	// BOTTOM RIGHT //
	//--------------//
	TextContext->setHotSpot(UTextContext::BottomRight);
	line = 0.f;
	//// POSITION
	CVector postmp = View.viewPos();
	// Pos
	TextContext->printfAt(1.f, line, "Position : %d %d %d",(int)postmp.x,(int)postmp.y,(int)postmp.z);
	line += lineStep;
	// Body Heading
	TextContext->printfAt(1.f, line, "Front : %.2f %.2f %.2f", UserEntity->front().x, UserEntity->front().y, UserEntity->front().z);
	line += lineStep;
	// Speed
	TextContext->printfAt(1.f, line, "Speed : %.2f", (float) UserEntity->speed());
	line += lineStep;
	// Zone
	if (!ClientCfg.Light)
	{
		if (Landscape)
		{
			TextContext->printfAt(1.f, line, "Zone: %s", Landscape->getZoneName(postmp).c_str());
			line += lineStep;
		}
	}
	// Prim File
	string primFile = PrimFiles.getCurrentPrimitive ();
	if (!primFile.empty ())
	{
		TextContext->printfAt(1.f, line, "Prim File: %s", primFile.c_str ());
		line += lineStep;
	}

	//// CONNECTION
	line += lineStep;
	// Ryzom Day.
	TextContext->printfAt(1.f, line, "Ryzom Day : %d", RT.getRyzomDay());
	line += lineStep;
	// hour in the game
	float dayNightCycleHour = (float)RT.getRyzomTime();
	TextContext->printfAt(1.f, line, "Ryzom Time : %2u:%02u", int(dayNightCycleHour), int((dayNightCycleHour-int(dayNightCycleHour))*60.0f));
	line += lineStep;
	// light hour in the game, used to display te day/night
	TextContext->printfAt(1.f, line, "Ryzom Light Time : %2u:%02u (%s)", int(DayNightCycleHour), int((DayNightCycleHour-int(DayNightCycleHour))*60.0f), LightCycleManager.getStateString().c_str());
	line += lineStep;
	// Server GameCycle
	TextContext->printfAt(1.f, line, "Server GameCycle : %u", (uint)NetMngr.getCurrentServerTick());
	line += lineStep;
	// Current GameCycle
	TextContext->printfAt(1.f, line, "Current GameCycle : %u", (uint)NetMngr.getCurrentClientTick());
	line += lineStep;
	// Current GameCycle
	TextContext->printfAt(1.f, line, "Ms per Cycle : %d", NetMngr.getMsPerTick());
	line += lineStep;
	// Packet Loss
	TextContext->printfAt(1.f, line, "Packet Loss : %.1f %%", NetMngr.getMeanPacketLoss()*100.0f);
	line += lineStep;
	// Packet Loss
	TextContext->printfAt(1.f, line, "Packets Lost : %u", NetMngr.getTotalLostPackets());
	line += lineStep;
	// Mean Upload
	TextContext->printfAt(1.f, line, "Mean Upld : %.3f kbps", NetMngr.getMeanUpload());
	line += lineStep;
	// Mean Download
	TextContext->printfAt(1.f, line, "Mean Dnld : %.3f kbps", NetMngr.getMeanDownload());
	line += lineStep;

	// Mean Download
	TextContext->printfAt(1.f, line, "Nb in Vision : %d(%d,%d,%d)",
		EntitiesMngr.nbEntitiesAllocated(),
		EntitiesMngr.nbUser(),
		EntitiesMngr.nbPlayer(),
		EntitiesMngr.nbChar());
	line += lineStep;

	// Number of database changes
	TextContext->printfAt(1.f, line, "DB Changes : %u", NbDatabaseChanges );
	line += lineStep;

	// Ping
	TextContext->printfAt(1.f, line, "DB Ping : %u ms", Ping.getValue());
	line += lineStep;





	// Manual weather setup
	{
		if(ContinentMngr.cur())	// Only usable if there is a continent loaded.
		{
			if (!ForceTrueWeatherValue)
			{
				const CWeatherFunction &wf = ContinentMngr.cur()->WeatherFunction[CurrSeason];
				float wv;
				if (ClientCfg.ManualWeatherSetup)
				{
					wv = std::max(wf.getNumWeatherSetups() - 1, 0u) * ManualWeatherValue;
				}
				else
				{
					wv = std::max(wf.getNumWeatherSetups() - 1, 0u) * ::getBlendedWeather(RT.getRyzomDay(), RT.getRyzomTime(), *WeatherFunctionParams, ContinentMngr.cur()->WeatherFunction);
				}
				const CWeatherSetup *ws = wf.getWeatherSetup((uint) floorf(wv));
				std::string name0 = ws ? NLMISC::CStringMapper::unmap(ws->SetupName) : "???";
				ws = wf.getWeatherSetup(std::min((uint) (floorf(wv) + 1), wf.getNumWeatherSetups() - 1));
				std::string name1 = ws ? NLMISC::CStringMapper::unmap(ws->SetupName) : "???";
				TextContext->printfAt(1.f, line, "Weather value : %.02f : %s -> %s", ws ? wv : 0.f, name0.c_str(), name1.c_str());
				line += lineStep;
			}
			else
			{
				TextContext->printfAt(1.f, line, "Weather value : %.02f", WeatherManager.getWeatherValue() * std::max(ContinentMngr.cur()->WeatherFunction[CurrSeason].getNumWeatherSetups() - 1, 0u));
				line += lineStep;
				TextContext->printfAt(1.f, line, "TEST WEATHER FUNCTION");
				line += lineStep;
			}
			// season
			TextContext->printfAt(1.f, line, "Season : %s", EGSPD::CSeason::toString(CurrSeason).c_str());
			line += lineStep;
		}
	}

	// fog dist
	if (ContinentMngr.cur())
	{
		TextContext->printfAt(1.f, line, "Continent fog min near = %.1f, max far = %.1f", ContinentMngr.cur()->FogStart, ContinentMngr.cur()->FogEnd);
		line += lineStep;
		CFogState tmpFog;
		ContinentMngr.getFogState(MainFog, LightCycleManager.getLightLevel(), LightCycleManager.getLightDesc().DuskRatio, LightCycleManager.getState(), View.viewPos(), tmpFog);
		TextContext->printfAt(1.f, line, "Continent fog curr near = %.1f, curr far = %.1f", tmpFog.FogStartDist, tmpFog.FogEndDist);
		line += lineStep;
	}
	const CWeatherState &ws = WeatherManager.getCurrWeatherState();
	TextContext->printfAt(1.f, line, "Weather fog near = %.1f, far = %.1f", ws.FogNear[MainFog], ws.FogFar[MainFog]);
	line += lineStep;
	TextContext->printfAt(1.f, line, "Final fog near = %.1f, far = %.1f", MainFogState.FogStartDist, MainFogState.FogEndDist);
	line += lineStep;
	float left, right, bottom, top, znear, zfar;
	Scene->getCam().getFrustum(left, right, bottom, top, znear, zfar);
	TextContext->printfAt(1.f, line, "Clip near = %.1f, far = %.1f", znear, zfar);
	line += lineStep;

	// Connection states
	TextContext->printfAt(1.f, line, "State : %s", NetMngr.getConnectionStateCStr() );
	line += lineStep;

//	UGlobalPosition globalPos;
//	UserEntity->getPrimitive()->getGlobalPosition(globalPos, dynamicWI);
//	uint32 material = GR->getMaterial( globalPos );
//	TextContext->printfAt(0.5f,0.5f,"Material : %d Gpos=(inst=%d,surf=%d,x=%.2f,y=%.2f",material, globalPos.InstanceId, globalPos.LocalPosition.Surface, globalPos.LocalPosition.Estimation.x, globalPos.LocalPosition.Estimation.y);

	// No more shadow when displaying a text.
	TextContext->setShaded(false);
}// displayDebug //

// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************

//---------------------------------------------------
// displayDebug :
// Display some debug infos.
//---------------------------------------------------
void displayDebugFps()
{
	float lineStep = ClientCfg.DebugLineStep;
	float line;

	// Initialize Pen //
	//----------------//
	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(ClientCfg.DebugFontSize);
	// Set the text color
	TextContext->setColor(ClientCfg.DebugFontColor);

	// TOP LEFT //
	//----------//
	TextContext->setHotSpot(UTextContext::TopLeft);
	line = 0.9f;
	// Ms per frame
	{
		float spf = smoothFPS.getSmoothValue ();
		// Ms per frame
		TextContext->printfAt(0.1f, line, "FPS %.1f ms - %.1f fps", spf*1000, 1.f/spf);
		line-= lineStep;
		// More Smoothed Ms per frame
		spf = moreSmoothFPS.getSmoothValue ();
		TextContext->printfAt(0.1f, line, "Smoothed FPS %.1f ms - %.1f fps", spf*1000, 1.f/spf);
		line-= lineStep;
	}
}

// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************

static NLMISC::CRefPtr<CInterfaceElement> HighlightedDebugUI;

// displayDebug :
// Display information about ui elements that are under the mouse
//---------------------------------------------------
void displayDebugUIUnderMouse()
{
	float lineStep = ClientCfg.DebugLineStep;
	float line;

	// Initialize Pen //
	//----------------//
	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(ClientCfg.DebugFontSize);



	// TOP LEFT //
	//----------//
	TextContext->setHotSpot(UTextContext::TopLeft);
	line = 0.9f;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	// for now only accessible with R2ED
	if (ClientCfg.R2EDEnabled)
	{
		TextContext->setColor(CRGBA::Cyan);
		TextContext->printfAt(0.1f, line, "Press default key (ctrl+shift+A) to cycle prev");
		line-= lineStep;
		TextContext->printfAt(0.1f, line, "Press default key (ctrl+shift+Q) to cycle next");
		line-= lineStep;
		TextContext->printfAt(0.1f, line, "Press default key (ctrl+shift+W) to inspect element");
		line-= 2 * lineStep;
	}
	//
	const std::vector<CCtrlBase *> &rICL = CWidgetManager::getInstance()->getCtrlsUnderPointer ();
	const std::vector<CInterfaceGroup *> &rIGL = CWidgetManager::getInstance()->getGroupsUnderPointer ();
	// If previous highlighted element is found in the list, then keep it, else reset to first element
	if (std::find(rICL.begin(), rICL.end(), HighlightedDebugUI) == rICL.end() &&
		std::find(rIGL.begin(), rIGL.end(), HighlightedDebugUI) == rIGL.end())
	{
		if (!rICL.empty())
		{
			HighlightedDebugUI = rICL[0];
		}
		else
		if (!rIGL.empty())
		{
			HighlightedDebugUI = rIGL[0];
		}
		else
		{
			HighlightedDebugUI = NULL;
		}
	}
	//
	TextContext->setColor(CRGBA::Green);
	TextContext->printfAt(0.1f, line, "Controls under cursor ");
	line -= lineStep * 1.4f;
	TextContext->printfAt(0.1f, line, "----------------------");
	line -= lineStep;
	for(uint k = 0; k < rICL.size(); ++k)
	{
		if (rICL[k])
		{
			TextContext->setColor(rICL[k] != HighlightedDebugUI ? ClientCfg.DebugFontColor : CRGBA::Red);
			TextContext->printfAt(0.1f, line, "id = %s, address = 0x%p, parent = 0x%p", rICL[k]->getId().c_str(), rICL[k], rICL[k]->getParent());
		}
		else
		{
			TextContext->setColor(CRGBA::Blue);
			TextContext->printfAt(0.1f, line, "<NULL> control found !!!");
		}
		line-= lineStep;
	}
	//
	TextContext->setColor(CRGBA::Green);
	TextContext->printfAt(0.1f, line, "Groups under cursor ");
	line -= lineStep * 1.4f;
	TextContext->printfAt(0.1f, line, "----------------------");
	line-= lineStep;
	for(uint k = 0; k < rIGL.size(); ++k)
	{
		if (rIGL[k])
		{
			TextContext->setColor(rIGL[k] != HighlightedDebugUI ? ClientCfg.DebugFontColor : CRGBA::Red);
			TextContext->printfAt(0.1f, line, "id = %s, address = 0x%p, parent = 0x%p", rIGL[k]->getId().c_str(), rIGL[k], rIGL[k]->getParent());
		}
		else
		{
			TextContext->setColor(CRGBA::Blue);
			TextContext->printfAt(0.1f, line, "<NULL> group found !!!");
		}
		line-= lineStep;
	}
}

// get all element under the mouse in a single vector
static void getElementsUnderMouse(std::vector<CInterfaceElement *> &ielem)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	const std::vector<CCtrlBase *> &rICL = CWidgetManager::getInstance()->getCtrlsUnderPointer();
	const std::vector<CInterfaceGroup *> &rIGL = CWidgetManager::getInstance()->getGroupsUnderPointer();
	ielem.clear();
	ielem.insert(ielem.end(), rICL.begin(), rICL.end());
	ielem.insert(ielem.end(), rIGL.begin(), rIGL.end());
}

class CHandlerDebugUiPrevElementUnderMouse : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		std::vector<CInterfaceElement *> ielem;
		getElementsUnderMouse(ielem);
		for(uint k = 0; k < ielem.size(); ++k)
		{
			if (HighlightedDebugUI == ielem[k])
			{
				HighlightedDebugUI = ielem[k == 0 ? ielem.size() - 1 : k - 1];
				return;
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerDebugUiPrevElementUnderMouse, "debug_ui_prev_element_under_mouse");

class CHandlerDebugUiNextElementUnderMouse : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		std::vector<CInterfaceElement *> ielem;
		getElementsUnderMouse(ielem);
		for(uint k = 0; k < ielem.size(); ++k)
		{
			if (HighlightedDebugUI == ielem[k])
			{
				HighlightedDebugUI = ielem[(k + 1) % ielem.size()];
				return;
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerDebugUiNextElementUnderMouse, "debug_ui_next_element_under_mouse");

class CHandlerDebugUiDumpElementUnderMouse : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if (HighlightedDebugUI == NULL) return;
		CLuaState *lua = CLuaManager::getInstance().getLuaState();
		if (!lua) return;
		CLuaStackRestorer lsr(lua, 0);
		CLuaIHM::pushUIOnStack(*lua, HighlightedDebugUI);
		lua->pushGlobalTable();
		CLuaObject env(*lua);
		env["inspect"].callNoThrow(1, 0);
	}
};
REGISTER_ACTION_HANDLER( CHandlerDebugUiDumpElementUnderMouse, "debug_ui_inspect_element_under_mouse");

// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************

//-----------------------------------------------
// Macro to Display a Text
//-----------------------------------------------
#define DISP_TEXT(x, text)                    \
	/* Display the text at the right place */ \
	TextContext->printfAt(x, line, text);     \
	/* Change the line */                     \
	line += lineStep;                         \

//---------------------------------------------------
// displayHelp :
// Display an Help.
//---------------------------------------------------
void displayHelp()
{
	float line     = 1.f;
	float lineStep = -ClientCfg.HelpLineStep;

	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(ClientCfg.HelpFontSize);
	// Set the text color
	TextContext->setColor(ClientCfg.HelpFontColor);


	line = 1.f;
	TextContext->setHotSpot(UTextContext::TopLeft);
	DISP_TEXT(0.0f, "SHIFT + F1 : This Menu")
	DISP_TEXT(0.0f, "SHIFT + F2 : Display Debug Infos")
	DISP_TEXT(0.0f, "SHIFT + F3 : Wire mode");
	DISP_TEXT(0.0f, "SHIFT + F4 : Do not Render the Scene");
	DISP_TEXT(0.0f, "SHIFT + F5 : Toogle Display OSD interfaces");
//	DISP_TEXT(0.0f, "SHIFT + F6 : Not used");
	DISP_TEXT(0.0f, "SHIFT + F7 : Compass Mode (User/Camera)");
	DISP_TEXT(0.0f, "SHIFT + F8 : Camera Mode (INSERT to change your position)");
	DISP_TEXT(0.0f, "SHIFT + F9 : Free Mouse");
	DISP_TEXT(0.0f, "SHIFT + F10 : Take a Screen Shot (+CTRL) for jpg");
//	DISP_TEXT(0.0f, "SHIFT + F11 : Test");
	DISP_TEXT(0.0f, "SHIFT + ESCAPE : Quit");
	DISP_TEXT(0.0f, "SHIFT + C : First/Third Person View");

	line = 1.f;
	TextContext->setHotSpot(UTextContext::TopRight);
	DISP_TEXT(1.0f, "UP : FORWARD");
	DISP_TEXT(1.0f, "DOWN : BACKWARD");
	DISP_TEXT(1.0f, "LEFT : ROTATE LEFT");
	DISP_TEXT(1.0f, "RIGHT : ROTATE RIGHT");
	DISP_TEXT(1.0f, "CTRL + LEFT : STRAFE LEFT");
	DISP_TEXT(1.0f, "CTRL + RIGHT : STRAFE RIGHT");
	DISP_TEXT(1.0f, "END : Auto Walk");
	DISP_TEXT(1.0f, "DELETE : Walk/Run");
	DISP_TEXT(1.0f, "PG UP : Look Up");
	DISP_TEXT(1.0f, "PG DOWN : Look Down");
//	DISP_TEXT(1.0f, "CTRL + I : Inventory");
//	DISP_TEXT(1.0f, "CTRL + C : Spells composition interface");
//	DISP_TEXT(1.0f, "CTRL + S : Memorized Spells interface");
	DISP_TEXT(1.0f, "CTRL + B : Show/Hide PACS Borders");
	DISP_TEXT(1.0f, "CTRL + P : Player target himself");
	DISP_TEXT(1.0f, "CTRL + D : Unselect target");
	DISP_TEXT(1.0f, "CTRL + TAB : Next Chat Mode (say/shout");
	DISP_TEXT(1.0f, "CTRL + R : Reload Client.cfg File");
//	DISP_TEXT(1.0f, "CTRL + N : Toggle Night / Day lighting");
	DISP_TEXT(1.0f, "CTRL + F2 : Profile on / off");
	DISP_TEXT(1.0f, "CTRL + F3 : Movie Shooter record / stop");
	DISP_TEXT(1.0f, "CTRL + F4 : Movie Shooter replay");
	DISP_TEXT(1.0f, "CTRL + F5 : Movie Shooter save");
#ifndef NL_USE_DEFAULT_MEMORY_MANAGER
	DISP_TEXT(1.0f, "CTRL + F6 : Save memory stat report");
#endif // NL_USE_DEFAULT_MEMORY_MANAGER
	DISP_TEXT(1.0f, "CTRL + F7 : Show / hide prim file");
	DISP_TEXT(1.0f, "CTRL + F8 : Change prim file UP");
	DISP_TEXT(1.0f, "CTRL + F9 : Change prim file DOWN");

	// No more shadow when displaying a text.
	TextContext->setShaded(false);
}// displayHelp //

// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************

/* end of file */