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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Misc
#include "nel/misc/vectord.h"
#include "nel/misc/path.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/progress_callback.h"
#include "nel/misc/async_file_manager.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/polygon.h"
// 3D Interface.
#include "nel/3d/u_landscape.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_texture.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
// Client
#include "global.h"
#include "continent.h"
#include "light_cycle_manager.h"	// \todo GUIGUI : maybe change this when the time will be better managed.
#include "door_manager.h"
#include "client_cfg.h"
#include "ig_client.h"
#include "pacs_client.h"
#include "village.h"
#include "ig_callback.h"
#include "streamable_ig.h"
#include "weather_manager_client.h"
#include "weather.h"
#include "entities.h"
#include "fog_map.h"
#include "demo.h"
#include "ingame_database_manager.h"
#include "sky_render.h"
#include "fix_season_data.h"
#include "ig_season_callback.h"
#include "user_entity.h"
#include "micro_life_manager.h"
#include "zone_util.h"
#include "mesh_camera_col_manager.h"
#include "misc.h"
#include "sheet_manager.h"
#include "continent_manager.h"
#include "connection.h"
#include "water_env_map_rdr.h"
#include "input.h"
#include "bg_downloader_access.h"

// Game Share
#include "game_share/georges_helper.h"
#include "game_share/season_file_ext.h"

// R2 Share
#include "game_share/scenario_entry_points.h"


#include "r2/editor.h"

// std
#include <vector>
#include <string>
#include <memory>
#include <algorithm>


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace std;
using namespace NLGEORGES;


////////////
// EXTERN //
////////////
extern ULandscape *Landscape;
extern UScene	  *Scene;
extern UScene	  *SceneRoot;
extern UInstanceGroup *BackgroundIG;
extern UDriver	  *Driver;
extern UVisualCollisionManager	*CollisionManager;
extern bool		   InitCloudScape;
extern bool			FirstFrame;
extern CContinentManager ContinentMngr;


//-----------------------------------------------
// CUserLandMark 
//-----------------------------------------------

NLMISC::CRGBA	CUserLandMark::getColor () const
{
	return _LandMarksColor[Type];
}


//-----------------------------------------------
// CFogOfWar
//-----------------------------------------------

//===================================================================================
CFogOfWar::~CFogOfWar()
{
	if (Tx != NULL)
		Driver->deleteTextureMem(Tx);
}

//===================================================================================
uint8 *CFogOfWar::getData()
{
	return (Tx != NULL) ? Tx->getPointer() : NULL;
}

//===================================================================================
sint16 CFogOfWar::getRealWidth()
{
	return (Tx != NULL) ? (sint16)Tx->getImageWidth() : 0;
}

//===================================================================================
sint16 CFogOfWar::getRealHeight()
{
	return (Tx != NULL) ? (sint16)Tx->getImageHeight() : 0;
}

//===================================================================================
bool CFogOfWar::createData(sint16 w, sint16 h)
{
	if (Tx != NULL)
		Driver->deleteTextureMem(Tx);

	MapWidth = w;
	MapHeight = h;
	uint32 WReal = NLMISC::raiseToNextPowerOf2(MapWidth);
	uint32 HReal = NLMISC::raiseToNextPowerOf2(MapHeight);

	Tx = Driver->createTextureMem(WReal, HReal, CBitmap::Alpha);
	if (Tx == NULL)
		return false;

	Tx->setWrapS(NL3D::UTexture::Clamp);
	Tx->setWrapT(NL3D::UTexture::Clamp);

	uint8 *pData = Tx->getPointer();
	// Set the texture to black (not discovered)
	for (uint32 j = 0; j < WReal*HReal; ++j)
		pData[j] = 0;

	// Upload it
	Tx->touch();
	return true;
}

//===================================================================================
void CFogOfWar::explored(sint16 /* mapPosX */, sint16 /* mapPosY */)
{
	// TODO trap : do something better than upload the whole texture
	if (Tx != NULL)
		Tx->touch();
}


//===================================================================================
void CFogOfWar::load(const std::string &/* contName */)
{
/* // Temporary comment : see this when server ready
	string fileName = "save/fow_" + contName + "_" + PlayerSelectedFileName + ".ibmp";
	// Try to load the file corresponding
	CIFile inFile;
	if (inFile.open(fileName))
	{
		serial(inFile);
	}
	else
	{
		// If the file do not exist create a new texture
		// Look in all the maps to know the texture map size and apply a ratio to this size
		CWorldSheet *pWS = dynamic_cast<CWorldSheet*>(SheetMngr.get(CSheetId("ryzom.world")));
		if (pWS == NULL) return;
		for (uint32 i = 0; i < pWS->Maps.size(); ++i)
		{
			SMap &rM = pWS->Maps[i];
			if (strncmp(rM.Name.c_str(), "continent", 9) == 0)
			{
				if (stricmp(rM.ContinentName.c_str(), contName.c_str()) == 0)
				{
					// get the size of the bitmap
					uint32 nWidth = 0, nHeight = 0;
					string path = CPath::lookup(rM.BitmapName, false);
					if (!path.empty())
						CBitmap::loadSize(path, nWidth, nHeight);
					// create the texture fow
					if ((nWidth != 0) && (nHeight != 0))
					{
						MapWidth = (sint16)nWidth / 4;
						MapHeight = (sint16)nHeight / 4;
						createData(MapWidth, MapHeight);
					}
					MinX = rM.MinX;
					MinY = rM.MinY;
					MaxX = rM.MaxX;
					MaxY = rM.MaxY;
					return;
				}
			}
		}
	}
*/
}

