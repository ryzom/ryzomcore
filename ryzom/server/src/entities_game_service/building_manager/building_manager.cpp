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
#include "nel/misc/algo.h"
#include "nel/net/service.h"
#include "server_share/log_item_gen.h"
#include "server_share/r2_variables.h"

#include "building_manager/building_manager.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "zone_manager.h"
#include "mission_manager/mission_parser.h"
#include "creature_manager/creature_manager.h"
#include "guild_manager/fame_manager.h"
#include "primitives_parser.h"
#include "building_template.h"
#include "building_manager/building_physical.h"
#include "destination.h"
#include "building_manager/room_instance.h"
#include "player_manager/player_room.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_member_module.h"

#include "egs_sheets/egs_sheets.h"

#include "outpost_manager/outpost_building.h"

extern CGenericXmlMsgHeaderManager GenericMsgManager;

CBuildingManager * CBuildingManager::_Instance = NULL;
extern bool GPMSIsUp;

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLLIGO;

NL_INSTANCE_COUNTER_IMPL(CTriggerRequestTimoutEvent);

/// a player entered in a lift
void cbLiftIn( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	sint32 triggerId;
	TDataSetRow rowId;
	msgin.serial(triggerId);
	msgin.serial(rowId);

	CBuildingManager::getInstance()->addTriggerRequest( rowId, triggerId );
}
/// a player left a lift
void cbLiftOut( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	sint32 triggerId;
	TDataSetRow rowId;
	msgin.serial(triggerId);
	msgin.serial(rowId);

	CBuildingManager::getInstance()->removeTriggerRequest( rowId );
}

//----------------------------------------------------------------------------
void CBuildingManager::init()
{
	nlassert( _Instance == NULL );

	_Instance = new CBuildingManager;

	// register lift callbacks
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "PTRIG_IN",				cbLiftIn		},
		{ "PTRIG_OUT",				cbLiftOut		},
	};

	// register call back for lift trigger
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );

	// init the player building sheet
	_Instance->_PlayerBuildingSheet = CSheetId( "player_building.sitem" );
	nlassert(_Instance->_PlayerBuildingSheet != CSheetId::Unknown );

	_Instance->_GuildBuildingSheet = CSheetId( "guild_main_building.sitem" );
	nlassert(_Instance->_GuildBuildingSheet != CSheetId::Unknown );
/*	_Instance->_RoleMasterSheets.resize( EGSPD::CSPType::EndSPType );

	_Instance->_RoleMasterSheets[ EGSPD::CSPType::Fight ] = CSheetId( "guild_rm_fight.sitem" );
	nlassert(_Instance->_RoleMasterSheets[ EGSPD::CSPType::Fight ] != CSheetId::Unknown );
	_Instance->_RoleMasterSheets[ EGSPD::CSPType::Magic ] = CSheetId( "guild_rm_magic.sitem" );
	nlassert(_Instance->_RoleMasterSheets[ EGSPD::CSPType::Magic ] != CSheetId::Unknown );
	_Instance->_RoleMasterSheets[ EGSPD::CSPType::Harvest ] = CSheetId( "guild_rm_harvest.sitem" );
	nlassert(_Instance->_RoleMasterSheets[ EGSPD::CSPType::Harvest ] != CSheetId::Unknown );
	_Instance->_RoleMasterSheets[ EGSPD::CSPType::Craft ] = CSheetId( "guild_rm_craft.sitem" );
	nlassert(_Instance->_RoleMasterSheets[ EGSPD::CSPType::Craft ] != CSheetId::Unknown );
*/
	// get primitives to parse
	const CPrimitivesParser::TPrimitivesList & primsList = CPrimitivesParser::getInstance().getPrimitives();
//	const vector<string> & primsFiles = CPrimitivesParser::getInstance().getPrimitiveFiles();


	nlinfo("PARSING TELEPORT INFOS");

	CPrimitivesParser::TPrimitivesList::const_iterator first, last;

	// build building templates
	CBuildingParseData parseData;
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! _Instance->parseBuildingTemplates(first->Primitive.RootNode ,parseData ) )
			nlwarning("<BUILDING> Error while building building templates in file '%s'", first->FileName.c_str());
	}

	// build "physical building" (see building_template.h for a description
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! _Instance->parsePhysicalBuildings(first->Primitive.RootNode, parseData ) )
			nlwarning("<BUILDING> Error while building triggers in file '%s'", first->FileName.c_str());
	}

	// build triggers
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! _Instance->parseTriggers(first->Primitive.RootNode, parseData ) )
			nlwarning("<BUILDING> Error while building triggers in file '%s'",first->FileName.c_str());
	}
	nlinfo("FINISHED PARSING TELEPORT INFOS");

	// init the room container
	_Instance->_FirstFreeRoomId = 0;
#ifdef NL_DEBUG
	_Instance->_RoomAllocStep = 1;
#else
	_Instance->_RoomAllocStep = 128;
#endif
	_Instance->reallocRooms();

}

//----------------------------------------------------------------------------
void CBuildingManager::release()
{
	nlassert( _Instance );
	delete _Instance;
	_Instance = NULL;
}

