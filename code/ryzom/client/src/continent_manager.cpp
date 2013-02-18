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


/////////////
// INCLUDE //
/////////////
// misc
#include "nel/misc/path.h"
#include "nel/misc/vectord.h"
#include "nel/misc/i18n.h"
#include "nel/misc/progress_callback.h"
// 3D Interface.
#include "nel/3d/u_landscape.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

// Client
#include "continent_manager.h"
#include "client_cfg.h"
#include "sheet_manager.h"
#include "sound_manager.h"
#include "entities.h"	// \todo Hld : a enlever lorsque unselect aura son bool bien pris en compte
#include "init_main_loop.h"
#include "weather.h"
#include "weather_manager_client.h"
#include "interface_v3/interface_manager.h"
#include "interface_v3/group_map.h"
//
#include "input.h"

#include "continent_manager_build.h"
///////////
// USING //
///////////
using namespace NLPACS;
using namespace NLMISC;
using namespace NL3D;
using namespace std;
using namespace NLGEORGES;

////////////
// EXTERN //
////////////
extern ULandscape		*Landscape;
extern UMoveContainer	*PACS;
extern UGlobalRetriever	*GR;
extern URetrieverBank	*RB;
extern class CIGCallback		*IGCallbacks;
extern NLLIGO::CLigoConfig LigoConfig;

UMoveContainer			*PACSHibernated = NULL;
UGlobalRetriever		*GRHibernated = NULL;
URetrieverBank			*RBHibernated = NULL;
CIGCallback				*IGCallbacksHibernated = NULL;

////////////
// GLOBAL //
////////////

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Continent_Mngr_Update_Streamable )

/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CContinentManager :
// Constructor.
//-----------------------------------------------
CContinentManager::CContinentManager()
{
	_Current = 0;
	_Hibernated = NULL;
}// CContinentManager //



void CContinentManager::reset()
{
	// stop the background sound
	if (SoundMngr)
		SoundMngr->stopBackgroundSound();

	// Unselect continent
	if (_Current)
		_Current->unselect();

	// Shared data must be NULL now
	_Current = NULL;
	nlassert (GR == NULL);
	nlassert (RB == NULL);
	nlassert (PACS == NULL);
	nlassert (IGCallbacks == NULL);

	// Swap the hibernated data
	std::swap(GR, GRHibernated);
	std::swap(RB, RBHibernated);
	std::swap(PACS, PACSHibernated);
	std::swap(_Current, _Hibernated);
	std::swap(IGCallbacks, IGCallbacksHibernated);

	// Unselect continent
	if (_Current)
		_Current->unselect();

	// remove villages
	removeVillages();

	// NB: landscape zones are deleted in USCene::deleteLandscape()

	// clear continents DB
	for(TContinents::iterator it = _Continents.begin(); it != _Continents.end(); ++it)
	{
		delete it->second;
	}
	_Continents.clear();
	_Current = NULL;
	_Hibernated = NULL;
}


//-----------------------------------------------
// load :
// Load all continent.
//-----------------------------------------------


/*
// Oldies now all these data are stored in the ryzom.world
const uint32 NBCONTINENT = 8;
SContInit vAllContinents[NBCONTINENT] = {
	{ "fyros",		"fyros",				15767,	20385,	-27098,	-23769	},
	{ "tryker",		"tryker",				15285,	18638,	-34485,	-30641	},
	{ "matis",		"lesfalaises",			235,	6316,	-7920,	-256	},
	{ "zorai",		"lepaysmalade",			6805,	12225,	-5680,	-1235	},
	{ "bagne",		"lebagne",				434,	1632,	-11230,	-9715	},
	{ "route",		"laroutedesombres",		5415,	7400,	-17000, -9575	},
	{ "sources",	"sources",				2520,	3875,	-11400, -9720	},
	{ "terres",		"lesterres",			100,	3075,	-15900, -13000	}
};
*/

// Read the ryzom.world which give the names and bboxes