//===================================================================================
void CFogOfWar::save(const std::string &/* contName */)
{
/* // Temporary comment : see this when server ready
	string fileName = "save/fow_" + contName + "_" + PlayerSelectedFileName + ".ibmp";
	// Try to load the file corresponding
	COFile inFile;
	if (inFile.open(fileName))
		serial(inFile);
*/
}


/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CContinent :
// Constructor.
//-----------------------------------------------
CContinent::CContinent()
{
//	Newbie = false;
}// CContinent //


class CChangeTaskPriority : public CTaskManager::IChangeTaskPriority
{
	virtual float getTaskPriority(const IRunnable &runnable)
	{
		const NLMISC::IRunnablePos *_3dRunnable = dynamic_cast<const NLMISC::IRunnablePos*>(&runnable);
		if (_3dRunnable)
		{
			return (_3dRunnable->Position - UserEntity->pos()).norm();
		}
		return 0;
	}
} ChangeTaskPriority;




//===================================================================================
void CContinent::setup()
{
	if (SheetName.empty())
	{
		nlwarning("SheetName not set : cannot setup continent");
		return;
	}

	CEntitySheet *pES = SheetMngr.get(CSheetId (SheetName));
	CContinentSheet *pCS = NULL;
	if (pES->type() == CEntitySheet::CONTINENT)
		pCS = (CContinentSheet*)pES;

	if (pCS == NULL)
	{
		nlwarning("Bad type for continent form %s.", SheetName.c_str());
		return;
	}

	// copy basics parameters of sheet
	CContinentParameters::operator = (pCS->Continent);

	// setup weather functions
	for(uint k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		WeatherFunction[k].buildFromSheet(pCS->WeatherFunction[k], WeatherManager);
	}

	/*
	nlinfo("Seasons for continent %s", SheetName.c_str());
	nlinfo("==================================");
	for(uint k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		nlinfo("Season = %s, num setups = %d", EGSPD::CSeason::toString((EGSPD::CSeason::TSeason) k).c_str(), (int) WeatherFunction[k].getNumWeatherSetups());
		for(uint l = 0; l < WeatherFunction[k].getNumWeatherSetups(); ++l)
		{
			if (WeatherFunction[k].getWeatherSetup(l))
			{
				nlinfo("Setup %d for season %s is %s", (int) l, EGSPD::CSeason::toString((EGSPD::CSeason::TSeason) k).c_str(), NLMISC::CStringMapper::unmap(WeatherFunction[k].getWeatherSetup(l)->SetupName).c_str());
			}
			else
			{
				nlinfo("Setup %d not found", (int) l);
			}
		}
	}
	*/

	uint k;
	// setup villages
	for(k = 0; k < pCS->Villages.size(); ++k)
	{
		CVillage *village = new CVillage;
		if (village->setupFromSheet(Scene, pCS->Villages[k], &IGLoaded))
			_Villages.add(village);
		else
			delete village;

	}

	// Setup outpost
	_Outposts.clear();
	for(k = 0; k < pCS->Continent.ZCList.size(); ++k)
	{
		// must add the ruins in 3D?
		bool	mustAddRuins= pCS->Continent.ZCList[k].EnableRuins;

		// add a village for ruins display?
		CVillage	*village= NULL;
		if(mustAddRuins)
		{
			village = new CVillage;
			if (village->setupOutpost(Scene, pCS->Continent.ZCList[k], k, &IGLoaded))
				_Villages.add(village);
			else
			{
				delete village;
				village= NULL;
			}
		}

		// add an outpost
		COutpost	outpost;
		if (outpost.setupOutpost(pCS->Continent.ZCList[k], k, village))
			_Outposts.push_back(outpost);
	}

	// setup fog of war
	FoW.load(Name);
}



//-----------------------------------------------
class CPCBank : public IProgressCallback
{
	virtual void progress (float /* progressValue */) {}
};
CPCBank PCBank;

static uint getNumZones()
{
	if (Landscape == NULL) return 0;
	std::vector<std::string> zoneLoaded;
	Landscape->getAllZoneLoaded(zoneLoaded);
	return (uint)zoneLoaded.size();
}