//----------------------------------------------------------------------------
void CBuildingManager::playerDisconnects(const CCharacter * user)
{
	nlassert(user);
	TTriggerRequestCont::iterator itReq = _TriggerRequests.find( user->getEntityRowId() );
	if ( itReq != _TriggerRequests.end() )
	{
		if ( (*itReq).second.Timer )
			delete (*itReq).second.Timer;
		_TriggerRequests.erase( itReq );
	}
}


//----------------------------------------------------------------------------
void CBuildingManager::gpmsConnection()
{
	// on non-ring shards we ask the GPMS to setup indoor room instances
	if (!IsRingShard)
	{
		// we register the cells
		for ( uint32 i = 0; i < _RoomInstances.size(); i++ )
		{
			//allocate the cells in GPMS
			NLNET::CMessage msgout("CREATE_INDOOR_UNIT");
			sint32 cellId = ( sint32(i) << 1 ) + 2;
			msgout.serial(cellId);
			sendMessageViaMirror ("GPMS", msgout);
		}
	}
}

//----------------------------------------------------------------------------
bool CBuildingManager::parseBuildingTemplates( const NLLIGO::IPrimitive* prim, CBuildingParseData & parseData )
{
	nlassert(prim);
	std::string value;
	if (prim->getPropertyByName("class",value) )
	{
		if ( value == "building_template" )
		{
			CBuildingTemplate * templ = new CBuildingTemplate;
			if ( !templ->build( prim,parseData ) )
			{
				delete templ;
				return false;
			}
			return true;
		}
	}
	bool ret = true;
	// Lookup recursively in the children
	for (uint i=0;i<prim->getNumChildren();++i)
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) && child )
			ret = parseBuildingTemplates(child,parseData) && ret;
	}
	return ret;
}

//----------------------------------------------------------------------------
bool CBuildingManager::parsePhysicalBuildings( const NLLIGO::IPrimitive* prim, CBuildingParseData & parseData )
{
	nlassert(prim);
	std::string value;
	bool ret = true;
	if (prim->getPropertyByName("class",value) )
	{
		if ( value == "building_instance" )
		{
			TAIAlias alias;
			nlverify( CPrimitivesParser::getAlias(prim, alias));
//			TAIAlias alias;
//			NLMISC::fromString(value, alias);
			if ( _BuildingPhysicals.find( alias ) != _BuildingPhysicals.end() )
			{
				nlwarning("<BUILDING>building instance %s exists more than once", CPrimitivesParser::aliasToString(alias).c_str());
				return false;
			}
			nlverify( prim->getPropertyByName("building_template",value) );
			map<string,CBuildingTemplate*>::iterator itTempl = parseData.BuildingTemplates.find( value );
			if ( itTempl == parseData.BuildingTemplates.end() )
			{
				nlwarning("<BUILDING>building instance %s has an invalid template '%s'", CPrimitivesParser::aliasToString(alias).c_str(), value.c_str() );
				return false;
			}
			IBuildingPhysical * building = NULL;
			if ( (*itTempl).second->Type == BUILDING_TYPES::Common )
				building = new CBuildingPhysicalCommon((*itTempl).second,alias);
			else if ( (*itTempl).second->Type == BUILDING_TYPES::Guild )
				building = new CBuildingPhysicalGuild((*itTempl).second,alias);
			else if ( (*itTempl).second->Type == BUILDING_TYPES::Player )
				building = new CBuildingPhysicalPlayer((*itTempl).second,alias);
			else
				nlerror("invalid build types previous checks not done");

			if ( !building->build( prim,parseData ) )
			{
				nlwarning("<BUILDING>error in building instance %s", CPrimitivesParser::aliasToString(alias).c_str() );
				delete building;
				return false;
			}
			else
			{
				_BuildingPhysicals.insert( make_pair( alias, building ) );
				_BuildingPhysicalsName.insert( make_pair( building->getName(), building));
				return true;
			}
		}
	}
	// Lookup recursively in the children
	for (uint i=0; i < prim->getNumChildren(); ++i)
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) && child )
			ret = parsePhysicalBuildings(child,parseData) && ret;
	}
	return ret;
}

