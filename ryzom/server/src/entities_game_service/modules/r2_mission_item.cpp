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

#include "nel/misc/command.h"

#include "game_share/inventories.h"

#include "modules/r2_mission_item.h"
#include "modules/easter_egg.h"
#include "modules/character_control.h"
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "server_share/log_item_gen.h"
#include "egs_sheets/egs_sheets.h"

using namespace std;
using namespace NLMISC;

const uint32 MaxItemTypePerScenario = 20;


//----------------------------------------------------------------------------
CR2MissionItem::~CR2MissionItem()
{
	_ScenarioMissionItemsDef.clear();
	_OwnerOfInstanciatedItemFromScenario.clear();
}

//-----------------------------------------------------------------------------
void CR2MissionItem::itemsDescriptionsForScenario(TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem)
{
	TSessionId scenarioId = ICharacterControl::getInstance()->getSessionId( sessionId); // Just to be sure
	// Here we must have sessionId == scenarioId
	if (scenarioId != sessionId)
	{
		nlwarning("Error Linked Session %u and %u are not correctly handled", sessionId.asInt(), scenarioId.asInt());
	}

	// remove olds item definitions for this scenario if exist, and add the new definitions
	TMissionItemContainer::iterator it = _ScenarioMissionItemsDef.find( scenarioId);

	if( it != _ScenarioMissionItemsDef.end() )
	{
		endScenario( scenarioId );
	}
	_ScenarioMissionItemsDef.insert(make_pair(scenarioId, missionItem));
}

//-----------------------------------------------------------------------------
void CR2MissionItem::endScenario(TSessionId sessionId)
{
	TSessionId scenarioId = ICharacterControl::getInstance()->getSessionId( sessionId); // Just to be sure
	// Here we must have sessionId == scenarioId
	if (scenarioId != sessionId)
	{
		nlwarning("Error Linked Session %u and %u are not correctly handled", sessionId.asInt(), scenarioId.asInt());
	}


	// erase definition of mission item for this scenario
	TMissionItemContainer::iterator it = _ScenarioMissionItemsDef.find( scenarioId );
	if( it != _ScenarioMissionItemsDef.end() )
	{
		_ScenarioMissionItemsDef.erase( it );
	}

	// delete all instance of mission item for this scenario
	TMissionItemInstanciatedOwner::iterator it2 = _OwnerOfInstanciatedItemFromScenario.find( scenarioId );
	if( it2 != _OwnerOfInstanciatedItemFromScenario.end() )
	{
		for( uint32 i = 0; i < (*it2).second.size(); ++i )
		{
			_removeAllR2ItemsOfInventories( (*it2).second[i] );
		}
		_OwnerOfInstanciatedItemFromScenario.erase( it2 );
	}
}