//-----------------------------------------------
void CContinentManager::preloadSheets()
{
	reset();

	CEntitySheet *sheet = SheetMngr.get(CSheetId("ryzom.world"));

	if (!sheet || sheet->type() != CEntitySheet::WORLD)
	{
		nlerror("World sheet not found or bad type");
	}
	uint32 i;

	CWorldSheet *ws = (CWorldSheet *) sheet;

	// Copy datas from the sheet

	for (i = 0; i < ws->ContLocs.size(); ++i)
	{
		const SContLoc &clTmp = ws->ContLocs[i];
		std::string continentSheetName = NLMISC::strlwr(clTmp.ContinentName);
		if (continentSheetName.find(".continent") == std::string::npos)
		{
			continentSheetName += ".continent";
		}
		// Get the continent form
		CSheetId continentId(continentSheetName);
		sheet = SheetMngr.get(continentId);
		if (sheet)
		{
			if (sheet->type() == CEntitySheet::CONTINENT)
			{
				CContinent *pCont = new CContinent;
				pCont->SheetName = continentSheetName;
				_Continents.insert(make_pair(clTmp.SelectionName, pCont));
			}
			else
			{
				nlwarning("Bad type for continent form %s.", continentSheetName.c_str());
			}
		}
		else
		{
			nlwarning("cant find continent sheet : %s.", continentSheetName.c_str());
		}
	}
}

//-----------------------------------------------
void CContinentManager::load ()
{
	// Continents are preloaded so setup them
	TContinents::iterator it = _Continents.begin();
	while (it != _Continents.end())
	{
		it->second->setup();
		it++;
	}

	loadContinentLandMarks();

	// \todo GUIGUI : Do it better when there will be "ecosystem"/Wind/etc.
	// Initialize the Landscape Vegetable.
	if(ClientCfg.MicroVeget)
	{
		if (Landscape)
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
	}
}// load //

//-----------------------------------------------
// select :
// Select continent from a name.
// \param const string &name : name of the continent to select.
//-----------------------------------------------
void CContinentManager::select(const string &name, const CVectorD &pos, NLMISC::IProgressCallback &progress)
{
	CNiceInputAuto niceInputs;
	// Find the continent.
	TContinents::iterator itCont = _Continents.find(name);
	if(itCont == _Continents.end())
	{
		nlwarning("CContinentManager::select: Continent '%s' is Unknown. Cannot Select it.", name.c_str());
		return;
	}

	// Dirt weather

	{
		H_AUTO(InitRZWorldSetLoadingContinent)
		// Set the loading continent
		setLoadingContinent (itCont->second);
	}

	// ** Update the weather manager for loading information

	// Update the weather manager
	{
		H_AUTO(InitRZWorldUpdateWeatherManager )
		updateWeatherManager (itCont->second);
	}


	// startup season can be changed now the player is safe
	StartupSeason = RT.getRyzomSeason();
	// Modify this season according to the continent reached. eg: newbieland continent force the winter to be autumn
	if(StartupSeason<EGSPD::CSeason::Invalid)
		StartupSeason= (*itCont).second->ForceDisplayedSeason[StartupSeason];

	// Compute the current season according to StartupSeason, Server driver season, R2 Editor season or manual debug season
	CurrSeason = computeCurrSeason();


	// Is it the same continent than the old one.
	{
		H_AUTO(InitRZWorldSelectCont)
		if(((*itCont).second != _Current) || ((*itCont).second->Season != CurrSeason))
		{
			// New continent is not an indoor ?
			if (!(*itCont).second->Indoor && _Current)
			{
				// Unselect the current continent
				_Current->unselect();

				// Shared data must be NULL now
				_Current = NULL;
				nlassert (GR == NULL);
				nlassert (RB == NULL);
				nlassert (PACS == NULL);
				nlassert (IGCallbacks == NULL);
			}
			else
			{
				// Remove the primitive for all entitites (new PACS coming soon and need new primitives).
				EntitiesMngr.removeCollision();
			}

			// Swap the hibernated data
			std::swap(GR, GRHibernated);
			std::swap(RB, RBHibernated);
			std::swap(PACS, PACSHibernated);
			std::swap(_Current, _Hibernated);
			std::swap(IGCallbacks, IGCallbacksHibernated);

			// Is it the same continent than the old one.
			if(((*itCont).second != _Current) || ((*itCont).second->Season != CurrSeason))
			{
				// Unselect the old continent.
				if(_Current)
					_Current->unselect();
				_Current = (*itCont).second;

				// Teleport in a new continent, complete load
				_Current->select(pos, progress, true, false, CurrSeason);

				// New continent is not an indoor ?
				if (!_Current->Indoor)
				{
					// Stop the background sound
					if (SoundMngr)
						SoundMngr->stopBackgroundSound();
				}
			}
			else
			{
				// Teleport in the hibernated continent
				_Current->select(pos, progress, false, true, CurrSeason);
			}
		}
		else
		{
			// Teleport in the same continent
			_Current->select(pos, progress, false, false, CurrSeason);
		}
	}

	{
		H_AUTO(InitRZWorldSound)

		// New continent is not an indoor ?
		if (!_Current->Indoor)
		{
			if(SoundMngr)
				SoundMngr->loadContinent(name, pos);
		}
	}

	// Map handling
	{
		H_AUTO(InitRZWorldMapHandling)
		CWorldSheet *pWS = dynamic_cast<CWorldSheet*>(SheetMngr.get(CSheetId("ryzom.world")));
		for (uint32 i = 0; i < pWS->Maps.size(); ++i)
		if (pWS->Maps[i].ContinentName == name)
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			CGroupMap *pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:actual_map"));
			if (pMap != NULL)
				pMap->setMap(pWS->Maps[i].Name);
			pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:respawn_map:content:map_content:actual_map"));
			if (pMap != NULL)
				pMap->setMap(pWS->Maps[i].Name);
			break;
		}

	}

}// select //

