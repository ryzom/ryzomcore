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
#include "continent.h"
#include "continent_inline.h"
#include "ai_mgr.h"
#include "ai_grp.h"
#include "ai_bot.h"
#include "ai_entity_matrix.h"
#include "ai_player.h"
#include "ai_mgr_fauna.h"
#include "ai_grp_fauna.h"
#include "ai_grp_npc.h"
#include "ai_grp_pet.h"
#include "ai_bot_npc.h"
#include "mirrors.h"
#include "server_share/mission_messages.h"
#include "game_share/fame.h"
#include "server_share/used_continent.h"
#include "ai_outpost.h"
#include "script_compiler.h"
#include "ais_actions.h"
#include "fx_entity_manager.h"
#include "ai_script_data_manager.h"
#include "commands.h"

#include "ais_user_models.h"

extern bool GrpHistoryRecordLog;
extern NLLIGO::CLigoConfig LigoConfig;

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace	AITYPES;
using namespace	AICOMP;
using namespace AIVM;

//////////////////////////////////////////////////////////////////////////////
// New set of commands                                                      //
//////////////////////////////////////////////////////////////////////////////

//- Generic commands ---------------------------------------------------------

RYAI_TEMPLATED_COMMAND(listInstances,	"display the list of ai instances",	"[<filter>]", CAIInstance,	buildInstanceList,	displayList)
RYAI_TEMPLATED_COMMAND(listContinents,	"display the list of continents",	"[<filter>]", CContinent,	buildContinentList,	displayList)
RYAI_TEMPLATED_COMMAND(listRegions,		"display the list of regions",		"[<filter>]", CRegion,		buildRegionList,	displayList)
RYAI_TEMPLATED_COMMAND(listCellZones,	"display the list of cell zones",	"[<filter>]", CCellZone,	buildCellZoneList,	displayList)
RYAI_TEMPLATED_COMMAND(listFamilyBehaviours,	"display the list of family behaviours",	"[<filter>]",	CFamilyBehavior,	buildFamilyBehaviorList,	displayList)
RYAI_TEMPLATED_COMMAND(listOutposts,	"display the list of outposts",		"[<filter>]", COutpost,		buildOutpostList,	displayList)
RYAI_TEMPLATED_COMMAND(listManagers,	"display the list of managers",		"[<filter>]", CManager,		buildManagerList,	displayList)
RYAI_TEMPLATED_COMMAND(listGroups,		"display the list of groups",		"[<filter>]", CGroup,		buildGroupList,		displayList)
RYAI_TEMPLATED_COMMAND(listBots,		"display the list of bots",			"[<filter>]", CBot,			buildBotList,		displayList)
RYAI_TEMPLATED_COMMAND(listPlayers,		"display the list of players",		"[<filter>]", CBotPlayer,	buildPlayerList,	displayList)
RYAI_TEMPLATED_COMMAND(listFaunaPlaces,	"display the list of fauna places",	"[<filter>]", CAIPlace,	buildFaunaPlaceList,	displayList)

RYAI_TEMPLATED_COMMAND(enableFaunaPlace, "enable a fauna place",	"[<filter>]", CAIPlace,	buildFaunaPlaceList, spawnList)
RYAI_TEMPLATED_COMMAND(disableFaunaPlace, "disable a fauna place",	"[<filter>]", CAIPlace,	buildFaunaPlaceList, despawnList)



RYAI_TEMPLATED_COMMAND(displayInstances,	"display extensively the list of ai instances",	"[<filter>]", CAIInstance,	buildInstanceList,	displayListEx)
RYAI_TEMPLATED_COMMAND(displayContinents,	"display extensively the list of continents",	"[<filter>]", CContinent,	buildContinentList,	displayListEx)
RYAI_TEMPLATED_COMMAND(displayRegions,		"display extensively the list of regions",		"[<filter>]", CRegion,		buildRegionList,	displayListEx)
RYAI_TEMPLATED_COMMAND(displayCellZones,	"display extensively the list of cell zones",	"[<filter>]", CCellZone,	buildCellZoneList,	displayListEx)
RYAI_TEMPLATED_COMMAND(displayFamilyBehaviours,	"display extensively the list of family behaviours",	"[<filter>]",	CFamilyBehavior,	buildFamilyBehaviorList,	displayListEx)
//RYAI_TEMPLATED_COMMAND(displayOutposts,		"display extensively the list of outposts",		"[<filter>]", COutpost,		buildOutpostList,	displayListEx)
RYAI_TEMPLATED_COMMAND(displayManagers,		"display extensively the list of managers",		"[<filter>]", CManager,		buildManagerList,	displayListEx)
RYAI_TEMPLATED_COMMAND(displayGroups,		"display extensively the list of groups",		"[<filter>]", CGroup,		buildGroupList,		displayListEx)
RYAI_TEMPLATED_COMMAND(displayBots,			"display extensively the list of bots",			"[<filter>]", CBot,			buildBotList,		displayListEx)
RYAI_TEMPLATED_COMMAND(displayPlayers,		"display extensively the list of players",		"[<filter>]", CBotPlayer,	buildPlayerList,	displayListEx)

RYAI_TEMPLATED_COMMAND(spawnInstances,	"spawns the ai instances",	"[<filter>]", CAIInstance,	buildInstanceList,	spawnList)
//RYAI_TEMPLATED_COMMAND(spawnContinents,	"spawns the continents",	"[<filter>]", CContinent,	buildContinentList,	spawnList)
//RYAI_TEMPLATED_COMMAND(spawnRegions,	"spawns the regions",		"[<filter>]", CRegion,		buildRegionList,	spawnList)
//RYAI_TEMPLATED_COMMAND(spawnCellZones,	"spawns the cell zones",	"[<filter>]", CCellZone,	buildCellZoneList,	spawnList)
//RYAI_TEMPLATED_COMMAND(spawnFamilyBehaviours,	"spawns the family behaviours",	"[<filter>]",	CFamilyBehavior,	buildFamilyBehaviorList,	spawnList)
//RYAI_TEMPLATED_COMMAND(spawnOutposts,	"spawns the outposts",		"[<filter>]", COutpost,		buildOutpostList,	spawnList)
RYAI_TEMPLATED_COMMAND(spawnManagers,	"spawns the managers",		"[<filter>]", CManager,		buildManagerList,	spawnList)
RYAI_TEMPLATED_COMMAND(spawnGroups,		"spawns the groups",		"[<filter>]", CGroup,		buildGroupList,		spawnList)
//RYAI_TEMPLATED_COMMAND(spawnBots,		"spawns the bots",			"[<filter>]", CBot,			buildBotList,		spawnList)
//RYAI_TEMPLATED_COMMAND(spawnPlayers,	"spawns the players",		"[<filter>]", CBotPlayer,	buildPlayerList,	spawnList)

RYAI_TEMPLATED_COMMAND(despawnInstances,	"despawns the ai instances",	"[<filter>]", CAIInstance,	buildInstanceList,	despawnList)
//RYAI_TEMPLATED_COMMAND(despawnContinents,	"despawns the continents",		"[<filter>]", CContinent,	buildContinentList,	despawnList)
//RYAI_TEMPLATED_COMMAND(despawnRegions,	"despawns the regions",			"[<filter>]", CRegion,		buildRegionList,	despawnList)
//RYAI_TEMPLATED_COMMAND(despawnCellZones,	"despawns the cell zones",		"[<filter>]", CCellZone,	buildCellZoneList,	despawnList)
//RYAI_TEMPLATED_COMMAND(despawnFamilyBehaviours,	"despawns the family behaviours",	"[<filter>]",	CFamilyBehavior,	buildFamilyBehaviorList,	despawnList)
//RYAI_TEMPLATED_COMMAND(despawnOutposts,	"despawns the outposts",		"[<filter>]", COutpost,		buildOutpostList,	despawnList)
RYAI_TEMPLATED_COMMAND(despawnManagers,		"despawns the managers",		"[<filter>]", CManager,		buildManagerList,	despawnList)
RYAI_TEMPLATED_COMMAND(despawnGroups,		"despawns the groups",			"[<filter>]", CGroup,		buildGroupList,		despawnList)
//RYAI_TEMPLATED_COMMAND(despawnBots,		"despawns the bots",			"[<filter>]", CBot,			buildBotList,		despawnList)
//RYAI_TEMPLATED_COMMAND(despawnPlayers,	"despawns the players",			"[<filter>]", CBotPlayer,	buildPlayerList,	despawnList)

//- Special commands ---------------------------------------------------------

NLMISC_COMMAND(search, "search all the data tree for a name part","<name_part>")
{
	if (args.size() != 1)
		return false;
	
	{
	log.displayNL("- Search results in instances ------------------------------------------------");
	std::deque<CAIInstance*> container;
	buildInstanceList(container);
	displayList(log, container, args[0]);
	}
	
	{
	log.displayNL("- Search results in continents -----------------------------------------------");
	std::deque<CContinent*> container;
	buildContinentList(container);
	displayList(log, container, args[0]);
	}
	
	{
	log.displayNL("- Search results in regions --------------------------------------------------");
	std::deque<CRegion*> container;
	buildRegionList(container);
	displayList(log, container, args[0]);
	}
	
	{
	log.displayNL("- Search results in cell zones -----------------------------------------------");
	std::deque<CCellZone*> container;
	buildCellZoneList(container);
	displayList(log, container, args[0]);
	}
	
	{
	log.displayNL("- Search results in family behaviors -----------------------------------------");
	std::deque<CFamilyBehavior*> container;
	buildFamilyBehaviorList(container);
	displayList(log, container, args[0]);
	}
	
	{
	log.displayNL("- Search results in outposts -------------------------------------------------");
	std::deque<COutpost*> container;
	buildOutpostList(container);
	displayList(log, container, args[0]);
	}
	
	{
	log.displayNL("- Search results in managers -------------------------------------------------");
	std::deque<CManager*> container;
	buildManagerList(container);
	displayList(log, container, args[0]);
	}
	
	{
	log.displayNL("- Search results in groups ---------------------------------------------------");
	std::deque<CGroup*> container;
	buildGroupList(container);
	displayList(log, container, args[0]);
	}
	
	{
	log.displayNL("- Search results in bots -----------------------------------------------------");
	std::deque<CBot*> container;
	buildBotList(container);
	displayList(log, container, args[0]);
	}
	
	{
	log.displayNL("- Search results in players --------------------------------------------------");
	std::deque<CBotPlayer*> container;
	buildPlayerList(container);
	displayList(log, container, args[0]);
	}
	
	return true;
}

//----------------------------------------------------------------------------

NLMISC_COMMAND(eventCreateNpcGroup, "create an event npc group", "<aiInstanceId> <nbBots> <sheet> <x> <y> [<dispersionRadius=10m> [<spawnBots=true> [<orientation=random|-360..360> [<name>]]]]")
{
	if (args.size()<5)
		return false;
	
	CLogStringWriter stringWriter(&log);
	
	CAIInstance* aiInstance = NULL;
	{
		uint32 aiInstanceId = NLMISC::atoui(args[0].c_str());
		if (aiInstanceId>=CAIS::instance().aiinstances().size())
		{
			stringWriter.append("Invalid AI Instance Number");
			return true;
		}
		aiInstance = CAIS::instance().aiinstances()[aiInstanceId];
		if (!aiInstance)
		{
			stringWriter.append("AI Instance do not exists");
			return true;
		}
	}
	
	uint32 nbBots = NLMISC::atoui(args[1].c_str());
	if (nbBots<=0)
	{
		stringWriter.append("invalid bot count");
		return true;
	}
	
	NLMISC::CSheetId sheetId(args[2]);
	if (sheetId==NLMISC::CSheetId::Unknown)
		sheetId = args[2] + ".creature";
	if (sheetId==NLMISC::CSheetId::Unknown)
	{
		stringWriter.append("invalid sheet id");
		return true;
	}
	
	double x = atof(args[3].c_str());
	double y = atof(args[4].c_str());

	double dispersionRadius = 10.;
	if (args.size()>5)
	{
		dispersionRadius = atof(args[5].c_str());
		if (dispersionRadius<0.)
		{
			stringWriter.append("invalid dispersion radius");
			return true;
		}
	}
	
	bool spawnBots = true;
	if (args.size()>6)
	{
		NLMISC::fromString(args[6], spawnBots);
	}

	double orientation = 6.666;
	if (args.size()>7 && args[7] != "random")
	{
		NLMISC::fromString(args[7], orientation);
		orientation = orientation / 360.0 * (NLMISC::Pi * 2.0);
	}

	std::string botsName;
	if (args.size()>8) botsName = args[8];

	aiInstance->eventCreateNpcGroup(nbBots, sheetId, CAIVector(x, y), dispersionRadius, spawnBots, orientation, botsName, "");
	
	return true;
}

//----------------------------------------------------------------------------

