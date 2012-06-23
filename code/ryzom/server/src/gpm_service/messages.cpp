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

// include files
#include "nel/misc/types_nl.h"

#include "game_share/utils.h"

#include "server_share/r2_variables.h"
#include "game_share/r2_messages.h"

#include "gpm_service.h"
#include "messages.h"
#include "world_position_manager.h"

//#include "game_share/tick_event_handler.h"
//#include "game_share/msg_client_server.h"
//#include "game_share/mode_and_behaviour.h" //TEMP!!!
//#include "game_share/news_types.h"
//#include "game_share/bot_chat_types.h"
//#include "game_share/brick_types.h"
//#include "game_share/loot_harvest_state.h"
//#include "game_share/ryzom_version.h"
//#include "game_share/synchronised_message.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;




/****************************************************************\
						cbAddEntity() 
\****************************************************************/
/*
void cbAddEntity( CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	// Only for invisible group entities (other entities use the mirror system)

	CEntityId id;
	msgin.serial( id );

	// entity pos (x, y, z, theta) at tick
	sint32 x;
	msgin.serial( x );
	sint32 y;
	msgin.serial( y );
	sint32 z;
	msgin.serial( z );
	float t;
	msgin.serial( t );

	// the tick when this entity should be added
//--------------------------------- DEBUG ONLY--------------------------------
	NLMISC::TGameCycle tick;
	//msgin.serial( tick );
	tick = CTickEventHandler::getGameCycle();
//--------------------------------- DEBUG ONLY--------------------------------

	// entity sheet
	CSheetId	sheet;
	msgin.serial( sheet );

	uint8	continent = INVALID_CONTINENT_INDEX;
	if (msgin.getPos() != (sint32)msgin.length())
		msgin.serial(continent);

	sint32	cell = 0;
	if (msgin.getPos() != (sint32)msgin.length())
		msgin.serial(cell);

	nlinfo("<cbAddEntity> Add entity %s at pos (%d,%d,%d)", id.toString().c_str(), x,y,z );
	CWorldPositionManager::addEntity( id, x, y, z, t, continent, cell, tick, sheet, id.getDynamicId() );
} // cbAddEntity //
*/

/****************************************************************\
						cbAddIAObject() 
\****************************************************************/
/*
void cbAddIAObject( CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	nlerror( "TODO: process ADD_IA_OBJECT" );

}
*/

/****************************************************************\
						cbAddEntities() 
\****************************************************************/
/*void cbAddEntities( CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	//nlinfo("received ADD_ENTITIES request");
	 
	uint32 entitiesCount;
	msgin.serial( entitiesCount );

	for( uint32 i = 0; i < entitiesCount; i++ )
	{
		CEntityId id;
		msgin.serial( id );

		// entity pos (x, y, z, theta) at tick
		sint32 x;
		msgin.serial( x );
		sint32 y;
		msgin.serial( y );
		sint32 z;
		msgin.serial( z );
		float t;
		msgin.serial( t );

		// the tick when this entity should be added
//--------------------------------- DEBUG ONLY--------------------------------
		NLMISC::TGameCycle tick;
		//msgin.serial( tick );
		tick = CTickEventHandler::getGameCycle();
//--------------------------------- DEBUG ONLY--------------------------------

		// entity sheet
		CSheetId	sheet;
		msgin.serial( sheet );

		nlinfo("<cbAddEntities> Add entity %s at pos (%d,%d,%d)", id.toString().c_str(), x,y,z );
		CWorldPositionManager::addEntity(id, x, y, z, t, INVALID_CONTINENT_INDEX, 0, tick, sheet, id.getDynamicId() );
	}
} // cbAddEntities //
/*

  /****************************************************************\
					cbRemoveEntity()
\****************************************************************/
/*
void cbRemoveEntity( CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	// Only for invisible group entities (other entities use the mirror system)

	CEntityId id;
	id.serial( msgin );
	
	nlinfo("received REMOVE_ENTITY request for id %s", id.toString().c_str() );

	CWorldPositionManager::onRemoveEntity( id );

} // cbRemoveEntity //
*/