//-----------------------------------------------
// select :
// Select closest continent from a vector.
//-----------------------------------------------
void CContinentManager::select(const CVectorD &pos, NLMISC::IProgressCallback &progress)
{

	CVector2f fPos;
	fPos.x = (float)pos.x;
	fPos.y = (float)pos.y;
	TContinents::iterator it = _Continents.begin();
	while (it != _Continents.end())
	{
		CContinent *pCont = it->second;
		nlinfo("Looking into %s", pCont->SheetName.c_str());
		if (pCont->Zone.VPoints.size() > 0) // Patch because some continent have not been done yet
		{
			if (pCont->Zone.contains(fPos))
			{
				// load the continent selected.
				select (it->first, pos, progress);
				return;
			}
			else
			{
				/*
				nlwarning("**********************************************");
				nlwarning("Start position (%s) not found in continent %s", NLMISC::toString(pos.asVector()).c_str(), it->first.c_str());
				for(uint k = 0; k < pCont->Zone.VPoints.size(); ++k)
				{
					nlwarning("zone point %d = %s", (int)k, NLMISC::toString(pCont->Zone.VPoints[k]).c_str());
				}
				*/
			}
		}
		it++;
	}

	nlwarning("cannot select any continent at pos (%f, %f)", fPos.x, fPos.y);

	/* *****************
		PLEASE DO *****NOT***** PUT AN ASSERT HERE
		While this is a bug, it is a data bug. Crashing is not a good solution in this case.
		If you put an assert, it can happens this scenario for example:
		- A levelDesigner put a bad Teleporter which teleport to an invalid position
		- The player teleport, but its position is invalid => crash
		- the next time he logs, it crashes at start, AND HENCE CANNOT ASK A GM TO TELEPORT HIM AWAY

		Other scenarios can happens like Data change, Continent change => Player no more at valid position etc...
		HENCE MUST NOT CRASH, but display a "Lost In Space screen" leaving the player the possibility to ask HELP.
	   *****************
	*/
	//nlassertex(0, ("can't select any continent"));

}// select //


bool CContinentManager::isLoadingforced(const NLMISC::CVector &playerPos) const
{
	if(_Current == 0)
		return false;

	return _Current->isLoadingforced(playerPos);
}

void CContinentManager::updateStreamable(const NLMISC::CVector &playerPos)
{
	H_AUTO_USE ( RZ_Client_Continent_Mngr_Update_Streamable )
	if(_Current)
		_Current->updateStreamable(playerPos);
}

void CContinentManager::forceUpdateStreamable(const NLMISC::CVector &playerPos, NLMISC::IProgressCallback &progress)
{
	H_AUTO_USE ( RZ_Client_Continent_Mngr_Update_Streamable )
	if (ClientCfg.VillagesEnabled)
	{
		if(_Current)
			_Current->forceUpdateStreamable(playerPos, progress);
	}
}



void CContinentManager::removeVillages()
{
	for(TContinents::iterator it = _Continents.begin(); it != _Continents.end(); ++it)
	{
		if (it->second)
			it->second->removeVillages();
	}
}