//----------------------------------------------------------------------------
bool CBuildingManager::parseTriggers( const NLLIGO::IPrimitive* prim, CBuildingParseData & parseData )
{
	nlassert(prim);
	std::string value;
	bool ret = true;
	if (prim->getPropertyByName("class",value) )
	{
		if ( value == "teleport_trigger" || value == "building_trigger" )
		{
			std::string name;
			nlverify( prim->getPropertyByName("name",name) );
			nlverify( prim->getPropertyByName("pacs_trigger_id",value) );
			uint32 triggerId;
			NLMISC::fromString(value, triggerId);

			if ( triggerId == 0 && CMissionParser::getNoBlankString( value ) != "0" )
			{
				nlwarning("<BUILDING>in tp '%s' : Invalid pacs_trigger_id '%s' : must be a number", name.c_str(),value.c_str() );
				return false;
			}
			CTrigger & trigger = _Triggers[triggerId];
	nlwarning("trig %d %s - %s", triggerId, name.c_str(), prim->getName().c_str());
			nlverify( prim->getPropertyByName("auto_teleport",value) );
			trigger.AutoTeleport = (value == "true");

			if ( prim->getNumChildren() == 0 )
			{
				nlwarning("<BUILDING>in tp '%s' : no destination", name.c_str() );
				return false;
			}

			// parse the children destinations
			const NLLIGO::IPrimitive * roomPrim = prim->getParent();

			for ( uint i = 0; i < prim->getNumChildren(); i++ )
			{
				const NLLIGO::IPrimitive* child = NULL;
				if ( prim->getChild(child,i) && child )
				{
					IDestination * dest = NULL;
					string name;
					nlverify( child->getPropertyByName("class",value) );
					nlverify( child->getPropertyByName("name",name) );

					if ( value == "teleport_destination" )
						dest = new CTPDestination(name);
					else if ( value == "building_destination" )
						dest = new  CBuildingDestination( name );
					else if ( value == "exit_destination" )
						dest = new CExitDestination(name);
					else if ( value == "room_destination" )
						dest = new CRoomDestination(name);
					else if ( value != "building_instance" )
						nlwarning("<BUILDING> invalid destination type '%s'",value.c_str() );
					if ( dest )
					{
						if ( dest->build(child,roomPrim,parseData) )
							trigger.Destinations.push_back( dest );
						else
							delete dest;
					}
				}
			}
			return ret;
		}
	}
	// Lookup recursively in the children
	for (uint i=0; i < prim->getNumChildren(); ++i)
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) && child )
			ret = parseTriggers(child,parseData) && ret;
	}
	return ret;
}

//----------------------------------------------------------------------------
void CBuildingManager::addTriggerRequest( const TDataSetRow & rowId, sint32 triggerId )
{
	//GPMS can send a trigger msg more than one. So check if we already received it
	TTriggerRequestCont::iterator itReq = _TriggerRequests.find( rowId );
	if ( itReq != _TriggerRequests.end() )
		return;

	/// get the trigger associated with the pacs trigger id
	CHashMap<sint,CTrigger>::iterator itTrigger =  _Triggers.find( triggerId );
	if ( itTrigger == _Triggers.end() )
	{
		nlwarning("<BUILDING> trigger %u is invalid. There was probably errors encountered during parsing", triggerId);
		return;
	}

	CTrigger & trigger = (*itTrigger).second;
	// ignore empty triggers
	if (  trigger.Destinations.empty() )
		return;

	// get user
	CCharacter * user= PlayerManager.getChar( rowId );
	if ( !user )
	{
		nlwarning("<BUILDING> row %u is invalid",rowId.getIndex() );
		return;
	}


	// build a request for our user
	CTriggerRequest request;
	request.Page = 0;
	request.Session = 0;

	CTriggerRequestEntry entry;
	const uint destCount = (uint)trigger.Destinations.size();
	for ( uint i = 0; i < destCount; i++ )
	{
		entry.Destination = trigger.Destinations[i];
		if (entry.Destination == NULL)
		{
			nlwarning("<BUILDING> NULL destination in trigger");
			return;
		}
		const uint16 ownerCount = entry.Destination->getEntryCount();
		bool addSession = false;
		for ( uint16 j = 0; j < ownerCount; j++ )
		{
			if ( entry.Destination->isUserAllowed(user,j) )
			{
				entry.OwnerIndex = j;
				request.Entries.push_back( entry );
				addSession = true;
			}
		}
		if ( addSession )
			request.Session += entry.Destination->getStateCounter();
	}
	if ( request.Entries.empty() )
	{
		return;
	}

	request.Timer = new CTimer;
	request.Timer->setRemaining( TriggerRequestTimout, new CTriggerRequestTimoutEvent(rowId) );
	// add the request to our manager
	_TriggerRequests.insert( make_pair( rowId, request ) );

	// if it is an auto teleport, force entity teleportation
	if ( trigger.AutoTeleport )
	{
		triggerTeleport( user , 0);
		return;
	}

	// tell client
	NLNET::CMessage msgout( "IMPULSION_ID" );
	CEntityId id = user->getId();
	msgout.serial( id );
	CBitMemStream bms;
	nlverify( GenericMsgManager.pushNameToStream( "GUILD:ASCENSOR", bms) );

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	sendMessageViaMirror( NLNET::TServiceId(id.getDynamicId()), msgout );
}


