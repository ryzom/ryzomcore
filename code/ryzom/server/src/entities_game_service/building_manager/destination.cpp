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
#include "destination.h"
#include "building_manager/building_physical.h"
#include "zone_manager.h"
#include "building_manager/building_manager.h"
#include "player_manager/character.h"
#include "building_manager/room_instance.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CTPDestination);
NL_INSTANCE_COUNTER_IMPL(CBuildingDestination);
NL_INSTANCE_COUNTER_IMPL(CRoomDestination);
NL_INSTANCE_COUNTER_IMPL(CExitDestination);

/*****************************************************************************/
//						IDestination implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
bool IDestination::addUser(CCharacter * user,uint16 ownerIdx, sint32 & cellId)
{
	cellId = 0;
	return true;
}

/*****************************************************************************/
//						IDescribedDestination implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
bool IDescribedDestination::buildDescription(const NLLIGO::IPrimitive* prim)
{
	// get properties
	string value;
	bool ret = true;
	nlverify( prim->getPropertyByName("icon",value) );
	_Icon = LIFT_ICONS::toLiftIcon( value );
	if ( _Icon == LIFT_ICONS::Unknown )
	{
		nlwarning("<BUILDING> : invalid icon '%s' in destination '%s'",value.c_str(), _Name.c_str() );
		ret = false;
	}
	nlverify( prim->getPropertyByName("place_name",_PhraseId));
	return ret;
}


/*****************************************************************************/
//						CTPDestination implementation
/*****************************************************************************/


//----------------------------------------------------------------------------
bool CTPDestination::build( const NLLIGO::IPrimitive* prim,const NLLIGO::IPrimitive* roomPrim, const CBuildingParseData & parseData )
{
	// get properties
	string value;
	bool ret = IDescribedDestination::buildDescription( prim );
	
	nlverify( prim->getPropertyByName("pets_allowed",value) );
	_TeleportPets = (value == "true");
	
	nlverify( prim->getPropertyByName("teleport_spawn_zone",value));
	_ArrivalSpawn = CZoneManager::getInstance().getTpSpawnZoneIdByName( value );
	if ( _ArrivalSpawn == InvalidSpawnZoneId )
	{
		nlwarning("<BUILDING> : in destination '%s' : invalid area '%s'", _Name.c_str(),value.c_str() );
		ret = false;
	}
	return ret;
}

//----------------------------------------------------------------------------
bool CTPDestination::arePetsAllowed()const
{
	return _TeleportPets;
} 

//----------------------------------------------------------------------------
uint16 CTPDestination::getSpawnZoneId(const CCharacter * user)const 
{ 
	return _ArrivalSpawn; 
}

