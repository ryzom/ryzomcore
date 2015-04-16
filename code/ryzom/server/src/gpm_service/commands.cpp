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

// Nel Misc
#include "nel/misc/command.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"

#include "gpm_service.h"
#include "world_position_manager.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


/*
 * Commands for GPMS initialisation
 */

// Enable/disable player speed checking
NLMISC_COMMAND(setPlayerSpeedCheck, "set player speed check (0=disable)", "entityId <0/1>")
{
	if (args.size() < 2)
		return false;

	CEntityId	id;
	id.fromString(args[0].c_str());
	bool		enable;
	NLMISC::fromString(args[1], enable);

	CWorldEntity	*entity = CWorldPositionManager::getEntityPtr( CWorldPositionManager::getEntityIndex(id) );
	if (entity == NULL || entity->PlayerInfos == NULL)
	{
		log.displayNL("CGPMPlayerPrivilegeInst::callback(): entity '%d' is not a player", id.toString().c_str());
		return false;
	}

	// enable/disable speed checking
	entity->PlayerInfos->CheckSpeed = enable;

	return true;
}


// Command to load a continent
NLMISC_COMMAND(loadContinent, "load a continent in the gpms","index name filename")
{
	if (args.size() < 3)
		return false;

	uint8	continent;
	NLMISC::fromString(args[0], continent);
	string	name = args[1];
	string	file = args[2];

	CWorldPositionManager::loadContinent(name, file, continent);

	return true;
}

// Command to remove a continent
NLMISC_COMMAND(removeContinent, "remove a continent from the gpms","index")
{
	if (args.size() < 1)
		return false;

	uint8	continent;
	NLMISC::fromString(args[0], continent);

	CWorldPositionManager::removeContinent(continent);

	return true;
}

// Command to init trigger manager
NLMISC_COMMAND(initPatatManager, "init/reset patat&pacs trigger manager", "")
{
	CWorldPositionManager::initPatatManager();
	return true;
}

// Command to load patat
NLMISC_COMMAND(loadPatatFile, "load a patat file in patat manager (.prim file)", "filename")
{
	if (args.size() != 1)
		return false;

	CWorldPositionManager::loadPatatsInFile(args[0]);
	return true;
}

// Command to load patat
NLMISC_COMMAND(loadPatatPath, "load a patat path in patat manager (all .prim files in path)", "pathname")
{
	if (args.size() != 1)
		return false;

	CWorldPositionManager::loadPatatsInPath(args[0]);
	return true;
}

// Command to load patat
NLMISC_COMMAND(loadPatatManagerFile, "load a patat manager file", "filename")
{
	if (args.size() != 1)
		return false;

	CWorldPositionManager::loadPatatManagerFile(args[0]);
	return true;
}

// Command to load patat
NLMISC_COMMAND(savePatatManagerFile, "save a patat manager file", "filename")
{
	if (args.size() != 1)
		return false;

	CWorldPositionManager::savePatatManagerFile(args[0]);
	return true;
}

// Add CPrimZone class filter
NLMISC_COMMAND(addPrimZoneFilter, "add one or more positive filters on CPrimZone for PatatSubscribeManager", "<className> ...")
{
	if (args.size() < 1)
		return false;

	uint	i;
	for (i=0; i<args.size(); ++i)
		CWorldPositionManager::addPrimZoneFilter(args[i]);

	return true;
}

// Remove CPrimZone class filter
NLMISC_COMMAND(removePrimZoneFilter, "remove one or more positive filters on CPrimZone for PatatSubscribeManager", "<className> ...")
{
	if (args.size() < 1)
		return false;

	uint	i;
	for (i=0; i<args.size(); ++i)
		CWorldPositionManager::removePrimZoneFilter(args[i]);

	return true;
}

// Reset CPrimZone class filter
NLMISC_COMMAND(resetPrimZoneFilter, "reset all positive filters on CPrimZone for PatatSubscribeManager", "")
{
	CWorldPositionManager::resetPrimZoneFilter();

	return true;
}