void CContinentManager::getFogState(TFogType fogType, float dayNight, float duskRatio, CLightCycleManager::TLightState lightState, const NLMISC::CVectorD &pos, CFogState &result)
{
	if(_Current)
		_Current->getFogState(fogType, dayNight, duskRatio, lightState, pos, result);
}

CContinent *CContinentManager::get(const std::string &contName)
{
	TContinents::iterator it = _Continents.find(contName);
	if (it != _Continents.end())
		return it->second;
	return NULL;
}

void CContinentManager::serialUserLandMarks(NLMISC::IStream &f)
{
	f.serialVersion(1);
	if (!f.isReading())
	{
		uint32 numCont = (uint32)_Continents.size();
		f.serial(numCont);
		for(TContinents::iterator it = _Continents.begin(); it != _Continents.end(); ++it)
		{
			std::string name = it->first;
			f.serial(name);
			if (it->second)
			{
				f.serialCont(it->second->UserLandMarks);
			}
			else
			{
				std::vector<CUserLandMark> dummy;
				f.serialCont(dummy);
			}
		}
	}
	else
	{
		uint32 numCont;
		f.serial(numCont);
		for(uint k = 0; k < numCont; ++k)
		{
			std::string contName;
			f.serial(contName);
			TContinents::iterator it = _Continents.find(contName);
			if (it != _Continents.end() && it->second)
			{
				f.serialCont(it->second->UserLandMarks);
			}
			else
			{
				std::vector<CUserLandMark> dummy;
				f.serialCont(dummy);
			}
		}

		// The number of stored landmarks is not checked at this time, but if we receive a
		// lower value in the server database, we will cut down using checkNumberOfUserLandmarks()
	}
}


//-----------------------------------------------
// checkNumberOfLandmarks
//-----------------------------------------------
void CContinentManager::checkNumberOfUserLandmarks( uint maxNumber )
{
	for ( TContinents::iterator it=_Continents.begin(); it!=_Continents.end(); ++it )
	{
		CContinent *cont = (*it).second;
		if ( cont->UserLandMarks.size() > maxNumber )
		{
			// Just cut down the last landmarks (in case of hacked file)
			if ( cont == _Current )
			{
				CGroupMap *pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:actual_map"));
				if ( pMap )
					pMap->removeExceedingUserLandMarks( maxNumber );
			}
			else
			{
				cont->UserLandMarks.resize( maxNumber );
			}
		}
	}
}


//-----------------------------------------------
// serialFOWMaps
//-----------------------------------------------
void CContinentManager::serialFOWMaps()
{
	for(TContinents::iterator it = _Continents.begin(); it != _Continents.end(); ++it)
	{
		CContinent *pCont = it->second;
		//nlinfo("Saving fow continent %s of name %s", it->first.c_str(), pCont->Name.c_str());
		it->second->FoW.save(pCont->Name);
	}
}

const std::string &CContinentManager::getCurrentContinentSelectName()
{
	TContinents::iterator it;

	for (it = _Continents.begin(); it != _Continents.end(); ++it)
	{
		if (it->second == _Current)
			return it->first;
	}

	static const string emptyString;
	return emptyString;
}


void CContinentManager::reloadWeather()
{
	WeatherManager.release();
	// reload the sheet
	std::vector<std::string> extensions;
	extensions.push_back("weather_setup");
	extensions.push_back("weather_function_params");
	extensions.push_back("continent");
	extensions.push_back("light_cycle");

	NLMISC::IProgressCallback pc;
	SheetMngr.loadAllSheet(pc, true, false, true, true, &extensions);

	WeatherManager.init();
	// Load description of light cycles for each season.
	loadWorldLightCycle();
	// Load global weather function parameters
	loadWeatherFunctionParams();




	for(TContinents::iterator it = _Continents.begin(); it != _Continents.end(); ++it)
	{
		NLMISC::CSheetId contSI(it->second->SheetName);
		CContinentSheet *cs = dynamic_cast<CContinentSheet *>(SheetMngr.get(contSI));
		if (cs)
		{
			// update continent weather part
			for(uint l = 0; l < EGSPD::CSeason::Invalid; ++l)
			{
				it->second->WeatherFunction[l].buildFromSheet(cs->WeatherFunction[l], WeatherManager);
			}
			// update misc params
			it->second->FogStart = cs->Continent.FogStart;
			it->second->FogEnd = cs->Continent.FogEnd;
			it->second->RootFogStart = cs->Continent.RootFogStart;
			it->second->RootFogEnd = cs->Continent.RootFogEnd;
			it->second->LandscapeLightDay = cs->Continent.LandscapeLightDay;
			it->second->LandscapeLightDusk = cs->Continent.LandscapeLightDusk;
			it->second->LandscapeLightNight = cs->Continent.LandscapeLightNight;
			it->second->EntityLightDay = cs->Continent.EntityLightDay;
			it->second->EntityLightDusk = cs->Continent.EntityLightDusk;
			it->second->EntityLightNight = cs->Continent.EntityLightNight;
			it->second->RootLightDay = cs->Continent.RootLightDay;
			it->second->RootLightDusk = cs->Continent.RootLightDusk;
			it->second->RootLightNight = cs->Continent.RootLightNight;
		}
	}
}

