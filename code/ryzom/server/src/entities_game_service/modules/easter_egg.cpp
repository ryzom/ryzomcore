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
#include "nel/net/unified_network.h"
#include "nel/misc/variable.h"

#include "world_instances.h"
#include "easter_egg.h"
#include "game_item_manager/game_item.h"
#include "game_item_manager/game_item_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "creature_manager/creature_manager.h"
#include "game_share/scenario.h"
#include "modules/character_control.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


void cbEasterEggSpawned(CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

NLMISC::CVariable<string> EaterEggBagSheet("egs","EaterEggBagSheet", "name of the sheet for easter egg bag", string("object_backpack_loot_player_r2.creature"), 0, true);
NLMISC::CVariable<string> EasterEggChestSheet("egs","EasterEggChestSheet", "name of the sheet for easter egg chest", string("object_chest_wisdom_std_sel.creature"), 0, true);


//----------------------------------------------------------------------------
CR2EasterEgg::CR2EasterEgg()
{ 
	_EasterEggNextId = 0;

	//array of callback items
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "EASTER_EGG_SPAWNED",				cbEasterEggSpawned },
	};
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
}

//----------------------------------------------------------------------------
void CR2EasterEgg::dropMissionItem(std::vector< CGameItemPtr > items, TSessionId scenarioId, uint32 aiInstanceId, const NLMISC::CEntityId &lastCharacterOwner)
{
	TEasterEggLoot egg;
	egg.EasterEggId = _GetEasterNextId();
	egg.Item.insert( egg.Item.end(), items.begin(), items.end() );
	egg.ScenarioId = scenarioId.asInt();
	egg.InstanceId = aiInstanceId;
	egg.LastOwnerCharacterId = lastCharacterOwner;
	
	_AddEntityEasterAssociation( lastCharacterOwner, egg.EasterEggId );

	_EasterEgg.insert( make_pair( egg.EasterEggId, egg ) );

	// send message to AIS for spawn the easter egg
	CCharacter * c = PlayerManager.getChar( lastCharacterOwner );
	if( c != 0 )
	{
		CEntityState &pos = c->getState();
		_SpawnEasterEgg( egg.EasterEggId, CSheetId(EaterEggBagSheet), aiInstanceId, pos.X, pos.Y, pos.Z, pos.Heading );
	}
}

//----------------------------------------------------------------------------
void CR2EasterEgg::creatureEggSpawned(const NLMISC::CEntityId &creatureId, uint32 easterEggId)
{
	nlassert( _EntityEasterEggId.find( creatureId ) == _EntityEasterEggId.end() );
	TEasterEggContainer::iterator it = _EasterEgg.find( easterEggId );

	if (it ==  _EasterEgg.end())
	{
		// Case possible ask to despawn just after a spawn
		return;
	}	
	(*it).second.CreatureIdAsEgg = creatureId;
	_AddEntityEasterAssociation( creatureId, easterEggId);
}