//
NLMISC_COMMAND(getPatatEntryIndex, "Get the patat entry index at a pos", "x, y")
{
	if (args.size() != 2)
		return false;

	CVector		pos;

	NLMISC::fromString(args[0], pos.x);
	NLMISC::fromString(args[1], pos.y);
	pos.z = 0;

	nlinfo("entryIndex(%.1f, %.1f) = %d", pos.x, pos.y, CWorldPositionManager::getEntryIndex(pos));

	return true;
}



/*
 * Commands for entity management
 */


// Command to add an entity to the GPMS
/*
NLMISC_COMMAND(addEntity,"Add entity to GPMS","entity Id, entity PosX(meters), entity PosY, entity PosZ, service Id")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 5) return false;
	
	// get the values
	CEntityId id;
	id.fromString(args[0].c_str());
	sint32 PosX, PosY, PosZ;
	NLMISC::fromString(args[1], PosX);
	NLMISC::fromString(args[2], PosY);
	NLMISC::fromString(args[3], PosZ);

	uint16 FeId;
	NLMISC::fromString(args[4], FeId);

	// display the result on the displayer
	log.displayNL("Add entity Id %s to GPMS", id.toString().c_str() );

	TheMirror.addEntity( id );
	CWorldPositionManager::onAddEntity( TheDataset.getDataSetRow(id) ); // because a local change is not notified
	//CWorldPositionManager::onAddEntity(id, 1000 * PosX, 1000*PosY, 1000*PosZ, 0.0f, INVALID_CONTINENT_INDEX, 0, CTickEventHandler::getGameCycle() - 1, CSheetId(0),FeId);
	CWorldPositionManager::teleport( id, PosX, PosY, PosZ, 0.0f, INVALID_CONTINENT_INDEX, 0, CTickEventHandler::getGameCycle() );

	return true;
}
*/

// Command to add x non player characters in the GPMS
/*NLMISC_COMMAND(addNpcs,"Add npcs in the GPMS","number of entities to add")
{
	srand( (uint) CTickEventHandler::getGameCycle() );

	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;
	
	// get the values
	uint32 num;
	NLMISC::fromString(args[0], num);

	// Init Entity
	CEntityId id;
	id.setType( RYZOMID::npc );

	for (uint i = 0 ; i < num ; ++i)
	{
		sint32 x = 1000 * (rand() % 1500 + 17000);
		sint32 y = 1000 * (rand() % 2000 - 31000);

		id.setShortId( i+9000 );
		CWorldPositionManager::addEntity(id, x, y, 0, 0.0f, INVALID_CONTINENT_INDEX, 0, CTickEventHandler::getGameCycle(),CSheetId(0),0);
	}

	log.displayNL("Add %d entities in the GPMS", num);

	return true;
}*/



// Command to add x player characters in the GPMS
/*NLMISC_COMMAND(addPlayers,"Add players in the GPMS","number of entities to add")
{
	srand( (uint) CTickEventHandler::getGameCycle() );

	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;
	
	// get the values
	uint32 num;
	NLMISC::fromString(args[0], num);


	// Init Entity
	CEntityId id;
	id.setType( RYZOMID::player );

	for (uint i = 0 ; i < num ; ++i)
	{
		sint32 x = 1000 * (rand() % 1500 + 17000);
		sint32 y = 1000 * (rand() % 2000 - 31000);

		id.setShortId( i+1000 );
		CWorldPositionManager::addEntity(id, x, y, 0 , 0.0f, INVALID_CONTINENT_INDEX, 0, CTickEventHandler::getGameCycle() - 1,CSheetId(0),0);
	}

	log.displayNL("Add %d entities in the GPMS", num);

	return true;
}*/