/****************************************************************\
					cbRemoveEntities()
\****************************************************************/
/*void cbRemoveEntities( CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	nlinfo("received REMOVE_ENTITIES request");

	list<CEntityId> ids;
	msgin.serialCont( ids );
	
	list<CEntityId> idsAgents;

	list<CEntityId>::iterator it;
	for( it = ids.begin(); it!=ids.end(); ++it )
	{
		CWorldPositionManager::removeEntity( *it );
		nlinfo("	REMOVE_ENTITY id %s", (*it).toString().c_str() );

		if( (*it).getType() == RYZOMID::npc )
		{			
			idsAgents.push_back( *it );
		}
	}

	if( idsAgents.size() != 0 )
	{
		CMessage msgout("REMOVE_ENTITY");
		
		for ( it = idsAgents.begin() ; it != idsAgents.end() ; ++it)
			msgout.serial( const_cast<CEntityId&> (*it) );

		sendMessageViaMirror( "AgS", msgout );
	}

}*/ // cbRemoveEntities //


/****************************************************************\
					CGPMPlayerPrivilegeInst::callback 
\****************************************************************/
void	CGPMPlayerPrivilegeInst::callback (const std::string &name, NLNET::TServiceId id)
{
	CWorldEntity*	entity = CWorldPositionManager::getEntityPtr(PlayerIndex);

	if (entity == NULL || entity->PlayerInfos == NULL)
	{
		nlwarning("CGPMPlayerPrivilegeInst::callback(): entity '%d' is not a player", PlayerIndex.getIndex());
		return;
	}

	// enable/disable speed checking, only enabled for basic players
	entity->PlayerInfos->CheckSpeed = (Type == CGPMPlayerPrivilege::Player);
}

/****************************************************************\
					cbSetPlayerFlags() 
\****************************************************************/
void cbSetPlayerFlags( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	R2::CMessageSetGPMSPlayerFlags msgData;
	msgin.serial(msgData);

	CWorldEntity*	entity = CWorldPositionManager::getEntityPtr(msgData.PlayerIndex);

	DROP_IF(entity == NULL || entity->PlayerInfos == NULL, NLMISC::toString("cbSetPlayerFlags(): entity '%d' is not a player", msgData.PlayerIndex.getIndex()), return);

	// enable/disable speed checking, only enabled for basic players
	entity->PlayerInfos->CheckSpeed = msgData.LimitSpeed;
}

/****************************************************************\
					cbBag()
\****************************************************************/
void cbBag( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId							id;
	vector<CEntitySheetId>				content;

	msgin.serial(id);
	msgin.serialCont(content);

	TDataSetRow							index = CWorldPositionManager::getEntityIndex(id);

	CWorldPositionManager::setContent(index, content);

} // cbBag //


/****************************************************************\
					cbLoadContinent() 
\****************************************************************/
void	cbLoadContinent( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	uint8	continent;
	string	name;
	string	file;

	// load continent index
	msgin.serial(continent);

	// load continent name
	msgin.serial(name);

	// load continent filename
	msgin.serial(file);

	// load continent in position manager
	CWorldPositionManager::loadContinent(name, file, continent);
}

/****************************************************************\
					cbRemoveContinent() 
\****************************************************************/
void	cbRemoveContinent( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	uint8	continent;

	// load continent index
	msgin.serial(continent);

	// remove continent from position manager
	CWorldPositionManager::removeContinent(continent);
}
	
	
/****************************************************************\
					cbCreateIndoorUnit() 
\****************************************************************/
void cbCreateIndoorUnit( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	DROP_IF(IsRingShard,"Creating indoor units not supported on ring shards!",return);

	sint32	cell;

	// cell id (should be >= 2)
	msgin.serial(cell);

	if (cell < 2)
	{
		nlwarning("cbCreateIndoorUnit(): try to create unit using id < 2");
		return;
	}

	CWorldPositionManager::createIndoorCell(cell);

	nlinfo("MSG: Creating indoor unit %d",cell);
}

