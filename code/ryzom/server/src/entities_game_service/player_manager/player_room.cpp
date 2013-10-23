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
#include "player_manager/player_room.h"
#include "building_manager/building_manager.h"
#include "player_manager/character.h"
#include "player_manager/player.h"
#include "player_manager/player_manager.h"
#include "building_manager/building_physical.h"
#include "building_manager/room_instance.h"
#include "game_item_manager/player_inv_room.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CPlayerRoomInterface);

//----------------------------------------------------------------------------
CPlayerRoomInterface::CPlayerRoomInterface()
: _Data(NULL)
{
}

//----------------------------------------------------------------------------
CPlayerRoomInterface::~CPlayerRoomInterface()
{
	if (_Data != NULL)
	{
		_Data->clear();
		delete _Data;
	}
}

//----------------------------------------------------------------------------
void CPlayerRoomInterface::store(CPersistentDataRecord & pdr) const
{
	if (_Data == NULL)
		return;
	_Data->store(pdr);
}

//----------------------------------------------------------------------------
void CPlayerRoomInterface::apply(CPersistentDataRecord & pdr, CCharacter * owner)
{
	if (_Data == NULL)
	{
		_Data = new CPlayerRoomData(owner, NULL);
	}
	_Data->apply(pdr);

	// if player has no building, reset _Data
	if (_Data->Building == NULL)
	{
		delete _Data;
		_Data = NULL;
	}
}

//----------------------------------------------------------------------------
void CPlayerRoomInterface::clear()
{
	if (_Data != NULL)
		_Data->clear();
}

//----------------------------------------------------------------------------
void CPlayerRoomInterface::init(CCharacter * owner, CBuildingPhysicalPlayer * building)
{
	nlassert(owner != NULL);

	if (_Data != NULL)
	{
		CBuildingManager::getInstance()->removePlayerBuilding(owner->getId());
		_Data->clear();
		delete _Data;
	}

	_Data = new CPlayerRoomData(owner, building);

	// update the destination by registering this room
	_Data->Building->addPlayer(owner->getId());
}

//----------------------------------------------------------------------------
//void CPlayerRoomInterface::serial(NLMISC::IStream & f, CCharacter * owner, uint16 playerVersion)
//{
//	f.xmlPush("room");
//	if ( f.isReading() )
//	{
//		bool hasRoom;
//		f.serial(hasRoom);
//		if ( hasRoom )
//		{
//			_Data = new CPlayerRoomData(owner, NULL);
//			_Data->serial(f, owner, playerVersion);
//		}
//	}
//	else
//	{
//		if ( _Data )
//		{
//			bool hasRoom = true;
//			f.serial(hasRoom);
//			_Data->serial(f, owner, playerVersion);
//		}
//		else
//		{
//			bool hasRoom = false;
//			f.serial(hasRoom);
//		}
//	}
//	
//	f.xmlPop();
//}

//----------------------------------------------------------------------------
bool CPlayerRoomInterface::isAllowedInBuilding(const CCharacter * owner, const CCharacter * user)
{
	// for the moment only owner is allowed in the room
	return (owner == user);
}

//----------------------------------------------------------------------------
CPlayerRoomInterface::CPlayerRoomData::CPlayerRoomData(CCharacter * owner, CBuildingPhysicalPlayer * building)
: Building(building)
{
	nlassert(owner != NULL);
	initInventory(owner);
}

//----------------------------------------------------------------------------
void CPlayerRoomInterface::CPlayerRoomData::initInventory(CCharacter * owner)
{
	nlassert(owner != NULL);
	nlassert(Inventory == NULL);

	// create the inventory
	Inventory = new CPlayerRoomInventory(owner);
	Inventory->setInventoryId(INVENTORIES::player_room);
	Inventory->setSlotCount(Inventory->getMaxSlot());

	// create the inventory view
	CPlayerRoomInvView * view = new CPlayerRoomInvView;
	view->setCharacter(owner);
	view->bindToInventory(Inventory);
}

//----------------------------------------------------------------------------
void CPlayerRoomInterface::CPlayerRoomData::clear()
{
	if (Inventory != NULL)
		Inventory->clearInventory();
}

//----------------------------------------------------------------------------
//void CPlayerRoomInterface::CPlayerRoomData::serial(NLMISC::IStream & f, CCharacter * owner, uint16 playerVersion)
//{
//	// We do not support saving this way
//	nlassert(f.isReading());
//
//	f.xmlPush("dest");
//	if ( f.isReading() )
//	{
//		initInventory(owner);
//		if ( playerVersion <= 58 )
//		{
//			string name;
//			f.serial(name);
//			Building = dynamic_cast<CBuildingPhysicalPlayer*> ( CBuildingManager::getInstance()->getBuildingPhysicalsByName( name ) );
//		}
//		else
//		{	
//			TAIAlias alias;
//			f.serial(alias);
//			if ( alias != CAIAliasTranslator::Invalid )
//			{
//				Building = dynamic_cast<CBuildingPhysicalPlayer*> ( CBuildingManager::getInstance()->getBuildingPhysicalsByAlias( alias ) );
//			}
//		}
////		Inventory->load( f, userId, playerVersion );
//		Inventory->legacyLoad( f, playerVersion, NULL );
//	}
//	else
//	{
//		nlassert(false);
//		TAIAlias alias = CAIAliasTranslator::Invalid;
//		if ( Building )
//			 alias = Building->getAlias();
//		f.serial( alias );
////		Inventory->save(f);
//	}
//	
//	//f.serial(Bulk);
//	//f.serial(MaxBulk);
//	sint32 dummy;
//	f.serial(dummy);
//	f.serial(dummy);
//
//	f.xmlPop();
//}

//----------------------------------------------------------------------------
bool CPlayerRoomInterface::canUseInventory(const CCharacter * owner, const CCharacter * user) const
{
	/// only the owner can use its room, and only if he is inside
	if (owner != user)
		return false;

	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( user->getId() ));
	BOMB_IF(p == NULL, "Failed to find player record for character: " << user->getId().toString(), return true);
	if ( p->isTrialPlayer() )
	{
		user->sendDynamicSystemMessage( user->getId(), "EGS_CANT_USE_ROOM_INV_IS_TRIAL_PLAYER" );
		return false;
	}

	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	const sint32 cell = mirrorCell;
	if ( !CBuildingManager::getInstance()->isRoomCell(cell) )
		return false;

	const CRoomInstancePlayer * room = dynamic_cast<CRoomInstancePlayer*>( CBuildingManager::getInstance()->getRoomInstanceFromCell( cell ) );
	if (room == NULL)
		return false;

	if (room->getPlayer() != user->getId())
		return false;

	return true;
}