//----------------------------------------------------------------------------
void CR2EasterEgg::characterLootEasterEgg(const NLMISC::CEntityId &characterId, NLMISC::CEntityId creatureId)
{
	TEntityEasterEggIdContainer::iterator ite = _EntityEasterEggId.find(creatureId);
	if( ite != _EntityEasterEggId.end() )
	{
		uint32 easterEggId = (*ite).second.back();
		TEasterEggContainer::iterator it = _EasterEgg.find( easterEggId );
		if( it != _EasterEgg.end() )
		{
			CCharacter * c = PlayerManager.getChar( characterId );
			if( c != 0 )
			{
				std::vector< CGameItemPtr > itemLeft;
				for( uint i = 0; i < (*it).second.Item.size(); ++i )
				{
					CSheetId itemSheet = (*it).second.Item[i]->getSheetId();
					sint32 stackSize = (sint32)(*it).second.Item[i]->getStackSize();
					sint32 itemQuality = (sint32)(*it).second.Item[i]->quality();
					if( c->addItemToInventory( INVENTORIES::bag, (*it).second.Item[i] ) == false )
					{
						itemLeft.push_back( (*it).second.Item[i] );
					}
					else
					{
						const R2::TMissionItem * mi = CR2MissionItem::getInstance().getR2ItemDefinition( (TSessionId)(*it).second.ScenarioId, itemSheet );
						if( mi )
						{
							SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
							params[0].Int = stackSize;
							if(params[0].Int == 0)
								params[0].Int = 1;
							params[1].SheetId = itemSheet;
							params[2].Int = (sint32)itemQuality;
							c->sendDynamicSystemMessage( c->getEntityRowId(), "LOOT_SUCCESS", params );
//							SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::literal, STRING_MANAGER::integer);
//							params[0].Int = stackSize;
//							if(params[0].Int == 0)
//								params[0].Int = 1;
//							params[1].Literal = mi->Description;
//							params[2].Int = itemQuality;
//							c->sendDynamicSystemMessage( c->getEntityRowId(), "R2LOOT_SUCCESS", params );
						}
						CR2MissionItem::getInstance().keepR2ItemAssociation(characterId, (*it).second.ScenarioId);
					}
				}
				(*it).second.Item.clear();
				uint32 aiInstanceId = (*it).second.InstanceId;
				TSessionId scenarioId = (TSessionId)(*it).second.ScenarioId;

				uint32 externalEasterEggId = 0xffffffff;
				for( TEasterEggScenarioIdTranslator::iterator its = _EasterEggScenarioIdTranslator.begin(); its != _EasterEggScenarioIdTranslator.end(); ++its )
				{
					if( (*its).second == easterEggId )
					{
						externalEasterEggId = (uint32)((*its).first >> 32);
						break;
					}
				}

				if( externalEasterEggId != 0xffffffff )
					ICharacterControl::getInstance()->lootEasterEggEvent( externalEasterEggId, scenarioId );

				_EasterEgg.erase(it);
				_RemoveEntityEasterAssociation( characterId, easterEggId );
				_RemoveEntityEasterAssociation( creatureId, easterEggId );
				_UnspawnEasterEgg( easterEggId, aiInstanceId );

				if( itemLeft.size() > 0 )
				{
					dropMissionItem( itemLeft, scenarioId, aiInstanceId, characterId );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
void CR2EasterEgg::activateEasterEgg(uint32 easterEggId, TSessionId scenarioId, uint32 aiInstanceId, const std::vector< R2::TItemAndQuantity > &items, const CFarPosition &pos, const std::string& name, const std::string &look)
{
	TEasterEggScenarioId eggScenarioId = easterEggId;
	eggScenarioId = eggScenarioId << 32 | scenarioId.asInt();

	TEasterEggScenarioIdTranslator::iterator it = _EasterEggScenarioIdTranslator.find(eggScenarioId);
	if( it != _EasterEggScenarioIdTranslator.end() )
		return;

	TEasterEggLoot egg;
	egg.EasterEggId = _GetEasterNextId();
	_EasterEggScenarioIdTranslator.insert(make_pair(eggScenarioId, egg.EasterEggId));

	for( uint32 i = 0; i < items.size(); ++i )
	{
		CGameItemPtr item = GameItemManager.createInGameItem( 1, items[i].Quantity, items[i].SheetId );
		if( item == 0 )
		{
			nlwarning("cannot create item %s", items[i].SheetId.toString().c_str());
			continue;
		}
		egg.Item.push_back(item);
	}
	egg.ScenarioId = scenarioId.asInt();
	egg.InstanceId = aiInstanceId;
	egg.Name = name;
	egg.Look = look; 
	_EasterEgg.insert( make_pair( egg.EasterEggId, egg ) );

	// send message to AIS for spawn the easter egg
	_SpawnEasterEgg( egg.EasterEggId, CSheetId(EasterEggChestSheet), aiInstanceId, pos.PosState.X, pos.PosState.Y, pos.PosState.Z, pos.PosState.Heading, name, look );
}

//----------------------------------------------------------------------------
void CR2EasterEgg::deactivateEasterEgg(uint32 easterEggId, TSessionId scenarioId)
{
	TEasterEggScenarioId eggScenarioId = easterEggId;
	eggScenarioId = eggScenarioId << 32 | scenarioId.asInt();

	TEasterEggScenarioIdTranslator::iterator its = _EasterEggScenarioIdTranslator.find(eggScenarioId);
	if( its == _EasterEggScenarioIdTranslator.end() )
		return;

	uint32  id = (*its).second;
	TEasterEggContainer::iterator it = _EasterEgg.find( id );
	if( it != _EasterEgg.end() )
	{
		if ( (*it).second.CreatureIdAsEgg == CEntityId::Unknown)
		{
			nlwarning("Try to spawn / unspawn a chest in the same tick! despawn was ignored.");
			return;
		}
	}

	//_deactivateEasterEgg(easterEggId);
	_deactivateEasterEgg(id);
//	_EasterEggScenarioIdTranslator.erase(its);
}

//----------------------------------------------------------------------------
void CR2EasterEgg::_deactivateEasterEgg(uint32 easterEggId)
{
	TEasterEggContainer::iterator it = _EasterEgg.find( easterEggId );
	if( it != _EasterEgg.end() )
	{
		for( uint i = 0; i < (*it).second.Item.size(); ++i )
		{
			(*it).second.Item[i].deleteItem();
		}
		(*it).second.Item.clear();
		_RemoveEntityEasterAssociation( (*it).second.CreatureIdAsEgg, easterEggId );
		_UnspawnEasterEgg( easterEggId, (*it).second.InstanceId );
		_EasterEgg.erase(it);
	}
}

//----------------------------------------------------------------------------
void CR2EasterEgg::easterEggTPActChange(const NLMISC::CEntityId &characterId, const CFarPosition &pos)
{ 
	TEntityEasterEggIdContainer::iterator ite = _EntityEasterEggId.find(characterId);
	if( ite != _EntityEasterEggId.end() )
	{
		vector< TEasterEggTPInfo > easterEggToTP;
		for( uint32 i = 0; i < (*ite).second.size(); ++ i )
		{
			TEasterEggContainer::iterator it = _EasterEgg.find( (*ite).second[i] );
			if( it != _EasterEgg.end() )
			{
				TEasterEggTPInfo eggTpInfo;
				eggTpInfo.CreatureId = (*it).second.CreatureIdAsEgg;
				eggTpInfo.EasterEggId = (*it).first;
				eggTpInfo.InstanceId = (*it).second.InstanceId;
				eggTpInfo.Name = (*it).second.Name;
				eggTpInfo.Look = (*it).second.Look;
				easterEggToTP.push_back( eggTpInfo );
			}
		}

		for( uint32 i = 0; i < easterEggToTP.size(); ++i )
		{
			_UnspawnEasterEgg( easterEggToTP[i].EasterEggId, easterEggToTP[i].InstanceId );
			_RemoveEntityEasterAssociation( easterEggToTP[i].CreatureId, easterEggToTP[i].EasterEggId );
			_SpawnEasterEgg(easterEggToTP[i].EasterEggId, CSheetId(EaterEggBagSheet), easterEggToTP[i].InstanceId, pos.PosState.X, pos.PosState.Y,pos.PosState.Z, pos.PosState.Heading, easterEggToTP[i].Name, easterEggToTP[i].Look );
		}
		easterEggToTP.clear();
	}
}

//----------------------------------------------------------------------------
void CR2EasterEgg::endScenario( TSessionId scenarioId )
{
	vector< uint32 > easterIdsMustRemoved;
	for( TEasterEggContainer::iterator it = _EasterEgg.begin(); it != _EasterEgg.end(); ++it )
	{
		if( (*it).second.ScenarioId == scenarioId.asInt() )
		{
			easterIdsMustRemoved.push_back( (*it).second.EasterEggId );
		}
	}

	for( uint32 i = 0; i < easterIdsMustRemoved.size(); ++i )
	{
		_deactivateEasterEgg( easterIdsMustRemoved[i] );
		_RemoveAllEntitiesAssociation( easterIdsMustRemoved[i] );
	}

	for( TEasterEggScenarioIdTranslator::iterator its = _EasterEggScenarioIdTranslator.begin(); its != _EasterEggScenarioIdTranslator.end(); )
	{
		if( ((*its).first & 0xffffffff) == scenarioId.asInt() )
		{
			TEasterEggScenarioIdTranslator::iterator tempIts = its;
			++its;
			_EasterEggScenarioIdTranslator.erase(tempIts);
		}
		else
			++its;
	}

	easterIdsMustRemoved.clear();
}

//----------------------------------------------------------------------------
void CR2EasterEgg::_SpawnEasterEgg(uint32 easterEggId, NLMISC::CSheetId sheet, uint32 aiInstanceId, sint32 x, sint32 y, sint32 z, float heading, const std::string&name, const std::string& look) const
{
	uint32 msgVersion = 1;
//	string botName;

	// send message to AIS for spawn the easter egg
	CMessage msg("SPAWN_EASTER_EGG");
	msg.serial( msgVersion );
	msg.serial( aiInstanceId );
	msg.serial( easterEggId );
	msg.serial( const_cast<CSheetId&>(sheet) );
	msg.serial( const_cast<std::string&>(name) );
	msg.serial( const_cast<std::string&>(look) );
	msg.serial( x, y, z, heading );
	CWorldInstances::instance().msgToAIInstance2(aiInstanceId, msg);
}


//----------------------------------------------------------------------------
void CR2EasterEgg::_UnspawnEasterEgg(uint32 easterEggId, uint32 aiInstanceId)
{
	uint32 msgVersion = 1;
	// send message to AIS for un-spawn the easter egg
	CMessage msg("DESPAWN_EASTER_EGG");
	msg.serial( msgVersion );
	msg.serial( aiInstanceId );
	msg.serial( easterEggId );
	CWorldInstances::instance().msgToAIInstance2(aiInstanceId, msg);

	for( TEasterEggScenarioIdTranslator::iterator its = _EasterEggScenarioIdTranslator.begin(); its != _EasterEggScenarioIdTranslator.end(); )
	{
		if( (*its).second == easterEggId )
		{
			TEasterEggScenarioIdTranslator::iterator tempIts = its;
			++its;
			_EasterEggScenarioIdTranslator.erase(tempIts);
		}
		else
			++its;
	}
}

//----------------------------------------------------------------------------
void CR2EasterEgg::_AddEntityEasterAssociation(const NLMISC::CEntityId &entityId, uint32 easterEggId)
{
	TEntityEasterEggIdContainer::iterator it = _EntityEasterEggId.find(entityId);
	if( it == _EntityEasterEggId.end() )
	{
		pair<TEntityEasterEggIdContainer::iterator, bool> res;
		res = _EntityEasterEggId.insert( make_pair(entityId, std::vector<TEasterEggId>() ) ); 
		nlassert(res.second == true);
		it = res.first;
	}
	(*it).second.push_back( easterEggId );
}

//----------------------------------------------------------------------------
void CR2EasterEgg::_RemoveEntityEasterAssociation(const NLMISC::CEntityId &entityId, uint32 easterEggId)
{
	TEntityEasterEggIdContainer::iterator it = _EntityEasterEggId.find(entityId);
	if( it != _EntityEasterEggId.end() )
	{
		for( uint32 i = 0; i < (*it).second.size(); ++i )
		{
			if( (*it).second[i] == easterEggId )
			{
				(*it).second[i] = (*it).second.back();
				(*it).second.pop_back();

				if( (*it).second.size() == 0 )
				{
					_EntityEasterEggId.erase( it );
				}
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------
void CR2EasterEgg::_RemoveAllEntitiesAssociation( uint32 easterEggId)
{
	TEntityEasterEggIdContainer::iterator it = _EntityEasterEggId.begin();
	while( it != _EntityEasterEggId.end() )
	{
		bool incIt = true;
		uint32 index = 0;
		while( index < (*it).second.size() )
		{
			if( (*it).second[index] == easterEggId )
			{
				(*it).second[index] = (*it).second.back();
				(*it).second.pop_back();

				if( (*it).second.size() == 0 )
				{
					TEntityEasterEggIdContainer::iterator itTmp = it;
					++it;
					_EntityEasterEggId.erase( itTmp );
					incIt = false;
					break;
				}
			}
			else
			{
				++index;
			}
		}
		if( incIt )
		{
			++it;
		}
	}
}

//----------------------------------------------------------------------------
void CR2EasterEgg::getEasterEggForScenario( TSessionId scenarioId, std::vector<R2::TEasterEggInfo> &easterEgg ) const
{
	for( TEasterEggContainer::const_iterator it = _EasterEgg.begin(); it != _EasterEgg.end(); ++it )
	{
		if( (*it).second.ScenarioId == scenarioId.asInt() )
		{
			R2::TEasterEggInfo eggInfo;
			eggInfo.EasterEggId = (*it).second.EasterEggId;
			for( uint32 i = 0 ; i < (*it).second.Item.size();  ++i )
			{
				eggInfo.Item.push_back( (*it).second.Item[i]->getSheetId() );
			}
			eggInfo.ScenarioId = (*it).second.ScenarioId;
			eggInfo.InstanceId = (*it).second.InstanceId;
			eggInfo.LastOwnerCharacterId = (*it).second.LastOwnerCharacterId;
			eggInfo.CreatureIdAsEgg = (*it).second.CreatureIdAsEgg;

			CCreature *c = CreatureManager.getCreature( eggInfo.CreatureIdAsEgg );
			if( c != 0 )
			{
				eggInfo.Position.PosState = c->getState();
				eggInfo.Position.SessionId = (TSessionId)eggInfo.InstanceId;
			}
			easterEgg.push_back(eggInfo);
		}
	}
}

//----------------------------------------------------------------------------
bool CR2EasterEgg::isEasterEgg( const NLMISC::CEntityId &eid ) const
{
	TEntityEasterEggIdContainer::const_iterator it = _EntityEasterEggId.find( eid );
	return (it != _EntityEasterEggId.end());
}

// Callbacks
//----------------------------------------------------------------------------
void cbEasterEggSpawned(CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	CEntityId creatureId;
	uint32 easterEggId;

	msgin.serial( creatureId );
	msgin.serial( easterEggId );

	CR2EasterEgg::getInstance().creatureEggSpawned( creatureId, easterEggId );
}


//============================================================================
//----------------------------------------------------------------------------
//============================================================================
NLMISC_COMMAND(activateEasterEgg, "activate an easter egg","<EasterEggId><ScenarioId><InstanceId><item><x><y>" )
{
	if( args.size() != 6 )
		return false;

	uint32 easterEggId;
	NLMISC::fromString(args[0], easterEggId);
	uint32 scId;
	NLMISC::fromString(args[1], scId);
	TSessionId scenarioId(scId);
	uint32 aiInstanceId;
	NLMISC::fromString(args[2], aiInstanceId);
	NLMISC::CSheetId itemSheet(args[3]);

	CFarPosition fPos;
	fPos.SessionId = scenarioId;
	NLMISC::fromString(args[4], fPos.PosState.X);;
	NLMISC::fromString(args[5], fPos.PosState.Y);;

	R2::TItemAndQuantity item;
	item.Quantity = 1;
	item.SheetId = itemSheet;
	
	std::vector< R2::TItemAndQuantity > items;
	items.push_back(item);

	CR2EasterEgg::getInstance().activateEasterEgg(easterEggId, scenarioId, aiInstanceId, items, fPos );
	return true;
} 

NLMISC_COMMAND(deactivateEasterEgg, "unactive an easter egg spwaned by scenario", "<EasterEggId><ScenarioId>" )
{
	if( args.size() != 2 )
		return false;

	uint32 easterEggId, scId;
	NLMISC::fromString(args[0], easterEggId);
	NLMISC::fromString(args[1], scId);
	TSessionId scenarioId(scId);

	CR2EasterEgg::getInstance().deactivateEasterEgg(easterEggId, scenarioId);
	return true;
}

NLMISC_COMMAND(easterEggTPActChange, "simulate an act change", "<CharacterId><x><y><instance>" )
{
	if( args.size() != 4 )
		return false;

	NLMISC::CEntityId eid;
	eid.fromString(args[0].c_str());

	CFarPosition fPos;
	NLMISC::fromString(args[1], fPos.PosState.X);
	NLMISC::fromString(args[2], fPos.PosState.Y);

	CR2EasterEgg::getInstance().easterEggTPActChange(eid, fPos);
	return true;
}