void CContinentManager::reloadSky()
{
	// reload new style sky
	std::vector<std::string> exts;
	CSheetManager sheetManager;
	exts.push_back("sky");
	exts.push_back("continent");
	NLMISC::IProgressCallback progress;
	sheetManager.loadAllSheet(progress, true, false, false, true, &exts);
	//
	const CSheetManager::TEntitySheetMap &sm = SheetMngr.getSheets();
	for(CSheetManager::TEntitySheetMap::const_iterator it = sm.begin(); it != sm.end(); ++it)
	{
		if (it->second.EntitySheet)
		{
			CEntitySheet::TType type = it->second.EntitySheet->Type;
			if (type == CEntitySheet::CONTINENT)
			{
				// find matching sheet in new sheetManager
				const CEntitySheet *other = sheetManager.get(it->first);
				if (other)
				{
					const CContinentParameters &cp = static_cast<const CContinentSheet *>(other)->Continent;
					// find matching continent in manager
					for(TContinents::iterator it = _Continents.begin(); it != _Continents.end(); ++it)
					{
						if (it->second && nlstricmp(it->second->Name, cp.Name) == 0)
						{
							std::copy(cp.SkySheet, cp.SkySheet + EGSPD::CSeason::Invalid, it->second->SkySheet);
							break;
						}
					}
				}
			}
			else if(type == CEntitySheet::SKY)
			{
				// find matching sheet in new sheetManager
				const CEntitySheet *other = sheetManager.get(it->first);
				if (other)
				{
					// replace data in place
					((CSkySheet &) *it->second.EntitySheet) = ((const CSkySheet &) *other);
				}
			}
		}
	}
	if (_Current)
	{
		_Current->releaseSky();
		_Current->initSky();
	}
}

// ***************************************************************************
void CContinentManager::loadContinentLandMarks()
{
	std::string dataPath = "../../client/data";

	if (ClientCfg.UpdatePackedSheet == false)
	{
		readLMConts(dataPath);
	}
	else
	{
		buildLMConts("ryzom.world", "../../common/data_leveldesign/primitives", dataPath);
		readLMConts(dataPath);
	}
}

// ***************************************************************************

void CContinentManager::readLMConts(const std::string &dataPath)
{
	CIFile f;

	string sPackedFileName = CPath::lookup(LM_PACKED_FILE, false);
	if (sPackedFileName.empty())
		sPackedFileName = CPath::standardizePath(dataPath) + LM_PACKED_FILE;

	if (f.open(sPackedFileName))
	{
		uint32 nNbCont = 0;
		sint ver= f.serialVersion(1);
		f.serial(nNbCont);
		for (uint32 i = 0; i < nNbCont; ++i)
		{
			string sContName;
			f.serial(sContName);
			TContinents::iterator itCont = _Continents.find(sContName);
			if(itCont != _Continents.end())
			{
				CContinent *pCont = itCont->second;
				f.serial(pCont->Zone);
				f.serial(pCont->ZoneCenter);
				f.serialCont(pCont->ContLandMarks);
			}
			else
			{
				CContinent dummy;
				f.serial(dummy.Zone);
				f.serial(dummy.ZoneCenter);
				f.serialCont(dummy.ContLandMarks);
				nlwarning("continent not found : %s", sContName.c_str());
			}
		}
		f.serialCont(WorldMap);

		if (ver >= 1)
			f.serialCont(aliasToRegionMap);
	}
	else
	{
		nlwarning("cannot load " LM_PACKED_FILE);
	}
}

// ***************************************************************************

string CContinentManager::getRegionNameByAlias(uint32 i)
{
	return aliasToRegionMap[i];
}