/****************************************************************\
					cbCreateBuilding() 
\****************************************************************/
void cbCreateBuilding( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	uint8		continent;
	string		id;
	CVectorD	position;

	msgin.serial(continent);
	msgin.serial(id);
	msgin.serial(position);
	
	CWorldPositionManager::createBuildingInstance(continent, id, position);
}

/****************************************************************\
					cbCreateObstacle() 
\****************************************************************/
void cbCreateObstacle( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	string		id, file;
	CVectorD	position;
	float		angle;

	while (msgin.getPos() < (sint32)msgin.length())
	{
		msgin.serial(id, file, position, angle);
		CWorldPositionManager::instanciatePacsPrim(id, file, position, angle);
	}
}

/****************************************************************\
					cbRemoveObstacle() 
\****************************************************************/
void cbRemoveObstacle( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	string		id;

	while (msgin.getPos() < (sint32)msgin.length())
	{
		msgin.serial(id);
		CWorldPositionManager::removePacsPrim(id);
	}
}

/****************************************************************\
					cbSetObstacle() 
\****************************************************************/
void cbSetObstacle( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	string		id;
	bool		obstacle;

	while (msgin.getPos() < (sint32)msgin.length())
	{
		msgin.serial(id, obstacle);
		CWorldPositionManager::setObstacle(id, obstacle);
	}
}

/****************************************************************\
					cbUpdateEntityPosition() 
\****************************************************************/
/*
void cbUpdateEntityPosition( CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	H_BEFORE(cbUpdatePosition);
	//Use only for tp, update position and clear behaviours list
	//nlinfo("received UPDATE_ENTITY_POSITION request");

	CEntityId id;
	id.serial( msgin );

	// entity pos (x, y, z, theta) at tick
	sint32 x;
	msgin.serial( x );
	sint32 y;
	msgin.serial( y );
	sint32 z;
	msgin.serial( z );
	float t;
	msgin.serial( t );
	NLMISC::TGameCycle tick;
	msgin.serial( tick );

//	nlwarning("GPMS Received Update Position, Pos(%.3f,%.3f,%.3f), theta = %.3f", x, y, z,t );

	CWorldPositionManager::setEntityPosition(id, x, y, z, t, tick);
	H_AFTER(cbUpdatePosition);
} // cbUpdateEntityPosition //
*/

/****************************************************************\
					cbUpdateEntitiesPositions() 
\****************************************************************/
/*
void cbUpdateEntitiesPositions( CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	H_BEFORE(cbUpdatePositions);
	//Use only for tp, update position and clear behaviours list
	//nlinfo("received UPDATE_ENTITIES_POSITIONS request");

	uint16 entitiesCount;
	msgin.serial( entitiesCount );

	NLMISC::CEntityId	sid;
	sint32				x, y, z;
	float				t;
	NLMISC::TGameCycle	tick;

	for (uint16 i = 0 ; i < entitiesCount ; ++i)
	{
		msgin.serial( sid );
		msgin.serial( x );
		msgin.serial( y );
		msgin.serial( z );
		msgin.serial( t );
		msgin.serial( tick );

		//nlinfo("Entity %s: received (%.3f,%.3f,%.3f), theta = %.3f", sid.toString().c_str(), x*0.001, y*0.001, z*0.001,t );

		CWorldPositionManager::setEntityPosition(sid, x, y, z, t, tick);
	}

	H_AFTER(cbUpdatePositions);
} // cbUpdateEntitiesPositions //
*/