// Command to remove an entity from the GPMS
NLMISC_COMMAND(removeEntity,"Remove an entity from the GPMS","entity Id")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;

	// get id
	CEntityId id;
	id.fromString(args[0].c_str());

	// display the result on the displayer
	log.displayNL("Remove entity Id %s from GPMS", id.toString().c_str());

	TDataSetRow	index = CWorldPositionManager::getEntityIndex(id);
	TheMirror.removeEntity( id );
	CWorldPositionManager::onRemoveEntity( index ); // because a local change is not notified

	return true;
}

//
NLMISC_COMMAND(removeAllEntities,"remove AiVision entities for the specified service (or all services if no param)","service id")
{
	if(args.size() >= 1) 
	{
		uint16 serviceId;
		NLMISC::fromString(args[0], serviceId);
		// get the values
		NLNET::TServiceId ServiceId(serviceId);

		//CWorldPositionManager::removeAiVisionEntitiesForService( ServiceId );
	}
	else
	{
		CWorldPositionManager::removeAllEntities();
	}
	
	return true;
}



/*
 * Commands for position management
 */

// Command to move an entity in the GPMS
NLMISC_COMMAND(moveEntity,"move an entity in the GPMS","entity Id, newPos X (meters), newPos Y, newPos Z")
{
/*
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 4) return false;
	
	// get the values
	uint32 Id;
	NLMISC::fromString(args[0], Id);
	uint32 PosX;
	NLMISC::fromString(args[1], PosX);
	uint32 PosY;
	NLMISC::fromString(args[2], PosY);
	sint32 PosZ;
	NLMISC::fromString(args[3], PosZ);
	
	// Init Entity
	CEntityId id;
	id.fromString(args[0].c_str());
	
	// display the result on the displayer
	log.displayNL("move entity Id %d from GPMS", Id);

	CWorldPositionManager::setEntityPosition(id,1000*PosX, 1000*PosY, 1000*PosZ, 0.0f, CTickEventHandler::getGameCycle() );
*/
	return true;
}

// Command to move an entity in the GPMS
NLMISC_COMMAND(teleportEntity,"teleport an entity", "entity Id, newPos X (meters), newPos Y, newPos Z, cell")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 5) return false;
	
	// Init Entity
	CEntityId	id;
	id.fromString(args[0].c_str());

	// get the values
	uint32		PosX;
	NLMISC::fromString(args[1], PosX);
	uint32		PosY;
	NLMISC::fromString(args[2], PosY);
	sint32		PosZ;
	NLMISC::fromString(args[3], PosZ);
	sint32		Cell;
	NLMISC::fromString(args[4], Cell);
	
	// display the result on the displayer
	log.displayNL("teleport entity %s", id.toString().c_str());

	TDataSetRow	index = CWorldPositionManager::getEntityIndex(id);
	CWorldPositionManager::teleport(index, 1000*PosX, 1000*PosY, 1000*PosZ, 0.0f, INVALID_CONTINENT_INDEX, Cell, CTickEventHandler::getGameCycle());

	return true;
}

// Command to move an entity in the GPMS
NLMISC_COMMAND(setEntityContent,"set an entity content", "CEntityId entityId, [CEntityId containeeId, CSheetId(int) containeeSheet]+")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() < 1)
		return false;

	if ((args.size() & 1) == 0)
		return false;
	

	// Init Entity
	CEntityId	id;
	id.fromString(args[0].c_str());

	// get the values
	vector<CEntitySheetId>	content;

	uint	arg = 1;
	while (arg < args.size())
	{
		CEntityId	id;
		id.fromString(args[arg].c_str());
		uint32 sheetId;
		NLMISC::fromString(args[arg+1], sheetId);
		CSheetId	sheet(sheetId);
		content.push_back(CEntitySheetId(id, sheet));
	}

	CWorldPositionManager::setContent(CWorldPositionManager::getEntityIndex(id), content);

	return true;
}