NLMISC_COMMAND(listFxEntities, "", "[<filter>]")
{
	if (args.size()>0)
		displayList(log, CFxEntityManager::getInstance()->getEntities(), args[0]);
	else
		displayList(log, CFxEntityManager::getInstance()->getEntities());
	return true;
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Old set of commands                                                      //
//////////////////////////////////////////////////////////////////////////////

// Dump the road/zone connectivity
NLMISC_COMMAND(dumpRoadCon, "dump road/cell connectivity graph","<continentName> [<ai instance number = 0>]")
{
	if (args.size() > 2)
		return false;

	uint instanceIndex = 0;

	if (args.size() == 2)
		NLMISC::fromString(args[1], instanceIndex);

	if (instanceIndex >= CAIS::instance().AIList().size())
	{
		log.displayNL("ai index %u is out of range ! (only %u ai instance)",
			instanceIndex,
			CAIS::instance().AIList().size());
		return false;
	}

	CAIInstance	*aii = CAIS::instance().AIList()[instanceIndex];

	for (uint i=0; i<aii->continents().size(); ++i)
	{
		CContinent *cont = aii->continents()[i];
		if	(!cont)
			continue;

		if	(	args.empty()
			||	cont->getName()==args[0])
		{
			log.displayRawNL("Dumping connection in '%s' (@ %p):",
				cont->getName().c_str(),
				cont);

			CAliasCont<CRegion> &regions = cont->regions();

			for (uint i=0; i<regions.size(); ++i)
			{
				CRegion *region = regions[i];
				if (!region)
					continue;
				
				CAliasCont<CCellZone> &cellZones = region->cellZones();

				for (uint i=0; i<cellZones.size(); ++i)
				{
					CCellZone *czone = cellZones[i];
					if (!czone)	continue;
					for (uint j=0; j<czone->cells().size(); ++j)
					{
						CCell *cell = czone->cells()[j];
						if (!cell)	continue;

						set<NLMISC::CDbgPtr<CCell> >::iterator first(cell->_NeighbourCells.begin()), last(cell->_NeighbourCells.end());
						for (; first != last; ++first)
						{
							nldebug("Link '%s' => '%s'", cell->getName().c_str(), (*first)->getName().c_str());
						}
					}
				}

				
				for (uint i=0; i<cellZones.size(); ++i)
				{
					CCellZone *czone = cellZones[i];
					if (!czone)	continue;
					for (uint j=0; j<czone->cells().size(); ++j)
					{
						CCell *cell = czone->cells()[j];
						if (!cell)	continue;
						// bound the point into the cell
						for (uint k=0; k<cell->npcZoneCount(); ++k)
						{
							CNpcZone *nz = cell->npcZone(k);
							if	(!nz)
								continue;

							for (uint l=0; l<nz->roads().size(); ++l)
							{
								nldebug("NpcZone '%s' host road '%s'", nz->getAliasTreeOwner().getName().c_str(), nz->roads()[l]->getName().c_str());
							}
							
						}

						CAliasCont<CRoad> &roads = cell->roads();
						for (uint i=0; i<roads.size(); ++i)
						{
							CRoad *road = roads[i];
							if (!road)
								continue;
							// clear existing link;
							nldebug("Road '%s' link from '%s' %s to '%s' %s", road->getName().c_str(), 
								road->startZone().isNULL() ? "none" : road->startZone()->getAliasTreeOwner().getName().c_str(),
								road->startExternal() ? "(external)" : "",
								road->endZone().isNULL() ? "none" : road->endZone()->getAliasTreeOwner().getName().c_str(),
								road->endExternal() ? "(external)" : "");
						}
						
					}
				}
			}
			
		}
	}

	return true;
}

bool dumpContinentImp(uint instanceIndex, const std::string &continentName, NLMISC::CLog &log);
// Dump the structure of a dynamic continent
NLMISC_COMMAND(dumpContinent, "dump the structure of a continent","<continentName> [<ai instance number = 0>]")
{
	if (args.size() > 2 || args.size() < 1)
		return false;

	uint instanceIndex = 0;

	if (args.size() == 2)
		NLMISC::fromString(args[1], instanceIndex);

	if (instanceIndex >= CAIS::instance().AIList().size())
	{
		log.displayNL("ai index %u is out of range ! (only %u ai instance)",
			instanceIndex,
			CAIS::instance().AIList().size());
		return false;
	}

	return dumpContinentImp(instanceIndex, args[0], log);
}

bool dumpContinentImp(uint instanceIndex, const std::string &continentName, NLMISC::CLog &log)
{
	CAIInstance	*aii = CAIS::instance().AIList()[instanceIndex];

	for (uint i=0; i<aii->continents().size(); ++i)
	{
		CContinent *cont = aii->continents()[i];
		if (!cont)	continue;

		if (cont->getName()==continentName)
		{
			log.displayRawNL("Dumping continent '%s' (@ %p):",
				cont->getName().c_str(),
				cont);

			for (uint j=0; j<cont->regions().size(); ++j)
			{
				CRegion *region = cont->regions()[j];
				if (!region)	
				{
					log.displayNL(" +- %u null entry", j);
					continue;
				}

				log.displayNL(" +- Region '%s'%s @ %p", 
					region->getName().c_str(), 
					region->getAliasString().c_str(), 
					static_cast<CAliasTreeOwner*>(region));

				for (uint k=0; k<region->cellZones().size(); ++k)
				{
					CCellZone *cz = region->cellZones()[k];
					if (!cz)
					{
						log.displayNL(" | +- %u null entry", k);
						continue;
					}

					log.displayNL(" | +- CellZone '%s'%s @ %p", 
						cz->getName().c_str(), 
						cz->getAliasString().c_str(), 
						static_cast<CAliasTreeOwner*>(cz));
	
					for (uint l=0; l<cz->cells().size(); ++l)
					{
						CCell *cell = cz->cells()[l];
						if (!cell)
						{
							log.displayNL(" | | +- %u null entry", l);
							continue;
						}

						log.displayNL(" | | +- Cell '%s'%s @ %p", 
							cell->getName().c_str(), 
							cell->getAliasString().c_str(), 
							static_cast<CAliasTreeOwner*>(cell));

						for (uint m=0; m<cell->npcZoneCount(); ++m)
						{
							CNpcZone *nz = cell->npcZone(m);
							if (!nz)
							{
								log.displayNL(" | | | +- %u null entry", m);
								continue;
							}

//							log.displayNL(" | | | +- NpcZone '%s' (alias A:%u @ %p)", nz->getName().c_str(), nz->getAlias(), static_cast<CAliasTreeOwner*>(nz));
							log.displayNL(" | | | +- NpcZone '%s'%s", 
								nz->getAliasTreeOwner().getName().c_str(), 
								nz->getAliasTreeOwner().getAliasString().c_str());
							
						}

						for (uint k=0; k<cell->roads().size(); ++k)
						{
							CRoad *road = cell->roads()[k];
							if (!road)
							{
								log.displayNL(" | | +- %u null entry", k);
								continue;
							}
							
							log.displayNL(" | | +- Road '%s'%s @ %p", 
								road->getName().c_str(), 
								road->getAliasString().c_str(), 
								static_cast<CAliasTreeOwner*>(road));
						}						

						for (uint m=0; m<cell->faunaZones().size(); ++m)
						{
							CFaunaZone *fz = cell->faunaZones()[m];
							if (!fz)
							{
								log.displayNL(" | | | +- %u null entry", m);
								continue;
							}

							log.displayNL(" | | | +- FaunaZone '%s'%s @ %p", 
								fz->getName().c_str(), 
								fz->getAliasString().c_str(), 
								static_cast<CAliasTreeOwner*>(fz));
						}
					}
				}
			}

			// no more to do
			return true;
		}
	}

	log.displayNL("Can't find continent '%s' in ai instance %u",
		continentName.c_str(),
		instanceIndex);
	
	return true;
}



// Load a collision map
NLMISC_COMMAND(loadContinent, "load a continent collision map","<landName>")
{
	if (args.size() != 1)
		return false;

	CWorldContainer::loadContinent(args[0]);
	
	return true;
}


CAIInstance *currentInstance = NULL;

NLMISC_COMMAND(createDynamicAIInstance, "Create a new dynamic AIInstance","")
{
	if	(args.size() != 1)
		return false;

	// find an unused continent id
	uint32 in;
	NLMISC::fromString(args[0], in);
	if( !CAIS::instance().getAIInstance(in) )
	{	
		std::string name= NLMISC::toString("ring_%d",in);
		const int index = CAIS::instance().createAIInstance(name, in);

		nlinfo("AIInstance %u created for continent '%s' (Instance Index : %u)", in, name.c_str(), index);
	}

	
	// activate the current AIInstance
	currentInstance = CAIS::instance().getAIInstance(in);	//CAIS::instance().getAIInstance(continentInstanceId);
	CWorkPtr::aiInstance(currentInstance);
	return true;
}

NLMISC_COMMAND(createStaticAIInstance, "Create a new static AIInstance for a given continent.","<continentName>")
{
	if	(args.size() != 1)
		return false;
	
	CUsedContinent &uc = CUsedContinent::instance();

	const	uint32 in = uc.getInstanceForContinent(args[0]);
	if (in == INVALID_AI_INSTANCE)
	{
		nlwarning("The continent '%s' is unknow or not active. Can't create instance, FATAL", args[0].c_str());
		nlassert(in != INVALID_AI_INSTANCE);
//		nlassertex(in != ~0, ("The continent '%s' is unknow or not active. Can't create instance, FATAL", args[0].c_str()));
	}

	if	(!CAIS::instance().getAIInstance(in))
	{
		const int index = CAIS::instance().createAIInstance(args[0], in);
		nlinfo("AIInstance %u created for continent '%s' (Instance Index : %u)", 
			in, args[0].c_str(), index);
	}

	// activate the current AIInstance
	currentInstance = CAIS::instance().getAIInstance(in);	//CAIS::instance().getAIInstance(continentInstanceId);
	return true;
}

NLMISC_COMMAND(autoConfig, "Automaticaly load collision map, create AIInstance and load the primitives","")
{
	if (!args.empty())
		return false;

	CUsedContinent &uc = CUsedContinent::instance();

	log.displayNL("AIS Auto config with %u continents...", uc.getContinents().size());
	for (uint i=0; i<uc.getContinents().size(); ++i)
	{
		const CUsedContinent::TContinentInfo &ci = uc.getContinents()[i];

		// translate logical continent name
		const string &phyName = uc.getPhysicalContinentName(ci.ContinentName);

		// load the collision map
		ICommand::execute(toString("loadContinent %s", phyName.c_str()), log);
		// create the AI Instance
		ICommand::execute(toString("createStaticAIInstance %s", ci.ContinentName.c_str()), log);
		// load the active maps
		ICommand::execute(toString("loadMapsFromCommon %s_all", ci.ContinentName.c_str()), log);
	}

	return true;
}

bool	getParameter(const	std::string	&srcStr,	const	std::string	&keyWord,	std::string	&returnStr)
{
	if	(srcStr.find(keyWord)!=std::string::npos)
	{
		returnStr=srcStr.substr(keyWord.size(),std::string::npos);
		return	true;
	}
	returnStr=std::string();
	return	false;
}

//-------------------------------------------------------------------------
// set/get the energy value for a family (global version)
NLMISC_COMMAND(energy, "set or get the effective energy (affects group type).","[cellZone-<cellZoneName>] [family-<familyName>] [value-<value>] [detailled]")
{
	std::string	family;
	std::string	cellZoneName;
	uint32	value=~0;
	bool	detailled=false;

	CLogStringWriter	stringWriter(&log);
	
	{
		for	(uint32	i=0;i<args.size();i++)
		{
			const	std::string	&str=args[i];
			string	res;

			if (getParameter(str,"cellZone-",res))
			{
				cellZoneName=res;
			}

			if (getParameter(str,"family-",res))
			{
				family = res;
			}

			if (getParameter(str,"value-",res))
			{
				float	v=float(atof(res.c_str()));
				clamp(v,0,1);
				value	=uint32(v * ENERGY_SCALE);
			}

			if (getParameter(str,"detailled",res))
			{
				detailled=true;
			}
		}
	}

	const	CStringFilter	familyFilter(family);

	for (uint i=0; i<CAIS::instance().AIList().size(); ++i)
	{
		CAIInstance *aii = CAIS::instance().AIList()[i];
		if (!aii)
			continue;

		for (uint j=0; j<aii->continents().size(); ++j)
		{
			CContinent *cont = aii->continents()[j];
			if (!cont)
				continue;

			for (uint k=0; k<cont->regions().size(); ++k)
			{
				CRegion *r = cont->regions()[k];
				if (!r)
					continue;

				for (uint l=0; l<r->cellZones().size(); ++l)
				{
					CCellZone *cz = r->cellZones()[l];
					if	(!cz)
						continue;

					if	(	!cellZoneName.empty()
						&&	cz->getAliasFullName()!=cellZoneName)
						continue;

					if	(value==~0)
						stringWriter.append("");

					for (uint m=0; m<cz->_Families.size(); ++m)
					{
						CFamilyBehavior *fb = cz->_Families[m];

						if	(!fb)
							//	TODO
							//							||	(	!family.empty()
//								&&	familyFilter!=fb->getFamily().getFamilyName()))
						{
							continue;
						}

						if	(value==~0)	//	not for affectation.
						{
							fb->displayLogOld	(stringWriter, detailled);
						}
						else
						{
							fb->setEffectiveLevel	(value);
						}
					}
				}
			}
		}
	}
	return true;
}


class CDoOnFamily
{
public:
	CDoOnFamily()
	{}
	virtual ~CDoOnFamily()
	{}
	virtual	void	doOnFamily		(CFamilyBehavior	*fb)	const	=	0;
	virtual	void	doOnCellZone	(CCellZone	*cz)	const	=	0;
protected:	
private:
};


bool	doOnFamily	(const std::vector<std::string> &args, CDoOnFamily	*fam)
{
	std::string	family;
	std::string	cellZoneName;
	
	for	(uint32	i=0;i<args.size();i++)
	{
		const	std::string	&str=args[i];
		string	res;
		
		if (getParameter(str,"cellZone-",res))
			cellZoneName=res;
		
		if (getParameter(str,"family-",res))
			family = res;
	}
		
	const	CStringFilter	familyFilter(family);
	
	FOREACH(aiiIt, CCont<CAIInstance>, CAIS::instance().AIList())
	{
		FOREACH(contIt, CCont<CContinent>, (*aiiIt)->continents())
		{
			FOREACH(rIt, CCont<CRegion>, (*contIt)->regions())
			{
				FOREACH(czIt, CCont<CCellZone>, (*rIt)->cellZones())
				{
					if	(	!cellZoneName.empty()
						&&	czIt->getAliasFullName()!=cellZoneName)
						continue;

					fam->doOnCellZone(*czIt);
					
					FOREACH(fb, CCont<CFamilyBehavior>, czIt->_Families)
					{
						if	(	!family.empty()
							&&	familyFilter!=fb->getName())
							continue;

						fam->doOnFamily(*fb);
					}
				}
			}
		}
	}
	return true;
}


class	CDoOnFamilyCommand
:public	CDoOnFamily
{
public:
	CDoOnFamilyCommand	(const std::vector<std::string> &args, NLMISC::CLog &log)
		:_stringWriter(&log)
	{
		_index=~0;
		_value=-1;
		_detailled=false;
		for	(uint32	i=0;i<args.size();i++)
		{
			const	std::string	&str=args[i];
			string	res;			
			if (getParameter(str,"index-",res))
				NLMISC::fromString(res, _index);
			if (getParameter(str,"value-",res))
				_value=float(atof(res.c_str()));
			_detailled|=getParameter(str,"detailled",res);
		}
	}
	
	void	doOnFamily(CFamilyBehavior	*fb)	const
	{
		if	(_value==-1)	//	not for affectation.
		{
			fb->displayLogOld	(_stringWriter, _detailled);
			return;
		}
		if	(_index==~0)	//	all indexs ?
		{
			for	(uint32 nrjIndex=0;nrjIndex<4;nrjIndex++)
				fb->setModifier	(_value, nrjIndex);
			return;
		}
		fb->setModifier	(_value, (uint32)_index);
	}
	
	virtual	void	doOnCellZone(CCellZone	*cz)	const
	{
		if	(_value==-1)
			_stringWriter.append("");
	}
protected:	
private:
	uint32	_index;
	float	_value;
	bool	_detailled;
	mutable	CLogStringWriter	_stringWriter;
};


NLMISC_COMMAND(energyscalemodifier, "set or get the energy scale (affects group number).","[cellZone-<cellZoneName>] [family-<familyName>] [index-<index>] [value-<value>] [detailled]")
{
	CDoOnFamilyCommand	command(args, log);
	return	doOnFamily	(args, &command);
}

NLMISC_COMMAND(energyScaleModifier2, "set or get the energy scale (affects group number).","[--cellZone=<cellZoneName>] [--family=<familyName>] [--index=<index>] [--value=<value>] [--detailled]")
{
	std::string	cellZoneName;
	std::string	familyName;
	int index = -1;
	float value = -1.f;
	bool detailed = false;
	for	(size_t	i=0; i<args.size(); ++i)
	{
		std::string	const& str = args[i];
		string res;
		if (getParameter(str, "--cellZone=", res))
			cellZoneName = res;
		if (getParameter(str, "--family=", res))
			familyName = res;
		if (getParameter(str, "--index=", res))
			fromString(res, index);
		if (getParameter(str, "--value=", res))
			fromString(res, value);
		if (str == "--detailed")
			detailed = true;
	}
	if (value<0.f)
	{
		// Display
		std::vector<size_t> widths(10, 0);
		CFamilyBehavior::checkLogHeadersWidths(widths, index, detailed);
		FOREACH(itInstance, CCont<CAIInstance>, CAIS::instance().AIList())
		{
			FOREACH(itContinent, CCont<CContinent>, itInstance->continents())
			{
				FOREACH(itRegion, CCont<CRegion>, itContinent->regions())
				{
					FOREACH(itCellZone, CCont<CCellZone>, itRegion->cellZones())
					{
						CCellZone* cellZone = *itCellZone;
						if (!cellZone || !cellZoneName.empty() && cellZone->getName()!=cellZoneName)
							continue;
						FOREACH(itFamilyBehavior, CCont<CFamilyBehavior>, cellZone->familyBehaviors())
						{
							CFamilyBehavior* familyBehavior = *itFamilyBehavior;
							if (!familyBehavior || !familyName.empty() && familyBehavior->getName()!=familyName)
								continue;
							familyBehavior->checkLogWidths(widths, index, detailed);
						}
					}
				}
			}
		}
		CLogStringWriter stringWriter(&log);
		CFamilyBehavior::displayLogLine(stringWriter, index, detailed, widths);
		CFamilyBehavior::displayLogHeaders(stringWriter, index, detailed, widths);
		CFamilyBehavior::displayLogLine(stringWriter, index, detailed, widths);
		FOREACH(itInstance, CCont<CAIInstance>, CAIS::instance().AIList())
		{
			FOREACH(itContinent, CCont<CContinent>, itInstance->continents())
			{
				FOREACH(itRegion, CCont<CRegion>, itContinent->regions())
				{
					FOREACH(itCellZone, CCont<CCellZone>, itRegion->cellZones())
					{
						CCellZone* cellZone = *itCellZone;
						if (!cellZone || !cellZoneName.empty() && cellZone->getName()!=cellZoneName)
							continue;
						FOREACH(itFamilyBehavior, CCont<CFamilyBehavior>, cellZone->familyBehaviors())
						{
							CFamilyBehavior* familyBehavior = *itFamilyBehavior;
							if (!familyBehavior || !familyName.empty() && familyBehavior->getName()!=familyName)
								continue;
							familyBehavior->displayLog(stringWriter, index, detailed, widths);
						}
					}
				}
			}
		}
		CFamilyBehavior::displayLogLine(stringWriter, index, detailed, widths);
	}
	else
	{
		// Change value
	}
	return true;
}



NLMISC_COMMAND(globalEnergy, "set or get the effective energy for a family.","[<familyName> [<value>]]")
{
	if (args.size() >  2)
		return false;

	string	family;

	CLogStringWriter	stringWriter(&log);
	
	if (args.size() > 0)
	{
		family=args[0];
	}

	uint32 value;
	if (args.size() == 2)
	{
		float v = float(atof(args[1].c_str()));
		clamp(v,0,1);
		stringWriter.append("Setting effective energy level for '"+family+"' to "+toString(v));
		value = uint32(v * ENERGY_SCALE);
	}

	for (uint i=0; i<CAIS::instance().AIList().size(); ++i)
	{
		CAIInstance *aii = CAIS::instance().AIList()[i];
		if (!aii)
			continue;

		for (uint j=0; j<aii->continents().size(); ++j)
		{
			CContinent *cont = aii->continents()[j];
			if (!cont)
				continue;

			for (uint k=0; k<cont->regions().size(); ++k)
			{
				CRegion *r = cont->regions()[k];
				if (!r)
					continue;

				for (uint l=0; l<r->cellZones().size(); ++l)
				{
					CCellZone *cz = r->cellZones()[l];
					if (!cz)
						continue;

					stringWriter.append("");

					for (uint m=0; m<cz->_Families.size(); ++m)
					{
						CFamilyBehavior *fb = cz->_Families[m];
						if (!fb)
							continue;

						if	(args.size() ==0
							||	(	args.size() == 1
								&&	family == fb->getName())
							)
						{
							fb->displayLogOld	(stringWriter, args.size()==1);
							continue;
						}

						if	(	args.size() == 2
							&&	family == fb->getName())
						{
							fb->setEffectiveLevel(value);
						}
					}
				}
			}
		}
	}
	return true;
}
	
//-------------------------------------------------------------------------
// memory report
// NLMISC_COMMAND(statMemory, "generate a memory usage statistic file","<begin|end>")
// {
// 	if (args.size() != 1)
// 		return false;
// 	
// 	CLogStringWriter	stringWriter(&log);
// 	
// 	if (args[0] == "begin")
// 	{
// 		stringWriter.append("Writing begin memory state in 'memory_report_begin.csv'...");
// 		NLMEMORY::StatisticsReport("memory_report_begin.csv", false);
// 		stringWriter.append("Memory stat Done.");
// 	}
// 	else if (args[0] == "end")
// 	{
// 		stringWriter.append("Writing end memory state report in 'memory_report_end.csv'...");
// 		NLMEMORY::StatisticsReport("memory_report_end.csv", false);
// 		stringWriter.append("Memory stat Done.");
// 	}
// 	else
// 		return false;
// 	
// 	return true;
// }

//-------------------------------------------------------------------------
// set a group variable

NLMISC_COMMAND(setGroupVar,"display the logic var of the group in the given AI Instance","<aiInstanceId> <group_name|group_alias> <varId> <value>")
{
	if	(args.size()!=4)
		return false;

	CGroup	*grp = NULL;

	CLogStringWriter	stringWriter(&log);

	CAIInstance	*aiInstance=NULL;
	{
		uint32 aiInstanceId = NLMISC::atoui(args[0].c_str());
		if (aiInstanceId>CAIS::instance().AIList().size())
		{
			stringWriter.append("Invalid AI Instance Number");
			return	false;
		}
		aiInstance=CAIS::instance().AIList()[aiInstanceId];
		if (!aiInstance)
		{
			stringWriter.append("AI Instance do not exists");
			return	false;
		}
	}
	
	
	// first, try to find the group by alias
	uint32 alias = NLMISC::atoui(args[1].c_str());
	if (alias != 0)
	{
		grp = aiInstance->findGroup(alias);
	}

	if (grp == NULL)
	{
		/// try to find the group bt name
		std::vector<CGroup*> grps;
		aiInstance->findGroup(grps, args[1]);
		if (grps.size() > 1)
		{
			std::string s;
			for (uint i=0; i<grps.size(); ++i)
			{
				s += NLMISC::toString("%s ", grps[i]->aliasTreeOwner()->getAliasString().c_str());
			}
			stringWriter.append("More than one group have name '"+args[1]+"', listing alias :");
			stringWriter.append(s);

			return false;
		}
		else if (grps.empty())
		{
			stringWriter.append("No group correspond to name '"+args[1]+"'");
			return false;
		}
		else
		{
			grp = grps.front();
		}
	}

	// retreive the varID.
//	uint32	varId = NLMISC::atoui(args[2].c_str());
	NLMISC::TStringId	varId=CStringMapper::map(args[2]);	//	NLMISC::atoui(args[2].c_str());
	float	value = float(atof(args[3].c_str()));


	CStateInstance	*stateInstance=grp->getPersistentStateInstance();
	stringWriter.append(grp->aliasTreeOwner()->getName()+":"+toString(*varId)+" = "+toString(value)+" (old value : "+ toString(stateInstance->getLogicVar(varId))+" )");
	stateInstance->setLogicVar(varId, value);
	return true;
}


//-------------------------------------------------------------------------
// display group variable

NLMISC_COMMAND(displayGroupVar,"display the logic var of the group in the given AI Instance","<aIInstanceId> <group_name|group_alias>")
{
	if	(args.size()!=2)
		return false;

	CLogStringWriter	stringWriter(&log);

	CAIInstance	*aiInstance=NULL;
	{
		uint32 aiInstanceId = NLMISC::atoui(args[0].c_str());
		if (aiInstanceId>CAIS::instance().AIList().size())
		{
			stringWriter.append("Invalid AI Instance Number");
			return	false;
		}
		aiInstance=CAIS::instance().AIList()[aiInstanceId];
		if (!aiInstance)
		{
			stringWriter.append("AI Instance do not exists");
			return	false;
		}
	}

	
	CGroup	*grp = NULL;

	// first, try to find the group by alias
	uint32 alias = NLMISC::atoui(args[1].c_str());
	if (alias != 0)
	{
		grp = aiInstance->findGroup(alias);
	}

	if (grp == NULL)
	{
		/// try to find the group bt name
		std::vector<CGroup*> grps;
		aiInstance->findGroup(grps, args[1]);
		if (grps.size() > 1)
		{
			std::string s;
			for (uint i=0; i<grps.size(); ++i)
			{
				s += NLMISC::toString("%s ", grps[i]->aliasTreeOwner()->getAliasString().c_str());
			}
			stringWriter.append("More than one group have name '"+args[1]+"', listing alias :");
			stringWriter.append(s);
			return false;
		}
		else if (grps.empty())
		{
			stringWriter.append("No group correspond to name '"+args[1]+"'");
			return false;
		}
		else
		{
			grp = grps.front();
		}
	}

	stringWriter.append("Logic var of group '"+grp->aliasTreeOwner()->getName()+"'");


//	parser les variables pour outputer le contenu ..

	{
		const	CStateInstance	*stateInstance=grp->getPersistentStateInstance();
		{
			string	str;
			stateInstance->logicVarsToString(str);
			stringWriter.append(str);
		}
	}

	return true;
}



//-------------------------------------------------------------------------
// display the player teams data

NLMISC_COMMAND(displayPlayerTeams,"display all the player teams"," [<-no_detail>]")
{
	bool noDetail = false;
	if (args.size()>1)
		return false;

	if	(	!args.empty()
		&&	args[args.size()-1]=="-no_detail")
		noDetail = true;

	CCont<CAIInstance>::iterator	it=CAIS::instance().AIList().begin(), itEnd=CAIS::instance().AIList().end();

	CLogStringWriter	stringWriter(&log);
	
	while (it!=itEnd)
	{
		std::vector<uint16>	teamIds;
		CAIInstance			*aiInstance=*it;
		aiInstance->getPlayerMgr()->getTeamIds(teamIds);
		
		if	(teamIds.empty())
		{
			continue;
		}
		
		std::vector<uint16>::iterator first(teamIds.begin()), last(teamIds.end());
		for (; first != last; ++first)
		{
			int id = *first;
			{
				stringWriter.append("Player Team "+toString(id)+" :");
				
				if (!noDetail)
				{
					const std::set<TDataSetRow> &team = aiInstance->getPlayerMgr()->getPlayerTeam(uint16(id));
					std::set<TDataSetRow>::const_iterator first(team.begin()), last(team.end());
					for (; first != last; ++first)
					{
						stringWriter.append("  Row "+toString((*first).getIndex())+" : "+CMirrors::DataSet->getEntityId(*first).toString());
					}
				}
			}
		}
		++it;
	}
	return true;
}

//-------------------------------------------------------------------------
// set the escrot team ID for a group

NLMISC_COMMAND(setEscortId,"set the escort team id for a group in the given aIInstance","<aIInstanceId> <teamId> <groupAlias>|<groupName>")
{
	if	(args.size()!=3)
		return false;

	CLogStringWriter	stringWriter(&log);

	CAIInstance	*aiInstance=NULL;
	{
		uint32 aiInstanceId = NLMISC::atoui(args[0].c_str());
		if (aiInstanceId>CAIS::instance().AIList().size())
		{
			stringWriter.append("Invalid AI Instance Number");
			return	false;
		}
		aiInstance=CAIS::instance().AIList()[aiInstanceId];
		if (!aiInstance)
		{
			stringWriter.append("AI Instance do not exists");
			return	false;
		}
	}
	
	CGroup	*grp = NULL;

	// first, try to find the group by alias
	uint32 alias = NLMISC::atoui(args[2].c_str());
	if (alias != 0)
	{
		grp = aiInstance->findGroup(alias);
	}

	if (grp == NULL)
	{
		/// try to find the group bt name
		std::vector<CGroup*> grps;
		aiInstance->findGroup(grps, args[2]);
		if (grps.size() > 1)
		{
			std::string s;
			for (uint i=0; i<grps.size(); ++i)
			{
				s += NLMISC::toString("%s ", grps[i]->aliasTreeOwner()->getAliasString().c_str());
			}
			stringWriter.append("More than one group have name '"+args[2]+"', listing alias :");
			stringWriter.append(s);
			return false;
		}
		else if (grps.empty())
		{
			stringWriter.append("No group correspond to name"+args[2]);
			return false;
		}
		else
		{
			grp = grps.front();
		}
	}

	uint16 teamId = uint16(NLMISC::atoui(args[1].c_str()));
	stringWriter.append("Setting escort teamId "
		+toString(teamId)
		+" on group '"
		+args[2]
		+"'"
		+grp->aliasTreeOwner()->getAliasString().c_str());
	grp->setEscortTeamId(teamId);

	return true;
}


//-------------------------------------------------------------------------
// buffered commands facility
typedef	vector<string>		TCommand;
typedef	vector<TCommand>	TCommandList;
static vector<string>	bufferedRetStrings;
void	clearBufferedRetStrings()
{
	bufferedRetStrings.clear();
}

TCommandList	setEventCommands;
TCommandList	scriptCommands;
TCommandList	scriptCommands2;
TCommandList	scriptCommandsBotById;
TCommandList	scriptCommandsGroupByName;
TCommandList	loadScriptCommands;

bool execSetEvent(	CStringWriter& stringWriter, TCommand const& args);
bool execScript(	CStringWriter& stringWriter, TCommand const& args);
bool execScript2(	CStringWriter& stringWriter, TCommand const& args);
bool execScriptBotById(	CStringWriter& stringWriter, TCommand const& args);
bool execScriptGroupByName(	CStringWriter& stringWriter, TCommand const& args);
bool execLoadScript(CStringWriter& stringWriter, TCommand const& args);

void	execBufferedCommands()
{
	vector<string>	retStrings;
	CArrayStringWriter	sw(retStrings);
	
	FOREACHC(it, TCommandList, setEventCommands)
	{
		execSetEvent(sw, *it);
	}
	setEventCommands.clear();
	FOREACHC(it, TCommandList, scriptCommands)
	{
		execScript(sw, *it);
	}
	scriptCommands.clear();
	FOREACHC(it, TCommandList, scriptCommands2)
	{
		execScript2(sw, *it);
	}
	scriptCommands2.clear();
	FOREACHC(it, TCommandList, scriptCommandsBotById)
	{
		execScriptBotById(sw, *it);
	}
	scriptCommandsBotById.clear();
	FOREACHC(it, TCommandList, scriptCommandsGroupByName)
	{
		execScriptGroupByName(sw, *it);
	}
	scriptCommandsGroupByName.clear();
	FOREACHC(it, TCommandList, loadScriptCommands)
	{
		execLoadScript(sw, *it);
	}
	loadScriptCommands.clear();

	// Show ret commands
	FOREACHC(strIt, vector<string>, retStrings)
		nlwarning(strIt->c_str());
	bufferedRetStrings.insert(bufferedRetStrings.end(), retStrings.begin(), retStrings.end());
}


//-------------------------------------------------------------------------
// set a user event on a group
bool	execSetEvent(CStringWriter	&stringWriter, const vector<string> &args)
{
	//	Temporize setEvent commands.
	if	(	args.size() != 3
		&&	!(	args.size() == 4
		&&	args[1] == "-no_detail"))
		return false;
		
	CAIInstance	*aiInstance=NULL;
	{
		const	uint32 aiInstanceId = NLMISC::atoui(args[0].c_str());
		if (aiInstanceId>CAIS::instance().AIList().size())
		{
			stringWriter.append("Invalid AI Instance Number");
			return	false;
		}
		aiInstance=CAIS::instance().AIList()[aiInstanceId];
		if (!aiInstance)
		{
			stringWriter.append("AI Instance do not exists");
			return	false;
		}
	}
	
	
	bool noDetail = false;
	
	if (args.size() == 4)
		noDetail = true;
	
	uint32 eventId = NLMISC::atoui(args[1].c_str());
	NLMISC::clamp(eventId, 0u, 9u);
	
	CGroup	*grp = NULL;
	
	// first, try to find the group by alias
	uint32 alias = NLMISC::atoui(args[2].c_str());
	if (alias != 0)
	{
		grp = aiInstance->findGroup(alias);
		
		if	(!grp)
			return	false;
		
		stringWriter.append("Setting event "+toString(eventId)+" on groups '"+grp->aliasTreeOwner()->getName()+"' ("+toString(grp->getChildIndex())+")");
		grp->setEvent(eventId);
		return true;
	}
	
	std::vector<CGroup*> grps;
	if	(!grp)
	{
		/// try to find the group bot name
		aiInstance->findGroup(grps, args[2]);
		if (grps.size() > 1)
		{
			if (noDetail)
			{
				std::string s;
				for (uint i=0; i<grps.size(); ++i)
				{
					s += NLMISC::toString("%s ", grps[i]->aliasTreeOwner()->getAliasString().c_str());
				}
				stringWriter.append("More than one group have name '"+args[2]+"', listing alias :");
				stringWriter.append(s);
				return false;
			}
		}
		else if (grps.empty())
		{
			stringWriter.append("No group correspond to name "+ args[2]);
			return false;
		}
	}
	
	stringWriter.append("Setting event "+toString(eventId)+" on groups '"+args[2]+"'");
	
	for (uint i=0; i<grps.size(); ++i)
		grps[i]->setEvent(eventId);
	
	return true;
}

NLMISC_COMMAND(setEvent,"set an event for a group in the given aIInstance [buffered]","<aIInstanceId> [<-no_detail>] <eventId> <groupAlias>|<groupName>")
{
	clearBufferedRetStrings();
	setEventCommands.push_back(args);
	return	true;
}

//-------------------------------------------------------------------------
// executes a script on groups matching a specified filter (1st arg)
bool execScript(CStringWriter& stringWriter, TCommand const& args)
{
	if (args.size()<1)
		return false;
	
	string const& groupName = args[0];
	typedef vector<CGroup*> TGroupContainer;
	TGroupContainer grps;
	{
		/// try to find the group bot name
		buildFilteredGroupList(grps, groupName);
		if (grps.empty())
		{
			stringWriter.append("No group correspond to name "+ groupName);
			return false;
		}
	}
	
	stringWriter.append("do script on groups '"+groupName+"'");
	
	if (args.size()>1)
	{
		vector<string> codeLines;
		for (size_t i=1; i<args.size(); ++i)
			codeLines.push_back(args[i]);
		
		CSmartPtr<const CByteCode> codePtr = CCompiler::getInstance().compileCode(codeLines, "script Command");
		
		FOREACHC(itGrp, TGroupContainer, grps)
		{
			CGroup* grp = *itGrp;
			if (grp->getPersistentStateInstance())
				grp->getPersistentStateInstance()->interpretCode(NULL, codePtr);
		}
	}
	else
	{
		FOREACHC(itGrp, TGroupContainer, grps)
		{
			CGroup* grp = *itGrp;
			if (grp->getPersistentStateInstance())
			{
				stringWriter.append("Group: "+grp->getFullName());
				grp->getPersistentStateInstance()->dumpVarsAndFunctions(stringWriter);
			}
		}
	}
	return true;
}

//-------------------------------------------------------------------------
// executes a script on groups containing a bot matching a specified filter (1st arg)
bool execScript2(CStringWriter& stringWriter, TCommand const& args)
{
	if (args.size()<1)
		return false;
	
	string const& botName = args[0];
	typedef set<CGroup*> TGroupContainer;
	TGroupContainer grps;
	{
		vector<CBot*> bots;
		/// try to find the bot name
		buildFilteredBotList(bots, botName);
		if (bots.empty())
		{
			stringWriter.append("No bot correspond to name "+ botName);
			return false;
		}
		else
		{
			FOREACH(itBot, vector<CBot*>, bots)
			{
				CBot* bot = *itBot;
				CGroup* group = bot->getOwner();
			//	if (group->getOwner()==_EventNpcManager) // This would restrict the command to event groups
				grps.insert(group);
			}
		}
	}
	
	stringWriter.append("do script on groups containing bot '"+botName+"'");
	
	if (args.size()>1)
	{
		vector<string> codeLines;
		for (size_t i=1; i<args.size(); ++i)
			codeLines.push_back(args[i]);
		
		CSmartPtr<const CByteCode> codePtr = CCompiler::getInstance().compileCode(codeLines, "script2 Command");
		
		FOREACHC(itGrp, TGroupContainer, grps)
		{
			CGroup* grp = *itGrp;
			if (grp->getPersistentStateInstance())
				grp->getPersistentStateInstance()->interpretCode(NULL, codePtr);
		}
	}
	else
	{
		FOREACHC(itGrp, TGroupContainer, grps)
		{
			CGroup* grp = *itGrp;
			if (grp->getPersistentStateInstance())
			{
				stringWriter.append("Group: "+grp->getFullName());
				grp->getPersistentStateInstance()->dumpVarsAndFunctions(stringWriter);
			}
		}
	}
	return true;
}

//-------------------------------------------------------------------------
// executes a script on groups containing a bot given its entity id
bool execScriptBotById(CStringWriter& stringWriter, TCommand const& args)
{
	if (args.size()<2)
		return false;
	
	NLMISC::CEntityId botId = NLMISC::CEntityId(args[0]);
	if (botId==NLMISC::CEntityId::Unknown)
		return false;
	CAIEntityPhysical* entity = CAIEntityPhysicalLocator::getInstance()->getEntity(botId);
	CSpawnBotNpc* bot = dynamic_cast<CSpawnBotNpc*>(entity);
	if (!bot)
		return false;
	if (!bot->getPersistent().getOwner())
		return false;
	if (!bot->getPersistent().getOwner()->getPersistentStateInstance())
		return false;
	
	vector<string> codeLines;
	for (size_t i=1; i<args.size(); ++i)
		codeLines.push_back(args[i]);
	
	CSmartPtr<const CByteCode> codePtr = CCompiler::getInstance().compileCode(codeLines, "script2 Command");
	
	bot->getPersistent().getOwner()->getPersistentStateInstance()->interpretCode(NULL, codePtr);
	
	return true;
}

//-------------------------------------------------------------------------
// executes a script on groups containing a bot given its entity id
bool execScriptGroupByName(CStringWriter& stringWriter, TCommand const& args)
{
	if (args.size()<2)
		return false;
	
	string const& groupName = args[0];
//	NLMISC::CEntityId botId = NLMISC::CEntityId(args[0]);
//	if (botId==NLMISC::CEntityId::Unknown)
//		return false;
	CGroupNpc* group = dynamic_cast<CGroupNpc*>(CAliasTreeOwnerLocator::getInstance()->getEntity(groupName));
	if (!group)
		return false;
	if (!group->getPersistentStateInstance())
		return false;
	
	vector<string> codeLines;
	for (size_t i=1; i<args.size(); ++i)
		codeLines.push_back(args[i]);
	
	CSmartPtr<const CByteCode> codePtr = CCompiler::getInstance().compileCode(codeLines, "script2 Command");
	
	group->getPersistentStateInstance()->interpretCode(NULL, codePtr);
	
	return true;
}

NLMISC_COMMAND(script,"execute a script for groups matching the given filter [buffered]","<groupFilter> <code>")
{
	clearBufferedRetStrings();
	scriptCommands.push_back(args);
	return true;
}

NLMISC_COMMAND(script2,"execute a script for groups containing a bot matching the given filter [buffered]","<groupFilter> <code>")
{
	clearBufferedRetStrings();
	scriptCommands2.push_back(args);
	return true;
}

static std::string scriptHex_decode(std::string str)
{
	std::string output;
	for (size_t i=0; i<(str.length()-1); i+=2)
	{
		char c1 = str[i], c2 = str[i+1];
		char buffer[3] = { c1, c2, '\0' };
		char c = (char)strtol(buffer, NULL, 16);
		output.push_back(c);
	}
	return output;
}

NLMISC_COMMAND(scriptHex,"execute a hex-encoded script for a group in the given aIInstance [buffered]","<groupName> <hexcode>")
{
	vector<string> _args = args;
	_args[1] = scriptHex_decode(_args[1]);
	clearBufferedRetStrings();
	scriptCommands.push_back(_args);
	return	true;
}

static const char* hexEncoderTcl =
"proc copy_encoded {} {"
"	# Get the args from the text fields"
"	set group [ .group.name get 1.0 end ]"
"	set script [ .script get 1.0 end ]"
"	# Initiate the AIS command"
"	set output [concat \"scriptHex\" ${group}]"
"	append output \" \""
"	# Convert the script itself into an hex string (-1 to remove trailing newline)"
"	for {set i 0} {$i < [string length $script]-1} {incr i} {"
"		# Get the character"
"		set c [string index $script $i]"
"		# Get its ascii value"
"		scan $c %c n"
"		# Print it as 2 hex digits"
"		append output [format %02x $n]"
"	}"
"	# Replace the clipboard with the AIS command"
"	clipboard clear"
"	clipboard append $output"
"}"
""
"frame .group"
"pack .group -anchor w -fill x"
"label .group.label -text \"Group:\""
"pack .group.label -side left"
"text .group.name -height 1 -width 70"
"pack .group.name -fill x"
"text .script"
"pack .script -fill both -expand 1"
"button .button -text \"Copy encoded!\""
"pack .button -side bottom"
""
"bind .button <ButtonPress-1> {"
"	copy_encoded"
"}"
"";

NLMISC_COMMAND(hexEncoder,"prints a script that can be used to encode a text for use with scriptHex","<language>")
{
	if (args.size()!=1)
		return false;

	if (args[0]=="tcl")
	{
		log.displayNL("%s", hexEncoderTcl);
	}
	else
	{
		log.displayNL("Invalid language name! Valid languages are: tcl");
	}
	return	true;
}

//-------------------------------------------------------------------------
// set a user event on a group
bool execLoadScript(CStringWriter& stringWriter, vector<string> const& args)
{
	if (args.size()!=3)
		return false;
	
	CAIInstance* aiInstance = NULL;
	{
		uint32 const aiInstanceId = NLMISC::atoui(args[0].c_str());
		if (aiInstanceId>CAIS::instance().AIList().size())
		{
			stringWriter.append("Invalid AI Instance Number");
			return false;
		}
		aiInstance=CAIS::instance().AIList()[aiInstanceId];
		if (!aiInstance)
		{
			stringWriter.append("AI Instance do not exists");
			return false;
		}
	}
	
	bool noDetail = false;
		
	string const& groupName = args[1];
	std::vector<CGroup*> grps;
	{
		/// try to find the group bot name
		aiInstance->findGroup(grps, groupName);
		if (grps.size() > 1)
		{
			if (noDetail)
			{
				std::string s;
				for (uint i=0; i<grps.size(); ++i)
				{
					s += NLMISC::toString("%s ", grps[i]->aliasTreeOwner()->getAliasString().c_str());
				}
				stringWriter.append("More than one group have name '"+groupName+"', listing alias :");
				stringWriter.append(s);
				return false;
			}
		}
		else if (grps.empty())
		{
			stringWriter.append("No group correspond to name "+ groupName);
			return false;
		}
	}
	
	stringWriter.append("do script on groups '"+groupName+"'");
	
	try
	{
		NLMISC::CIFile file(NLMISC::CPath::lookup(args[2]));
		
		vector<string> lines;
		while (!file.eof())
		{
			const size_t bufferSize = 4*1024;
			char buffer[bufferSize];
			file.getline(buffer, bufferSize);
			lines.push_back(buffer);
		}
		// Compile the buffer
		CSmartPtr<const CByteCode>	codePtr=CCompiler::getInstance().compileCode(lines, "script Command");
		
		// Interpret the code for each group
		FOREACHC(itGrp, vector<CGroup*>, grps)
			(*itGrp)->getPersistentStateInstance()->interpretCode(NULL, codePtr);
	}
	catch (const EPathNotFound &)
	{
		nlwarning("Path not found while loading AIS script %s", args[2].c_str());
		return false;
	}
	return true;
}

NLMISC_COMMAND(loadScript, "execute a script from a file for a group in the given aIInstance [buffered]", "<aIInstanceId> <groupName> <filename>")
{
	clearBufferedRetStrings();
	loadScriptCommands.push_back(args);
	return	true;
}

NLMISC_COMMAND(getInfo,"display returned values of buffered commands","")
{
	CLogStringWriter	stringWriter(&log);
	
	FOREACHC(strIt, vector<string>, bufferedRetStrings)
	{
		stringWriter.append(*strIt);
	}
	
	return true;
}


//-------------------------------------------------------------------------
// DISPLAYING Managers, Groups and Bots

static void displayNode(const CAIAliasDescriptionNode *node,uint indent=0)
{
	nlinfo("%*s- %s %-12s - %s",
		indent,
		"",
		node->getAliasString().c_str(),
		getName(node->getType()),
		node->fullName().c_str());
	for (uint i=0;i<node->getChildCount();++i)
		displayNode(node->getChild(i),indent+2);
}

NLMISC_COMMAND(displayNodeTreeMgr,"display node tree for given manager(s)","<manager id>[...]")
{
	if(args.size()<1)
		return false;

	for (uint i=0;i<args.size();++i)
	{
		CManager*	ManagerPtr	=	CAIS::instance().tryToGetManager(args[i].c_str());
		if (ManagerPtr)
		{
			nlinfo("Manager: %d",i);
			displayNode(ManagerPtr->getAliasTreeOwner()->getAliasNode());
			continue;
		}
		nlwarning("Failed to find mgr: %s",args[i].c_str());
	}
	return true;
}


NLMISC_COMMAND(displayTarget,"display bot target status for given bot(s) or player(s)","<bot id>[...]")
{
	if(args.size() <1)
		return false;
	
	CLogStringWriter	stringWriter(&log);

	for (uint i=0;i<args.size();++i)
	{
		CAIEntityPhysical*	EntityPtr	=	CAIS::instance().tryToGetEntityPhysical(args[i].c_str());
		if (!EntityPtr)
		{
//			log.displayNL("=> can't find entity: %s", args[i].c_str());
			continue;
		}

		CAIEntityPhysical	*phys=EntityPtr->getTarget();

		if	(!phys)
		{
			log.displayNL("=> bot %s have no target", args[i].c_str());
			continue;
		}

		bool	found=false;
		
		switch	(phys->getRyzomType())
		{
		case RYZOMID::npc:
		case RYZOMID::creature:
		case RYZOMID::pack_animal:				
			break;
		case RYZOMID::player:
				log.displayNL("=> target is a player");
			break;
		default:
			{
				CSpawnBot*	spawnBot=dynamic_cast<CSpawnBot*>(phys);
				if	(spawnBot)
				{
					vector<string> strings = spawnBot->getPersistent().getMultiLineInfoString();
					FOREACHC(itString, vector<string>, strings)
						log.displayNL("%s", itString->c_str());
					found=true;
				}

			}
			break;
		}

		if	(!found)
		{
			log.displayNL("=> can't display information for the target of: %s", args[i].c_str());		
		}
		
	}
	return	true;
}

NLMISC_COMMAND(displayVision3x3,"display 3x3 cell vision centred on a given coordinate in the given aIInstance","<aiInstance> <x><y>")
{
	if(args.size()!=3)
		return false;

	CAIInstance	*aiInstance=NULL;
	{
		uint32 aiInstanceId = NLMISC::atoui(args[0].c_str());
		if (aiInstanceId>CAIS::instance().AIList().size())
		{
			log.displayNL("Invalid AI Instance Number");
			return	false;
		}
		aiInstance=CAIS::instance().AIList()[aiInstanceId];
		if (!aiInstance)
		{
			log.displayNL("AI Instance do not exists");
			return	false;
		}
	}

	CAICoord x, y;
	x=atof(args[1].c_str());
	y=atof(args[2].c_str());
	log.displayNL("3x3 Vision around (%.3f,%.3f)", x.asDouble(), y.asDouble());

	uint32 botCount=0;
	uint32 plrCount=0;

	CAIVector	position(x,y);

	{
		CAIEntityMatrix<CPersistentOfPhysical>::CEntityIteratorRandom it;
		for (it=aiInstance->playerMatrix().beginEntities(CAIS::instance().matrixIterator3x3(),position);!it.end();++it)
		{
			++plrCount;
			//		log.displayNL("=> PLR: %s (%s)", (*it).id().toString().c_str(), (*it).pos().toString().c_str());
			if ((*it).isSpawned())
				log.displayNL("=> (messageToChange)PLR: %d (%s)", (*it).getSpawnObj()->dataSetRow().getIndex(), (*it).getSpawnObj()->pos().toString().c_str());
		}
	}

	{
		CAIEntityMatrix<CPersistentOfPhysical>::CEntityIteratorRandom it;
		for (it=aiInstance->botMatrix().beginEntities(CAIS::instance().matrixIterator3x3(),position);!it.end();++it)
		{
			CPersistentOfPhysical	&persRef=(*it);
			if (!persRef.isSpawned())
				continue;

			++botCount;
			CBot*	botPtr=NLMISC::safe_cast<CBot*>(&persRef);

			if (botPtr->getAliasTreeOwner())
				log.displayNL("=> BOT: %s (%s)", botPtr->getAliasTreeOwner()->getName().c_str(),	persRef.getSpawnObj()->pos().toString().c_str());
			else
				log.displayNL("=> BOT: unknown (%s)", persRef.getSpawnObj()->pos().toString().c_str());

	//		log.displayNL("=> BOT: %s (%s)", (*it).id().toString().c_str(), (*it).pos().toString().c_str());
		}
	}

	log.displayNL("Entites found: %d bots, %d players, total: %d", botCount, plrCount, botCount+plrCount);

	return true;
}

NLMISC_COMMAND(displayVisionRadius,"display roughly 'radius' cell vision centred on a given coordinate in the given aIInstance","<aIInstance> <x><y>[<radius>=100]")
{
	if	(	args.size()!=3
		&&	args.size()!=4	)
		return false;

	CAIInstance	*aiInstance=NULL;
	{
		uint32 aiInstanceId = NLMISC::atoui(args[0].c_str());
		if (aiInstanceId>CAIS::instance().AIList().size())
		{
			log.displayNL("Invalid AI Instance Number");
			return	false;
		}
		aiInstance=CAIS::instance().AIList()[aiInstanceId];
		if (!aiInstance)
		{
			log.displayNL("AI Instance do not exists");
			return	false;
		}
	}
	
	uint32 dist=100;
	CAICoord x, y;
	x=atof(args[1].c_str());
	y=atof(args[2].c_str());
	if (args.size()==4)
		NLMISC::fromString(args[3], dist);
	log.displayNL("%dm Vision around (%.3f,%.3f)", dist, x.asDouble(), y.asDouble());

	uint32 botCount=0;
	uint32 plrCount=0;

	const CAIEntityMatrixIteratorTblLinear *tbl=CAIS::instance().bestLinearMatrixIteratorTbl(dist);

	CAIEntityMatrix<CPersistentOfPhysical>::CEntityIteratorLinear it;

	CAIVector	position(x,y);
	for (it=aiInstance->playerMatrix().beginEntities(tbl,position);!it.end();++it)
	{
		++plrCount;
		const	CPersistentOfPhysical&	entityPtr	=	(*it);
		const	CBotPlayer*	const	playerPtr=dynamic_cast<const CBotPlayer* const>(&entityPtr);
		if (!playerPtr)
			continue;
		
		//	have to implement players.
#ifdef NL_DEBUG
		nlwarning("Not Implemented");
#endif
		//		log.displayNL("=> PLR: %s (%s)", (*it).id().toString().c_str(), (*it).pos().toString().c_str());
		//		log.displayNL("=> PLR: %s (%s)", (*it).id().toString().c_str(), (*it).pos().toString().c_str());
	}

	for (it=aiInstance->botMatrix().beginEntities(tbl,position);!it.end();++it)
	{
		if ((*it).isSpawned())
		{
			++botCount;
			//		log.displayNL("=> BOT: %s (%s)", (*it).id().toString().c_str(), (*it).pos().toString().c_str());
			log.displayNL("=> (messageToChange)BOT: %d (%s)", (*it).getSpawnObj()->dataSetRow().getIndex(), (*it).getSpawnObj()->pos().toString().c_str());
		}
	}

	log.displayNL("Entites found: %d bots, %d players, total: %d", botCount, plrCount, botCount+plrCount);
	return true;
}




NLMISC_COMMAND(countEntitesInVisionMatrix,"scan the vision matrix for entities in the given aiInstance","<aiInstance>")
{
	if	(args.size()!=1)
		return false;

	CAIInstance	*aiInstance=NULL;
	{
		uint32 aiInstanceId = NLMISC::atoui(args[0].c_str());
		if (aiInstanceId>CAIS::instance().AIList().size())
		{
			log.displayNL("Invalid AI Instance Number");
			return	false;
		}
		aiInstance=CAIS::instance().AIList()[aiInstanceId];
		if (!aiInstance)
		{
			log.displayNL("AI Instance do not exists");
			return	false;
		}
	}
	
	// build an iterator table for our scan
	CAIEntityMatrixIteratorTblRandom tbl;
	tbl.push_back(0,0);
	for (int i=0;i<256;++i)
	{
		for (int j=0;j<255;++j)
			tbl.push_back(1,0);
		if (i<255)
			tbl.push_back(-255,1);
	}

	// some variables
	CAIEntityMatrix<CPersistentOfPhysical>::CCellTblIteratorRandom it;
	uint32 botCount=0;
	uint32 plrCount=0;

	CAIVector	position(0,0);
	// count entities in bot matrix
	for (it=aiInstance->botMatrix().beginCells(&tbl,position);!it.end();++it)
	{		
		for (CEntityListLink<CPersistentOfPhysical> *listLink=(*it)->next();listLink!=*it;listLink=listLink->next())
			++botCount;
	}

	// count entities in player matrix
	for (it=aiInstance->playerMatrix().beginCells(&tbl,position);!it.end();++it)
	{
		for (CEntityListLink<CPersistentOfPhysical> *listLink=(*it)->next();listLink!=*it;listLink=listLink->next())
			++plrCount;
	}

	// display the results
	log.displayNL("Entites found: %d bots, %d players, total: %d", botCount, plrCount, botCount+plrCount);

	return true;
}


//-------------------------------------------------------------------------
// DISPLAYING Places

NLMISC_COMMAND(displayPlaces,"display the list of places for specified manager","[<manager id>]")
{
	if(args.size() !=1)
		return false;

	// manager specified so lookup manager by name
	CMgrFauna*	mgrFaunaPtr	=	dynamic_cast<CMgrFauna*>(CAIS::instance().tryToGetManager(args[0].c_str()));
	if (mgrFaunaPtr==NULL)
		return	false;

	CLogStringWriter	stringWriter(&log);

	CCont<CGroup >::iterator	it		=	mgrFaunaPtr->groups().begin();
	CCont<CGroup >::iterator	itEnd	=	mgrFaunaPtr->groups().end();
	while (it!=itEnd)
	{
		NLMISC::safe_cast<CGrpFauna*>(*it)->displayPlaces(stringWriter);
		++it;
	}
	return true;
}


//-------------------------------------------------------------------------
// WATCHES FOR TRACKING MGR, GRP & BOT OBJECTS

static std::string	WATCH0,WATCH1,WATCH2,WATCH3,WATCH4,
					WATCH5,WATCH6,WATCH7,WATCH8,WATCH9;
static std::string *watchStrings[]=
{
	&WATCH0,&WATCH1,&WATCH2,&WATCH3,&WATCH4,
	&WATCH5,&WATCH6,&WATCH7,&WATCH8,&WATCH9
};

static CAIEntity*	watchEntity[sizeof(watchStrings)/sizeof(watchStrings[0])];
static uint			watchIdx[sizeof(watchStrings)/sizeof(watchStrings[0])];

NLMISC_VARIABLE(string, WATCH0, "watch string 0");
NLMISC_VARIABLE(string, WATCH1, "watch string 1");
NLMISC_VARIABLE(string, WATCH2, "watch string 2");
NLMISC_VARIABLE(string, WATCH3, "watch string 3");
NLMISC_VARIABLE(string, WATCH4, "watch string 4");
NLMISC_VARIABLE(string, WATCH5, "watch string 5");
NLMISC_VARIABLE(string, WATCH6, "watch string 6");
NLMISC_VARIABLE(string, WATCH7, "watch string 7");
NLMISC_VARIABLE(string, WATCH8, "watch string 8");
NLMISC_VARIABLE(string, WATCH9, "watch string 9");

// 
bool FAUNA_GRAPH_USES_DEBUG_TIME = false;
NLMISC_VARIABLE(bool, FAUNA_GRAPH_USES_DEBUG_TIME, "force fauna graph to use local time (for debug)");

// global routine called by the service main update()

void UpdateWatches()
{
//	if (!CAIS::initialised())
//		return;
	
	for (uint i=0;i<sizeof(watchStrings)/sizeof(watchStrings[0]);++i)
	{
		if (watchEntity[i])
			*watchStrings[i] = watchEntity[i]->getOneLineInfoString();
		else
			*watchStrings[i] = string("<NULL ENTITY>");
	}

}

void	removeFromWatch(CAIEntity	*entity)
{
	for (uint i=0;i<sizeof(watchStrings)/sizeof(watchStrings[0]);++i)
	{
		if (watchEntity[i]==entity)
			watchEntity[i]=NULL;
	}

}


NLMISC_COMMAND(setWatch,"setup one of the watch variables","<watch id> <mgr, grp or bot id> [<index>]")
{
	if (args.size()!=2 && args.size()!=3)
		return false;

	uint	idx;
	NLMISC::fromString(args[0], idx);
	if	(	toString(idx)!=args[0]
		||	idx>=sizeof(watchStrings)/sizeof(watchStrings[0]))
		return false;

	CAIEntity*	CAIEntityPtr	=	CAIS::instance().tryToGetAIEntity(args[1].c_str());

	if (!CAIEntityPtr)
		return true;

	watchEntity[idx]=CAIEntityPtr;
	
	if	(args.size()==3)
		NLMISC::fromString(args[2], watchIdx[idx]);
	else
		watchIdx[idx]=0;
	return	true;
}

static bool setBotRecordHistory(std::vector<std::string> args,bool onOff)
{
	if (args.size()==0)
	{
		CAIInstance*	instancePtr	=	CAIS::instance().AIList().getNextValidChild();
		while (instancePtr!=NULL)
		{
			CManager*	mgrPtr	=	instancePtr->managers().getNextValidChild();
			while (mgrPtr!=NULL)
			{
				CGroup*	grpPtr=mgrPtr->getNextValidGroupChild();
				while (grpPtr!=NULL)
				{
					grpPtr->getDebugHistory()->setRecording(onOff);
					
					CBot*	botPtr	=	grpPtr->getNextValidBotChild();
					while (botPtr!=NULL)
					{
						botPtr->getDebugHistory()->setRecording(onOff);

						botPtr=grpPtr->getNextValidBotChild(botPtr);
					}			
					grpPtr=mgrPtr->getNextValidGroupChild(grpPtr);
				}
				mgrPtr=instancePtr->managers().getNextValidChild(mgrPtr);
			}
			instancePtr=CAIS::instance().AIList().getNextValidChild(instancePtr);
		}
	}
	else
	{
		CAIInstance*	instancePtr	=	CAIS::instance().AIList().getNextValidChild();
		while (instancePtr!=NULL)
		{			
			CManager*	mgrPtr	=	instancePtr->managers().getNextValidChild();
			while (mgrPtr!=NULL)
			{
				CGroup*	grpPtr=mgrPtr->getNextValidGroupChild();
				while (grpPtr!=NULL)
				{
					if (onOff)
						grpPtr->getDebugHistory()->setRecording(onOff);
					
					CBot*	botPtr	=	grpPtr->getNextValidBotChild();
					while (botPtr!=NULL)
					{
						for (uint i=0;i<args.size();++i)
							if (NLMISC::nlstricmp(botPtr->getAliasTreeOwner()->getName(),args[i])==0)
							{
								botPtr->getDebugHistory()->setRecording(onOff);
							}
							botPtr=grpPtr->getNextValidBotChild(botPtr);
					}			
					grpPtr=mgrPtr->getNextValidGroupChild(grpPtr);
				}
				mgrPtr=instancePtr->managers().getNextValidChild(mgrPtr);
			}
			instancePtr=CAIS::instance().AIList().getNextValidChild(instancePtr);
		}
	}	
	return true;
}

NLMISC_COMMAND(botHistoryRecordBegin,"setup history recording for all or named entities","[<bot name> [...]]")
{
	return setBotRecordHistory(args,true);
}

NLMISC_COMMAND(botHistoryRecordEnd,"setup history recording for all or named entities","[<bot name> [...]]")
{
	return setBotRecordHistory(args,false);
}

NLMISC_COMMAND(botViewHistory,"view recorded history for named entity","<bot name>")
{
	if (args.size()==0)
		return	false;

	CAIInstance*	instancePtr	=	CAIS::instance().AIList().getNextValidChild();
	while (instancePtr!=NULL)
	{
		CManager*	mgrPtr	=	instancePtr->managers().getNextValidChild();
		while (mgrPtr!=NULL)
		{
			CGroup*	grpPtr=mgrPtr->getNextValidGroupChild();
			while (grpPtr!=NULL)
			{
				CBot*	botPtr	=	grpPtr->getNextValidBotChild();
				while (botPtr!=NULL)
				{
					for (uint i=0;i<args.size();++i)
					{
						if (NLMISC::nlstricmp(botPtr->getAliasTreeOwner()->getName(),args[i])==0)
						{
							botPtr->getDebugHistory()->writeAsInfo();
							break;	//	don't need to write info more than one time.
						}
						
					}				
					botPtr=grpPtr->getNextValidBotChild(botPtr);
				}			
				grpPtr=mgrPtr->getNextValidGroupChild(grpPtr);
			}
			mgrPtr=instancePtr->managers().getNextValidChild(mgrPtr);
		}
		instancePtr=CAIS::instance().AIList().getNextValidChild(instancePtr);
	}	
	return true;
}


NLMISC_COMMAND(grpHistoryRecordLog,"toggle grp history recorder 'nlinfo' logging","")
{
	GrpHistoryRecordLog=!GrpHistoryRecordLog;

	nlinfo("GrpHistoryRecordLog: %s",GrpHistoryRecordLog?"ON":"OFF");
	return true;
}


//-------------------------------------------------------------------------
// DISPLAYING VISION OF FAUNA GROUPS

NLMISC_COMMAND(displayFaunaGrpVision,"display the vision of a fauna group","<grp id>")
{	
	return	false;
}

//-------------------------------------------------------------------------
// MANIPULATING STATE OF FAUNA GROUPS

NLMISC_COMMAND(advanceGrpState,"advance the group to the next state in its cycle","<grp id>")
{
	if(args.size() !=1)
		return false;

	CGrpFauna *grp=dynamic_cast<CGrpFauna *>(CAIS::instance().tryToGetGroup(args[0].c_str()));
	if (!grp)
	{
		log.displayNL("Failed to identify group: %s",args[0].c_str());
		return true;
	}
	if (!grp->isSpawned())
	{
		log.displayNL("Group not spawned: %s",args[0].c_str());
	}
	else
	{
		grp->getSpawnObj()->resetTimer();
	}		
	return true;
}

NLMISC_COMMAND(setGrpTimers,"set the timer values for a given group","<grp id> <eat time> <rest time>")
{
	if(args.size()!=3)
		return false;

	CGrpFauna *grp=dynamic_cast<CGrpFauna *>(CAIS::instance().tryToGetGroup(args[0].c_str()));
	if (!grp)
	{
		log.displayNL("Failed to identify group: %s",args[0].c_str());
		return true;
	}

	uint32 eatTime, restTime;
	NLMISC::fromString(args[1], eatTime);
	NLMISC::fromString(args[2], restTime);

	if (eatTime<1 || restTime<1)
	{
		log.displayNL("Invalid time parameters");
		return true;
	}

	grp->setTimer(CGrpFauna::EAT_TIME, eatTime*10);
	grp->setTimer(CGrpFauna::REST_TIME, restTime*10);
	return true;
}

//-------------------------------------------------------------------------
// RUNNING THE AI UPDATE FOR TESTING OFFLINE

void cbTick();
extern uint ForceTicks;

NLMISC_COMMAND(updateAI,"call CAIS::update() (simulate a tick off-line)","[tick]")
{
	if(args.size() >1)
		return false;

	// if there's an argument make sure its a positive integer
	if (args.size()==1)
	{
		uint tick;
		NLMISC::fromString(args[0], tick);
		if ((tick < 1) || (toString(tick)!=args[0]))
			return false;

		ForceTicks = tick;
		return true;
	}

	cbTick();

	return true;
}



NLMISC_COMMAND(addPetsToPlayer,"Add some pets specified with a sheet to the specified player.","<player id> <nb pets> <pet_sheet>")
{
	if	(	args.size() !=3	)
		return	false;	

#ifdef NL_DEBUG
	nlstopex(("Not Implemented"));
#endif
	return	true;	
}

//-------------------------------------------------------------------------
// DISPLAYING FIGHT SHEET.

NLMISC_COMMAND(displayFightSheet,"display the sheet","<sheet name>")
{	
	if	(args.size() !=1)
	{
		log.displayNL("One argument needed");
		return	false;
	}

	AISHEETS::ICreatureCPtr sheet = AISHEETS::CSheets::getInstance()->lookup(CSheetId(args[0]));
	if (!sheet)
	{
		log.displayNL("Unknown sheet %s", args[0].c_str());
		return true;
	}
	if (sheet->SheetId()==NLMISC::CSheetId::Unknown)
	{
		std::string	creatureString=args[0]+std::string(".creature");
		sheet = /*const_cast<AISHEETS::ICreature*>(*/AISHEETS::CSheets::getInstance()->lookup(CSheetId(creatureString))/*)*/;
	}
	
	if (sheet->SheetId()==NLMISC::CSheetId::Unknown)
	{
		log.displayNL("Failed to identify sheet: %s",args[0].c_str());
		return	false;
	}
	
	log.displayNL("all these values are used to compute a score for a possible new target");
	log.displayNL("beware the fact that in this version we used 'level' to represent relative force, it may be changed, so don't take too much time tuning courage");
	
	log.displayNL("- DistModulator      [0 n]:   %f value=1/(1+distance*DistModulator)",	sheet->DistModulator());
	log.displayNL("value means the distance sensitivity of the bot, equals to zero means the bot never mind the distance");

	log.displayNL("- TargetModulator    [0 n]:   %f value=1/(1+nbTarget*TargetModulator)",	sheet->TargetModulator());
	log.displayNL("value means the number of attacker sensitivity of the bot, equals to zero means the bot never mind the number of attackers on the target");

	log.displayNL("- ScoreModulator     [0 1]:   %f score>ScoreModulator",	sheet->ScoreModulator());
	log.displayNL("value means the minimum value (threshold) needed for the bot to attack, 0 means always, 1 never (also impossible)");
	
	log.displayNL("- FearModulator      [0 1]:   %f score>FearModulator",	sheet->FearModulator());
	log.displayNL("value means the minimum value (threshold) needed for the bot to flee, 0 means always, 1 never (also impossible)");
	
	log.displayNL("- LifeLevelModulator [0 1]:   %f value=LifeLevelModulator*lifeCoef+(1.f-LifeLevelModulator)*levelCoef",	sheet->LifeLevelModulator());
	log.displayNL("value means the ratio between the life and level ratio, 1 means life is only take in count, 0 means level only take in count");

	log.displayNL("- CourageModulator   [-n +n]: %f",	sheet->CourageModulator());
	log.displayNL("value used to make a bot courageous or feared, -n the bot is feared, +n its courageous");

	log.displayNL("- GroupCohesionModulator   [0 1]: %f",	sheet->GroupCohesionModulator());
	log.displayNL("value used to make bots of a group aware of each other, 0 each bot is individual, 1 they fight in group");
	
	log.displayNL("- GroupDispersion   [0 1]: %f",	sheet->GroupDispersion());
	log.displayNL("value used to make bots of a group far or not of each other, 0 each all bots are at the same place, 1 they go everywere in the place");
	
	return true;
}



// all these values are used to compute a score for a possible new target
// beware the fact that in this version we used 'level' to represent relative force, it may be changed, so don't take too much time tuning courage
// 
// - DistModulator      [0 n]:   value=1/(1+distance*DistModulator)
// value means the distance sensitivity of the bot, equals to zero means the bot never mind the distance
// 
// - TargetModulator    [0 n]:   value=1/(1+nbTarget*TargetModulator)
// value means the number of attacker sensitivity of the bot, equals to zero means the bot never mind the number of attackers on the target
// 
// - ScoreModulator     [0 1]:   score>ScoreModulator
// value means the minimum value (threshold) needed for the bot to attack, 0 means always, 1 never (also impossible)
// 
// - FearModulator      [0 1]:   score>FearModulator
// value means the minimum value (threshold) needed for the bot to flee, 0 means always, 1 never (also impossible)
// 
// - LifeLevelModulator [0 1]:   value=LifeLevelModulator*lifeCoef+(1.f-LifeLevelModulator)*levelCoef
// value means the ratio between the life and level ratio, 1 means life is only take in count, 0 means level only take in count
// 
// - CourageModulator   [-n +n]:
// value used to make a bot courageous or feared, -n the bot is feared, +n its courageous
// 
// - GroupCohesionModulator   [0 1]:
// value used to make bots of a group aware of each other, 0 each bot is individual, 1 they fight in group
/*
NLMISC_COMMAND(setFightSheet,"change a value in the fight sheet","<sheet name> <value name> <value> (use name without 'modulator', ex: 'Dist')")
{	
	if	(args.size() !=3)
	{
		log.displayNL("three argument needed");
		return	false;
	}

	AISHEETS::ICreature	*sheet=const_cast<AISHEETS::CCreature*>(AISHEETS::CSheets::getInstance()->lookup(CSheetId(args[0])));
	if (sheet->SheetId==NLMISC::CSheetId::Unknown)
	{
		std::string	creatureString=args[0]+std::string(".creature");
		sheet=const_cast<AISHEETS::ICreature*>(AISHEETS::CSheets::getInstance()->lookup(CSheetId(creatureString)));
	}

	if (sheet->SheetId==NLMISC::CSheetId::Unknown)
	{
		log.displayNL("Failed to identify sheet: %s",args[0].c_str());
		return	false;
	}
	
	float	value = float(atof(args[2].c_str()));
	
	if (NLMISC::nlstricmp(args[1].c_str(), "dist")==0)
	{
		if (value<0)
		{
			log.displayNL("Failed, out of bounds, values accepted are [0 +n]");
			return	false;
		}
		sheet->DistModulator=value;
		log.displayNL("Done, DistModulator=%f", value);
		return	true;
	}
	
	if (NLMISC::nlstricmp(args[1].c_str(), "target")==0)
	{
		if (value<0)
		{
			log.displayNL("Failed, out of bounds, values accepted are [0 +n]");
			return	false;
		}
		sheet->TargetModulator=value;
		log.displayNL("Done, TargetModulator=%f", value);
		return	true;
	}

	if (NLMISC::nlstricmp(args[1].c_str(), "score")==0)
	{
		if (value<0 || value>1)
		{
			log.displayNL("Failed, out of bounds, values accepted are [0 +1]");
			return	false;
		}
		sheet->ScoreModulator=value;
		log.displayNL("Done, ScoreModulator=%f", value);
		return	true;
	}

	if (NLMISC::nlstricmp(args[1].c_str(), "fear")==0)
	{
		if (value<0 || value>1)
		{
			log.displayNL("Failed, out of bounds, values accepted are [0 +1]");
			return	false;
		}
		sheet->FearModulator=value;
		log.displayNL("Done, FearModulator=%f", value);
		return	true;
	}

	if (NLMISC::nlstricmp(args[1].c_str(), "LifeLevel")==0)
	{
		if (value<0 || value>1)
		{
			log.displayNL("Failed, out of bounds, values accepted are [0 +1]");
			return	false;
		}
		sheet->LifeLevelModulator=value;
		log.displayNL("Done, LifeLevelModulator=%f", value);
		return	true;
	}

	if (NLMISC::nlstricmp(args[1].c_str(), "Courage")==0)
	{
		sheet->CourageModulator=value;
		log.displayNL("Done, CourageModulator=%f", value);
		return	true;
	}
	
	if (NLMISC::nlstricmp(args[1].c_str(), "GroupCohesion")==0)
	{
		if (value<0 || value>1)
		{
			log.displayNL("Failed, out of bounds, values accepted are [0 +1]");
			return	false;
		}
		sheet->GroupCohesionModulator=value;
		log.displayNL("Done, GroupCohesionModulator=%f", value);
		return	true;
	}

	if (NLMISC::nlstricmp(args[1].c_str(), "GroupDispersion")==0)
	{
		if (value<0 || value>1)
		{
			log.displayNL("Failed, out of bounds, values accepted are [0 +1]");
			return	false;
		}
		sheet->GroupDispersion=value;
		log.displayNL("Done, GroupDispersion=%f", value);
		return	true;
	}

	log.displayNL("Failed to identify %s", args[1].c_str());
	return false;
}
*/
static void botSetPosition(CBot* bot, std::vector<std::string> const& args, std::string const& botid, float x, float y, uint z, NLMISC::CLog& log)
{
	CSpawnBot* spawnBot = bot->getSpawnObj();
	if (spawnBot!=NULL)
	{
		std::string botname = bot->getAliasTreeOwner()->getName();
		NLMISC::CEntityId eid = spawnBot->getEntityId();
		std::string eidstr = eid.toString();
		if (botid=="*" || (botname!="")&&(botname.find(botid)!=std::string::npos) || eidstr.find(botid)!=std::string::npos)
		{
			CAIPosMirror p = spawnBot->pos();
			if (args.size()>0)
			{
				if (args.size()==3)
					z = p.h();
				spawnBot->setPos(CAIPos(x, y, z, p.theta()));
			}
			p = spawnBot->pos();
			if (botname!="")
				log.displayNL("Bot position for %s %s is %f;%f;%d", eidstr.c_str(), botname.c_str(), p.x().asDouble(), p.y().asDouble(), p.h());
			else
				log.displayNL("Bot position for %s is %f;%f;%d", eidstr.c_str(), p.x().asDouble(), p.y().asDouble(), p.h());
		}
	}
}

static void botSetPosition(CGroup* grp, std::vector<std::string> const& args, std::string const& botid, float x, float y, uint z, NLMISC::CLog& log)
{
	FOREACH (itBot, CCont<CBot>, grp->bots())
	{
		botSetPosition(*itBot, args, botid, x, y, z, log);
	}
}

static void botSetPosition(CManager* mgr, std::vector<std::string> const& args, std::string const& botid, float x, float y, uint z, NLMISC::CLog& log)
{
	FOREACH (itGroup, CCont<CGroup>, mgr->groups())
	{
		botSetPosition(*itGroup, args, botid, x, y, z, log);
	}
}

NLMISC_COMMAND(botSetPosition,"set the position of one or several bots","<eid> [<x> <y> [<z>]]")
{
	if (args.size()!=1 && args.size()!=3 && args.size()!=4)
		return false;

	std::string botid = args[0];
	float x = 0.f;
	float y = 0.f;
	uint z = 0;
	if (args.size()>1)
	{
		x = (float)atof(args[1].c_str());
		y = (float)atof(args[2].c_str());
		if (args.size()==4)
			NLMISC::fromString(args[3], z);
	}
	
	// For each bot
	FOREACH (itInstance, CCont<CAIInstance>, CAIS::instance().AIList())
	{
		FOREACH (itManager, CCont<CManager>, itInstance->managers())
		{
			botSetPosition(*itManager, args, botid, x, y, z, log);
		}
		FOREACH (itCont, CCont<CContinent>, itInstance->continents())
		{
			FOREACH (itRegion, CCont<CRegion>, itCont->regions())
			{
				FOREACH (itCellZone, CCont<CCellZone>, itRegion->cellZones())
				{
					FOREACH (itFamilyBehavior, CCont<CFamilyBehavior>, itCellZone->familyBehaviors())
					{
						botSetPosition(itFamilyBehavior->mgrNpc(), args, botid, x, y, z, log);
						botSetPosition(itFamilyBehavior->mgrFauna(), args, botid, x, y, z, log);
					}
				}
			}
		}
	}
	return true;
}

NLMISC_COMMAND(helpAboutId, "display a short explanation about id used in ai commands", "")
{
	log.displayNL("AI Identifier are a string composed as follow :");
	log.displayNL("  [AIS_<serice_id>:]{[<instance_idx>][:mgr_idx][:grp_idx][:bot_idx] | dyn_<cont_idx>[:<region_idx>][:<cellzone_idx>][:<family_idx>][:fauna|:npc]}");
	log.displayNL("  e.g. : - AIS_138:1:2:3:4 for a 'static' bot");
	log.displayNL("         - AIS_138:dyn_0:2:3:4:npc for a 'dynmaic' npc manager");
	log.displayNL("The initial 'AIS_<serivce_id>:' is optional.");
	log.displayNL("Most of the commands that need an AI id support 'multi selection' by specifying only part of the id.");
	return true;
}

NLMISC_COMMAND(despawnEntity, "despawn an entity.","[entityId]")
{
	if	(args.size()!=1)
		return	false;

	const	std::string	&entityStr=args[0];
	const	CEntityId	entity(entityStr.c_str());

	const	TDataSetRow	dataSetRow=TheDataset.getDataSetRow(entity);

	CAIEntityPhysical	*const	phys=CAIS::instance().getEntityPhysical(dataSetRow);
	
	if (!phys) { return true;}

	switch(phys->getRyzomType())
	{
	case RYZOMID::creature:
	case RYZOMID::npc:
		{
			CSpawnBot	*const	bot=safe_cast<CSpawnBot*>(phys);
			if	(!bot)
				break;

			bot->getPersistent().getGroup().bots().removeChildByIndex(bot->getPersistent().getChildIndex());
		}
		break;
	case RYZOMID::pack_animal:
		{
			CSpawnBotPet	*const	pet=safe_cast<CSpawnBotPet*>(phys);
			if	(!pet)
				break;

			pet->getPersistent().setDespawn();
		}
		break;
	default:
		break;
	}

	return	true;
}

/* Unload a primitive file
 * Unreference all the managers and the regions that are registered for this file.
 */
NLMISC_COMMAND(unloadPrimitiveFile,"unload a primitive file","<file name>")
{
	if	(args.size()!=1)
		return	false;

	// Get the string ID
	const	NLMISC::TStringId	stringId=NLMISC::CStringMapper::map(CFile::getFilenameWithoutExtension(CFile::getFilename(args[0])));

	uint managerUnreferenced = 0;
	uint continentUnreferenced = 0;

	// Parse AI instance
	CAIInstance *aiInstance = CWorkPtr::aiInstance();
	if (aiInstance)
	{
		// Parse managers
		uint i;
		for (i=0; i<aiInstance->managers().size(); i++)
		{
			CSmartPtr<CManager> manager=aiInstance->managers()[i];
			if (manager)
			{
				// Remove this manager
				if (manager->isRegisteredForFile (stringId))
				{
					aiInstance->managers().removeChildByIndex (i);
					managerUnreferenced++;

					// Check the pointer is not referenced anymore
					nlassert (manager->getRefCount() == 1);
				}
			}
		}

		// Parse continents
		for (i=0; i<aiInstance->continents().size(); i++)
		{
			CContinent	*continent=aiInstance->continents()[i];
			if (continent)
			{
				// Parse regions
				uint j;
				for (j=0; j<continent->regions().size(); j++)
				{
					CSmartPtr<CRegion>	region=continent->regions()[j];
					if (region)
					{
						// Remove this region
						if (region->isRegisteredForFile (stringId))
						{
							continent->regions().removeChildByIndex (j);
							continentUnreferenced++;

							// Check the pointer is not referenced anymore
							nlassert (region->getRefCount() == 1);
						}
					}
				}
			}
		}
		aiInstance=CAIS::instance().AIList().getNextValidChild(aiInstance);
		uint32 primAlias = LigoConfig.getFileStaticAliasMapping(CFile::getFilename(args[0]));
		nldebug("<unloadPrimitiveFile> Sending alias '%u' to AIS for custom data range deletion", primAlias);
		CAIUserModelManager::getInstance()->deleteCustomDataByPrimAlias(primAlias);
		log.displayNL("unloadPrimitiveFile %s : %d manager removed, %d region removed", args[0].c_str(), managerUnreferenced, continentUnreferenced);
	}
	else
		log.displayNL("unloadPrimitiveFile failed : no active ai instance");

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// Bug simulation                                                           //
//////////////////////////////////////////////////////////////////////////////

static bool bugSimulationInited = false;
static bool bugSimulationTextInited = false;
static int const bugSimulationCount = 10;
static bool simulateBugs[bugSimulationCount];
static char const* simulateBugTexts[bugSimulationCount];

inline
static void initBugSimulations()
{
	if (!bugSimulationInited)
	{
		bugSimulationInited = true;
		for (int i=0; i<bugSimulationCount; ++i)
		{
			simulateBugs[i] = false;
		}
	}
}

inline
static void initBugSimulationTexts()
{
	if (!bugSimulationTextInited)
	{
		bugSimulationTextInited = true;
		for (int i=0; i<bugSimulationCount; ++i)
			simulateBugTexts[i] = "[No bug with this id]                    ";
		
		simulateBugTexts[0] = "Bots didn't use range weapons            ";
		simulateBugTexts[1] = "Assign. a string var in another group    ";
		simulateBugTexts[2] = "Destination reach detection was delayed  ";
		simulateBugTexts[3] = "Npc grp behav. wasn't updtd often enough ";
		simulateBugTexts[4] = "User event only worked in AI instance 0  ";
		simulateBugTexts[5] = "Mektoubs following in Water              ";
		simulateBugTexts[6] = "When looted, fauna respawned immediately ";
		simulateBugTexts[7] = "Added a member init. in normal profile   ";
		simulateBugTexts[8] = "Repulsion vector calc. was misordered    ";
	}
}

inline
static char const* simulateBugText(int bugId)
{
	initBugSimulations();
	if (bugId>=0 && bugId<bugSimulationCount)
		return simulateBugTexts[bugId];
	else
		return NULL;
}

/// Past this declaration where needed
extern bool simulateBug(int bugId);

bool simulateBug(int bugId)
{
	initBugSimulations();
	if (bugId>=0 && bugId<bugSimulationCount)
		return simulateBugs[bugId];
	else
		return false;
}

NLMISC_COMMAND(simulateBug, "simulate an old AIS bug; command is one of 'list', 'enable', 'disable'; enable and disable take as command_arg the id of the bug to simulate","<command> [<command_arg>]")
{

	if (args.size()>=1)
	{
		if (args[0] == "list")
		{
			if (args.size()==1)
			{
				initBugSimulations();
				initBugSimulationTexts();
				log.displayNL("Bug simulations");
				log.displayNL("Id|Description                              |State");
				for (int i=0; i<bugSimulationCount; ++i)
				{
					log.displayNL("%02d|%s|%s", i, simulateBugText(i), simulateBugs[i]?"on":"off");
				}
				return true;
			}
		}
		else if (args[0] == "enable")
		{
			if (args.size()==2)
			{
				initBugSimulations();
				if (args[1]=="all")
				{
					for (int i=0; i<bugSimulationCount; ++i)
						simulateBugs[i] = true;
				}
				else
				{
					sint i;
					NLMISC::fromString(args[1], i);
					if (i>=0 && i<bugSimulationCount)
						simulateBugs[i] = true;
					else
						log.displayNL("No bug simulation with id %02d", i);
				}
				return true;
			}
		}
		else if (args[0] == "disable")
		{
			if (args.size()==2)
			{
				initBugSimulations();
				if (args[1]=="all")
				{
					for (sint i=0; i<bugSimulationCount; ++i)
						simulateBugs[i] = false;
				}
				else
				{
					sint i;
					NLMISC::fromString(args[1], i);
					if (i>=0 && i<bugSimulationCount)
						simulateBugs[i] = false;
					else
						log.displayNL("No bug simulation with id %02d", i);
				}
				return true;
			}
		}
	}
	return false;
}


static void displayTime(const CRyzomTime &rt, NLMISC::CLog &log)
{
	std::string result;
	result = NLMISC::toString("hh:mm = %d:%d; ", (int) floorf(rt.getRyzomTime()) , (int) floorf(60.f * fmodf(rt.getRyzomTime(), 1.f)));
	log.displayNL(result.c_str());
	uint32 month = rt.getRyzomMonth();
	MONTH::EMonth monthInCycle = rt.getRyzomMonthInCurrentCycle();
	std::string monthName = MONTH::toString((MONTH::EMonth) monthInCycle);
	uint32 dayOfMonth = rt.getRyzomDayOfMonth();			
	std::string dayName = WEEKDAY::toString((WEEKDAY::EWeekDay) rt.getRyzomDayOfWeek());
	result = NLMISC::toString("mm:dd:yy = %d:%d:%d  (%s:%s)",
		                       (int)  (month + 1),
							   (int) (dayOfMonth + 1),
							   (int) rt.getRyzomYear(),
							   monthName.c_str(),
							   dayName.c_str());
	log.displayNL(result.c_str());
	log.displayNL("day of year = %d/%d", (int) (rt.getRyzomDayOfYear() + 1), (int) RYZOM_YEAR_IN_DAY);
	log.displayNL("season = %d/4 (%s)", (int) rt.getRyzomSeason() + 1, EGSPD::CSeason::toString(rt.getRyzomSeason()).c_str());
}

NLMISC_COMMAND(time, "Display current time.", "<>")
{
	if (!args.empty()) return false;
	const CRyzomTime &rt = CTimeInterface::getRyzomTime();	
	displayTime(rt, log);
	return true;
}

NLMISC_COMMAND(localTime, "Display current local time (debug time).", "<>")
{
	if (!args.empty()) return false;
	const CRyzomTime &rt = CTimeInterface::getRyzomDebugTime();	
	displayTime(rt, log);
	return true;
}


struct CRyzomDate
{
	float Time;	
	uint Day;	
	uint Year;
};

static void getRyzomDebugDate(CRyzomDate &rd)
{
	const CRyzomTime &rt = CTimeInterface::getRyzomDebugTime();
	rd.Time = rt.getRyzomTime();
	rd.Day = rt.getRyzomDayOfYear();
	rd.Year = rt.getRyzomYear();
}

static void setRyzomDebugDate(CRyzomDate &rd)
{
	CRyzomTime &rt = CTimeInterface::getRyzomDebugTime();
	rt.updateRyzomClock((uint32) (rd.Time * RYZOM_HOURS_IN_TICKS) + (rd.Day + rd.Year * RYZOM_YEAR_IN_DAY) * RYZOM_DAY_IN_TICKS);
}

NLMISC_COMMAND(setDebugHour, "set the current debug hour", "<hour>")
{
	if (args.size() != 1) return false;	
	int hour;
	if (sscanf(args[0].c_str(), "%d", &hour) != 1) return false;
	CRyzomDate rd;
	getRyzomDebugDate(rd);
	rd.Time = fmodf(rd.Time, 1.f) + (float) hour;
	setRyzomDebugDate(rd);
	return true;
}

NLMISC_COMMAND(setDebugDayOfYear, "set the current debug day of year (first day has index 1)", "<day>")
{
	if (args.size() != 1) return false;	
	int day;
	if (sscanf(args[0].c_str(), "%d", &day) != 1) return false;
	CRyzomDate rd;	
	getRyzomDebugDate(rd);
	rd.Day = day - 1; // for the user, days start at '1'
	setRyzomDebugDate(rd);
	return true;
}

//
// setPersistentVar setname:varName value
//
NLMISC_COMMAND(setPersistentVar, "sets an AI script persistent var", "<command> [<command_arg>]")
{	
	if (args.size() == 2)
	{
		std::string varName = args[0].c_str();
		std::string value = args[1].c_str();

		CAIScriptDataManager::getInstance()->setVar(varName, value);

		log.displayNL("Var '%s' set to '%s'", varName.c_str(), value.c_str());
		return true;
	}
	
	return false;
}

//
// listPersistentVar
//
NLMISC_COMMAND(listPersistentVar, "display all persistent vars", "")
{
	if (!args.empty()) return false;
	
	const CPersistentVariables &persistentVar = CAIScriptDataManager::getInstance()->getPersistentVariables();

	const TVariableSets &variableSet = persistentVar.getVariableSet();

	for (TVariableSets::const_iterator i = variableSet.begin(); i != variableSet.end(); ++i)
	{
		log.displayNL("#Set: '%s'#", i->first.c_str());
		for (TVariables::const_iterator j = i->second.Variables.begin(); j != i->second.Variables.end(); ++j)
		{
			log.displayNL("Name: '%s' -- Value: '%s'", j->first.c_str(), j->second.c_str());
		}
	}

	return true;
}


//
// getPersistentVar setname:varname
//
NLMISC_COMMAND(getPersistentVarAsString, "get a persistent ai var", "")
{
	if (args.size() == 1)
	{
		std::string varName = args[0];

		std::string value = CAIScriptDataManager::getInstance()->getVar_s(varName);
		log.displayNL("Variable '%s' has value '%s'", varName.c_str(), value.c_str() );
		return true;
	}
	
	return false;
}

//
// getPersistentVar setname:varname
//
NLMISC_COMMAND(getPersistentVarAsFloat, "get a persistent ai var", "")
{
	if (args.size() == 1)
	{
		std::string varName = args[0];

		float value = CAIScriptDataManager::getInstance()->getVar_f(varName);
		nlinfo("Variable '%s' has value '%f'", varName.c_str(), value );
		return true;
	}
	
	return false;
}

NLMISC_COMMAND(deletePersistentVar, "deletes an AI script persistent var", "")
{	
	if (args.size() == 1)
	{
		std::string varName = args[0].c_str();

		CAIScriptDataManager::getInstance()->deleteVar(varName);

		log.displayNL("Var '%s' deleted", varName.c_str());
		return true;
	}
	
	return false;
}


//----------------------------------------------------------------------------