//-----------------------------------------------
// select :
// Update global parameters like the texture for the micro veget.
//-----------------------------------------------
void CContinent::select(const CVectorD &pos, NLMISC::IProgressCallback &progress, bool complete, bool unhibernate, EGSPD::CSeason::TSeason season)
{
	pauseBGDownloader();
	CNiceInputAuto niceInputs;
	// Season has changed ?
	Season = season;

	if (Landscape) Landscape->setTileColor(TileColorMono[Season], TileColorFactor[Season]);

	// First frame
	FirstFrame = true;

	{
		H_AUTO(InitRZWorldLanscapeShow)
		// If continent is an indoor, hde the landscape
		if (Landscape)
		{
			if (Indoor)
				Landscape->hide();
			else
				Landscape->show();
		}
	}

	bool toto = false;
	if(complete)
	{
		// Progress bar
		progress.progress (0);
		progress.pushCropedValues (0, 1.f/3.f);

		{
			H_AUTO(InitRZWorldLightCycle)
			loadWorldLightCycle(CSheetId(LightCycle));
		}

		{
			H_AUTO(InitRZWorldIndoor)
			if (!ClientCfg.Light)
			{
				if (Landscape)
				{
					Landscape->removeAllZones();
					toto= true;
					// If continent is an indoor, hde the landscape
					if (!Indoor)
					{
						// Change the MicroVeget Texture.
						string microVegetSeason = MicroVeget;
						SeasonPostfixTextureFilename (microVegetSeason, Season);
						Landscape->loadVegetableTexture (microVegetSeason);

						// Change the Material for static and dynamic PointLights
						Landscape->setPointLightDiffuseMaterial(LandscapePointLightMaterial);
					}
				}
			}
		}
		// Initialize Landscape IG Callbacks.
		{
			H_AUTO(InitRZWorldInitCallbacks)
			initLandscapeIGCallbacks();
			nlassert(IGCallbacks); // forgot to call ::initLandscapeIGCallbacks()
			if (!ClientCfg.Light)
			{
				// Add weather as an observer (to know when an ig is added, and to set its lightmaps color accordingly)
				registerObserver(&LightCycleManager);
				IGCallbacks->registerObserver(&LightCycleManager);

				// register the LightCycleManager as an observer to Objects that may create IGs
				IGSeasonCallback.Season = Season;
				registerObserver(&IGSeasonCallback);
				IGCallbacks->registerObserver(&IGSeasonCallback);

				// register the Camera collision manager
				registerObserver(&MeshCameraColManager);

				if (!Indoor)
				{
					// Change LOD texture
					try
					{
						// Get the CoarseMeshMap for this season
						string seasonname = CoarseMeshMap;
						SeasonPostfixTextureFilename (seasonname, Season);

						// Set the texture for the coarse mesh manager
						Scene->setCoarseMeshManagerTexture (CPath::lookup(seasonname).c_str());
					}
					catch (const Exception &e)
					{
						nlwarning (e.what());
					}
				}
			}
			// Register door handling (when an ig is added check all shapes for doors and add to the door manager)
			registerObserver(&IGDoorCallback);
		}

		{
			H_AUTO(InitRZWorldPacs)
			releasePACS();
			// Init PACS
			std::string pacsRBankPath = CPath::lookup(PacsRBank, false);
			std::string pacsGRPath = CPath::lookup(PacsGR, false);
			if(!pacsRBankPath.empty() && !pacsGRPath.empty())
				initPACS(pacsRBankPath.c_str(), pacsGRPath.c_str(), progress);
			else
				initPACS(0, 0, progress);
			// Set the move container
			if (IGCallbacks) IGCallbacks->setMoveContainer(PACS);
		}

		{
			H_AUTO(InitRZWorldBuildings)
			// No landscape IG in indoors
			if (!Indoor)
			{
				if (!ClientCfg.Light && Landscape)
				{
					// Init ig
					try
					{
						LandscapeIGManager.initIG (Scene, CPath::lookup(LandscapeIG).c_str(), Driver, Season, &progress);
					}
					catch (const Exception &e)
					{
						nlwarning (e.what());
					}
				}

				// Get IGs for the continent with their name.
				vector<pair<UInstanceGroup *, string> > igsWithNames;
				LandscapeIGManager.getAllIGWithNames(igsWithNames);

				// Associate IGs with the ZC number or -1 if there is no ZC.
				for(uint i = 0; i<igsWithNames.size(); ++i)
				{
					string igZone = strlwr(CFile::getFilenameWithoutExtension(igsWithNames[i].second));

					// Search for the IG name in the ZC list to associate.
					for(uint j = 0; j<ZCList.size(); ++j)
					{
						// Get the outpost ZC
						COutpost	*outpost = getOutpost (j);
						if (outpost)
						{
							// If name matching -> this zone should be a ZC.
							string outpostZone = strlwr(CFile::getFilenameWithoutExtension(ZCList[j].Name));
							if(igZone == outpostZone)
							{
								nlctassert(RZ_MAX_BUILDING_PER_OUTPOST==4);
								// Check there is all information in the IG for a ZC and initialize.
								bool zcOK[RZ_MAX_BUILDING_PER_OUTPOST+1] = {false, false, false, false, false};
								UInstanceGroup *ig = igsWithNames[i].first;
								uint k;
								for(k=0; k<ig->getNumInstance(); ++k)
								{
									if(ig->getInstanceName(k) == "flag_zc")
									{
										// todo hulud handle outpost flag
										zcOK[0] = true;
									}

									// For each building
									uint b;
									for (b=0; b<RZ_MAX_BUILDING_PER_OUTPOST; b++)
									{
										// Good one ?
										if (ig->getInstanceName(k) == ("bat_zc_0"+toString(b+1)))
										{
											// Set the position
											CVector pos = ig->getInstancePos(k);
											CQuat rot = ig->getInstanceRot(k);
											outpost->setBuildingPosition (b, rot, pos);
											zcOK[b+1] = true;
										}
									}
								}

								// There are not enough information in the IG for ZC.
								for (k=0; k<RZ_MAX_BUILDING_PER_OUTPOST+1; k++)
									if (!zcOK[k])
										break;

								// Failed ?
								if (k != RZ_MAX_BUILDING_PER_OUTPOST+1)
									nlwarning("CContinent::select : Zone '%s' should be a ZC but some doomy are missing.", igsWithNames[i].second.c_str());
								// Outpost found, and setuped. stop
								break;
							}
						}
					}
					// Add IGs with the Num ZC associated in the callback system.
					if (IGCallbacks) IGCallbacks->addIG(igsWithNames[i].first);
				}

				// Register outpost collisions and Display
				initOutpost();
			}
		}

		if (!ClientCfg.Light)
		{
			// Progress bar
			progress.popCropedValues ();
			progress.progress (1.f/3.f);
			progress.pushCropedValues (1.f/3.f, 2.f/3.f);

			{
				H_AUTO(InitRZWorldTileBank)
				// Select the tile bank
				if (Landscape)
				{
					if (!Indoor)
					{
						string farBankSeason = FarBank;
						SeasonPostfixTextureFilename (farBankSeason, Season);
						if (!SmallBank.empty() && !farBankSeason.empty())
						{
							std::string smallBankPath = CPath::lookup(SmallBank, false);
							std::string farBankSeasonPath = CPath::lookup(farBankSeason, false);
							if (!smallBankPath.empty() && !farBankSeasonPath.empty())
							{
								Landscape->loadBankFiles (smallBankPath, farBankSeasonPath);
							}
							// Postfix tiles and vegetset by the season string
							Landscape->postfixTileFilename (CSeasonFileExt::getExtension (Season));
							Landscape->postfixTileVegetableDesc (CSeasonFileExt::getExtension (Season));

							// Flush the tiles
							Landscape->flushTiles (progress);
						}
					}
				}
			}

			// Not an indoor ?
			if (!Indoor)
			{
				{
					H_AUTO(InitRZWorldSky)
					// load the sky if any
					initSky();
					// build the canopee
					try
					{
						nlassert(CurrSeason < EGSPD::CSeason::Invalid);
						if (!CanopyIGfileName[CurrSeason].empty() && CurrSeason != EGSPD::CSeason::Invalid)
						{
							BackgroundIG = UInstanceGroup::createInstanceGroup(CanopyIGfileName[CurrSeason]);
						}
						else
						{
							BackgroundIG = UInstanceGroup::createInstanceGroup(BackgroundIGName);
						}
					}
					catch (const Exception &e)
					{
						nlwarning (e.what());
					}
					if(BackgroundIG)
					{
						if (SceneRoot)
						{
							// Add the IG to the scene
							BackgroundIG->addToScene(*SceneRoot, Driver);

							// See for which objects distance should be overriden (object which use a .PLANT sheet)
							uint numInstances = BackgroundIG->getNumInstance();
							uint k;
							for(k = 0; k < numInstances; ++k)
							{
								// Select the texture function of the season
								UInstance instance = BackgroundIG->getInstance (k);
								if (!instance.empty())
								{
									instance.selectTextureSet (Season);
								}
							}
						}
					}
				}
			}
		}

		// Progress bar
		progress.popCropedValues ();
		progress.progress (2.f/3.f);
		progress.pushCropedValues (2.f/3.f, 1);
	}// complete

	if (complete || unhibernate)
	{
		H_AUTO(InitRZWorldLightCycleManager)
		// Touch light setup
		LightCycleManager.touch();
	}

	if (complete)
	{
		H_AUTO(InitRZWorldWaterMap)
		// init water map
		initWaterMap();
	}

	const R2::CScenarioEntryPoints::CCompleteIsland *completeIsland = NULL;
	{
		progress.pushCropedValues (0, 3.f/4.f);

		if (!ClientCfg.Light)
		{
			if (!Indoor)
			{
				if (Landscape)
				{
					H_AUTO(InitRZWorldLandscapeIGManager)
					// If any zone already loaded into the landscape, must load them to the IGManager.
					//	Yoyo: this is important for teleportation near a current position, since not so many landscape zones
					//	will be removed/added.
					//	Hence, a solution is to reload all igs.
					vector<string>		zonesLoaded;
					Landscape->getAllZoneLoaded(zonesLoaded);
					// and try to load the ones which fit the IGManager
					vector<NL3D::UInstanceGroup *> igAdded;
					LandscapeIGManager.loadArrayZoneIG(zonesLoaded, &igAdded);

					// Refresh zone to load/remove.
					vector<string>		zonesAdded;
					vector<string>		zonesRemoved;
					completeIsland = R2::CScenarioEntryPoints::getInstance().getCompleteIslandFromCoords(CVector2f((float) UserEntity->pos().x, (float) UserEntity->pos().y));
					Landscape->refreshAllZonesAround(pos, ClientCfg.Vision + ExtraZoneLoadingVision, zonesAdded, zonesRemoved, progress, completeIsland ? &(completeIsland->ZoneIDs) : NULL);
					LandscapeIGManager.unloadArrayZoneIG(zonesRemoved);
					LandscapeIGManager.loadArrayZoneIG(zonesAdded, &igAdded);
				}
			}
		}

		progress.popCropedValues ();
		progress.progress (3.f/4.f);
		progress.pushCropedValues (3.f/4.f, 1);

		// Refresh PACS
		if(GR)
		{
			H_AUTO(InitRZWorldRefreshPacs)
			GR->refreshLrAroundNow(pos, LRRefeshRadius);
		}

		// Refresh streamable object (villages ..)
		if (!ClientCfg.Light)
		{
			if (ClientCfg.VillagesEnabled)
			{
				H_AUTO(InitRZWorldForceUpdateStreamable)
				forceUpdateStreamable(pos, progress);
			}
		}

		progress.popCropedValues ();
	}

	// Execute only when you change the targeted continent is not the current one.
	if(complete || unhibernate)
	{
		// Continent changed -> update entities.
		EntitiesMngr.changeContinent();
		R2::getEditor().onContinentChanged();

		// Ask to reinit cloudscape
		InitCloudScape  = true;

		// Set the entity sun contribution parameters
		CollisionManager->setSunContributionPower (EntitySunContributionPower, EntitySunContributionMaxThreshold);
	}

	if(complete)
	{
		reloadFogMap();
		// temp for debug : create visualisation of current pacs primitives
		// createInstancesFromMoveContainer(Scene, IGCallbacks->getMoveContainer(), NULL);
		//
		// Progress bar
		progress.popCropedValues ();
	}

	if (complete || unhibernate)
	{
		if (!ClientCfg.Light && ClientCfg.LandscapeEnabled && ClientCfg.MicroLifeEnabled)
		{
			H_AUTO(InitRZWorldLoadMicroLife)
			loadMicroLife();
		}
	}
	// Register callback on streaming
	CAsyncFileManager::getInstance().registerTaskPriorityCallback(&ChangeTaskPriority);
}// select //