//----------------------------------------------------------------------------
void CBuildingManager::fillTriggerPage(const NLMISC::CEntityId & eId, uint16 clientSession, bool resetPage )
{
	// get user
	CCharacter * user = PlayerManager.getChar( eId );
	if ( !user )
	{
		nlwarning("<BUILDING> invalid user %s",eId.toString().c_str() );
		return;
	}
	// get its request
	TTriggerRequestCont::iterator it =  _TriggerRequests.find( user->getEntityRowId() );
	if ( it == _TriggerRequests.end() )
	{
		// dont warn :client  may have quitted the trigger before sending the request
		return;
	}
	CTriggerRequest & request = (*it).second;

	// set basic database information
	if ( resetPage )
		request.Page = 0;
//	user->_PropertyDatabase.setProp( "ASCENSOR:SESSION",clientSession);
	CBankAccessor_PLR::getASCENSOR().setSESSION(user->_PropertyDatabase, clientSession);
//	user->_PropertyDatabase.setProp( "ASCENSOR:PAGE_ID",request.Page );
	CBankAccessor_PLR::getASCENSOR().setPAGE_ID(user->_PropertyDatabase, request.Page );

	// get the first entry to send
	uint start = request.Page * MaxEntryPerLiftPage;
	uint end = start + MaxEntryPerLiftPage;
	if ( end >= request.Entries.size() )
	{
		end = (uint)request.Entries.size();
//		user->_PropertyDatabase.setProp( "ASCENSOR:HAS_NEXT",0 );
		CBankAccessor_PLR::getASCENSOR().setHAS_NEXT(user->_PropertyDatabase, false);
	}
	else
//		user->_PropertyDatabase.setProp( "ASCENSOR:HAS_NEXT",1 );
		CBankAccessor_PLR::getASCENSOR().setHAS_NEXT(user->_PropertyDatabase, true);

	uint index = 0;
	for ( uint i = start; i < end; i++ )
	{
		IDestination * dest = request.Entries[i].Destination;
//		TVectorParamCheck params(1);
		uint32 txt;
		uint64 icon;
		dest->getClientDescription(request.Entries[i].OwnerIndex, user,icon,txt);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "ASCENSOR:%u:NAME",index ),txt );
		CBankAccessor_PLR::getASCENSOR().getArray(index).setNAME(user->_PropertyDatabase, txt);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "ASCENSOR:%u:ICON",index ),icon );
		CBankAccessor_PLR::getASCENSOR().getArray(index).setICON(user->_PropertyDatabase,icon);
		index++;
	}
	// reset remaining parameters
	for ( uint i = end; i < MaxEntryPerLiftPage; i++ )
	{
//		user->_PropertyDatabase.setProp( NLMISC::toString( "ASCENSOR:%u:ICON",i ),0 );
		CBankAccessor_PLR::getASCENSOR().getArray(i).setICON(user->_PropertyDatabase,0 );
//		user->_PropertyDatabase.setProp( NLMISC::toString( "ASCENSOR:%u:NAME",i ),0 );
		CBankAccessor_PLR::getASCENSOR().getArray(i).setNAME(user->_PropertyDatabase,0 );
	}
	request.Page++;
}

//----------------------------------------------------------------------------
void CBuildingManager::removeTriggerRequest( const TDataSetRow & rowId )
{
	// remove the request
	TTriggerRequestCont::iterator itReq = _TriggerRequests.find( rowId );
	if ( itReq == _TriggerRequests.end() )
		return;

	if ( (*itReq).second.Timer )
		delete (*itReq).second.Timer;
	_TriggerRequests.erase( itReq );

	CCharacter * user = PlayerManager.getChar( rowId );
	if ( !user )
	{
		nlwarning("<BUILDING> row %u is invalid",rowId.getIndex() );
		return;
	}

	// tell client
	CEntityId id = user->getId();
	NLNET::CMessage msgout( "IMPULSION_ID" );
	msgout.serial( id );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "GUILD:LEAVE_ASCENSOR", bms) )
	{
		nlwarning("<BUILDING> Msg name GUILD:LEAVE_ASCENSOR not found");
		return;
	}
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	sendMessageViaMirror( NLNET::TServiceId(id.getDynamicId()), msgout );

	// clear database
//	user->_PropertyDatabase.setProp( "ASCENSOR:SESSION",0 );
	CBankAccessor_PLR::getASCENSOR().setSESSION(user->_PropertyDatabase, 0);
//	user->_PropertyDatabase.setProp( "ASCENSOR:PAGE_ID",0 );
	CBankAccessor_PLR::getASCENSOR().setPAGE_ID(user->_PropertyDatabase, 0);
	for ( uint i = 0; i < MaxEntryPerLiftPage; i++ )
	{
//		user->_PropertyDatabase.setProp( NLMISC::toString( "ASCENSOR:%u:ICON",i ),0 );
		CBankAccessor_PLR::getASCENSOR().getArray(i).setICON(user->_PropertyDatabase, 0);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "ASCENSOR:%u:NAME",i), 0 );
		CBankAccessor_PLR::getASCENSOR().getArray(i).setNAME(user->_PropertyDatabase, 0 );
	}
}


//----------------------------------------------------------------------------
void CBuildingManager::removeGuildBuilding( uint32 guildId )
{
	if( this == 0 )
	{
		nlwarning("<BUILDING> called with null this pointer !!!");
		return;
	}
	// WARNING : to avoid problems of guild deinstanciation, this should be called in the CGuild dtor.
	// lift lists should not be buggy in case of deinstanciation if lift sessions are managed correctly
	map<TAIAlias,IBuildingPhysical*>::iterator itBuilding = _BuildingPhysicals.begin();
	for (; itBuilding != _BuildingPhysicals.end(); ++itBuilding )
	{
		(*itBuilding).second->onGuildDeletion(guildId);
	}
}

//----------------------------------------------------------------------------
void CBuildingManager::removePlayerBuilding( const NLMISC::CEntityId & userId )
{
	// WARNING : to avoid problems of guild deinstanciation, this should be called in the deleteChar method
	// lift lists should not be buggy in case of deinstanciation if lift sessions are managed correctly
	// WARNING : there will be a problem on player deletion. count flag the room as 'to be destroyed' and wait that nobody is in to actulally destroy it
	// WARNING : to avoid problems of guild deinstanciation, this should be called in the CGuild dtor.
	// lift lists should not be buggy in case of deinstanciation if lift sessions are managed correctly
	map<TAIAlias,IBuildingPhysical*>::iterator itBuilding = _BuildingPhysicals.begin();
	for (; itBuilding != _BuildingPhysicals.end(); ++itBuilding )
	{
		(*itBuilding).second->onPlayerDeletion(userId);
	}
}