// Command to display all vision or the vision for the specified entity
NLMISC_COMMAND(mirrorAroundEntity,"Ask a local mirror arround an entity","service Id, entity Id")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 2) 
	{
		return false;
	}

	// get the values
	uint16 serviceId;
	NLMISC::fromString(args[0], serviceId);
	NLNET::TServiceId ServiceId(serviceId);
	uint32 Id;
	NLMISC::fromString(args[1], Id);

	list< string > properties;
	properties.push_back( "X" );
	properties.push_back( "Y" );
	properties.push_back( "Z" );
	properties.push_back( "Theta" );
	properties.push_back( "Sheet" );

	// Init Entity
	CEntityId id;
	id.fromString(args[0].c_str());
	
	CWorldPositionManager::requestForEntityAround( ServiceId, id, properties );
	
	return true;
}




// Command to display a single entity or more, with full debug info
NLMISC_COMMAND(displayEntity,"display single or more entities in the gpms","[CEntityId]*")
{
	if (args.size() == 0)
	{
		CWorldPositionManager::displayAllEntitiesFullDebug(&log);
	}
	else
	{
		uint	i;
		for (i=0; i<args.size(); ++i)
		{
			CEntityId	id;
			id.fromString(args[i].c_str());
			CWorldPositionManager::displayEntityFullDebug(CWorldPositionManager::getEntityIndex(id), &log);
		}
	}

	return true;
}

// Command to display all entities
NLMISC_COMMAND(displayEntities,"display entities in the gpms","[CEntityId]*")
{
	if (args.size() == 0)
	{
		CWorldPositionManager::displayAllEntities(&log);
	}
	else
	{
		uint	i;
		for (i=0; i<args.size(); ++i)
		{
			CEntityId	id;
			id.fromString(args[i].c_str());
			CWorldPositionManager::displayEntity(CWorldPositionManager::getEntityIndex(id), &log);
		}
	}

	return true;
}

// Command to display all entities
NLMISC_COMMAND(displayPlayers,"display all players in the gpms","")
{
	CWorldPositionManager::displayAllPlayers(&log);

	return true;
}

// Command to display all entities
NLMISC_COMMAND(displayPlayersPosHistory, "display all players positions history","")
{
	CWorldPositionManager::displayPlayersPosHistory(&log);

	return true;
}

// Command to display entities vision (no parameter = all players)
NLMISC_COMMAND(displayVision, "display entities vision", "[CEntityId]*")
{
	if (args.size() == 0)
	{
		CWorldPositionManager::displayAllVisions(&log);
	}
	else
	{
		uint	i;
		for (i=0; i<args.size(); ++i)
		{
			CEntityId	id;
			id.fromString(args[i].c_str());
			CWorldPositionManager::displayVision(id, &log);
		}
	}

	return true;
}


// Command to display pacs triggers
NLMISC_COMMAND(displayPacsTriggers, "display pacs triggers", "")
{
	CWorldPositionManager::displayPacsTriggers(&log);
	return true;
}

// Command to display entities vision (no parameter = all players)
NLMISC_COMMAND(displayTriggers, "display triggers", "")
{
	CWorldPositionManager::displayTriggers(&log);
	return true;
}

// Command to display entities vision (no parameter = all players)
NLMISC_COMMAND(displayTriggerInfo, "display trigger info", "triggerName(string)")
{
	if (args.size() != 1)
		return false;
	CWorldPositionManager::displayTriggerInfo(args[0], &log);
	return true;
}

// Command to display entities vision (no parameter = all players)
NLMISC_COMMAND(displayTriggerSubscribers, "display triggerSubscribers", "")
{
	CWorldPositionManager::displaySubscribers(&log);
	return true;
}

// Command to display entities vision (no parameter = all players)
NLMISC_COMMAND(displayTriggerSubscriberInfo, "display subscriber info", "triggerName(string)")
{
	if (args.size() != 1)
		return false;
	uint16 serviceId;
	NLMISC::fromString(args[0], serviceId);

	CWorldPositionManager::displaySubscriberInfo(TServiceId(serviceId), &log);
	return true;
}