/****************************************************************\
					cbUpdateEntitiesPositions() 
\****************************************************************/
/*
void cbUpdateEntitiesPositionsUsingSize( CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	H_BEFORE(cbUpdatePositions);
	//nlinfo("received UPDATE_ENTITIES_POSITIONS request");

	NLMISC::CEntityId	sid;
	sint32				x, y, z;
	float				t;
	NLMISC::TGameCycle	tick;

	while (msgin.getPos() < (sint32)msgin.length())
	{
		msgin.serial( sid );
		msgin.serial( x );
		msgin.serial( y );
		msgin.serial( z );
		msgin.serial( t );
		msgin.serial( tick );

		//nlinfo("Entity %s: received (%.3f,%.3f,%.3f), theta = %.3f", id.toString().c_str(), x*0.001, y*0.001, z*0.001,t );

		CWorldPositionManager::setEntityPosition(sid, x, y, z, t, tick);
	}

	H_AFTER(cbUpdatePositions);
} // cbUpdateEntitiesPositions //
*/

/*
/// Update orientation of several entities
void cbUpdateEntitiesOrientations( CMessage& msgin, const string &serviceName, uint16 serviceId )

/// Update position of several entities (for re-spawn, position correction, server resync...)
void cbUpdateEntitiesPositionsUsingSize( CMessage& msgin, const string &serviceName, uint16 serviceId )

/// Update orientation of several entities
void cbUpdateEntitiesOrientationsUsingSize( CMessage& msgin, const string &serviceName, uint16 serviceId )
*/



/****************************************************************\
					cbEntityTeleport() 
\****************************************************************/
void cbEntityTeleportation( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId id;
	NLMISC::TGameCycle tick;

	id.serial( msgin );
	TDataSetRow	index = CWorldPositionManager::getEntityIndex(id);
	BOMB_IF(IsRingShard && !index.isValid(),"Ignoring request to TP entity withe invalid data set row: "<<id.toString(),return);

	// if no position provided, teleport entity to nowhere
	if (msgin.getPos() == (sint32)msgin.length())
	{
		if (IsRingShard)
		{
			// update the entity position in the ring vision universe
			pCGPMS->RingVisionUniverse->setEntityPosition(index,0,0);

			// update the player coordinates in the mirror
			CMirrorPropValue1DS<sint32>( TheDataset, index, DSPropertyPOSX )= 0; 
			CMirrorPropValue1DS<sint32>( TheDataset, index, DSPropertyPOSY )= 0;
			CMirrorPropValue1DS<sint32>( TheDataset, index, DSPropertyPOSZ )= 0;
		}
		else
		{
			CWorldPositionManager::teleport(index, 0, 0, 0, 0.0f, NO_CONTINENT_INDEX, 0, CTickEventHandler::getGameCycle());
		}

		nlinfo("MSG: Teleporting entity %d to continent 0 (0, 0, 0)",index.getIndex());
		return;
	}

	// entity pos (x, y, z, theta) at tick
	sint32 x;
	msgin.serial( x );
	sint32 y;
	msgin.serial( y );
	sint32 z;
	msgin.serial( z );
	float t;
	msgin.serial( t );

	msgin.serial( tick );

	// use default continent 0
	uint8	continent = INVALID_CONTINENT_INDEX;
	if (msgin.getPos() != (sint32)msgin.length())
		msgin.serial(continent);

	sint32	cell = 0;
	if (msgin.getPos() != (sint32)msgin.length())
		msgin.serial(cell);

	if (cell > 0)
	{
		nlwarning("cbEntityTeleportation(): cell=%d for %s should be zero or negative, forced to 0", cell, id.toString().c_str());
		cell = 0;
	}

	if (IsRingShard)
	{
		// update the ring vision universe position
		pCGPMS->RingVisionUniverse->setEntityPositionDelayed(index,x,y,tick);

		// tell the move checker to teleport the entity too
		pCGPMS->MoveChecker->teleport(index, x, y, tick);

		// update the player coordinates in the mirror
		CMirrorPropValue1DS<sint32>( TheDataset, index, DSPropertyPOSX )= x; 
		CMirrorPropValue1DS<sint32>( TheDataset, index, DSPropertyPOSY )= y;
		CMirrorPropValue1DS<sint32>( TheDataset, index, DSPropertyPOSZ )= z;
		CMirrorPropValue1DS<float>( TheDataset, index, DSPropertyORIENTATION )= t;
	}
	else
	{
		CWorldPositionManager::teleport(index, x, y, z, t, continent, cell, tick);
	}


	// lock entity after a real teleport
	CWorldPositionManager::lock(index, 10);

	nlinfo("MSG: Teleporting entity %d to continent %d (%d, %d, %d) at tick: %d",index.getIndex(),continent,x,y,z,tick);
} // cbEntityTeleportation //