//----------------------------------------------------------------------------
void CBuildingManager::registerGuild( EGSPD::TGuildId guildId, TAIAlias alias )
{
	H_AUTO(BM_RegisterGuild);
	CBuildingPhysicalGuild * building = dynamic_cast<CBuildingPhysicalGuild *>( getBuildingPhysicalsByAlias( alias ) );
	if ( building )
		building->addGuild( guildId );
	else
		nlwarning("<BUILDING> invalid building %u", building );
}

//----------------------------------------------------------------------------
void CBuildingManager::registerPlayer( CCharacter * user )
{
	nlassert(user);
	if ( user->getRoomInterface().getBuilding() )
	{
		CBuildingPhysicalPlayer * building = const_cast<CBuildingPhysicalPlayer *>( user->getRoomInterface().getBuilding() );
		building->addPlayer( user->getId() );
	}
}

//----------------------------------------------------------------------------
void CBuildingManager::removePlayerFromRoom( CCharacter * user )
{
#ifdef NL_DEBUG
	nlassert(user);
#endif

	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;
	if ( !isRoomCell(cell) )
		return;
	uint idx = getRoomIdxFromCell( cell );
	if ( idx >= _RoomInstances.size() )
	{
		nlwarning("<BUILDING>%s cell %d is not a valid room ( count is  %u)", user->getId().toString().c_str(), cell,_RoomInstances.size() );
		return;
	}
	if ( !_RoomInstances[idx].Ptr )
	{
		nlwarning("<BUILDING>%s cell %d is not a valid room!", user->getId().toString().c_str(), cell);
		return;
	}
	// remove a reference from the room
	_RoomInstances[idx].Ptr->removeUser( user );
	// if there is nobody in the room, remove it
	if ( !_RoomInstances[idx].Ptr->isValid() )
	{
		delete _RoomInstances[idx].Ptr;
		_RoomInstances[idx].Ptr = NULL;
		_RoomInstances[idx].NextFreeId = _FirstFreeRoomId;
		_FirstFreeRoomId = idx;
	}
}

//----------------------------------------------------------------------------
IRoomInstance *  CBuildingManager::allocateRoom( sint32 & cellRet, BUILDING_TYPES::TBuildingType type)
{
	// update room vector
	if ( _FirstFreeRoomId >= _RoomInstances.size() )
		reallocRooms();
	uint idx = _FirstFreeRoomId;
	_FirstFreeRoomId = _RoomInstances[_FirstFreeRoomId].NextFreeId;
	cellRet = -2 - ( (sint32) idx << 1 );
	if ( type == BUILDING_TYPES::Common )
		_RoomInstances[idx].Ptr = new CRoomInstanceCommon;
	else if ( type == BUILDING_TYPES::Guild )
		_RoomInstances[idx].Ptr = new CRoomInstanceGuild;
	else if ( type == BUILDING_TYPES::Player )
		_RoomInstances[idx].Ptr = new CRoomInstancePlayer;
	else
	{
		nlwarning("<BUILDING>invalid room type %d",type);
		return NULL;
	}
	return _RoomInstances[idx].Ptr;
}


//----------------------------------------------------------------------------
inline void  CBuildingManager::reallocRooms()
{
	// here the current size of the vector is _FirstFreeBuildingId
	_RoomInstances.resize(_FirstFreeRoomId + _RoomAllocStep);

	// we init the new building as unset
	// if the GPMS is up we register the cells. There is code copy here because we want to avoid a test on the GPMS at each iteration
	if ( GPMSIsUp )
	{
		// on non-ring shards we ask the GPMS to setup indoor room instances
		if (!IsRingShard)
		{
			for (uint32 i = _FirstFreeRoomId; i < _RoomInstances.size(); i++)
			{
				_RoomInstances[i].NextFreeId =i+1 ;
				_RoomInstances[i].Ptr = NULL;
				//allocate the cell in GPMS ( here cell values must be > 0 )
				NLNET::CMessage msgout("CREATE_INDOOR_UNIT");
				sint32 cellId = -getRoomCellFromIdx(i);
				msgout.serial(cellId);
				sendMessageViaMirror ("GPMS", msgout);
			}
		}
	}
	else
	{
		for (uint32 i = _FirstFreeRoomId; i < _RoomInstances.size(); i++)
		{
			_RoomInstances[i].NextFreeId = i+1;
			_RoomInstances[i].Ptr = NULL;
		}
	}

}