//=========================================================================
void CContinent::reloadFogMap()
{
	const R2::CScenarioEntryPoints::CCompleteIsland *completeIsland = R2::CScenarioEntryPoints::getInstance().getCompleteIslandFromCoords(CVector2f((float) UserEntity->pos().x, (float) UserEntity->pos().y));
	if (completeIsland)
	{
		std::string islandName = strlwr(completeIsland->Island);
		std::string::size_type lastPos = islandName.find_last_of("_");
		if (lastPos != std::string::npos)
		{
			islandName = islandName.substr(lastPos + 1);
		}
		// special fogmaps are use for islands
		CFogMapBuild fmb;
		enum TMapType { Day = 0, Night, Dusk, Distance, Depth, NoPrecipitation, NumMap };
		fmb.Map[CFogMapBuild::Day]      = "fog_" + islandName + "_day.tga";
		fmb.Map[CFogMapBuild::Night]    = "fog_" + islandName + "_night.tga";
		fmb.Map[CFogMapBuild::Dusk]     = "fog_" + islandName + "_day.tga";
		fmb.Map[CFogMapBuild::Distance] = "fog_" + islandName + "_dist.tga";
		fmb.Map[CFogMapBuild::Depth]    =  "fog_" + islandName + "_depth.tga";
		fmb.Map[CFogMapBuild::NoPrecipitation] = islandName + "_no_rain.tga";
		// there is an outtline of 50%, take it in account
		float deltaX = 0.25f * (completeIsland->XMax - completeIsland->XMin);
		float deltaY = 0.25f * (completeIsland->XMax - completeIsland->XMin);
		FogMap.init(fmb,
				    CVector((float) completeIsland->XMin + deltaX, (float) completeIsland->YMin + deltaY, 0.f),
					CVector((float) completeIsland->XMax - deltaX, (float) completeIsland->YMax - deltaY, 0.f)
				   );
		bool found = false;
		for(uint k = 0; k < CFogMapBuild::NumMap && !found; ++k)
		{
			if (FogMap.getMap((CFogMap::TMapType) k).getWidth() != 0) found = true;
		}
		// as a fallback, assume that new fog maps have not been installed, so fallback on the old ones ...
		if (!found)
		{
			FogMap.init(FogMapBuild);
		}
	}
	else
	{
		FogMap.init(FogMapBuild);
	}
	#ifdef NL_DEBUG
		nlinfo("fog map : (minX, minY) = (%.1f, %.1f), (maxX, maxY) = (%.1f, %.1f)", FogMap.getMinCoord().x, FogMap.getMinCoord().y, FogMap.getMaxCoord().x, FogMap.getMaxCoord().y);
	#endif
}