//-----------------------------------------------------------------------------
void CR2MissionItem::_removeAllR2ItemsOfInventories(const NLMISC::CEntityId &eid, std::vector< CGameItemPtr > *items)
{
	CCharacter * c = PlayerManager.getChar( eid );
	{
		if( c != 0 )
		{
			CInventoryPtr inv = c->getInventory(INVENTORIES::bag);
			nlassert( inv != NULL );
			_removeAllR2ItemsOfInventory(inv, items);
			for( uint32 j = INVENTORIES::pet_animal; j < INVENTORIES::max_pet_animal; ++j )
			{
				inv = c->getInventory((INVENTORIES::TInventory)j);
				_removeAllR2ItemsOfInventory(inv, items);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CR2MissionItem::_removeAllR2ItemsOfInventory(CInventoryPtr inv, std::vector< CGameItemPtr > *items)
{
	if( inv != NULL )
	{
		for( uint32 i = 0; i < inv->getSlotCount(); ++ i)
		{
			CGameItemPtr itemPtr = inv->getItem(i);
			if( itemPtr != NULL )
			{
				if( itemPtr->getStaticForm()->Family == ITEMFAMILY::SCROLL_R2 )
				{
					if( items == NULL )
					{
						inv->deleteItem(i);
					}
					else
					{
						items->push_back( inv->removeItem(i) );
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CR2MissionItem::giveMissionItem(const NLMISC::CEntityId &eid, TSessionId sessionId, const std::vector< R2::TItemAndQuantity > &items)
{
	TLogContext_Item_Mission	logContext(eid);

	TSessionId scenarioId = ICharacterControl::getInstance()->getSessionId( sessionId); // Just to be sure
	// Here we must have sessionId == scenarioId
	if (scenarioId != sessionId)
	{
		nlwarning("Error Linked Session %u and %u are not correctly handled", sessionId.asInt(), scenarioId.asInt());
	}

	// give an mission item o a scenario to a player character,
	//and give a reference of where the item is with a reference inventory on this player character
	CCharacter *c = PlayerManager.getChar(eid);
	if( c != 0 )
	{
		std::vector< CGameItemPtr > itemDropToEgg;
		for( uint32 j = 0; j < items.size(); ++j )
		{
			const CStaticItem* sitem = CSheets::getForm(items[j].SheetId);
			if (sitem == NULL)
			{
				nlwarning("Attempted to give deprecated sitem sheet %s to player character %s in session %i", items[j].SheetId.toString().c_str(), c->getName().toUtf8().c_str(), sessionId.asInt());
			}
			else if (sitem->Family != ITEMFAMILY::SCROLL_R2)
			{
				nlwarning("Attempted hack to give non-R2 item %s to player character %s in session %i", items[j].SheetId.toString().c_str(), c->getName().toUtf8().c_str(), sessionId.asInt());
			}
			else
			{
				CGameItemPtr item = c->createItem(1, items[j].Quantity, items[j].SheetId);

				if( item != NULL )
				{
					if( c->addItemToInventory(INVENTORIES::bag, item) )
					{
	/*					// check eid is registered as character have instantiated mission item for this scenario
						TMissionItemInstanciatedOwner::iterator it = _OwnerOfInstanciatedItemFromScenario.find(scenarioId);
						if( it == _OwnerOfInstanciatedItemFromScenario.end() )
						{
							pair< TMissionItemInstanciatedOwner::iterator, bool > ret = _OwnerOfInstanciatedItemFromScenario.insert( make_pair( scenarioId, vector< CEntityId >() ) );
							if( ret.second )
							{
								(*ret.first).second.push_back( eid );
							}
						}
						else
						{
							bool found = false;
							for( uint32 i = 0; i < (*it).second.size(); ++ i )
							{
								if( (*it).second[i] == eid )
								{
									found = true;
									break;
								}
							}
							if ( ! found) { (*it).second.push_back(eid); }
						}
	*/
						keepR2ItemAssociation(eid, scenarioId);
					}
					else
					{
						itemDropToEgg.push_back(item);
					}
				}
				else
				{
					nlwarning("CR2MissionItem::giveMissionItem: can't create item %s", items[j].SheetId.toString().c_str());
				}
			}
		}
		if(itemDropToEgg.size() != 0)
		{
			CR2EasterEgg::getInstance().dropMissionItem(itemDropToEgg, scenarioId, c->getInstanceNumber(), eid);
		}
	}
}

//-----------------------------------------------------------------------------
void CR2MissionItem::characterDisconnection(const NLMISC::CEntityId &eid)
{
	CCharacter *c = PlayerManager.getChar(eid);
	if( c != 0 )
	{
		// on character disconnection, delete all instantiated item mission item he have.
		for( TMissionItemInstanciatedOwner::iterator ito = _OwnerOfInstanciatedItemFromScenario.begin(); ito != _OwnerOfInstanciatedItemFromScenario.end(); ++ito )
		{
			for( uint32 i = 0; i < (*ito).second.size(); ++i )
			{
				if( eid == (*ito).second[i])
				{
					vector< CGameItemPtr > missionItems;
					_removeAllR2ItemsOfInventories( eid, &missionItems );
					if( missionItems.size() != 0)
					{
						TSessionId sessionId = (*ito).first;
						TSessionId scenarioId = ICharacterControl::getInstance()->getSessionId( sessionId); // Just to be sure
						// Here we must have sessionId == scenarioId
						if (scenarioId != sessionId)
						{
							nlwarning("Error Linked Session %u and %u are not correctly handled", sessionId.asInt(), scenarioId.asInt());
						}
						CR2EasterEgg::getInstance().dropMissionItem(missionItems, scenarioId, c->getInstanceNumber(), eid);
					}
					_OwnerOfInstanciatedItemFromScenario.erase(ito);
					return;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
const R2::TMissionItem * CR2MissionItem::getR2ItemDefinition( TSessionId sessionId, const NLMISC::CSheetId &itemSheet ) const
{
	TSessionId scenarioId = ICharacterControl::getInstance()->getSessionId( sessionId); // Just to be sure
	// Here we must have sessionId == scenarioId
	if (scenarioId != sessionId)
	{
		nlwarning("Error Linked Session %u and %u are not correctly handled", sessionId.asInt(), scenarioId.asInt());
	}

	TMissionItemContainer::const_iterator it = _ScenarioMissionItemsDef.find( scenarioId );
	if( it != _ScenarioMissionItemsDef.end() )
	{
		const vector<R2::TMissionItem>& itemDesc = (*it).second;
		for( uint32 i = 0; i < itemDesc.size(); ++i )
		{
			if( itemDesc[i].SheetId == itemSheet )
				return &itemDesc[i];
		}
	}
	nlwarning("CR2MissionItem::getR2ItemDefinition: can't found definition for item %s", itemSheet.toString().c_str());
	return 0;
}

//----------------------------------------------------------------------------
void CR2MissionItem::destroyMissionItem(const NLMISC::CEntityId &eid, const std::vector< R2::TItemAndQuantity > &items)
{
	CCharacter *c = PlayerManager.getChar(eid);
	if( c != 0 )
	{
		for( uint32 j = 0; j < items.size(); ++j )
		{
			CSheetId itemSheetId = items[j].SheetId;
			uint32 quantity = items[j].Quantity;

			const CStaticItem* sitem = CSheets::getForm(items[j].SheetId);
			if (sitem == NULL)
			{
				nlwarning("Attempted to take deprecated sitem sheet %s from player character %s", items[j].SheetId.toString().c_str(), c->getName().toUtf8().c_str());
			}
			else if (sitem->Family != ITEMFAMILY::SCROLL_R2)
			{
				nlwarning("Attempted hack to take non-R2 item %s from player character %s", items[j].SheetId.toString().c_str(), c->getName().toUtf8().c_str());
			}
			else
			{
				CInventoryPtr inv = c->getInventory(INVENTORIES::bag);
				nlassert( inv != NULL );
				_destroyMissionItem( inv, itemSheetId, quantity );
				if( quantity > 0)
				{
					for( uint32 j = INVENTORIES::pet_animal; j < INVENTORIES::max_pet_animal; ++j )
					{
						inv = c->getInventory((INVENTORIES::TInventory)j);
						nlassert(inv != NULL);
						_destroyMissionItem( inv, itemSheetId, quantity );
						if(quantity == 0)
							break;
					}
				}
				// TODO: if we can't found enough quantity of item to destroy, we need decide if we must manage that as an error
	//			if(quantity > 0)
	//			{
	//			}
			}
		}
	}
}

//----------------------------------------------------------------------------
void CR2MissionItem::_destroyMissionItem( CInventoryPtr inv, const NLMISC::CSheetId &itemSheetId, uint32 &quantity )
{
	for( uint32 i = 0; i < inv->getSlotCount(); ++ i)
	{
		CGameItemPtr itemPtr = inv->getItem(i);
		if( itemPtr != NULL )
		{
			if( itemPtr->getStaticForm()->Family == ITEMFAMILY::SCROLL_R2 )
			{
				if( itemPtr->getSheetId() == itemSheetId )
				{
					quantity -= inv->deleteStackItem(i, quantity);
					if(quantity == 0)
						break;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
uint32 CR2MissionItem::getNumberMissionItem(const NLMISC::CEntityId &eid, const NLMISC::CSheetId &itemSheetId) const
{
	uint32 numberItem = 0;
	CCharacter *c = PlayerManager.getChar(eid);
	if( c != 0 )
	{
		CInventoryPtr inv = c->getInventory(INVENTORIES::bag);
		nlassert( inv != NULL );
		numberItem = _getNumberMissionItem( inv, itemSheetId );
		for( uint32 j = INVENTORIES::pet_animal; j < INVENTORIES::max_pet_animal; ++j )
		{
			inv = c->getInventory((INVENTORIES::TInventory)j);
			nlassert(inv != NULL);
			numberItem += _getNumberMissionItem( inv, itemSheetId );
		}
	}
	return numberItem;
}

//----------------------------------------------------------------------------
uint32 CR2MissionItem::_getNumberMissionItem(CInventoryPtr inv, const NLMISC::CSheetId &itemSheetId) const
{
	uint32 numberItem = 0;
	for( uint32 i = 0; i < inv->getSlotCount(); ++ i)
	{
		const CGameItemPtr itemPtr = inv->getItem(i);
		if( itemPtr != NULL )
		{
			if( itemPtr->getStaticForm()->Family == ITEMFAMILY::SCROLL_R2 )
			{
				if( itemPtr->getSheetId() == itemSheetId )
				{
					numberItem += itemPtr->getStackSize();
				}
			}
		}
	}
	return numberItem;
}

//----------------------------------------------------------------------------
void CR2MissionItem::getMissionItemOwnedByCharacter(const NLMISC::CEntityId & eid, std::vector< R2::TItemAndQuantity > &items) const
{
	CCharacter *c = PlayerManager.getChar(eid);
	if( c != 0 )
	{
		CInventoryPtr inv = c->getInventory(INVENTORIES::bag);
		nlassert( inv != NULL );
		_getMissionItemOwnedByCharacter( inv, items );
		for( uint32 j = INVENTORIES::pet_animal; j < INVENTORIES::max_pet_animal; ++j )
		{
			inv = c->getInventory((INVENTORIES::TInventory)j);
			nlassert(inv != NULL);
			_getMissionItemOwnedByCharacter( inv, items );
		}
	}
}

//----------------------------------------------------------------------------
void CR2MissionItem::_getMissionItemOwnedByCharacter( CInventoryPtr inv, std::vector< R2::TItemAndQuantity > &items) const
{
	for( uint32 i = 0; i < inv->getSlotCount(); ++ i)
	{
		const CGameItemPtr itemPtr = inv->getItem(i);
		if( itemPtr != NULL )
		{
			if( itemPtr->getStaticForm()->Family == ITEMFAMILY::SCROLL_R2 )
			{
				bool found = false;
				for( uint32 j = 0; j < items.size(); ++j )
				{
					if(items[j].SheetId == itemPtr->getSheetId())
					{
						items[j].Quantity += itemPtr->getStackSize();
						found = true;
						break;
					}
				}
				if(!found)
				{
					items.push_back(R2::TItemAndQuantity(itemPtr->getSheetId(), itemPtr->getStackSize()));
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
void CR2MissionItem::dmGiftBegin( NLMISC::CEntityId eid )
{
	CCharacter *c = PlayerManager.getChar( eid );
	if( !c )
	{
		nlwarning("DM_GIFT:ITEM: can't found DM character %s", eid.toString().c_str());
		return;
	}
	CEntityId target = c->getTarget();
	if( target == CEntityId::Unknown )
	{
		nlwarning("DM_GIFT:ITEM: can't found DM targeted character %s", target.toString().c_str());
		return;
	}
	if( target.getType() != RYZOMID::player )
	{
		nlwarning("DM_GIFT:ITEM: DM targeted %s is not a player charcter", target.toString().c_str());
		return;
	}

	map< CEntityId, CEntityId >::iterator it = _DMSelectedPlayer.find( eid );
	if( it == _DMSelectedPlayer.end() )
	{
		_DMSelectedPlayer.insert( make_pair( eid, target ) );
	}
	else
	{
		(*it).second = target;
	}
}

//----------------------------------------------------------------------------
void CR2MissionItem::dmGiftValidate( NLMISC::CEntityId eid, std::vector< R2::TItemAndQuantity > items )
{
	map< CEntityId, CEntityId >::iterator it = _DMSelectedPlayer.find( eid );
	if( it == _DMSelectedPlayer.end() )
	{
		nlwarning("DM_GIFT:ITEM: can't found DM(%s) targeted character", eid.toString().c_str());
		return;
	}

	CCharacter *c = PlayerManager.getChar( (*it).second );
	if( !c )
	{
		nlwarning("DM_GIFT:ITEM: can't found DM(%s) targeted character %s", eid.toString().c_str(), (*it).second.toString().c_str());
		return;
	}
	TSessionId sessionId = ICharacterControl::getInstance()->getSessionId( c->currentSessionId());
	// sessionId() don't work with edition session
	giveMissionItem((*it).second, sessionId, items);
	_DMSelectedPlayer.erase(it);
}

//----------------------------------------------------------------------------
void CR2MissionItem::keepR2ItemAssociation(const NLMISC::CEntityId& eid, TScenarioId scenarioId)
{
	// check eid is registered as character have instantiated mission item for this scenario
	TMissionItemInstanciatedOwner::iterator it = _OwnerOfInstanciatedItemFromScenario.find(scenarioId);
	if( it == _OwnerOfInstanciatedItemFromScenario.end() )
	{
		pair< TMissionItemInstanciatedOwner::iterator, bool > ret = _OwnerOfInstanciatedItemFromScenario.insert( make_pair( scenarioId, vector< CEntityId >() ) );
		if( ret.second )
		{
			(*ret.first).second.push_back( eid );
		}
	}
	else
	{
		bool found = false;
		for( uint32 i = 0; i < (*it).second.size(); ++ i )
		{
			if( (*it).second[i] == eid )
			{
				found = true;
				break;
			}
		}
		if ( ! found) { (*it).second.push_back(eid); }
	}
}

//============================================================================

NLMISC_COMMAND(itemR2Description, "set definition of a R2 item", "<ScenarioId> <SheetId> <Name> <Description> <Comment>")
{
	if( args.size() != 5 )
		return false;

	uint32 sessionId;
	NLMISC::fromString(args[0], sessionId);
	TSessionId scenarioId(sessionId);

	R2::TMissionItem itemDesc;
	itemDesc.SheetId = CSheetId(args[1]);
	itemDesc.Name = args[2];
	itemDesc.Description = args[3];
	itemDesc.Comment = args[4];

	vector<R2::TMissionItem> missionItem;
	missionItem.push_back(itemDesc);

	CR2MissionItem::getInstance().itemsDescriptionsForScenario(scenarioId, missionItem);
	return true;
}

NLMISC_COMMAND(giveItemR2ToCharacter, "Give an item R2 to a character", "<Character Eid> <ScenarioId> <SheetID> <Quantity>")
{
	if( args.size() != 4)
		return false;

	CEntityId eid;
	eid.fromString(args[0].c_str());
	uint32 sessionId;
	NLMISC::fromString(args[1], sessionId);
	TSessionId scenarioId(sessionId);
	R2::TItemAndQuantity item;
	item.SheetId = CSheetId(args[2]);
	NLMISC::fromString(args[3], item.Quantity);

	std::vector< R2::TItemAndQuantity > items;
	items.push_back(item);

	CR2MissionItem::getInstance().giveMissionItem(eid, scenarioId, items);
	return true;
}

NLMISC_COMMAND(characterDisconnectionForR2MissionItem, "Simulate a disconnection of character for R2 item", "<Character Eid>")
{
	if( args.size() != 1)
		return false;

	CEntityId eid;
	eid.fromString(args[0].c_str());

	CR2MissionItem::getInstance().characterDisconnection(eid);
	return true;
}

NLMISC_COMMAND(endScenario, "Simulate the end of a R2 scenario", "<ScenarioId>")
{
	if( args.size() != 1)
		return false;

	uint32 sessionId;
	NLMISC::fromString(args[0], sessionId);
	TSessionId scenarioId(sessionId);
	CR2MissionItem::getInstance().endScenario( scenarioId );
	return true;
}

NLMISC_COMMAND(destroyMissionR2Item, "Destroy an R2 itme mission owned by a player character", "<Character Eid> <SheetId> <Quantity>")
{
	if( args.size() != 3 )
		return false;

	NLMISC::CEntityId eid;
	eid.fromString(args[0].c_str());
	R2::TItemAndQuantity item;
	item.SheetId = CSheetId(args[1]);
	NLMISC::fromString(args[2], item.Quantity);
	std::vector<R2::TItemAndQuantity> items;
	items.push_back(item);

	CR2MissionItem::getInstance().destroyMissionItem(eid, items);
	return true;
}

NLMISC_COMMAND(getNumberMissionItem, "return number of R2 mission item of asked type a player character have","<Character Eid> <SheetId>")
{
	if( args.size() != 2 )
		return false;

	NLMISC::CEntityId eid;
	eid.fromString(args[0].c_str());
	CSheetId sheet = CSheetId(args[1]);

	uint32 nbItem = CR2MissionItem::getInstance().getNumberMissionItem(eid, sheet);

	log.displayNL("Number of item type: %s player character: %s have is %u", sheet.toString().c_str(), eid.toString().c_str(), nbItem );
	return true;
}