//----------------------------------------------------------------------------
void CBuildingManager::triggerTeleport(CCharacter * user, uint16 index)
{
#ifdef NL_DEBUG
	nlassert(user);
#endif
	// find the user request
	TTriggerRequestCont::iterator it = _TriggerRequests.find( user->getEntityRowId() );
	if ( it == _TriggerRequests.end() )
	{
		nlwarning( "<BUILDING> char %s has no valid request",user->getId().toString().c_str() );
		return;
	}



	// check if sessions mactch between user and system
	uint16 session = 0;
	const uint16 destCount = (uint16)(*it).second.Entries.size();
	IDestination * currentDest = NULL;
	for ( uint i = 0; i < destCount; i++ )
	{
		// sum up all state counters to compute the session
		if ( (*it).second.Entries[i].Destination != currentDest )
		{
			currentDest = (*it).second.Entries[i].Destination;
			session += currentDest->getStateCounter();
		}
	}

	if ( session != (*it).second.Session )
	{
		CCharacter::sendDynamicSystemMessage(user->getEntityRowId(),"TELEPORT_BAD_SESSION");
		_TriggerRequests.erase( it );
		return;
	}

	// find the selected entry
	if ( index >= (*it).second.Entries.size() )
	{
		nlwarning( "<BUILDING> char %s : dest = %u, count = %u",user->getId().toString().c_str(), index, (*it).second.Entries.size() );
		_TriggerRequests.erase( it );
		return;
	}
	CTriggerRequestEntry & entry = (*it).second.Entries[index];
	IDestination * dest = entry.Destination;
	uint16 ownerIdx = entry.OwnerIndex;

	// remove the requests
	while ( it != _TriggerRequests.end() )
	{
		_TriggerRequests.erase( it );
		it = _TriggerRequests.find( user->getEntityRowId() );
	}

	if ( !dest->isUserAllowed(user,ownerIdx) )
	{
		return;
	}

	// check if mounts are allowed
	if ( !dest->arePetsAllowed() )
	{
		user->forbidNearPetTp();
		if ( ! user->getEntityMounted().isNull() )
		{
			CCharacter::sendDynamicSystemMessage( user->getId(), "TELEPORT_NO_PET" );
			return;
		}
/*
		for ( uint i = 0; i < user->getPlayerPets().size(); i++ )
		{
			if ( user->getPlayerPets()[i].PetStatus == CPetAnimal::landscape )
			{
				CCharacter::sendDynamicSystemMessage( user->getId(), "TELEPORT_NO_PET" );
				return;
			}
		}
*/	}
	else
		user->allowNearPetTp();

	// reset database
//	user->_PropertyDatabase.setProp( "ASCENSOR:SESSION",0 );
	CBankAccessor_PLR::getASCENSOR().setSESSION(user->_PropertyDatabase,0 );
//	user->_PropertyDatabase.setProp( "ASCENSOR:PAGE_ID",0 );
	CBankAccessor_PLR::getASCENSOR().setPAGE_ID(user->_PropertyDatabase,0 );
	for ( uint i = 0; i < MaxEntryPerLiftPage; i++ )
	{
//		user->_PropertyDatabase.setProp( NLMISC::toString( "ASCENSOR:%u:ICON",i ),0 );
		CBankAccessor_PLR::getASCENSOR().getArray(i).setICON(user->_PropertyDatabase,0 );
//		user->_PropertyDatabase.setProp( NLMISC::toString( "ASCENSOR:%u:NAME",i), 0 );
		CBankAccessor_PLR::getASCENSOR().getArray(i).setNAME(user->_PropertyDatabase, 0 );
	}

	// get the destination zone
	const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone( dest->getSpawnZoneId(user) );
	if ( !zone )
	{
		nlwarning( "<BUILDING> char %s : invalid zone %u", user->getId().toString().c_str(), dest->getSpawnZoneId(user) );
		return;
	}

	// get destination coords from the zone
	sint32 x,y,z;
	float heading;
	zone->getRandomPoint( x,y,z, heading );

	// add a user to the room
	sint32 cellId;
	if ( dest->addUser(user,ownerIdx,cellId) )
	{
		// teleport the player
		if ( cellId )
		{
			user->tpWanted(x,y,z,true,heading,0xFF,cellId);
			if ( dest->isGuildRoomDestination() )
				PlayerManager.sendImpulseToClient(user->getId(), "GUILD:OPEN_INVENTORY");
		}
		else
			user->tpWanted(x,y,z,true,heading);
	}
	else
	{
		nlwarning("<BUILDING>%s error while getting destination cell id %s",user->getId().toString().c_str(), dest->getName().c_str() );
		//Note: what were these two lines for?
		//CMirrorPropValueRO<TYPE_CELL> mirrorValue( TheDataset, user->getEntityRowId(), DSPropertyCELL );
		//const sint32 oldCellId = mirrorValue;
		removePlayerFromRoom( user );
	}
}

//----------------------------------------------------------------------------
IBuildingPhysical * CBuildingManager::getBuildingPhysicalsByAlias( TAIAlias alias )
{
	std::map<TAIAlias,IBuildingPhysical*>::iterator it =  _BuildingPhysicals.find( alias );
	if ( it != _BuildingPhysicals.end() )
	{
		return (*it).second;
	}
	return NULL;
}

//----------------------------------------------------------------------------
IBuildingPhysical* CBuildingManager::getBuildingPhysicalsByName( const std::string & name )
{
	std::map<std::string,IBuildingPhysical*>::iterator it =  _BuildingPhysicalsName.find( name );
	if ( it != _BuildingPhysicalsName.end() )
	{
		if( (*it).second == NULL )
		{
			nlwarning("<BUILDING> NULL building in building maps. Checks should be done at init time");
			return NULL;
		}
		return (*it).second;
	}

	return NULL;
}