//=========================================================================
bool CContinent::getCorners(NLMISC::CVector2f &cornerMin, NLMISC::CVector2f &cornerMax) const
{
	if (!getPosFromZoneName(ZoneMin, cornerMin))
	{
		nlwarning("Can't retrieve continent min corner");
		return false;
	}
	if (!getPosFromZoneName(ZoneMax, cornerMax))
	{
		nlwarning("Can't retrieve continent max corner");
		return false;
	}
	if(cornerMin.x > cornerMax.x) std::swap(cornerMin.x, cornerMax.x);
	if(cornerMin.y > cornerMax.y) std::swap(cornerMin.y, cornerMax.y);
	cornerMax.x += 160.f;
	cornerMax.y += 160.f;
	return true;
}

//=========================================================================
void CContinent::initWaterMap()
{
	NLMISC::CVector2f cornerMin, cornerMax;
	if (!getCorners(cornerMin, cornerMax)) return;
	WaterMap.init(cornerMin, cornerMax);
}

//=========================================================================
void CContinent::loadMicroLife()
{
	CMicroLifeManager::getInstance().release();
	NLMISC::CVector2f cornerMin, cornerMax;
	if (!getCorners(cornerMin, cornerMax)) return;
	CMicroLifeManager::getInstance().init(cornerMin, cornerMax);
	// register all micro life primitives
	CMicroLifeManager::getInstance().build(MicroLifeZones);
}

//=========================================================================
bool CContinent::isLoadingforced(const NLMISC::CVector &pos) const
{
	return _Villages.needCompleteLoading(pos);
}


//=========================================================================
void CContinent::updateStreamable(const NLMISC::CVector &pos)
{
	_Villages.update(pos);
}

//=========================================================================
void CContinent::forceUpdateStreamable(const NLMISC::CVector &playerPos, NLMISC::IProgressCallback &progress)
{
	_Villages.forceUpdate(playerPos, progress);
}

//=========================================================================
void CContinent::removeVillages()
{
	_Villages.removeAll();
}