//
NLMISC_COMMAND(trackEntity, "get track of an entity position", "id")
{
	if (args.size() < 1)
		return false;

	CEntityId	eid;

	uint64		id;
	uint		type;
	uint		creatorId;
	uint		dynamicId;

	if (sscanf(args[0].c_str(), "(%"NL_I64"x:%x:%x:%x)", &id, &type, &creatorId, &dynamicId) != 4)
		return false;

	eid.setShortId( id );
	eid.setType( type );
	eid.setCreatorId( creatorId );
	eid.setDynamicId( dynamicId );

	pCGPMS->Tracked.push_back(eid);

	pCGPMS->EntityTrack1 = pCGPMS->EntityTrack0;
	pCGPMS->EntityTrack0 = eid;

	return true;
}

NLMISC_COMMAND(removeTracks, "remove all entities tracks", "")
{
	pCGPMS->Tracked.clear();

	return true;
}

NLMISC_COMMAND(autoCheck, "perform autocheck", "")
{
	CWorldPositionManager::autoCheck(&log);

	return true;
}

NLMISC_COMMAND(displayVisionCells, "display VisionCells info", "")
{
	CWorldPositionManager::displayVisionCells(&log);

	return true;
}
//



NLMISC_COMMAND(test_vision, "", "")
{
	CVector		centerPos(4700, -3500, 0);

	sint	i, j, k=0;

	for (i=-7; i<=+7; ++i)
	{
		for (j=-7; j<=+7; ++j)
		{
			CEntityId id;
			id.setType( RYZOMID::npc );

			sint32	x = (sint32)((centerPos.x+i*16)*1000);
			sint32	y = (sint32)((centerPos.y+j*16)*1000);

			id.setShortId( k+9000 );
			++k;

			TheMirror.addEntity( false, id );
			TDataSetRow	index = TheDataset.getDataSetRow(id);
			CWorldPositionManager::onAddEntity( index ); // because a local change is not notified
			//CWorldPositionManager::onAddEntity(id, 1000 * PosX, 1000*PosY, 1000*PosZ, 0.0f, INVALID_CONTINENT_INDEX, 0, CTickEventHandler::getGameCycle() - 1/*CTickEventHandler::getGameCycles()*/,/*sheet*/CSheetId(0),FeId);
			CWorldPositionManager::teleport( index, x, y, 0, 0.0f, INVALID_CONTINENT_INDEX, 0, CTickEventHandler::getGameCycle() );
		}
	}

	CEntityId id;
	id.setType( RYZOMID::player );
	sint32	x = (sint32)((centerPos.x)*1000);
	sint32	y = (sint32)((centerPos.y)*1000);
	id.setShortId( k+9000 );
	++k;
	TheMirror.addEntity( false, id );
	TDataSetRow	index = TheDataset.getDataSetRow(id);
	CWorldPositionManager::onAddEntity( index ); // because a local change is not notified
	//CWorldPositionManager::onAddEntity(id, 1000 * PosX, 1000*PosY, 1000*PosZ, 0.0f, INVALID_CONTINENT_INDEX, 0, CTickEventHandler::getGameCycle() - 1/*CTickEventHandler::getGameCycles()*/,/*sheet*/CSheetId(0),FeId);
	CWorldPositionManager::teleport( index, x, y, 0, 0.0f, INVALID_CONTINENT_INDEX, 0, CTickEventHandler::getGameCycle() );

	return true;
}


NLMISC_COMMAND(dumpRingVisionUniverse, "dump state information from the ring vision universe", "")
{
	if (!args.empty())
		return false;

	pCGPMS->RingVisionUniverse->dump(log);
	return true;
}

NLMISC_COMMAND(resetRingVision, "resets a character's vision in ring mode", "<datasetrow>")
{
	if (args.size()!=1)
		return false;

	uint32 idx=NLMISC::CSString(args[0]).atoui();
	TDataSetRow row(*(TDataSetRow*)&idx);

	pCGPMS->RingVisionUniverse->forceResetVision(row);
	return true;
}