/****************************************************************\
					cbEntityPosition()
\****************************************************************/
void cbEntityPosition( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
//	CEntityId sender;
//	sender.serial( msgin );
	
	CEntityId id;
	id.serial( msgin );

	TDataSetRow	index = CWorldPositionManager::getEntityIndex(id);

	const CWorldEntity *pEntity = CWorldPositionManager::getEntity( index );
	
	if( pEntity )
	{
		CMessage msgout( "ENTITY_POS" );

//		msgout.serial( sender );
		msgout.serial( id );
		
		sint32 val;
		val = pEntity->X();
		msgout.serial( val );
		
		val = pEntity->Y();
		msgout.serial( val );

		val = pEntity->Z();
		msgout.serial( val );

		float t = pEntity->Theta();
		msgout.serial( t );

		sendMessageViaMirror (serviceId, msgout);
	}
	else
	{
		nlwarning("Entity position : entity %s is not in the GPMS", id.toString().c_str());
	}

} // cbEntityPosition //

/****************************************************************\
			Attach child entity to a father
\****************************************************************/
void cbAttach( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	father, child;
	sint32	x, y, z;

	msgin.serial(father);
	msgin.serial(child);
	msgin.serial(x, y, z);

	TDataSetRow	indexFather = CWorldPositionManager::getEntityIndex(father);
	TDataSetRow	indexChild = CWorldPositionManager::getEntityIndex(child);

	CWorldPositionManager::attach(indexFather, indexChild, x, y, z);
}

/****************************************************************\
			Detach child entity of its current father
\****************************************************************/
void cbDetach( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	child;

	msgin.serial(child);

	TDataSetRow	childIndex = CWorldPositionManager::getEntityIndex(child);

	CWorldPositionManager::detach(childIndex);
}


/****************************************************************\
			acquire control
\****************************************************************/
void cbAcquireControl( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	slave, master;
	sint32		x, y, z;

	msgin.serial(slave, master);
	msgin.serial(x, y, z);

	TDataSetRow	indexSlave = CWorldPositionManager::getEntityIndex(slave);
	TDataSetRow	indexMaster = CWorldPositionManager::getEntityIndex(master);

	CWorldPositionManager::acquireControl(indexSlave, indexMaster, x, y, z);

	if (IsRingShard)
	{
		pCGPMS->RingVisionUniverse->setEntityPosition(indexMaster,x,y);
		pCGPMS->RingVisionUniverse->setEntityPosition(indexSlave,x,y);
	}

	nlinfo("MSG: Master %s acquire control of slave %s at (%d, %d, %d)",master.toString().c_str(),slave.toString().c_str(),x,y,z);
}

/****************************************************************\
			leave control
\****************************************************************/
void cbLeaveControl( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	master;

	msgin.serial(master);

	TDataSetRow	indexMaster = CWorldPositionManager::getEntityIndex(master);

	CWorldPositionManager::leaveControl(indexMaster);

	nlinfo("MSG: Master %s release control of slave",master.toString().c_str());
}


/****************************************************************\
			activate player self slot
\****************************************************************/
void cbActivateSelf( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
#ifdef HANDLE_SLOT0_SPECIAL
	CEntityId	id;

	msgin.serial(id);

	TDataSetRow		index = CWorldPositionManager::getEntityIndex(id);
	CWorldEntity	*entity = CWorldPositionManager::getEntityPtr(index);
	CWorldPositionManager::activateSelfSlot(entity);
#endif
}