//=========================================================================
void CContinent::unselect()
{
	// remove envmap
	if (SceneRoot && !WaterEnvMapCanopyCam.empty())
	{
		SceneRoot->deleteCamera(WaterEnvMapCanopyCam);
	}
	WaterEnvMapCanopyCam.detach();
	if (CurrentSky.getScene() && !WaterEnvMapSkyCam.empty())
	{
		CurrentSky.getScene()->deleteCamera(WaterEnvMapSkyCam);
	}
	WaterEnvMapSkyCam.detach();
	//
	if (!Indoor)
	{
		releaseSky();

		// Setup the Root scene.
		if (BackgroundIG)
		{
			BackgroundIG->removeFromScene (*SceneRoot);
			SceneRoot->deleteInstanceGroup (BackgroundIG);
			BackgroundIG = NULL;
		}
	}

	// Remove outpost (collisions etc...)
	removeOutpost();

	// Unregister callback on streaming
	CAsyncFileManager::getInstance().registerTaskPriorityCallback(NULL);

	// Remove the primitive for all entitites (new PACS coming soon and need new primitives).
	EntitiesMngr.removeCollision();

	// release collision primitives
	if (IGCallbacks)
	{
		IGCallbacks->resetContainer();
	}

	// Unload villages
	_Villages.forceUnload();

	// Release PACS
	releasePACS ();

	// Initialize Landscape IG Callbacks.
	releaseLandscapeIGCallbacks();

	// Release the IG manager only if where are not in indoor
	if (!Indoor)
	{
		// Release the IG manager
		if (Landscape) LandscapeIGManager.reset ();
	}

	// release fog maps
	FogMap.release();

	// Remove weather as an observer
	removeObserver(&LightCycleManager);

	// Remove season observer
	removeObserver(&IGSeasonCallback);

	// Remove Camera collision observer
	removeObserver(&MeshCameraColManager);

	// Remove door management observers
	removeObserver(&IGDoorCallback);

	// Release water map
	WaterMap.release();

	// Release water envmap
	#ifdef USE_WATER_ENV_MAP
	Driver->deleteWaterEnvMap(WaterEnvMap);
	WaterEnvMap = NULL;
	#endif
}

//=========================================================================
/** Override current fog by the fog in the weather system. This is used when there is bad weather or precipitations
  * \param dest the fog state to modify
  * \param fogIndex the fog to modify (0 for main, 1 for canopy)
  */
static void overrideFog(CFogState &dest, TFogType fogType, float dayNight, float duskRatio, CLightCycleManager::TLightState lightState, bool /* overrideDist */, bool overideByWeatherFogDist = true)
{
	nlassert(fogType < NumFogType); // For now, 2 kinds of fogs : main & canopy
	if (!dest.FogEnabled) return;
	const CWeatherState &ws = WeatherManager.getCurrWeatherState();
	if (ws.FogRatio == 0.f) return; // no need to override fog
	// override the fog by the fog indicated in the WeatherManager
	NLMISC::CRGBA fogColor;
	// if there's a new style sky with a fog bitmap *(giving fog depending on hour & weather, use it)
	if (ContinentMngr.cur() && ContinentMngr.cur()->CurrentSky.getScene() != NULL && ContinentMngr.cur()->CurrentSky.hasFogColor())
	{
		fogColor = ContinentMngr.cur()->CurrentSky.computeFogColor(CClientDate(RT.getRyzomDay(), (float) DayNightCycleHour), WeatherManager.getWeatherValue());
	}
	else
	{
		switch(lightState)
		{
			case CLightCycleManager::DayToNight:
			{
				if (dayNight < duskRatio)
				{
					float blendFactor = duskRatio != 0 ? dayNight / duskRatio : 0.f;
					fogColor.blendFromui(ws.FogColorDay, ws.FogColorDusk, (uint) (blendFactor * 256.f));
				}
				else
				{
					float blendFactor = duskRatio != 1 ? (dayNight - duskRatio) / (1.f - duskRatio) : 0.f;
					fogColor.blendFromui(ws.FogColorDusk, ws.FogColorNight, (uint) (blendFactor * 256.f));
				}
			}
			break;
			default: // no dusk transition
				fogColor.blendFromui(ws.FogColorDay, ws.FogColorNight, (uint) (dayNight * 256.f));
			break;
		}
	}
	float finalFogAlpha = (float)dest.FogColor.A * 256.f / 255.f;
	dest.FogColor.blendFromui(dest.FogColor, fogColor, (uint) ((256.f-finalFogAlpha) * ws.FogRatio));
	if (overideByWeatherFogDist)
	{
		dest.FogEndDist = std::min (ws.FogFar[fogType], dest.FogEndDist);
		dest.FogStartDist = std::min (ws.FogNear[fogType], dest.FogStartDist);
	}
}

//=========================================================================
/** Adapt fog dist to the viewport
  */
static inline void makeRangedFog(CFogState &result, UScene &scene)
{
	if (result.FogEndDist != 0.f)
	{
			float farPlaneDist  = scene.getCam().getFrustum().Far;
			float newEndFogDist = std::min(result.FogEndDist, farPlaneDist);
			result.FogStartDist = result.FogStartDist * newEndFogDist / result.FogEndDist;
			result.FogEndDist   = newEndFogDist;
			result.FogEnabled   = true;
	}
	else
	{
		result.FogEnabled = false;
	}
}

//=========================================================================
void CContinent::getFogState(TFogType fogType, float dayNight, float duskRatio, CLightCycleManager::TLightState lightState, const NLMISC::CVectorD &pos, CFogState &result, bool overideByWeatherFogDist /*= true*/)
{
	if (FogStart == FogEnd)
	{
		result.FogEnabled = false;
		return;
	}
	switch(fogType)
	{
		case MainFog: FogMap.getFogParams(FogStart, FogEnd, (float) pos.x, (float) pos.y, dayNight, duskRatio, lightState, result); break;

		case CanopyFog: FogMap.getFogParams(RootFogStart, RootFogEnd, (float) pos.x, (float) pos.y, dayNight, duskRatio, lightState, result); break;
		default:
			nlstop;
		break;
	}
	// Override fog because of weather.
	overrideFog(result, fogType, dayNight, duskRatio, lightState, overideByWeatherFogDist);
	// normalize fog dist to view frustum
	/*
	switch(fogType)
	{
		case MainFog: if (Scene) makeRangedFog(result, *Scene); break;
		case CanopyFog: if (SceneRoot) makeRangedFog(result, *SceneRoot); break;
	}
	*/
	/*
	if (fogType == MainFog)
	{
		result.FogStartDist *= (ClientCfg.Vision / ClientCfg.Vision_max);
		result.FogEndDist *= (ClientCfg.Vision / ClientCfg.Vision_max);
	}
	*/
}