//----------------------------------------------------------------------------
void CTPDestination::getClientDescription(uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const
{
#ifdef NL_DEBUG
	nlassert(user);
#endif
	icon = UINT64_CONSTANT(0x8000000000000000) + _Icon;
	textId = CZoneManager::getInstance().sendPlaceName( user->getEntityRowId(), _PhraseId );
}


/*****************************************************************************/
//						CBuildingDestination implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
uint16 CBuildingDestination::getStateCounter()const
{
	return _ArrivalBuilding->getStateCounter();
}

//----------------------------------------------------------------------------
void CBuildingDestination::getClientDescription(uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const
{
#ifdef NL_DEBUG
	nlassert(user);
#endif
	_ArrivalBuilding->getClientDescription( _ArrivalRoomIndex, ownerIndex,user,icon,textId );
}

//----------------------------------------------------------------------------
uint16 CBuildingDestination::getSpawnZoneId(const CCharacter * user)const
{
	return _ArrivalSpawn;
}

//----------------------------------------------------------------------------
bool CBuildingDestination::build( const NLLIGO::IPrimitive* prim , const NLLIGO::IPrimitive* roomPrim, const CBuildingParseData & parseData)
{
	// get properties
	string value;
	bool ret = true;
	nlverify( prim->getPropertyByName("arrival_spawn_zone",value) );
	_ArrivalSpawn = CZoneManager::getInstance().getTpSpawnZoneIdByName( value );
	if ( _ArrivalSpawn == InvalidSpawnZoneId )
	{
		nlwarning("<BUILDING> : in destination '%s' : invalid spawn zone '%s'", _Name.c_str(),value.c_str() );
		ret = false;
	}
	nlverify( prim->getPropertyByName("building_instance",value) );
	_ArrivalBuilding = CBuildingManager::getInstance()->getBuildingPhysicalsByName( value );
	if ( _ArrivalBuilding )
	{
		nlverify( prim->getPropertyByName("arrival_room",value) );
		for ( uint i = 0; i < _ArrivalBuilding->getTemplate()->Rooms.size(); i++ )
		{
			if ( _ArrivalBuilding->getTemplate()->Rooms[i].Name == value )
			{
				_ArrivalRoomIndex = i;
				return true;
			}
		}
		nlwarning("<BUILDING> : in destination '%s' : invalid room '%s'", _Name.c_str(),value.c_str() );
		return false;
	}
	nlwarning("<BUILDING> : in destination '%s' : invalid building instance '%s'", _Name.c_str(),value.c_str() );
	return false;
}

//----------------------------------------------------------------------------
bool CBuildingDestination::addUser(CCharacter * user,uint16 ownerIdx, sint32 & cellId)
{
	return _ArrivalBuilding->addUser(user,_ArrivalRoomIndex,ownerIdx,cellId);
}

//----------------------------------------------------------------------------
uint16 CBuildingDestination::getEntryCount() const
{
	return _ArrivalBuilding->getOwnerCount();
}

//----------------------------------------------------------------------------
bool CBuildingDestination::isUserAllowed(CCharacter * user,uint16 ownerIdx)
{
	return _ArrivalBuilding->isUserAllowed(user,ownerIdx,_ArrivalRoomIndex);
}



/*****************************************************************************/
//						CRoomDestination implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
bool CRoomDestination::isGuildRoomDestination()
{
	return _Template->Type == BUILDING_TYPES::Guild;
}

//----------------------------------------------------------------------------
void CRoomDestination::getClientDescription( uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const
{
#ifdef NL_DEBUG
	nlassert(user);
#endif
	/// get the room where the user is
	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;			
	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
	if ( !room )
	{
		nlwarning("<BUILDING> invalid cell %d for user %s. in destination '%s'", cell, user->getId().toString().c_str(),_Name.c_str() );
		return;
	}
	nlassert(room->getBuilding());
	nlassert( _ArrivalRoomIndex < room->getBuilding()->getTemplate()->Rooms.size() );
	
	const CRoomTemplate & roomTempl = room->getBuilding()->getTemplate()->Rooms[_ArrivalRoomIndex];
	
	icon = UINT64_CONSTANT(0x8000000000000000) + roomTempl.Icon;
	textId = CZoneManager::getInstance().sendPlaceName( user->getEntityRowId(), roomTempl.PhraseId );
}

//----------------------------------------------------------------------------
uint16 CRoomDestination::getSpawnZoneId(const CCharacter * user)const
{
	return _ArrivalSpawn;
}

//----------------------------------------------------------------------------
bool CRoomDestination::build( const NLLIGO::IPrimitive* prim,const NLLIGO::IPrimitive* roomPrim, const CBuildingParseData & parseData )
{
	string value;
	bool ret = true;
	// get spawn zone
	nlverify( prim->getPropertyByName("teleport_spawn_zone",value) );
	_ArrivalSpawn = CZoneManager::getInstance().getTpSpawnZoneIdByName( value );
	if ( _ArrivalSpawn == InvalidSpawnZoneId )
	{
		nlwarning("<BUILDING> in destination '%s' invalid spawn zone '%s'", _Name.c_str(), value.c_str() );
		ret = false;
	}

	// get parent building
	nlassert( roomPrim );
	nlassert( roomPrim->getParent() );
	nlverify( roomPrim->getParent()->getPropertyByName("name",value) );
	map<string,CBuildingTemplate*>::const_iterator it = parseData.BuildingTemplates.find( value );
	if ( it == parseData.BuildingTemplates.end() || (*it).second == NULL )
	{
		nlwarning("<BUILDING> in destination '%s' invalid building '%s'", _Name.c_str(), value.c_str() );
		return false;
	}
	_Template = (*it).second;
	// get parent room
	string parentRoom;
	nlverify( roomPrim->getPropertyByName("name",parentRoom) );
	nlverify( prim->getPropertyByName("room",value) );
	
	_ArrivalRoomIndex = _StartRoomIndex = 0xFFFF;
	for ( uint i = 0; i < _Template->Rooms.size(); i++ )
	{
		if  ( _Template->Rooms[i].Name == value )
		{
			_ArrivalRoomIndex = i;
		}
		else if( _Template->Rooms[i].Name == parentRoom )
		{
			_StartRoomIndex = i;
		}
	}
	if ( _StartRoomIndex == 0xFFFF )
	{
		nlwarning("<BUILDING> in destination '%s' parent room %s' is invalid", _Name.c_str(), parentRoom.c_str() );
		ret = false;
	}
	if ( _ArrivalRoomIndex == 0xFFFF )
	{
		nlwarning("<BUILDING> in destination '%s' room '%s' is invalid", _Name.c_str(), value.c_str() );
		ret = false;
	}
	return ret;
}

//----------------------------------------------------------------------------
bool CRoomDestination::addUser(CCharacter * user,uint16 ownerIdx, sint32 & cellId)
{
	/// get the room where the user is
	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;			
	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
	if ( !room )
	{
		nlwarning("<BUILDING> invalid cell %d for user %s. in destination '%s'", cell, user->getId().toString().c_str(),_Name.c_str() );
		return false;;
	}
	nlassert(room->getBuilding());
	return room->getBuilding()->addUser(user,_ArrivalRoomIndex,room->getOwnerIndex(),cellId);
}

//----------------------------------------------------------------------------
bool CRoomDestination::isUserAllowed(CCharacter * user,uint16 ownerIdx)
{
#ifdef NL_DEBUG
	nlassert(user);
#endif

	if (user && user->isDead())
		return false;

	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;			
	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
	if ( !room )
	{
		nlwarning("<BUILDING> invalid cell %d for user %s. in destination '%s'", cell, user->getId().toString().c_str(),_Name.c_str() );
		return false;
	}
	nlassert(room->getBuilding());
	if ( _StartRoomIndex != room->getRoomIndex() )
		return false;
	if ( _Template != room->getBuilding()->getTemplate() )
		return false;

	return room->getBuilding()->isUserAllowed(user,room->getOwnerIndex(),_ArrivalRoomIndex);
}


/*****************************************************************************/
//						CExitDestination implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
void CExitDestination::getClientDescription( uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const
{
#ifdef NL_DEBUG
	nlassert(user);
#endif
	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;			
	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
	if ( !room )
	{
		nlwarning("<BUILDING> invalid cell %d for user %s. in destination '%s'", cell, user->getId().toString().c_str(),_Name.c_str() );
		return;
	}
	nlassert(room->getBuilding());
	const IDestination* exit = room->getBuilding()->getExit( _ExitIndex );
	if ( !exit )
	{
		nlwarning("<BUILDING> %s asked for exit %u in destination '%s'", user->getId().toString().c_str(), _ExitIndex ,_Name.c_str());
		return;
	}
	exit->getClientDescription( ownerIndex,user,icon,textId );
}

//----------------------------------------------------------------------------
uint16 CExitDestination::getSpawnZoneId(const CCharacter * user)const
{
#ifdef NL_DEBUG
	nlassert(user);
#endif
	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;			
	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
	if ( !room )
	{
		nlwarning("<BUILDING> invalid cell %d for user %s. in destination '%s'", cell, user->getId().toString().c_str(),_Name.c_str() );
		return InvalidSpawnZoneId;
	}
	nlassert(room->getBuilding());
	const IDestination* exit = room->getBuilding()->getExit( _ExitIndex );
	if ( !exit )
	{
		nlwarning("<BUILDING> %s asked for exit %u in destination '%s'", user->getId().toString().c_str(), _ExitIndex ,_Name.c_str());
		return InvalidSpawnZoneId;
	}
	return exit->getSpawnZoneId(user);
}


//----------------------------------------------------------------------------
bool CExitDestination::build( const NLLIGO::IPrimitive* prim,const NLLIGO::IPrimitive* roomPrim, const CBuildingParseData & parseData )
{
	string value;
	// get properties
	nlverify( prim->getPropertyByName("exit_index_in_instance",value) );
	NLMISC::fromString(value, _ExitIndex);
	--_ExitIndex;
	if ( _ExitIndex == 0xFF )
	{
		nlwarning("<BUILDING> in destination '%s' invalid building '%s' starts at 1", _Name.c_str(), value.c_str() );
		return false;
	}
	// get parent building
	nlassert( roomPrim );
	nlassert( roomPrim->getParent() );
	nlverify( roomPrim->getParent()->getPropertyByName("name",value) );
	map<string,CBuildingTemplate*>::const_iterator it = parseData.BuildingTemplates.find( value );
	_Template = (*it).second ;
	if ( it == parseData.BuildingTemplates.end() || _Template == NULL )
	{
		nlwarning("<BUILDING> in destination '%s' invalid building '%s'", _Name.c_str(), value.c_str() );
		return false;
	}
	// get parent room index
	string parentRoom;
	nlverify( roomPrim->getPropertyByName("name",parentRoom) );
	
	_StartRoomIndex = 0xFFFF;
	for ( uint i = 0; i < (*it).second->Rooms.size(); i++ )
	{
		if( (*it).second->Rooms[i].Name == parentRoom )
			_StartRoomIndex = i;
	}
	if ( _StartRoomIndex == 0xFFFF )
	{
		nlwarning("<BUILDING> in destination '%s' parent room %s' is invalid", _Name.c_str(), parentRoom.c_str() );
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
bool CExitDestination::isUserAllowed(CCharacter * user, uint16 ownerIdx)
{
	if (user && user->isDead())
		return false;

	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;			
	const IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
	if ( !room )
	{
		nlwarning("<BUILDING> invalid cell %d for user %s. in destination '%s'", cell, user->getId().toString().c_str(),_Name.c_str() );
		return false;
	}
	nlassert( room->getBuilding() );
	return ( room->getBuilding()->getTemplate() == _Template && room->getRoomIndex() == _StartRoomIndex );
}