/****************************************************************\
			desactivate player self slot
\****************************************************************/
void cbDesactivateSelf( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
#ifdef HANDLE_SLOT0_SPECIAL
	CEntityId	id;

	msgin.serial(id);

	TDataSetRow		index = CWorldPositionManager::getEntityIndex(id);
	CWorldEntity	*entity = CWorldPositionManager::getEntityPtr(index);
	CWorldPositionManager::desactivateSelfSlot(entity);
#endif
}

// The following removed by Sadge because the entity has been removed from the mirror by the time the message arrives,
// making it redundant
///****************************************************************\
//			disable vision processing
//\****************************************************************/
//void cbDisableVisionProcessing( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
//{
//	CEntityId	id;
//
//	msgin.serial(id);
//
//	TDataSetRow		index = CWorldPositionManager::getEntityIndex(id);
//	CWorldPositionManager::setPlayerVisionProcessing(index, false);
//}


/****************************************************************\
			Subscribe to a trigger
\****************************************************************/
void	cbTriggerSubscribe( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	while ((uint32)msgin.getPos() < msgin.length())
	{
		string	name;
		uint16	id;
		msgin.serial(name, id);
		CWorldPositionManager::triggerSubscribe(serviceId, name, id);
	}
}

/****************************************************************\
			Unsubscribe to a trigger
\****************************************************************/
void	cbTriggerUnsubscribe( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	while ((uint32)msgin.getPos() < msgin.length())
	{
		uint16	id;
		msgin.serial(id);
		CWorldPositionManager::triggerUnsubscribe(serviceId, id);
	}
}


/****************************************************************\
			request all entities arround an entity
\****************************************************************/
void cbEntitiesArroundEntity( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId id;	// base entity for decide list of entities arround this
	list< string > propertySubscribe; // list of properties subscribe

	msgin.serial( id );
	msgin.serialCont( propertySubscribe );

	CWorldPositionManager::requestForEntityAround( serviceId, id, propertySubscribe );

	nlinfo("Service %s asked for local mirror around entity %s",serviceName.c_str(), id.toString().c_str() );
}

/****************************************************************\
	  end subscription for all entities arround an entity
\****************************************************************/
void cbEndEntitiesArroundEntity( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId id;	// base entity for decide list of entities arround this
	
	msgin.serial( id );

	CWorldPositionManager::unrequestForEntityAround( serviceId, id );
	nlinfo("Service %s unsubscribe for local mirror around entity %s",serviceName.c_str(), id.toString().c_str() );
}


/****************************************************************\
	  ponctual vision request
\****************************************************************/
void cbVisionRequest( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	sint32	rid;
	sint32	x, y;
	sint32	range;

	msgin.serial(rid, x, y, range);

	static vector<pair<CEntityId, sint32> >	entities;
	entities.clear();

	// get vision
	CWorldPositionManager::visionRequest(x, y, range, entities);

	// build vision answer
	CMessage	msgout("VISION_ANSWER");
	msgout.serial(rid);
	uint	i;
	const uint size = (uint)entities.size();
	for ( i = 0 ; i < size ; ++i)
		msgout.serial(entities[i].first, entities[i].second);

	// send it to requester
	sendMessageViaMirror(serviceId, msgout);
}

/****************************************************************\
	  ponctual vision request
\****************************************************************/
void cbR2ForceVisionReset( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	eid;

	msgin.serial(eid);

	
	TDataSetRow entityIndex = TheDataset.getDataSetRow( eid );
	BOMB_IF(!entityIndex.isValid() , "Try to reset the vision of a invalid player "+eid.toString(), return);
	
	pCGPMS->RingVisionUniverse->forceResetVision(entityIndex);

}