//=========================================================================
bool CContinent::enumIGs(IIGEnum *callback)
{
	uint numEntities = _Villages.getNumEntities();
	bool continueEnum = true;
	for(uint k = 0; k < numEntities && continueEnum; ++k)
	{
		CVillage *v = dynamic_cast<CVillage *>(_Villages.getEntity(k));
		if (v) // well this should always be the case, but it may change in the future
		{
			continueEnum = v->enumIGs(callback);
		}
	}
	return continueEnum;
}

//=========================================================================
void CContinent::registerObserver(IIGObserver *obs)
{
	uint numEntities = _Villages.getNumEntities();
	for(uint k = 0; k < numEntities; ++k)
	{
		CVillage *v = dynamic_cast<CVillage *>(_Villages.getEntity(k));
		if (v) // well this should always be the case, but it may change in the future
		{
			v->registerObserver(obs);
		}
	}
}


//=========================================================================
void CContinent::removeObserver(IIGObserver *obs)
{
	uint numEntities = _Villages.getNumEntities();
	for(uint k = 0; k < numEntities; ++k)
	{
		CVillage *v = dynamic_cast<CVillage *>(_Villages.getEntity(k));
		if (v) // well this should always be the case, but it may change in the future
		{
			v->removeObserver(obs);
		}
	}
}


//=========================================================================
bool CContinent::isObserver(IIGObserver *obs) const
{
	uint numEntities = _Villages.getNumEntities();
	for(uint k = 0; k < numEntities; ++k)
	{
		CVillage *v = dynamic_cast<CVillage *>(_Villages.getEntity(k));
		if (v) // well this should always be the case, but it may change in the future
		{
			if (v->isObserver(obs))
				return true;
		}
	}
	return false;
}


//=========================================================================
COutpost	*CContinent::getOutpost (uint outpostId)
{
	for (uint i=0; i<_Outposts.size(); i++)
	{
		// Outpost ?
		if(_Outposts[i].getOutpostId()==(sint)outpostId)
			return &_Outposts[i];
	}
	return NULL;
}

//=========================================================================
void CContinent::initOutpost()
{
	for (uint i=0; i<_Outposts.size(); i++)
	{
		_Outposts[i].initOutpost();
	}
}

//=========================================================================
void CContinent::removeOutpost()
{
	for (uint i=0; i<_Outposts.size(); i++)
	{
		_Outposts[i].removeOutpost();
	}
}


//=========================================================================
void CContinent::dumpVillagesLoadingZones(const std::string &filename)
{
	const uint NUM_METER_PER_PIX = 4;
	CBitmap outBitmap;
	CVector2f minPos, maxPos;
	getPosFromZoneName(ZoneMin, minPos);
	getPosFromZoneName(ZoneMax, maxPos);
	if (minPos.x > maxPos.x) std::swap(minPos.x, maxPos.x);
	if (minPos.y > maxPos.y) std::swap(minPos.y, maxPos.y);
	uint width = (uint) ((maxPos.x - minPos.x) / NUM_METER_PER_PIX);
	uint height = (uint) ((maxPos.y - minPos.y) / NUM_METER_PER_PIX);
	outBitmap.resize(width, height);
	for(uint k = 0; k < _Villages.getNumEntities(); ++k)
	{
		CVillage &v = *safe_cast<CVillage *>(_Villages.getEntity(k));
		if(!v.isOutpost())
		{
			const CStreamableIG &sig = v.getIG();
			drawDisc(outBitmap, (sig.getPos().x - minPos.x) / NUM_METER_PER_PIX, (sig.getPos().y - minPos.y) / NUM_METER_PER_PIX, sig.getUnloadRadius() / NUM_METER_PER_PIX, CRGBA(0, 0, 100), true);
			drawDisc(outBitmap, (sig.getPos().x - minPos.x) / NUM_METER_PER_PIX, (sig.getPos().y - minPos.y) / NUM_METER_PER_PIX, sig.getLoadRadius() / NUM_METER_PER_PIX, CRGBA(0, 100, 0), true);
			drawDisc(outBitmap, (sig.getPos().x - minPos.x) / NUM_METER_PER_PIX, (sig.getPos().y - minPos.y) / NUM_METER_PER_PIX, sig.getForceLoadRadius() / NUM_METER_PER_PIX, CRGBA(100, 0, 0), true);
		}
	}
	// load the map and do a lerp between the 2 bitmaps
	if (!WorldMap.empty())
	{
		std::string path = CPath::lookup(WorldMap, false);
		if (!path.empty())
		{
			CIFile stream;
			if (stream.open(path))
			{
				CBitmap worldMap;
				if (worldMap.load(stream))
				{
					worldMap.convertToType(CBitmap::RGBA);
					worldMap.flipV();
					worldMap.resample(outBitmap.getWidth(), outBitmap.getHeight());
					outBitmap.blend(outBitmap, worldMap, 127, true);
				}
			}
		}
	}
	drawDisc(outBitmap,  ((float) UserEntity->pos().x - minPos.x) / (float) NUM_METER_PER_PIX, ((float) UserEntity->pos().y - minPos.y) / (float) NUM_METER_PER_PIX, 2, CRGBA::Magenta);

	// draw grid
	const	uint	subdiv= 500;
	const	CRGBA	lineCol(CRGBA::Yellow);
	sint	minY= (sint)floor(minPos.y/subdiv);
	sint	maxY= (sint)ceil (maxPos.y/subdiv);
	sint	minX= (sint)floor(minPos.x/subdiv);
	sint	maxX= (sint)ceil (maxPos.x/subdiv);
	// draw HLines
	for(sint y= minY;y<maxY;y++)
	{
		sint	ypix= (sint)((y*subdiv-minPos.y)/NUM_METER_PER_PIX);
		if(ypix>=0 && ypix<(sint)height)
		{
			for(uint x=0;x<width;x++)
			{
				CRGBA	*pCol= (CRGBA*)(&outBitmap.getPixels()[0]);
				pCol[ypix*width+x]= lineCol;
			}
		}
	}
	// draw VLines
	for(sint x= minX;x<maxX;x++)
	{
		sint	xpix= (sint)((x*subdiv-minPos.x)/NUM_METER_PER_PIX);
		if(xpix>=0 && xpix<(sint)width)
		{
			for(uint y=0;y<height;y++)
			{
				CRGBA	*pCol= (CRGBA*)(&outBitmap.getPixels()[0]);
				pCol[y*width+xpix]= lineCol;
			}
		}
	}


	COFile outFile;
	if (outFile.open(filename))
	{
		try
		{
			outBitmap.writeTGA(outFile, 24, true);
		}
		catch(const EStream &)
		{
		}
	}
}