//----------------------------------------------------------------------------
uint16 CBuildingManager::getDefaultExitZone( sint32 cellId )
{
	IRoomInstance* room = getRoomInstanceFromCell( cellId );
	if ( !room )
		return InvalidSpawnZoneId;
	return room->getBuilding()->getDefaultExitSpawn();
}

//----------------------------------------------------------------------------
CBuildingPhysicalGuild * CBuildingManager::parseGuildCaretaker( const std::string & script )
{
	CBuildingPhysicalGuild * dest = dynamic_cast<CBuildingPhysicalGuild*>( getBuildingPhysicalsByName( script ) );
	if ( !dest )
	{
		nlwarning("<BUILDING>invalid building instance in a caretaker '%s'", script.c_str());
		return NULL;
	}
	return dest;
}

//----------------------------------------------------------------------------
CBuildingPhysicalPlayer * CBuildingManager::parsePlayerCaretaker( const std::string & script )
{
	CBuildingPhysicalPlayer * dest = dynamic_cast<CBuildingPhysicalPlayer*>( getBuildingPhysicalsByName( script ) );
	if ( !dest )
	{
		nlwarning("<BUILDING>invalid building instance in a caretaker '%s'", script.c_str());
		return NULL;
	}
	return dest;
}

//----------------------------------------------------------------------------
void CBuildingManager::buildBuildingTradeList(const NLMISC::CEntityId & userId, uint16 session)
{
	H_AUTO(CBuildingManager_buildTradeGuildOptions);

	// Get the player
	CCharacter * user = PlayerManager.getChar( userId );
	if ( !user )
	{
		nlwarning("<BUILDING> user %s : invalid bot %s ",userId.toString().c_str(), user->getCurrentInterlocutor().toString().c_str());
		return;
	}
	user->setAfkState(false);

	// start a new trade session
	user->setCurrentTradeSession(session);

	// start a bot chat
	CCreature * bot = CreatureManager.getCreature(user->getTarget());
	if ( !bot)
	{
		nlwarning("<BUILDING> user %s : invalid bot %s ",userId.toString().c_str(),user->getTarget().toString().c_str());
		return;
	}
	if (bot->getOutpostBuilding())
		bot = user->startBotChat( BOTCHATTYPE::TradeOutpostBuilding );
	else
		bot = user->startBotChat( BOTCHATTYPE::TradeBuildingOptions );
	if ( !bot)
	{
		nlwarning("<BUILDING> user %s : invalid bot %s ",userId.toString().c_str(),user->getCurrentInterlocutor().toString().c_str());
		return;
	}

	// build the list
	vector< CTradePhrase > & tradeList =  user->currentPhrasesTradeList();
	tradeList.clear();

	bool enableBuildingLossWarning = false;

	// *** if the bot sells guild building options
	if ( bot->getGuildBuilding() && user->getGuildId() != 0 )
	{
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
		if (!guild)
		{
			nlwarning("user %s has an invalid guild %u", userId.toString().c_str(), user->getGuildId() );
			return;
		}

		bool canBuyBuilding = true;

		if ( guild->getBuilding() != CAIAliasTranslator::Invalid )
		{
			// check rolemasters : they are sold if the guild don't own them already
//			for ( uint i = 0; i < EGSPD::CSPType::EndSPType; i++ )
//			{
//				// if role master is not there, propose it
//				if ( guild->hasRoleMaster( (EGSPD::CSPType::TSPType) i ) )
//				{
//					CTradePhrase tradePhrase;
//					tradePhrase.SheetId = _RoleMasterSheets[i];
//					tradeList.push_back( tradePhrase );
//				}
//			}

			IBuildingPhysical * building = getBuildingPhysicalsByAlias( guild->getBuilding() );
			if (building != NULL)
			{
				if (building->getAlias() == bot->getGuildBuilding()->getAlias())
					canBuyBuilding = false;
				else
					enableBuildingLossWarning = true;
			}
		}

		if (canBuyBuilding)
		{
			CTradePhrase tradePhrase;
			tradePhrase.SheetId = _GuildBuildingSheet;
			tradeList.push_back( tradePhrase );
		}
	}

	// *** if the bot sells player building options
	if ( bot->getPlayerBuilding() )
	{
		bool canBuyBuilding = true;

		if (user->getRoomInterface().getBuilding() != NULL)
		{
			if (user->getRoomInterface().getBuilding()->getAlias() == bot->getPlayerBuilding()->getAlias())
				canBuyBuilding = false;
			else
				enableBuildingLossWarning = true;
		}

		if (canBuyBuilding)
		{
			CTradePhrase tradePhrase;
			tradePhrase.SheetId = _PlayerBuildingSheet;
			tradeList.push_back( tradePhrase );
		}
	}

	// *** if the bot sells outpost building options
	if (bot->getOutpostBuilding() != NULL)
	{
		const COutpostBuilding *pOB = bot->getOutpostBuilding();
		const COutpost *pO = pOB->getParent();
		if ((pOB != NULL) && (pO != NULL))
		{
			const CStaticOutpostBuilding *pSOB = pOB->getStaticData();
			for (uint i = 0; i < pSOB->Upgrade.size(); ++i)
			{
				CTradePhrase tradePhrase;
				tradePhrase.SheetId = pSOB->Upgrade[i];
				// Check we can effectively build this building
				if (pO->canConstructBuilding(pSOB->Upgrade[i], pOB))
					tradeList.push_back( tradePhrase );
			}
		}
	}

	// *** lets fill the selling pages
	if( !tradeList.empty() )
	{
		// get nb of pages for trade list
		nlassert(NB_SLOT_PER_PAGE > 0);
		const uint16 nbPages = (uint16)ceil( double(tradeList.size()) / NB_SLOT_PER_PAGE );
		for (uint i = 0 ; i < nbPages ; ++i)
			user->addTradePageToUpdate(i);
		user->fillTradePage(session, enableBuildingLossWarning);
	}
	else
	{
		if (bot->getOutpostBuilding() != NULL)
		{
			CCharacter::sendDynamicSystemMessage( user->getId(), "EGS_CANT_CONSTRUCT_ANYTHING" );
			user->fillTradePage(session);
		}
		else
		{
			CCharacter::sendDynamicSystemMessage( user->getId(), "EGS_CANT_SELL_ANYTHING" );
		}
	}
}