//----------------------------
//	CbArray
//----------------------------
TUnifiedCallbackItem CbGPMSArray[]=
{
// Callbacks for all service use GPMS
	//{ "ADD_ENTITY",					cbAddEntity },					// create a single entity
	//{ "ADD_ENTITIES",				cbAddEntities },				// create a set of entities
	//{ "ADD_IA_OBJECT",				cbAddIAObject },			// create an object only visible from ia agents
	//{ "REPLACE_ENTITY",				cbReplaceEntity},				// replace an entity by another entity
	//{ "REMOVE_ENTITY",				cbRemoveEntity },				// remove a single entity
	//{ "REMOVE_ENTITIES",			cbRemoveEntities },				// removes a set of entities
/*?DEAD?*/	{ "BAG",						cbBag },						// updates content of an entity (bag only ?)

/*?DEAD?*/	{ "LOAD_CONTINENT",				cbLoadContinent },				// load a continent
/*?DEAD?*/	{ "REMOVE_CONTINENT",			cbRemoveContinent },			// remove a continent
	{ "CREATE_INDOOR_UNIT",			cbCreateIndoorUnit },			// create an indoor unit, for latter teleport
/*?DEAD?*/	{ "CREATE_BUILDING",			cbCreateBuilding },				// create a dynamic outdoor building
/*?DEAD?*/	{ "CREATE_OBSTACLE",			cbCreateObstacle },				// create a dynamic obstacle from a pacs_prim file
/*?DEAD?*/	{ "REMOVE_OBSTACLE",			cbRemoveObstacle },				// remove a dynamic obstacle
/*?DEAD?*/	{ "SET_OBSTACLE",				cbSetObstacle },				// set dynamic obstacle state

/*?DEAD?*/	{ "TRIGGER_SUBSCRIBE",			cbTriggerSubscribe },
/*?DEAD?*/	{ "TRIGGER_UNSUBSCRIBE",		cbTriggerUnsubscribe },

	//{ "UPDATE_ENTITY_POS",			cbUpdateEntityPosition},		// update a single entity position
	//{ "UPDATE_ENTITIES_POSITIONS",  cbUpdateEntitiesPositions},		// update a set of entities positions
	//{ "UPDATE_ENTITIES_POS",		cbUpdateEntitiesPositionsUsingSize },	// update a set of entities positions
	{ "ENTITY_TELEPORTATION",		cbEntityTeleportation},			// teleport a single entity (if no position, remove from cell map)

/*?DEAD?*/	{ "ATTACH",						cbAttach },						// attach an entity (child) to an another (father) using direct local position
/*?DEAD?*/	{ "DETACH",						cbDetach },						// detach an entity of its father
	{ "ACQUIRE_CONTROL",			cbAcquireControl },				// entity acquire control of another entity
	{ "LEAVE_CONTROL",				cbLeaveControl },				// entity leave control of another entity

/*?DEAD?*/	{ "ACTIV_SELF",					cbActivateSelf },
/*?DEAD?*/	{ "DESACTIV_SELF",				cbDesactivateSelf },

// The following removed by Sadge because the entity has been removed from the mirror by the time the message arrives,
// making it redundant
// / *?DEAD?* /	{ "DISABLE_VISION_PROC",		cbDisableVisionProcessing },	// ask for player vision not to be updated any longer

/*?DEAD?*/	{ "ENTITY_POS",					cbEntityPosition },				// ask for position of an entity
	
/*?DEAD?*/	{ "ASK_VISION_ARROUND_ENTITY",	cbEntitiesArroundEntity },		// ask for vision update around an entity
/*?DEAD?*/	{ "UNASK_VISION_ARROUND_ENTITY",cbEndEntitiesArroundEntity },	// remove vision update around an entity

/*?DEAD?*/	{ "VISION_REQUEST",				cbVisionRequest },
	{ "SET_PLAYER_FLAGS",			cbSetPlayerFlags },				// set flags for player (limit speed, etc)
	{ "R2_VISION_REFRESH",			cbR2ForceVisionReset },				// force the update of the vision (because some message must have been discared in Ring (in edition mode the network is not listen)
}; 



//-------------------------------------------------------------------------
// singleton initialisation and release

void CMessages::init()
{
	// setup the callback array
	CUnifiedNetwork::getInstance()->addCallbackArray( CbGPMSArray, sizeof(CbGPMSArray)/sizeof(CbGPMSArray[0]) );
}

void CMessages::release()
{
}