//=========================================================================
void CContinent::dumpFogMap(CFogMapBuild::TMapType mapType, const std::string &filename, TChannel channel /*= ChannelRGBA*/, const CRGBA channelLookup[256] /* = NULL */)
{
	CBitmap outBitmap = FogMap.getMap(mapType);
	outBitmap.convertToType(CBitmap::RGBA);
	if (channel != ChannelRGBA)
	{

		CRGBA *pix = (CRGBA *) &outBitmap.getPixels(0)[0];
		const CRGBA *endPix = pix + outBitmap.getWidth() * outBitmap.getHeight();
		while (pix != endPix)
		{
			uint8 intensity = ((uint8 *) pix)[channel];
			if (!channelLookup)
			{
				*pix = CRGBA(intensity, intensity, intensity, intensity);
			}
			else
			{
				*pix = channelLookup[intensity];
			}
			++pix;
		}
	}
	if (outBitmap.getWidth() == 0 || outBitmap.getHeight() == 0) return;
	CVector pos = MainCam.getMatrix().getPos();
	float mx, my;
	FogMap.worldPosToMapPos(pos.x, pos.y, mx, my);
	// load the map and do a lerp between the 2 bitmaps
	if (!WorldMap.empty())
	{
		std::string path = CPath::lookup(WorldMap, false);
		if (!path.empty())
		{
			CIFile stream;
			if (stream.open(path))
			{
				CBitmap worldMap;
				if (worldMap.load(stream))
				{
					worldMap.convertToType(CBitmap::RGBA);
					worldMap.resample(outBitmap.getWidth(), outBitmap.getHeight());
					outBitmap.blend(outBitmap, worldMap, 127, true);
				}
			}
		}
	}
	drawDisc(outBitmap, mx * outBitmap.getWidth(), my * outBitmap.getHeight(), 2.f, CRGBA::Magenta);
	COFile outFile;
	if (outFile.open(filename))
	{
		try
		{
			outBitmap.writeTGA(outFile, 24);
		}
		catch(const EStream &)
		{
		}
	}
}

//=========================================================================
void CContinent::initSky()
{
	if (!SkySheet[CurrSeason].empty())
	{
		// get the sky sheet
		CEntitySheet *skySheet = SheetMngr.get(NLMISC::CSheetId(SkySheet[CurrSeason]));
		if (skySheet && skySheet->Type == CEntitySheet::SKY)
		{
			// new-style sky
			CurrentSky.init(Driver, *static_cast<CSkySheet *>(skySheet), false, WorldLightCycle.NumHours);
		}
		#ifdef USE_WATER_ENV_MAP
			WaterEnvMapRdr.Sky = &CurrentSky;
		#endif
	}
	else
	{
		createSkyScene();
		// fallback to previous sky version
		Sky        = SkyScene->createInstance(SkyDay);
		Sky2ndPass = SkyScene->createInstance(SkyDay); // Sky shape used for second pass. We use it to keep pointers on textures
		SkyFogPart = SkyScene->createInstance(SkyFogPartName);
		// Setup the 2nd
		if (!Sky.empty())
		{
			DaySkySetup.buildFromInstance(Sky, 0); // day textures are at stage 0
			NightSkySetup.buildFromInstance(Sky, 1); // night textures are at stage 1
		}
	}
}

//=========================================================================
void CContinent::releaseSky()
{
	// Remove the (old-style) sky
	if (!Sky.empty())
	{
		SkyScene->deleteInstance (Sky);
		SkyScene->deleteInstance (Sky2ndPass);
		SkyScene->deleteInstance(SkyFogPart);
		Sky = NULL;
		Sky2ndPass = NULL;
		SkyFogPart = NULL;
	}
	deleteSkyScene();

	// Release the (new-style) sky
	CurrentSky.release();
}


//=========================================================================
/*static*/ uint CContinent::getMaxNbUserLandMarks()
{
	uint nbBonusLandmarks = (uint)IngameDbMngr.getProp( "INTERFACES:NB_BONUS_LANDMARKS" );
	return STANDARD_NUM_USER_LANDMARKS + nbBonusLandmarks;
}