//----------------------------------------------------------------------------
void CBuildingManager::buyBuildingOption(const NLMISC::CEntityId & userId, uint8 idx)
{
	// item log context
	TLogContext_Item_BuyGuildOption	itemCtx(userId);

	// get all trade parameters
	CCharacter * user = PlayerManager.getChar(userId);
	if ( !user )
	{
		nlwarning("<BUILDING> Invalid char %s",userId.toString().c_str());
		return;
	}
	user->setAfkState(false);

	if ((user->getBotChatType() != (uint8)BOTCHATTYPE::TradeBuildingOptions) &&
		(user->getBotChatType() != (uint8)BOTCHATTYPE::TradeOutpostBuilding))
	{
		nlwarning("<BUILDING> char %s is not trading guild options or outpost building!!",userId.toString().c_str());
		return;
	}

	const vector<CTradePhrase> & phrases = user->currentPhrasesTradeList();
	if ( idx >= phrases.size() )
	{
		nlwarning("<BUILDING> char %s, idx = %u, count = %u",userId.toString().c_str(),idx,phrases.size());
		return;
	}

	const CStaticItem * form = CSheets::getForm( phrases[idx].SheetId );
	if ( form == NULL )
	{
		// ok this is not an item it could be an outpost building
		const CStaticOutpostBuilding *pSOB = CSheets::getOutpostBuildingForm(phrases[idx].SheetId);
		if (pSOB == NULL)
		{
			nlwarning("<BUILDING> char %s, idx = %u, count = %u. sheet %s is invalid",userId.toString().c_str(),idx,phrases.size(),phrases[idx].SheetId.toString().c_str());
			return;
		}

		CGuildMemberModule *pGuildMember;
		if (user->getModuleParent().getModule(pGuildMember))
			pGuildMember->buyOutpostBuilding(phrases[idx].SheetId);
		return;
	}
	if ( form->GuildOption == NULL )
	{
		nlwarning("<BUILDING> char %s, idx = %u, count = %u. sheet %s has no Guild Option",userId.toString().c_str(),idx,phrases.size(),phrases[idx].SheetId.toString().c_str());
		return;
	}
	if ( form->GuildOption->Type == GUILD_OPTION::PlayerMainBuilding )
	{
		if ( user->getMoney() < form->GuildOption->MoneyCost )
		{
			nlwarning("<BUILDING> char %s, idx = %u, count = %u. sheet %s not enough money",userId.toString().c_str(),idx,phrases.size(),phrases[idx].SheetId.toString().c_str());
			return;
		}
		if ( form->GuildOption->Type == GUILD_OPTION::PlayerMainBuilding )
		{
			CCreature * bot = CreatureManager.getCreature( user->getCurrentInterlocutor() );
			if ( !bot )
			{
				nlwarning("<BUILDING> char %s no interlocutor",userId.toString().c_str());
				return;
			}
			sint32 fame = CFameInterface::getInstance().getFameIndexed( user->getId(), bot->getForm()->getFaction());
			if ( fame < MinFameToBuyPlayerBuilding )
			{
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::integer, STRING_MANAGER::race);
				params[0].Int = MinFameToBuyPlayerBuilding;
				params[1].Enum = bot->getRace();
				CCharacter::sendDynamicSystemMessage( user->getId(), "PLAYER_BUILDING_BAD_FAME", params);
				return;
			}
			user->getRoomInterface().init( user, const_cast<CBuildingPhysicalPlayer*>(bot->getPlayerBuilding()) );
			user->spendMoney( form->GuildOption->MoneyCost );
		}
	}
	else
	{
		CGuildMemberModule * module;
		if ( user->getModuleParent().getModule( module ) )
			module->buyGuildOption( form );
	}
}

//----------------------------------------------------------------------------
void CBuildingManager::buyBuilding(const NLMISC::CEntityId & userId, TAIAlias alias)
{
	CCharacter * user = PlayerManager.getChar(userId);
	user->getRoomInterface().init( user, dynamic_cast<CBuildingPhysicalPlayer*>(getBuildingPhysicalsByAlias(alias)) );
}
