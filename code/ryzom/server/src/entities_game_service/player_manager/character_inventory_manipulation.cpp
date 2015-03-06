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
//Misc
#include "nel/misc/sheet_id.h"

#include "game_share/slot_equipment.h"
#include "game_share/inventories.h"
#include "game_share/temp_inventory_mode.h"
#include "game_share/bot_chat_types.h"

#include "game_share/scenario.h"

#include "egs_sheets/egs_sheets.h"

#include "mission_manager/mission_manager.h"
#include "creature_manager/creature_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/player_room.h"
#include "phrase_manager/phrase_manager.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "phrase_manager/magic_phrase.h"
#include "pvp_manager/pvp_manager_2.h"
#include "shop_type/static_items.h"

#include "team_manager/team_manager.h"
#include "world_instances.h"
#include "zone_manager.h"
#include "building_manager/building_manager.h"
#include "mission_manager/mission_solo.h"
#include "mission_manager/mission_team.h"
#include "mission_manager/mission_guild.h"
#include "building_manager/room_instance.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "entities_game_service.h"
#include "shop_type/character_shopping_list.h"
#include "player_manager/gear_latency.h"
#include "progression/progression_pvp.h"
#include "game_item_manager/game_item.h"
#include "game_item_manager/player_inventory.h"
#include "game_item_manager/player_inv_bag.h"
#include "game_item_manager/player_inv_equip.h"
#include "game_item_manager/player_inv_hand.h"
#include "game_item_manager/player_inv_pet.h"
#include "game_item_manager/player_inv_temp.h"
#include "game_item_manager/player_inv_xchg.h"

#include "modules/r2_mission_item.h"
#include "server_share/log_item_gen.h"
#include "egs_dynamic_sheet_manager.h"

#include "game_share/visual_fx.h"
#include "game_share/teleport_types.h"

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;

////////////
// EXTERN //
////////////
extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern float						MaxHarvestDistance;
extern SKILLS::ESkills				BarehandCombatSkill;

CVariable<uint32>			DefaultWeightHands("egs", "DefaultWeightHands", "Weight of hands for calculate STA consumed by action for handed fight", 500,0,true);

// ****************************************************************************
void CCharacter::initInventories()
{
	if (_Inventory[INVENTORIES::handling] != NULL)
		return;

	// handling inventory
	CHandlingInventory *hi = new CHandlingInventory;
	hi->setInventoryId(INVENTORIES::handling);
	_Inventory[INVENTORIES::handling] = hi;
	CHandlingInvView *hiv = new CHandlingInvView;
	hiv->setCharacter(this);
	hiv->bindToInventory(hi);

	// temporary inventory
	CTempInventory *ti = new CTempInventory;
	ti->setInventoryId(INVENTORIES::temporary);
	_Inventory[INVENTORIES::temporary] = ti;
	CTempInvView *tiv = new CTempInvView;
	tiv->setCharacter(this);
	tiv->bindToInventory(ti);

	// equipment inventory
	CEquipInventory *ei = new CEquipInventory;
	ei->setInventoryId(INVENTORIES::equipment);
	_Inventory[INVENTORIES::equipment] = ei;
	CEquipInvView *eiv = new CEquipInvView;
	eiv->setCharacter(this);
	eiv->bindToInventory(ei);

	// bag inventory
	CBagInventory *bi = new CBagInventory;
	bi->setInventoryId(INVENTORIES::bag);
	_Inventory[INVENTORIES::bag] = bi;
	CBagInvView *biv = new CBagInvView;
	biv->setCharacter(this);
	biv->bindToInventory(bi);

	// pet animal inventories (one by pet animal)
	for (uint i=INVENTORIES::pet_animal; i<INVENTORIES::max_pet_animal; ++i)
	{
		CPetInventory *pi = new CPetInventory;
		pi->setInventoryId((INVENTORIES::TInventory)i);
		
		_Inventory[i] = pi;

		CPetInvView *piv = new CPetInvView;
		piv->setCharacter(this);
		piv->bindToInventory(pi);
	}


	// TEMP : should be replace by some specific code for inventory bulk-driven
	// Initialize all inventories with their max slot number 
	for (uint ni=0; ni < INVENTORIES::NUM_INVENTORY; ++ni)
	{
		_Inventory[ni]->setSlotCount(_Inventory[ni]->getMaxSlot());
	}
	// TEMP
}

// ****************************************************************************
void CCharacter::initInventoriesDb()
{
	H_AUTO(CCharacter_initInventoriesDb);

//	_PropertyDatabase.setProp( "TRADING:FAME_PRICE_FACTOR", 1000 );
	CBankAccessor_PLR::getTRADING().setFAME_PRICE_FACTOR(_PropertyDatabase, 1000 );

	// Force the update of all the inventories database

	nlassert(_Inventory[INVENTORIES::handling] != NULL);
	_Inventory[INVENTORIES::handling]->forceViewUpdateOfInventory(true);

	nlassert(_Inventory[INVENTORIES::equipment] != NULL);
	_Inventory[INVENTORIES::equipment]->forceViewUpdateOfInventory(true);
	
	nlassert(_Inventory[INVENTORIES::bag] != NULL);
	_Inventory[INVENTORIES::bag]->forceViewUpdateOfInventory(true);

	if (getRoomInterface().isValid())
		getRoomInterface().getInventory()->forceViewUpdateOfInventory(true);

	for (uint p = INVENTORIES::pet_animal; p < INVENTORIES::max_pet_animal; ++p)
	{
		if (_Inventory[p] != NULL)
			_Inventory[p]->forceViewUpdateOfInventory(true);
	}

	// Setup wear equipment malus
	_WearEquipmentMalus = 0.0f;
	CHandlingInventory *pHand = (CHandlingInventory *)(CInventoryBase*)_Inventory[INVENTORIES::handling];
	_WearEquipmentMalus += pHand->getWearMalus();
	CEquipInventory *pEquip = (CEquipInventory*)(CInventoryBase*)_Inventory[INVENTORIES::equipment];
	_WearEquipmentMalus += pEquip->getWearMalus();
//	_PropertyDatabase.setProp( _DataIndexReminder->Modifiers.TotalMalusEquip, (uint32)(_WearEquipmentMalus * 50) );
	CBankAccessor_PLR::getMODIFIERS().setTOTAL_MALUS_EQUIP(_PropertyDatabase, checkedCast<uint8>(_WearEquipmentMalus * 50) );

	// update parry skill if no item in right hand
	if (getRightHandItem() == NULL)
	{
		_CurrentParrySkill = BarehandCombatSkill;
		_BaseParryLevel = getSkillBaseValue(_CurrentParrySkill);
		_CurrentParryLevel = max( sint32(0), _BaseParryLevel + _ParryModifier);
		
//		_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryBase, _BaseParryLevel);
		CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setBase(_PropertyDatabase, checkedCast<uint16>(_BaseParryLevel));
//		_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryCurrent, _CurrentParryLevel );
		CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentParryLevel) );
	}
}

// ****************************************************************************
void CCharacter::releaseInventories()
{
	CEntityId id = _Id;
	
	if (_LootContainer!=NULL)
	{
		pickUpItemClose();
	}
	
	for (uint i = 0; i < INVENTORIES::NUM_INVENTORY; ++i)
		if (_Inventory[i] != NULL)
			_Inventory[i]->clearInventory();
}

// ****************************************************************************
CInventoryPtr CCharacter::getInventory(INVENTORIES::TInventory invId) const
{
	if (invId == INVENTORIES::player_room)
		return _PlayerRoom->getInventory();

	if (invId == INVENTORIES::guild)
	{
		if (_GuildId == 0)
			return NULL;
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId(_GuildId);
		if (guild == NULL)
			return NULL;

		return (CGuildInventory *)guild->getInventory();
	}

	// TODO: return other "extra" inventories here

	if (invId < INVENTORIES::NUM_INVENTORY)
		return _Inventory[invId];

	return NULL;
}

// pickup on entity, for loot or harvest
// ****************************************************************************
void CCharacter::itemPickup( const NLMISC::CEntityId& entity, bool harvest )
{
	H_AUTO(CCharacter_itemPickup);
	
	if ( entity.getType() == RYZOMID::object )
	{
		pickUpItem( entity );
		sendCloseTempInventoryImpulsion();
		return;
	}

	CEntityBase * e = CCreatureManager::getEntityBasePtr( entity );
	if( e == 0 )
	{
		nlwarning("<CCharacter::itemPickup> character %s perform itemPickup on entity %s, but entity not exist", _Id.toString().c_str(), entity.toString().c_str() ); 
		sendCloseTempInventoryImpulsion();
		return;
	}
	
	// check if creature is dead for preventsome exploit
	if( e->isDead() == false )
	{
		nlwarning("<CCharacter::itemPickup> character %s perform itemPickup on entity %s, but entity is not dead, It's an EXPLOIT with /ah context_loot macro. Ban the player...", _Id.toString().c_str(), entity.toString().c_str() ); 
		sendCloseTempInventoryImpulsion();
		return;
	}
	// if entity is 'lootable'
	if( e->getContextualProperty().directAccessForStructMembers().lootable() && !harvest )
	{
		harvestedEntity( entity );

		if( !pickUpItem( entity ) )
		{
			sendCloseTempInventoryImpulsion();
		}
		return;
	}
	
	// if entity is harvestable
	if( e->getContextualProperty().directAccessForStructMembers().harvestable() && harvest)
	{
		// if character have skill for harvest entity
		const CStaticCreatures * form = e->getForm();
		if( form /*&& form->getHarvestSkill() < SKILLS::NUM_SKILLS*/ )
		{
			//if( _Skills._Skills[ form->getHarvestSkill() ].Base > 0 ) // MUST 
			{
				switch (entity.getType() )
				{
				case RYZOMID::creature:
				case RYZOMID::npc: // workaround because some kitins in invasions have type 'npc'!
					{
						// get creature being harvested
						CCreature *creature = CreatureManager.getCreature( entity );
						if (creature == NULL)
						{
							nlwarning("<CCharacter::itemPickup> Invalid creature Id %s", entity.toString().c_str() );
							sendCloseTempInventoryImpulsion();
							return;
						}
						
						// check creature is dead
						if (creature->getScores()._PhysicalScores[SCORES::hit_points].Current > 0 )
						{
							nlwarning("<CCharacter::itemPickup> Creature %s isn't dead, cancel", entity.toString().c_str() );
							sendCloseTempInventoryImpulsion();
							return;
						}
						
						// check creature and character are not too far away
						const double distance = PHRASE_UTILITIES::getDistance( _Id, entity);
						if (distance < 0 || distance > MaxHarvestDistance)
						{
							sendDynamicSystemMessage( _Id, "EGS_HARVEST_CORPSE_TOO_FAR" );
							sendCloseTempInventoryImpulsion();
							return;
						}
						
						// if creature already being harvested, send an error message to the client
						if ( TheDataset.isAccessible(creature->harvesterRowId())  )
						{
							const CEntityId &harvesterId = TheDataset.getEntityId(creature->harvesterRowId());
							CCharacter *harvester = PlayerManager.getChar(creature->harvesterRowId());
							if ( harvester != NULL)
							{
								if ( harvesterId != _Id )
								{
									// TEMP ANTI BUG !!!!
									if ( harvester->harvestedEntity() != entity )
									{
										nlwarning("<CCharacter::itemPickup> BUG MUST BE CORRECTED LATER");
										nlwarning("<CCharacter::itemPickup> Creature (%s) supposed Harvester, id %s, is harvesting entity %s, which is not the same !!!!! Error in harvest, reset harvester on creature and continue harvest action",entity.toString().c_str(), harvesterId.toString().c_str(), harvester->harvestedEntity().toString().c_str() );
										creature->resetHarvesterRowId();
									}
									// END TEMP ANTI BUG 
									else
									{
										SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::entity);
										params[0].setEIdAIAlias( entity, CAIAliasTranslator::getInstance()->getAIAlias(entity) );
										params[1].setEIdAIAlias( harvesterId, CAIAliasTranslator::getInstance()->getAIAlias(harvesterId) );
										sendDynamicSystemMessage(_EntityRowId, "HARVEST_QUARTER_ALREADY_IN_PROGRESS_OTHER", params);

										sendCloseTempInventoryImpulsion();
										return;
									}
								}
								else
								{
									// check this player was really harvesting this entity
									if ( _MpSourceId != entity )
									{
										nlwarning("<CCharacter::itemPickup> BUG MUST BE CORRECTED LATER");
										nlwarning("<CCharacter::itemPickup> Creature %s supposed to be harvested by current player (id %s) but he is harvestng entity %s, which is not the same !!!!! Error in harvest, reset harvester on creature and conitnue harvest action",entity.toString().c_str(), harvesterId.toString().c_str(), _MpSourceId.toString().c_str() );
										creature->resetHarvesterRowId();
									}
									else
									{
										// do nothing as player already harvesting this entity
										//return;
										// reset harvest infos and restart harvest as the client window has been cleared
										//endHarvest(false);
									}
								}
							}
							else
							{
								nlwarning("<CCharacter::itemPickup> BUG MUST BE CORRECTED LATER");
								nlwarning("<CCharacter::itemPickup> For creature %s , Harvester row id cannot be found !!!! Error in harvest, reset harvester on creature and conitnue harvest action",entity.toString().c_str());
								creature->resetHarvesterRowId();
							}
						}

						// if creature has no mp to harvest, return
						const vector< CCreatureRawMaterial> &mps = creature->getMps();
						uint qty = 0;
						for (uint i = 0 ; i < mps.size() ; ++i)
						{
							qty += mps[i].Quantity;
						}
						if ( qty == 0 )
						{
							if( !e->getContextualProperty().directAccessForStructMembers().lootable() )
							{
								SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
								params[0].setEIdAIAlias( creature->getId(), CAIAliasTranslator::getInstance()->getAIAlias(creature->getId()) );
								sendDynamicSystemMessage(_EntityRowId, "HARVEST_NOTHING_TO_QUARTER", params);

								sendCloseTempInventoryImpulsion();
								return;
							}
							else
							{
								// no MP but loot
								pickUpItem( entity );
								leaveTempInventoryMode();
								enterTempInventoryMode(TEMP_INV_MODE::Quarter);
								return;
							}
						}
						
						endHarvest(false);

						staticActionInProgress( false );

						
						openHarvest();
						harvestedEntity( entity );
						CSheetId creatureType = creature->getType();
						harvestedEntitySheetId( creatureType );
						
						creature->harvesterRowId( TheDataset.getDataSetRow(_Id) );
//						creature->writeMpInfos();

						//_TempInventoryMode = TEMP_INV_MODE::Harvest;

// CHANGED : wait new harvest rules /////////////////////////////////////////////////
// execute phrase
						for (uint i = 0 ; i < mps.size() ; ++i)
						{
							if( mps[i].Quantity > 0 )
							{
								harvestAsked(i); // make harvest phrase with the first non empty mp
								break;
							}
						}

// CHANGED : wait new harvest rules /////////////////////////////////////////////////
					}
					
					break;
					/*
				case RYZOMID::deposit:
					{
						staticActionInProgress( true );
						nlwarning("<cbItemPickup> Invalid entity type deposit");
						// TO DO
					}
					break;
					*/
				default:
					nlwarning("<cbItemPickup> Invalid entity type %u", entity.getType() );
					break;
				}
			}
		}
	}
}

// close the temp inv and restore all items to the looted container
// ****************************************************************************
void CCharacter::pickUpItemClose()
{
	if (_LootContainer == NULL)
		return;

	// If there still some items in the temp inventory transfert them to the Looted Container
	CInventoryPtr tempInv = _Inventory[INVENTORIES::temporary];
	nlassert(tempInv != NULL);

	for( uint i = 0; i < tempInv->getSlotCount(); ++i )
	{
		if (tempInv->getItem( i ) != NULL)
		{
			CGameItemPtr ptr = tempInv->removeItem(i);
			_LootContainer->insertItem(ptr);
		}
	}
	// Clean up the temp inventory (so close it)
	tempInv->clearInventory();

	// For the moment we dont loot any objects
	nlassert(_EntityLoot.getType() != RYZOMID::object);

	// clean up all stuff in the creature or npc looted
	{
		CCreature *pCreature = CreatureManager.getCreature(_EntityLoot);
		if (pCreature != NULL)
		{
			pCreature->setLooter( NLMISC::CEntityId::Unknown, true );

			// character was looting, set the looted entity flag to lootable again
			if (_EntityLoot.getType() == RYZOMID::npc)
			{
				pCreature->getContextualProperty().directAccessForStructMembers().lootable(true);
				pCreature->getContextualProperty().setChanged();
			}
		}
	}

	// No more loot container needed because all items transfert have been done
	_LootContainer = NULL;
	_EntityLoot = CEntityId::Unknown;

}

// open the temp inventory with all things to loot and directly give the money to this character
// ****************************************************************************
bool CCharacter::pickUpItem(const CEntityId& entity)
{	
	// If the loot container is already affected to this character so we can't retry a pickup item
	if (_LootContainer != NULL)
		return false;

	staticActionInProgress(true);

	CCreature *pCreature = NULL;
	
	// if the target is a creature, get its loot inventory
	if ((entity.getType() == RYZOMID::creature) || (entity.getType() == RYZOMID::npc))
	{
		pCreature = CreatureManager.getCreature( entity );
		if (pCreature != NULL)
		{
			// Prevent hacking : the creature must be lootable
			if (! pCreature->getContextualProperty()().lootable())
				return false;

			// And not already looted by someone
			if (pCreature->getLooter() != CEntityId::Unknown)
			{
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::entity);
				params[0].setEIdAIAlias( pCreature->getId(), CAIAliasTranslator::getInstance()->getAIAlias(pCreature->getId()) );
				params[1].setEIdAIAlias( pCreature->getLooter(), CAIAliasTranslator::getInstance()->getAIAlias(pCreature->getLooter()) );
				sendDynamicSystemMessage(_EntityRowId, "HARVEST_LOOT_ALREADY_IN_PROGRESS_OTHER", params);

				return false;
			}

			// Check if this character have loot right
			uint32 nRightSize = (uint32)pCreature->getLootRight().size();
			if (nRightSize > 0)
			{
				bool bLootRight = false;
				for (uint i = 0; i < nRightSize; ++i)
				{
					if (pCreature->getLootRight()[i] == _EntityRowId)
					{
						bLootRight = true;
						break;
					}
				}
				// we have not the right to loot this creature or npc
				if (bLootRight == false)
				{
					TVectorParamCheck params;
					sendDynamicSystemMessage(_EntityRowId, "YOU_NOT_HAVE_LOOT_RIGHT", params);
					return false;
				}
			}
	
			// Ok -> setup the loot context (looter, loot inventory, etc...)
			if (pCreature->setLooter(_Id))
			{
				_LootContainer = pCreature->getLootInventory();

				// give money for loot (nb : only first loot table is used for money)
				if ( (pCreature->getForm() != NULL) && !pCreature->moneyHasBeenLooted() && !IsRingShard)
				{
					const CStaticLootTable * lootTable;
					if (!pCreature->getCustomLootTableId().empty())
					{
						//if custom, retrieve the CStaticLootTable stored in the DynamicSheetManager
						lootTable = CDynamicSheetManager::getInstance()->getLootTable(pCreature->getPrimAlias(), pCreature->getCustomLootTableId());
					}
					else
						lootTable = CSheets::getLootTableForm( pCreature->getLootTable(0) );
					if( lootTable != NULL )
					{
						uint8 roll = (uint8) RandomGenerator.rand(99);
						if( roll < (uint8) (lootTable->MoneyDropProbability * 100) )
						{
							uint64 nAmount = (uint64)(((float)pCreature->getForm()->getXPLevel()) * lootTable->MoneyLvlFactor + lootTable->MoneyBase);
							if( nAmount > 0 )
							{
								SM_STATIC_PARAMS_1(params, STRING_MANAGER::money);
								params[0].Money = nAmount;
								sendDynamicSystemMessage(_EntityRowId, "EGS_LOOT_MONEY", params);
								giveMoney(nAmount);
								// Prevent another player to loot again
								pCreature->getContextualProperty().directAccessForStructMembers().lootable(false);
								pCreature->getContextualProperty().setChanged();
							}
						}
						pCreature->moneyHasBeenLooted(true);
					}
				}
			}
		}
	}

	// if something in the loot container move items from loot container to temp inventory and open temp inv
	bool bNothingToLoot = true;
	
	if (_LootContainer != NULL)
	{
		endForageSession();
		_EntityLoot = entity;

		uint size = _LootContainer->getUsedSlotCount();
		if (size != 0)
		{
			uint32 totalQuantity = 0;
			CInventoryPtr tempInv = _Inventory[INVENTORIES::temporary];
			nlassert(tempInv != NULL);
			
			// No more than 8 items in the temp inventory
			
			if (size > tempInv->getSlotCount())
				size = tempInv->getSlotCount();
			
			// Move items from loot container to temp inventory

			for (uint i = 0; i < size; ++i)
			{
				if (_LootContainer->getItem(i) != NULL)
				{
					CGameItemPtr item = _LootContainer->removeItem(i);
					tempInv->insertItem(item);

					if (item->getSheetId() != CSheetId::Unknown)
						totalQuantity += item->getStackSize();
				}
			}

			// TODO : make money.sitem
//			uint64 moneyLoot = 0;
//			if (moneyLoot != (uint64) 0) // not used here, see "Auto-give money for NPC loot" above
//			{
//				giveMoney( moneyLoot );
//				SM_STATIC_PARAMS_1(params, STRING_MANAGER::money);
//				params[0].Money = moneyLoot;
//				sendDynamicSystemMessage( _Id, "EGS_LOOT_MONEY", params );
//			}
			
			// if totalQuantity is zero : no loot : there is a problem with the loot properties 
			// of a npc or creature, all items are no more present in the sheet_id.bin
			if (totalQuantity > 0)
			{
				bNothingToLoot = false;
				if (!enterTempInventoryMode(TEMP_INV_MODE::Loot))
					return true;
			}
		}
	}

	// Send message if nothing to loot
	if (bNothingToLoot)
	{
		if ((pCreature != NULL) && (pCreature->getMps().size() == 0))
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
			params[0].setEIdAIAlias( entity, CAIAliasTranslator::getInstance()->getAIAlias(entity) );
			sendDynamicSystemMessage(_EntityRowId, "HARVEST_NOTHING_TO_LOOT", params);
			
			const double waitTime = 1.0; // in seconds
			const TGameCycle waitCycles = TGameCycle(waitTime / CTickEventHandler::getGameTimeStep());
			pCreature->requestDespawn(waitCycles);
			sendCloseTempInventoryImpulsion();
		}
	}
	
	return !bNothingToLoot;
}

// ****************************************************************************
void CCharacter::destroyItem(INVENTORIES::TInventory invId, uint32 slot, uint32 quantity, bool sendChatMessage)
{
	TLogContext_Item_Destroy logContext(_Id);
	if (quantity == 0)
		return;

	// get inventory
	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
	{
		nlwarning("trying to destroy an item but inventory %u is NULL.", invId);
		return;
	}

	// check if slot is valid
	if (slot >= inv->getSlotCount())
	{
		nlwarning("specified slot %u but there are only %u slots in inventory %u", slot, inv->getSlotCount(), invId);
		return;
	}

	// check if slot is not empty	
	CGameItemPtr item = inv->getItem(slot);
	if (item == NULL)
		return;

	// keep item infos for stats
	uint16 quality = item->quality();
	CSheetId sheetId = item->getSheetId();
	uint32 remainingQuantity = item->getStackSize();

	// if the item is equipped, unequip first
	CInventoryPtr refInv = item->getRefInventory();
	if (refInv != NULL && (refInv->getInventoryId() == INVENTORIES::handling || refInv->getInventoryId() == INVENTORIES::equipment))
	{
		unequipCharacter(refInv->getInventoryId(), item->getRefInventorySlot());
	}

	// ensure that quantity is correct
	quantity = min(quantity, item->getNonLockedStackSize());
	if (quantity == 0)
	{
		// TODO: send error message to client?
		return;
	}
	else if (quantity == item->getStackSize())
	{
		// just delete the item
		inv->deleteItem(slot);
		remainingQuantity = 0;
	}
	else
	{
		// change the stack size
		remainingQuantity = item->getStackSize() - quantity;
		item->setStackSize(remainingQuantity);
	}

	// !!!!!!!
	// WARNING: from here item ptr cannot be used anymore, item may have been deleted
	// !!!!!!!

	if (sendChatMessage && sheetId != CSheetId::Unknown)
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::item, STRING_MANAGER::integer);
		params[0].SheetId = sheetId;
		params[1].Int = quantity;
		sendDynamicSystemMessage(_EntityRowId, "INVENTORY_DESTROY_ITEM", params);
	}

	// output stats
//	EGSPD::destroyItem(_Id, sheetId.toString(), quality, quantity);

	// refresh xp catalyser database value if needed
	if( invId == INVENTORIES::bag )
	{
		if( _XpCatalyserSlot != INVENTORIES::INVALID_INVENTORY_SLOT )
		{
			if( slot == _XpCatalyserSlot )
			{
//				_PropertyDatabase.setProp( "CHARACTER_INFO:XP_CATALYSER:Count", remainingQuantity );
				CBankAccessor_PLR::getCHARACTER_INFO().getXP_CATALYSER().setCount(_PropertyDatabase, checkedCast<uint16>(remainingQuantity) );
			}
		}

		if( _RingXpCatalyserSlot != INVENTORIES::INVALID_INVENTORY_SLOT )
		{
			if( slot == _RingXpCatalyserSlot )
			{
//				_PropertyDatabase.setProp( "CHARACTER_INFO:RING_XP_CATALYSER:Count", remainingQuantity );
				CBankAccessor_PLR::getCHARACTER_INFO().getRING_XP_CATALYSER().setCount(_PropertyDatabase, checkedCast<uint16>(remainingQuantity) );
			}
		}
	}
}

// ****************************************************************************
void CCharacter::moveItem(INVENTORIES::TInventory srcInvId, uint32 srcSlot, INVENTORIES::TInventory dstInvId, uint32 dstSlot, uint32 quantity)
{
	// cannot move an item in the same inventory
	if (srcInvId == dstInvId)
		return;

	// get inventories
	CInventoryPtr srcInv = getInventory(srcInvId);
	if (srcInv == NULL)
	{
		nlwarning("invalid src inventory %u", srcInvId);
		return;
	}

	CInventoryPtr dstInv = getInventory(dstInvId);
	if (dstInv == NULL)
	{
		nlwarning("invalid dst inventory %u", dstInvId);
		return;
	}

	// check that slots are valid
	if (srcSlot >= srcInv->getSlotCount())
	{
		nlwarning("invalid src slot %u, only %u slots in src inventory %u", srcSlot, srcInv->getSlotCount(), srcInvId);
		return;
	}

	if (dstSlot != INVENTORIES::INSERT_IN_FIRST_FREE_SLOT && dstSlot >= dstInv->getSlotCount())
	{
		nlwarning("invalid dst slot %u, only %u slots in dst inventory %u", dstSlot, dstInv->getSlotCount(), dstInvId);
		return;
	}

	// check if source slot is not empty
	CGameItemPtr srcItem = srcInv->getItem(srcSlot);
	if (srcItem == NULL)
	{
		nlwarning("src slot %u in inventory %u is empty", srcSlot, srcInvId);
		return;
	}

	// TODO: factorize these rules. See method removeItemFromInventory()

	/********************************/
	/*** BEGIN OF GAME PLAY RULES ***/

	const CStaticItem * srcForm = srcItem->getStaticForm();
	if (srcForm == NULL)
		return;

	// cannot move a non dropable item or an active xp catalyser
	if ((!srcForm->DropOrSell && !canPutNonDropableItemInInventory(dstInvId)) || isAnActiveXpCatalyser(srcItem))
		return;

	// You cannot exchange genesis named items
	if (srcItem->getPhraseId().find("genesis_") == 0 && !canPutNonDropableItemInInventory(dstInvId))
	{
		nlwarning("Character %s tries to move '%s' to inv %u", _Id.toString().c_str(), srcItem->getPhraseId().c_str(), dstInvId );
		return;
	}

	// cannot move a pet animal ticket
	if (srcForm->Family == ITEMFAMILY::PET_ANIMAL_TICKET)
		return;

	// worned items (except tools) are no longer 'movable'
	if (srcItem->getItemWornState() == ITEM_WORN_STATE::Worned && (srcForm->Family != ITEMFAMILY::CRAFTING_TOOL && srcForm->Family != ITEMFAMILY::HARVEST_TOOL))
	{
		/// TODO : send more explicit message
		sendDynamicSystemMessage(_Id, "NON_DROPABLE_ITEM");
		return;
	}

	// if one of inventories is player room check that it is accessible
	if (srcInvId == INVENTORIES::player_room || dstInvId == INVENTORIES::player_room)
	{
		if (!getRoomInterface().canUseInventory(this, this))
			return;
	}

	// if one of inventories is a pet animal check that it is accessible
	if (srcInvId >= INVENTORIES::pet_animal && srcInvId < INVENTORIES::max_pet_animal)
	{
		if (!petInventoryDistance(srcInvId - INVENTORIES::pet_animal))
			return;
	}
	if (dstInvId >= INVENTORIES::pet_animal && dstInvId < INVENTORIES::max_pet_animal)
	{
		if (!petInventoryDistance(dstInvId - INVENTORIES::pet_animal))
			return;
	}

	/***  END OF GAME PLAY RULES  ***/
	/********************************/

	// move the item
	CInventoryBase::TInventoryOpResult res = CInventoryBase::moveItem(srcInv, srcSlot, dstInv, dstSlot, quantity);
	switch (res)
	{
	case CInventoryBase::ior_ok:
		// ok
		break;

	case CInventoryBase::ior_overbulk:
	case CInventoryBase::ior_overweight:
		{
			if (dstInvId == INVENTORIES::bag)
				sendDynamicSystemMessage(_EntityRowId, "TOO_ENCUMBERED");
			else if (dstInvId == INVENTORIES::player_room)
				sendDynamicSystemMessage(_EntityRowId, "ROOM_TOO_ENCUMBERED");
			else if (dstInvId >= INVENTORIES::pet_animal && dstInvId < INVENTORIES::max_pet_animal)
				sendDynamicSystemMessage(_EntityRowId, "ANIMAL_PACKER_TOO_ENCUMBERED");
			return;
		}

	default:
		// for other errors just return
		return;
	}
}

// ****************************************************************************
bool CCharacter::canPutNonDropableItemInInventory(INVENTORIES::TInventory invId) const
{
	if (	invId == INVENTORIES::bag
		||	invId == INVENTORIES::player_room
		||	(invId >= INVENTORIES::pet_animal && invId < INVENTORIES::max_pet_animal)
		)
		return true;

	return false;
}

// ****************************************************************************
void CCharacter::equipCharacter(INVENTORIES::TInventory dstInvId, uint32 dstSlot, uint32 bagSlot, bool sendChatMessage)
{
	if (dstInvId != INVENTORIES::handling && dstInvId != INVENTORIES::equipment)
	{
		nlwarning("invalid equipment inventory %u", dstInvId);
		return;
	}

	cancelStaticActionInProgress(STATIC_ACT_TYPES::Neutral, false, false);

	CInventoryPtr bagInv = getInventory(INVENTORIES::bag);
	CInventoryPtr dstInv = getInventory(dstInvId);
	nlassert(bagInv != NULL);
	nlassert(dstInv != NULL);

	// check if slots are valid
	if (bagSlot >= bagInv->getSlotCount())
	{
		nlwarning("invalid bag slot %u, only %u slots", bagSlot, bagInv->getSlotCount());
		return;
	}
	if (dstSlot >= dstInv->getSlotCount())
	{
		nlwarning("invalid dst slot %u, only %u slots", dstSlot, dstInv->getSlotCount());
		return;
	}

	// check if bag slot is not empty
	CGameItemPtr item = bagInv->getItem(bagSlot);
	if (item == NULL)
	{
		nlwarning("bag slot %u is empty", bagSlot);
		return;
	}

	// if item is already referenced (maybe equipped), do not equip it
	if (item->getRefInventory() != NULL)
		return;

	// check item isn't locked
	if (item->getNonLockedStackSize() == 0)
		return;

	if( checkItemValidityWithSlot( item->getSheetId(), dstInvId, (uint16)dstSlot ) == false )
		return;
	
	// check if item can be equipped
	if (!checkPreRequired(item,true))
		return;

	const CStaticItem * form = item->getStaticForm();
	if (form == NULL)
		return;

	// cannot equip a worned item(except tools)
	if (item->getItemWornState() == ITEM_WORN_STATE::Worned && (form->Family != ITEMFAMILY::CRAFTING_TOOL && form->Family != ITEMFAMILY::HARVEST_TOOL) )
		return;

	// if an item is equipped in destination slot unequip it
	if (dstInv->getItem(dstSlot) != NULL)
	{
		if (dstInv->getItem(dstSlot)->getLockCount() != 0)
		{
			// if item is locked just return
			return;
		}
		unequipCharacter(dstInvId, dstSlot);
	}

	// set the item in ref inventory
	dstInv->insertItem(item, dstSlot);

	// if client ready
	if (getEnterFlag())
	{
		if (sendChatMessage)
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::item, STRING_MANAGER::integer);
			params[0].SheetId = item->getSheetId();
			params[1].Int = item->getStackSize();
			sendDynamicSystemMessage(_EntityRowId, "INVENTORY_EQUIP", params);
		}

		// if item is worned, warn player
		if (item->getItemWornState() == ITEM_WORN_STATE::Worned)
		{
			const string & msgName = ITEM_WORN_STATE::getMessageForState(ITEM_WORN_STATE::Worned);
			if (!msgName.empty())
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
				params[0].SheetId = item->getSheetId();
				sendDynamicSystemMessage(_EntityRowId, msgName, params);
			}
		}

		// update latency
		_GearLatency->setSlot(dstInvId, dstSlot, form, this);
	}

	// if equip right hand item, compute parry level and disengage if player is engaged in combat
	if (dstInvId == INVENTORIES::handling && dstSlot == INVENTORIES::right)
	{
		//updateParry(form->Family, form->Skill);
		CPhraseManager::getInstance().disengage(_EntityRowId, true);
	}

	// output stats
//	EGSPD::equipItem(_Id, item->getSheetId().toString(), item->quality(), INVENTORIES::toString(dstInvId), SLOT_EQUIPMENT::toString((SLOT_EQUIPMENT::TSlotEquipment)dstSlot));
}

// ****************************************************************************
void CCharacter::unequipCharacter(INVENTORIES::TInventory invId, uint32 slot, bool sendChatMessage)
{
	if (invId != INVENTORIES::handling && invId != INVENTORIES::equipment)
	{
		nlwarning("invalid equipment inventory %u", invId);
		return;
	}

	CInventoryPtr inv = getInventory(invId);
	nlassert(inv != NULL);

	// check if slot is valid
	if (slot >= inv->getSlotCount())
	{
		nlwarning("invalid slot %u, only %u slots", slot, inv->getSlotCount());
		return;
	}

	// check if slot is not empty
	CGameItemPtr item = inv->getItem(slot);
	if (item == NULL)
	{
		nlwarning("slot %u of inventory %u is empty", slot, invId);
		return;
	}

	uint nbItems = item->getStackSize();
	CSheetId itemSheet = item->getSheetId();

	if (item->getLockCount() != 0)
	{
		// item is locked, return
		return;
	}

	// remove the item
	if (item->getRefInventory() != NULL)
		item->getRefInventory()->removeItem(item->getRefInventorySlot());

	const CStaticItem * form = item->getStaticForm();
	if (form)
	{
		//if( form->Family == ITEMFAMILY::ARMOR || form->Family == ITEMFAMILY::MELEE_WEAPON || form->Family == ITEMFAMILY::RANGE_WEAPON )
		_WearEquipmentMalus -= form->WearEquipmentMalus;
	}
//	_PropertyDatabase.setProp(_DataIndexReminder->Modifiers.TotalMalusEquip, uint32(_WearEquipmentMalus * 50));
	CBankAccessor_PLR::getMODIFIERS().setTOTAL_MALUS_EQUIP(_PropertyDatabase, checkedCast<uint8>(_WearEquipmentMalus * 50));

	// remove item modifiers
	removeItemModifiers(item);

	// if unequipped item may have a protection type, we must re-compute max protection
	updateMagicProtectionAndResistance();
	
	// if client ready
	if (getEnterFlag())
	{
		if (sendChatMessage)
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::item, STRING_MANAGER::integer);
			params[0].SheetId = item->getSheetId();
			params[1].Int = item->getStackSize();
			sendDynamicSystemMessage(_EntityRowId, "INVENTORY_UNEQUIP", params);
		}

		// update latency
		_GearLatency->unsetSlot((INVENTORIES::TInventory)invId,slot,this);
	}

	// if equip right hand item, compute parry level and disengage if player is engaged in combat
	if (invId == INVENTORIES::handling && slot == INVENTORIES::right)
	{
		_CurrentParrySkill = BarehandCombatSkill;
		_BaseParryLevel = getSkillBaseValue(_CurrentParrySkill);
		_CurrentParryLevel = max(sint32(0), _BaseParryLevel + _ParryModifier);

//		_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryBase, _BaseParryLevel);
		CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setBase(_PropertyDatabase, checkedCast<uint16>(_BaseParryLevel));
//		_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryCurrent, _CurrentParryLevel);
		CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentParryLevel));

		CPhraseManager::getInstance().disengage( _EntityRowId, true );
	}

	// Remove enchant weapon effects as they are linked to equipped item
	if (invId==INVENTORIES::handling && slot==0)
	{
		CSEffectPtr const effect = lookForActiveEffect(EFFECT_FAMILIES::PowerEnchantWeapon);
		if (effect)
			removeSabrinaEffect(effect);
	}
	
	// output stats
//	EGSPD::unequipItem(_Id, item->getSheetId().toString(), item->quality(), INVENTORIES::toString((INVENTORIES::TInventory)invId), SLOT_EQUIPMENT::toString((SLOT_EQUIPMENT::TSlotEquipment)slot));
}

//--------------------------------------------------------------------------
bool CCharacter::checkItemValidityWithSlot( const CSheetId& sheet, INVENTORIES::TInventory inv, uint16 slot )
{
	vector< uint16 > slots;

	// if equipment inventory, check item match with slot
	if( inv == INVENTORIES::equipment )
	{
		return checkItemValidityWithEquipmentSlot( sheet, slot );
	}

	const CStaticItem * form = CSheets::getForm( sheet );
	BOMB_IF( form == 0, NLMISC::toString( "Item %s have no static form, recompute packed sheet.", sheet.toString().c_str() ), return false );
		
	// if hands inventory, check if item match with slot (right hand and left hand)
	if( inv == INVENTORIES::handling )
	{
		bool result;
		if( slot == INVENTORIES::right )
		{
			slots.push_back( SLOTTYPE::RIGHT_HAND );
			slots.push_back( SLOTTYPE::TWO_HANDS );
			slots.push_back( SLOTTYPE::RIGHT_HAND_EXCLUSIVE );
			result = checkIfItemCompatibleWithSlots( form->Slots, slots );
			if( result )
			{
				CInventoryPtr inventory = getInventory(inv);
				nlassert(inventory != NULL);
				CGameItemPtr item = inventory->getItem(INVENTORIES::left);
				if( item != 0 )
				{
					const CStaticItem * form2 = CSheets::getForm( item->getSheetId() );
					BOMB_IF( form2 == 0, NLMISC::toString( "Item %s have no static form, recompute packed sheet.", item->getSheetId().toString().c_str() ), return false );
					result = checkRightLeftHandCompatibility( form->Slots, form2->Slots );
					if( result )
					{
						if( form2->Family == ITEMFAMILY::AMMO )
						{
							result = checkIfAmmoCompatibleWithWeapon( form2->Skill, form->Skill );
						}
					}

				}
				return true;
			}
			return result;			
		}
		else if( slot == INVENTORIES::left )
		{
			slots.push_back( SLOTTYPE::LEFT_HAND );
			slots.push_back( SLOTTYPE::AMMO );
			result = checkIfItemCompatibleWithSlots( form->Slots, slots );
			if( result )
			{
				CInventoryPtr inventory = getInventory(inv);
				nlassert(inventory != NULL);
				CGameItemPtr item = inventory->getItem(INVENTORIES::right);
				if( item != 0 )
				{
					const CStaticItem * form2 = CSheets::getForm( item->getSheetId() );
					BOMB_IF( form2 == 0, NLMISC::toString( "Item %s have no static form, recompute packed sheet.", item->getSheetId().toString().c_str() ), return false );

					if (form->Family == ITEMFAMILY::AMMO && form2->Family == ITEMFAMILY::RANGE_WEAPON)
					{
						result = checkIfAmmoCompatibleWithWeapon( form->Skill, form2->Skill );
					}
					else
					{
						result = checkRightLeftHandCompatibility( form2->Slots, form->Slots );
					}
				}
			}
			return result;
		}
	}
	return true;
}

//--------------------------------------------------------------------------
bool CCharacter::checkItemValidityWithEquipmentSlot( const CSheetId& sheet, uint16 Slot )
{
	// Check if slot is compatible with item
	const CStaticItem * form = CSheets::getForm( sheet );
	if( form )
	{
		vector< uint16 > slots;
		switch( Slot )
		{
		case SLOT_EQUIPMENT::HEADDRESS:
			slots.push_back( SLOTTYPE::HEADDRESS );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::HEAD:
			slots.push_back( SLOTTYPE::HEAD );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::NECKLACE:
			slots.push_back( SLOTTYPE::NECKLACE );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::CHEST:
			slots.push_back( SLOTTYPE::CHEST );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::ARMS:
			slots.push_back( SLOTTYPE::ARMS );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::HANDS:
			slots.push_back( SLOTTYPE::HANDS );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::HANDL:
			slots.push_back( SLOTTYPE::LEFT_HAND_SLOT );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::HANDR:
			slots.push_back( SLOTTYPE::RIGHT_HAND_SLOT );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::LEGS:		
			slots.push_back( SLOTTYPE::LEGS );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::FEET:
			slots.push_back( SLOTTYPE::FEET );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::EARL:
		case SLOT_EQUIPMENT::EARR:
			slots.push_back( SLOTTYPE::EARS );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::WRISTL:
		case SLOT_EQUIPMENT::WRISTR:
			slots.push_back( SLOTTYPE::WRIST );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::FINGERL:
		case SLOT_EQUIPMENT::FINGERR:
			slots.push_back( SLOTTYPE::FINGERS );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		case SLOT_EQUIPMENT::ANKLEL:
		case SLOT_EQUIPMENT::ANKLER:
			slots.push_back( SLOTTYPE::ANKLE );
			return checkIfItemCompatibleWithSlots( form->Slots, slots );
		default:
			break;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
bool CCharacter::checkRightLeftHandCompatibility( const std::vector<std::string> itemRight, const std::vector<std::string> itemLeft )
{
	const sint itemRightSlotSize = (sint)itemRight.size();
	const sint itemLeftSlotSize = (sint)itemLeft.size();
	
	for( int i = 0; i < itemRightSlotSize; ++i )
	{
		for( int j = 0; j < itemLeftSlotSize; ++j )
		{
			switch( SLOTTYPE::stringToSlotType( itemRight[ i ] ) )
			{
			case SLOTTYPE::TWO_HANDS:
			case SLOTTYPE::RIGHT_HAND_EXCLUSIVE:
				return false;
			default: break;
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------------
bool CCharacter::checkIfItemCompatibleWithSlots( const std::vector<std::string> itemSlot, std::vector< uint16 > slots )
{
	const sint itemSlotSize = (sint)itemSlot.size();
	const sint typeSlotSize = (sint)slots.size();
	for( int i = 0; i < itemSlotSize; ++i )
	{
		for( int j = 0; j < typeSlotSize; ++j )
		{
			if( SLOTTYPE::stringToSlotType( itemSlot[ i ] ) == slots[ j ] )
			{
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------------
bool CCharacter::checkIfAmmoCompatibleWithWeapon( SKILLS::ESkills ammoType, SKILLS::ESkills weaponType )
{
	if( ammoType != SKILLS::unknown && weaponType != SKILLS::unknown )
	{
		return ( ammoType == weaponType );
	}
	return false;
}

//----------------------------------------------------------------------------
bool CCharacter::checkPreRequired(const CGameItemPtr & item, bool equipCheck )
{
	if (item == NULL)
		return false;

	const CStaticItem * form = item->getStaticForm();
	if (form == NULL)
		return false;

	bool requiredRespected = true;

	requiredRespected &= checkRequiredForSkill( item->getRequiredSkillLevel(), item->getRequiredSkill() );
	requiredRespected &= checkRequiredForSkill( item->getRequiredSkillLevel2(), item->getRequiredSkill2() );
	
	if( requiredRespected )
	{
		if (item->getRequiredCharac() >= 0 && item->getRequiredCharac() < CHARACTERISTICS::NUM_CHARACTERISTICS)
		{
			if (item->getRequiredCharacLevel() > _PhysCharacs._PhysicalCharacteristics[item->getRequiredCharac()].Base)
			{
				requiredRespected = false;
			}
		}
	}
	
	if( requiredRespected == false && equipCheck )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "REQUIRED_EQUIP" );
		return false;
	}
	return requiredRespected;
}

// ****************************************************************************
bool CCharacter::checkRequiredForSkill( uint32 skillRequired, SKILLS::ESkills requiredSkill )
{
	if( requiredSkill != SKILLS::unknown )
	{
		if( skillRequired > (uint32)getBestSkillValue( requiredSkill ) )
		{
			return false;
		}
	}
	else
	{
		if( skillRequired > (uint32)getBestSkillValue( SKILLS::SF ) &&
			skillRequired > (uint32)getBestSkillValue( SKILLS::SM ) &&
			skillRequired > (uint32)getBestSkillValue( SKILLS::SC ) &&
			skillRequired > (uint32)getBestSkillValue( SKILLS::SH ) )
		{
			return false;
		}
	}
	return true;
}

// ****************************************************************************
uint32 CCharacter::getWeightOfEquippedWeapon()
{
	CGameItemPtr itemPtr;
	itemPtr = getRightHandItem();
	uint32 weight = DefaultWeightHands;
	if ( itemPtr != NULL && itemPtr->getStaticForm() != NULL 
		&& (itemPtr->getStaticForm()->Family == ITEMFAMILY::MELEE_WEAPON || itemPtr->getStaticForm()->Family == ITEMFAMILY::RANGE_WEAPON)
		)
	{
		weight = itemPtr->weight();
	}
	
	itemPtr = getLeftHandItem();
	if ( itemPtr != NULL && itemPtr->getStaticForm() != NULL 
		&& (itemPtr->getStaticForm()->Family == ITEMFAMILY::MELEE_WEAPON /*|| itemPtr->getStaticForm()->Family == ITEMFAMILY::AMMO*/)
		)
	{
		weight += itemPtr->weight();
		weight /= 2;
	}
	return weight;
}

// ****************************************************************************
bool CCharacter::checkExchangeActors(bool * exchangeWithBot) const
{
	CEntityBase * interlocutor = CEntityBaseManager::getEntityBasePtr(_CurrentInterlocutor);
	if (interlocutor == NULL)
	{
		nlwarning("Player %s, invalid exchange partner %s", _Id.toString().c_str(), _CurrentInterlocutor.toString().c_str());
		return false;
	}

	bool allExchanging = isExchanging();

	if (exchangeWithBot != NULL)
		*exchangeWithBot = true;

	CCharacter * trader = NULL;
	if (_CurrentInterlocutor.getType() == RYZOMID::player)
	{
		trader = dynamic_cast<CCharacter *>(interlocutor);
		if (trader == NULL)
		{
			nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", interlocutor->getId().toString().c_str());
			return false;
		}

		if (exchangeWithBot != NULL)
			*exchangeWithBot = false;

		allExchanging &= trader->isExchanging();
	}
	else if (_CurrentInterlocutor.getType() != RYZOMID::npc)
	{
		nlwarning("Player %s, bad exchange partner type %s", _Id.toString().c_str(), _CurrentInterlocutor.toString().c_str());
		return false;
	}

	if (!allExchanging)
	{
		if (trader == NULL)
			nlwarning("Player %s is not exchanging!", _Id.toString().c_str());
		else
			nlwarning("Player %s is %s Interlocutor is %s",
				_Id.toString().c_str(),
				(isExchanging() ? "exchanging." : "not exchanging!"),
				_CurrentInterlocutor.toString().c_str(),
				(trader->isExchanging() ? "exchanging." : "not exchanging!")
				);
		return false;
	}

	return true;
}

// ****************************************************************************
void CCharacter::itemBagToExchange(uint32 bagSlot, uint32 exchangeSlot, uint32 quantity)
{
	// check exchange integrity
	bool exchangeWithBot;
	if (!checkExchangeActors(&exchangeWithBot))
	{
		DEBUG_STOP;
		return;
	}

	// player modify exchange so it should be accepted again
	invalidateExchange();

	// put item in the exchange view
	nlassert(_ExchangeView != NULL);
	_ExchangeView->putItemInExchange(bagSlot, exchangeSlot, quantity);

	if (exchangeWithBot)
	{
		checkBotGift();
	}
}

// ****************************************************************************
void CCharacter::itemExchangeToBag(uint32 exchangeSlot)
{
	// check exchange integrity
	bool exchangeWithBot;
	if (!checkExchangeActors(&exchangeWithBot))
	{
		DEBUG_STOP;
		return;
	}

	// player modify exchange so it should be accepted again
	invalidateExchange();

	// remove item from the exchange view
	nlassert(_ExchangeView != NULL);
	_ExchangeView->removeItemFromExchange(exchangeSlot);

	if (exchangeWithBot)
	{
		checkBotGift();
	}
}

// ****************************************************************************
bool CCharacter::lockItem(INVENTORIES::TInventory invId, uint32 slot, uint32 quantity)
{
	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return false;

	// check if slot is valid
	if (slot >= inv->getSlotCount())
		return false;

	// check if slot contains an item
	CGameItemPtr item = inv->getItem(slot);
	if (item != NULL)
	{
		uint32 lockQt = min(item->getNonLockedStackSize(), item->getLockCount() + quantity);
		if (lockQt > 0)
		{
			item->setLockCount(lockQt);
			return true;
		}
	}

	return false;
}

// ****************************************************************************
void CCharacter::unLockItem(INVENTORIES::TInventory invId, uint32 slot, uint32 quantity)
{
	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return;

	// check if slot is valid
	if (slot >= inv->getSlotCount())
		return;

	// check if slot contains an item
	CGameItemPtr item = inv->getItem(slot);
	if (item != NULL)
	{
		uint32 unlockQt = min(quantity, item->getLockCount());
		if (unlockQt > 0)
		{
			item->setLockCount(item->getLockCount() - unlockQt);
			return;
		}
	}
}

// ****************************************************************************
CGameItemPtr CCharacter::getItem(INVENTORIES::TInventory invId, uint32 slot) const
{
	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return NULL;

	// check if slot is valid
	if (slot >= inv->getSlotCount())
	{
		nlwarning("invalid slot %u, inventory %u only has %u slots", slot, invId, inv->getSlotCount());
		return NULL;
	}

	return inv->getItem(slot);
}

// ****************************************************************************
void CCharacter::consumeAmmo(uint32 quantity)
{
	TLogContext_Item_ConsumeAmmo contextLog(_Id);

	if (quantity == 0)
		return;

	CInventoryPtr handlingInv = getInventory(INVENTORIES::handling);
	nlassert(handlingInv != NULL);

	CGameItemPtr ammoItem = handlingInv->getItem(INVENTORIES::left);
	if (ammoItem == NULL )
		return;

	if (quantity >= ammoItem->getStackSize())
	{
		ammoItem.deleteItem();
		sendDynamicSystemMessage(_Id, "EGS_USE_LAST_AMMO");
	}
	else
	{
		ammoItem->setStackSize(ammoItem->getStackSize() - quantity);
	}
}

// ****************************************************************************
CGameItemPtr CCharacter::createItem(uint16 obtainedQuality, uint32 quantity, const NLMISC::CSheetId & obtainedItem, const CEntityId & creatorId, const std::string * phraseId)
{
	static const CSheetId preorderSheetId("pre_order.sitem");

	if (quantity == 0 || obtainedQuality == 0)
		return NULL;

	const CStaticItem * form = CSheets::getForm(obtainedItem);
	if (form == NULL)
	{
		nlwarning("cannot find form of item %s", obtainedItem.toString().c_str());
		return NULL;
	}
	
	// if item can be sold, get it in the sold items list (TODO: remove this horrible hardcoding!)
	CGameItemPtr sellingItem;
	if (	form->Family != ITEMFAMILY::RAW_MATERIAL 
		&&	form->Family != ITEMFAMILY::HARVEST_TOOL
		&&	form->Family != ITEMFAMILY::CRAFTING_TOOL
		&&	form->Family != ITEMFAMILY::CRYSTALLIZED_SPELL
		&&	form->Family != ITEMFAMILY::ITEM_SAP_RECHARGE
		&&	form->Family != ITEMFAMILY::FOOD
		)
	{
		vector<CGameItemPtr>::const_iterator it;
		const vector<CGameItemPtr>::const_iterator itEnd = CStaticItems::getStaticItems().end();
		for (it = CStaticItems::getStaticItems().begin(); it != itEnd; ++it)
		{
			if ((*it)->getSheetId() == obtainedItem)
			{
				sellingItem = *it;
				break;
			}
		}
	}

	// try to create the item
	CGameItemPtr item;
	switch (form->Family)
	{
		case ITEMFAMILY::CRAFTING_TOOL:
		case ITEMFAMILY::HARVEST_TOOL:
		case ITEMFAMILY::RAW_MATERIAL:
		case ITEMFAMILY::TELEPORT:
		case ITEMFAMILY::CRYSTALLIZED_SPELL:
		case ITEMFAMILY::ITEM_SAP_RECHARGE:
		case ITEMFAMILY::MISSION_ITEM:
		case ITEMFAMILY::PET_ANIMAL_TICKET:
		case ITEMFAMILY::HANDLED_ITEM:
		case ITEMFAMILY::CONSUMABLE:
		case ITEMFAMILY::XP_CATALYSER:
		case ITEMFAMILY::SCROLL:
		case ITEMFAMILY::FOOD: // TODO: remove this horrible hardcoding
		case ITEMFAMILY::SCROLL_R2:
		case ITEMFAMILY::COMMAND_TICKET:
		case ITEMFAMILY::GENERIC_ITEM:
		{
			item = GameItemManager.createItem(obtainedItem, obtainedQuality, true, true, creatorId);
		}
		break;

		default:
		{
			if (sellingItem != NULL)
			{
				item = sellingItem->getItemCopy();
				item->quality(obtainedQuality);
				if (phraseId)
					item->setPhraseId(*phraseId);
			}
			else if (obtainedItem == preorderSheetId)
			{
				item = GameItemManager.createItem(obtainedItem, obtainedQuality, true, form->DropOrSell, creatorId);
			}
			else
			{
				nlwarning("<CCharacter::createItem> item %s could be sold, but can't get copy of it from static item list (note : sheet name must start with 'ic', and not with '_', if name starts with 'ic' then craft plan is surely missing", obtainedItem.toString().c_str());	
			}
		}
	}

	if (item == NULL)
	{
		nlwarning("<CCharacter::createItem> error while creating item %s -> returned a NULL pointer", form->SheetId.toString().c_str());
		return NULL;
	}

	// set stack size
	// it cannot be more than the max stack size
	quantity = min(quantity, item->getMaxStackSize());
	item->setStackSize(quantity);

	if(form->Family == ITEMFAMILY::COMMAND_TICKET)
	{
		item->quality((uint16)form->CommandTicket->NbRun);
	}

	return item;
}

// ****************************************************************************
bool CCharacter::addItemToInventory(INVENTORIES::TInventory invId, CGameItemPtr & item, bool autoStack)
{
	H_AUTO(CCharacter_addItemToInventory);

	if (item == NULL)
		return false;

	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return false;

	if (invId == INVENTORIES::temporary)
		endForageSession();

	// TODO: check if player is allowed to do the operation. Cf. moveItem()
	INVENTORIES::TItemId itemId = item->getItemId();

	// insert item in inventory
	CInventoryBase::TInventoryOpResult res = inv->insertItem(item, INVENTORIES::INSERT_IN_FIRST_FREE_SLOT, autoStack);
	if (res == CInventoryBase::ior_ok)
	{
		log_Item_Move(itemId, INVENTORIES::UNDEFINED, inv->getInventoryId());
		return true;
	}

	// special case for temporary inventory
	// do our best to insert the item
	if (invId == INVENTORIES::temporary)
	{
		getAllTempInventoryItems();
		res = inv->insertItem(item, INVENTORIES::INSERT_IN_FIRST_FREE_SLOT, autoStack);
		if (res == CInventoryBase::ior_ok)
		{
			log_Item_Move(itemId, INVENTORIES::UNDEFINED, inv->getInventoryId());
			return true;
		}
		
		clearTempInventory();
		res = inv->insertItem(item, INVENTORIES::INSERT_IN_FIRST_FREE_SLOT, autoStack);
		if (res == CInventoryBase::ior_ok)
		{
			log_Item_Move(itemId, INVENTORIES::UNDEFINED, inv->getInventoryId());
			return true;
		}
	}

	// treat errors
	switch (res)
	{
	case CInventoryBase::ior_no_free_slot:
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = inv->getSlotCount();
			sendDynamicSystemMessage(_Id, "BAG_FULL", params);
		}
		break;

	case CInventoryBase::ior_overbulk:
	case CInventoryBase::ior_overweight:
		{
			if (invId == INVENTORIES::bag)
				sendDynamicSystemMessage(_Id,"TOO_ENCUMBERED");
			else if (invId == INVENTORIES::player_room)
				sendDynamicSystemMessage(_Id,"ROOM_TOO_ENCUMBERED");
			else if (invId >= INVENTORIES::pet_animal && invId < INVENTORIES::max_pet_animal)
				sendDynamicSystemMessage(_Id, "ANIMAL_PACKER_TOO_ENCUMBERED");
		}
		break;
	}

	log_Item_FailedAddBoughtItem(item->getItemId(), invId);
	return false;
}

// ****************************************************************************
CGameItemPtr CCharacter::removeItemFromInventory(INVENTORIES::TInventory invId, uint32 slot, uint32 quantity)
{
	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return NULL;

	if (slot >= inv->getSlotCount())
		return NULL;

	CGameItemPtr item = inv->getItem(slot);
	if (item == NULL)
		return NULL;

	const CStaticItem * form = item->getStaticForm();
	if (form == NULL)
		return NULL;

	// TODO: check if player is allowed to do the operation. Cf. moveItem()
	if (form->Family == ITEMFAMILY::PET_ANIMAL_TICKET || !form->DropOrSell)
	{
		sendDynamicSystemMessage(_Id, "NON_DROPABLE_ITEM");
		return NULL;
	}

	// worned items (except tools) are no longer 'movable'
	if (item->getItemWornState() == ITEM_WORN_STATE::Worned && (form->Family != ITEMFAMILY::CRAFTING_TOOL && form->Family != ITEMFAMILY::HARVEST_TOOL))
	{
		/// TODO : send more explicit message
		sendDynamicSystemMessage(_Id, "NON_DROPABLE_ITEM");
		return NULL;
	}

	// if item is equipped, unequip
	// TODO : still needed ?
//	bool hand;
//	uint16 slotImg;
//	if ( itemSrc->getSlotImage(hand,slotImg) )
//	{
//		unequipCharacter( (hand)? (uint16) INVENTORIES::handling : (uint16) INVENTORIES::equipment,slotImg );
//	}

	return inv->removeItem(slot, quantity);
}

// ****************************************************************************
bool CCharacter::createItemInInventory(INVENTORIES::TInventory invId, uint16 obtainedQuality, uint32 quantity, const NLMISC::CSheetId & obtainedItem, const CEntityId & creatorId, const std::string * phraseId)
{
	H_AUTO(CCharacter_createItemInInventory);

	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return false;

	CGameItemPtr item = createItem(obtainedQuality, quantity, obtainedItem, creatorId, phraseId);
	if (item == NULL)
		return false;

	if (!addItemToInventory(invId, item))
	{
		item.deleteItem();
		return false;
	}

	return true;
}

// ****************************************************************************
CGameItemPtr CCharacter::createItemInInventoryFreeSlot(INVENTORIES::TInventory invId, uint16 obtainedQuality, uint32 quantity, const NLMISC::CSheetId & obtainedItem, const NLMISC::CEntityId & creatorId, const std::string * phraseId)
{
	H_AUTO(CCharacter_createItemInInventoryFreeSlot);

	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return NULL;

	CGameItemPtr item = createItem(obtainedQuality, quantity, obtainedItem, creatorId, phraseId);
	if (item == NULL)
		return NULL;

	if (!addItemToInventory(invId, item, false))
	{
		item.deleteItem();
		return NULL;
	}

	return item;
}

// ****************************************************************************
void CCharacter::itemTempInventoryToBag(uint32 srcSlot, bool sendCloseTempImpulsion)
{
	H_AUTO(CCharacter_itemTempInventoryToBag);
	
	CInventoryPtr tempInv = getInventory(INVENTORIES::temporary);
	nlassert(tempInv != NULL);
	
	// Check srcSlot is in bound
	const uint nbItems = tempInv->getSlotCount();
	if (srcSlot >= nbItems)
		return;

	bool lastMaterial = false;

	switch (tempInventoryMode())
	{
		case TEMP_INV_MODE::Forage:
		{
			TLogContext_Item_Forage logContext(_Id);
			bool lastMaterial;
			if (pickUpRawMaterial(srcSlot, &lastMaterial))
			{
				endForageSession();
				if (lastMaterial)
				{
					endHarvest();

					// inform IA that everything was looted
					CCreatureDespawnMsg msg;
					msg.Entities.push_back(TheDataset.getDataSetRow(_EntityLoot));
					CWorldInstances::instance().msgToAIInstance(getInstanceNumber(), msg);
					//	leaveTempInventoryMode(); // done in endForageSession()
				}
			}
			else
			{
				// Force temp inv to reopen, because closed by client (TODO: opti)
				tempInv->forceViewUpdateOfSlot(0);
				return; // the character is too encumbered, let him make room
			}
		}
		break;

		case TEMP_INV_MODE::Quarter:

			// trap think this is a fake and don't know the reason of this stuff...
			// trap change the code where there is some quartering by entering the mode quarter
			// it seems obvious, but apparently it is not for the person who has coded that
			
		case TEMP_INV_MODE::Harvest: //quarter
		{
			TLogContext_Item_QuarterOrLoot logContext(_Id);
			// try to quarter, else try to loot

			if ( !pickUpRawMaterial((uint8)srcSlot, &lastMaterial) )
			{
				if( tempInv->getUsedSlotCount() == 0 )
				{
					tempInv->forceViewUpdateOfSlot(0);
					return;
				}
			}
			
			// quarter + loot
			if( tempInv->getUsedSlotCount() > 0 )
			{
				// remove item from temp and add it to bag
				CGameItemPtr item = tempInv->removeItem(srcSlot);
				if (item != NULL)
				{
					CSheetId itemSheet = item->getSheetId();
					uint32 itemStackSize = item->getStackSize();
					uint16 itemQuality = item->quality();
					
					if (!addItemToInventory(INVENTORIES::bag, item))
					{
						// Cant add item to the bag -> reput it in the temp inventory
						tempInv->insertItem(item, srcSlot);
						return;
					}
					// item can be NULL here (autostack)
					
					CMissionEventLootItem event(itemSheet, itemStackSize, itemQuality);
					processMissionEvent(event);
					
					SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
					params[0].Int = (sint32)itemStackSize;
					if(params[0].Int == 0)
						params[0].Int = 1;
					params[1].SheetId = itemSheet;
					params[2].Int = (sint32)itemQuality;
					sendDynamicSystemMessage( _EntityRowId, "LOOT_SUCCESS", params );

					// send loot txt to every team members (except looter ..)
					CTeam * team = TeamManager.getTeam(_TeamId);
					if (team)
					{
						SM_STATIC_PARAMS_4(paramsOther, STRING_MANAGER::player,STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
						paramsOther[0].setEIdAIAlias( getId(), CAIAliasTranslator::getInstance()->getAIAlias(getId()) );
						paramsOther[1].Int = (sint32)itemStackSize;
						if(paramsOther[1].Int == 0)
							paramsOther[1].Int = 1;
						paramsOther[2].SheetId = itemSheet;
						paramsOther[3].Int = (sint32)itemQuality;

						const list<CEntityId> &ids = team->getTeamMembers();
					
						list<CEntityId>::const_iterator itEnd = ids.end();
						for (list<CEntityId>::const_iterator it = ids.begin() ; it != itEnd ; ++it)
						{
							TDataSetRow entityRowId = TheDataset.getDataSetRow(*it);
							if( entityRowId != _EntityRowId )
							{
								sendDynamicSystemMessage( entityRowId, "LOOT_SUCCESS_OTHER", paramsOther );
							}
						}
					}
					
					item = NULL; // NEVER USE THE ITEM AFTER A addItemToInventory because of autostack
				}
				
				// if no more lootable items we check if we can close the temp inventory
				if( tempInv->getUsedSlotCount() == 0 )
				{
					// remove item from temp and add it to bag
					CGameItemPtr item = tempInv->removeItem(srcSlot);
					if (item != NULL)
					{
						CSheetId itemSheet = item->getSheetId();
						uint32 itemStackSize = item->getStackSize();
						uint16 itemQuality = item->quality();
						
						if (!addItemToInventory(INVENTORIES::bag, item))
						{
							// Cant add item to the bag -> reput it in the temp inventory
							tempInv->insertItem(item, srcSlot);
							return;
						}
						// item can be NULL here (autostack)
						
						CMissionEventLootItem event(itemSheet, itemStackSize, itemQuality);
						processMissionEvent(event);
						
						SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
						params[0].Int = (sint32)itemStackSize;
						if(params[0].Int == 0)
							params[0].Int = 1;
						params[1].SheetId = itemSheet;
						params[2].Int = (sint32)itemQuality;
						sendDynamicSystemMessage( _EntityRowId, "LOOT_SUCCESS", params );
						
						item = NULL; // NEVER USE THE ITEM AFTER A addItemToInventory because of autostack
					}
				}
			}

			// if no more lootable items we check if we can close the temp inventory
			CCreature *creature = CreatureManager.getCreature( _MpSourceId );
			if( tempInv->getUsedSlotCount() == 0 )
			{
				CCreature *creature = CreatureManager.getCreature( _MpSourceId );
					
				// if nor more RM left we can close the temp inventory
				if( lastMaterial || !creature || (creature && creature->getMps().size()==0) )
				{
					// if the creature object exists then inform the ai
					if (creature)
					{
						// inform IA that everything was looted
						CCreatureDespawnMsg msg;
						msg.Entities.push_back(TheDataset.getDataSetRow(creature->getId()));
						CWorldInstances::instance().msgToAIInstance(getInstanceNumber(), msg);
					}
					
					endHarvest(sendCloseTempImpulsion);
					
					leaveTempInventoryMode();
				}
			}
		}
		break;
		
		default:
		{
			TLogContext_Item_OtherTempPickup logContext(_Id);

			CGameItemPtr item = tempInv->removeItem(srcSlot);
			if (item != NULL)
			{
				CSheetId itemSheet = item->getSheetId();
				uint32 itemStackSize = item->getStackSize();
				uint16 itemQuality = item->quality();
				
				if (!addItemToInventory(INVENTORIES::bag, item))
				{
					// Cant add item to the bag -> reput it in the temp inventory
					tempInv->insertItem(item, srcSlot);
					return;
				}
				// item can be NULL here (autostack)
								
				// Check missions with loot event and send message to the client loot success
				if (tempInventoryMode() == TEMP_INV_MODE::Loot)
				{
					CMissionEventLootItem event(itemSheet, itemStackSize, itemQuality);
					processMissionEvent(event);
					
					SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
					params[0].Int = (sint32)itemStackSize;
					if(params[0].Int == 0)
						params[0].Int = 1;
					params[1].SheetId = itemSheet;
					params[2].Int = (sint32)itemQuality;
					sendDynamicSystemMessage( _EntityRowId, "LOOT_SUCCESS", params );
				}

				// send loot txt to every team members (except looter ..)
				CTeam * team = TeamManager.getTeam(_TeamId);
				if (team)
				{
					SM_STATIC_PARAMS_4(paramsOther, STRING_MANAGER::player,STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
					paramsOther[0].setEIdAIAlias( getId(), CAIAliasTranslator::getInstance()->getAIAlias(getId()) );
					paramsOther[1].Int = (sint32)itemStackSize;
					if(paramsOther[1].Int == 0)
						paramsOther[1].Int = 1;
					paramsOther[2].SheetId = itemSheet;
					paramsOther[3].Int = (sint32)itemQuality;
					
					const list<CEntityId> &ids = team->getTeamMembers();
					
					list<CEntityId>::const_iterator itEnd = ids.end();
					for (list<CEntityId>::const_iterator it = ids.begin() ; it != itEnd ; ++it)
					{
						TDataSetRow entityRowId = TheDataset.getDataSetRow(*it);
						if( entityRowId != _EntityRowId )
						{
							sendDynamicSystemMessage( entityRowId, "LOOT_SUCCESS_OTHER", paramsOther );
						}
					}
				}

				item = NULL; // NEVER USE THE ITEM AFTER A addItemToInventory because of autostack
			}
			
			bool tempInvEmpty = (tempInv->getUsedSlotCount() == 0);
			if (tempInvEmpty)
			{
				CCreature *creature = CreatureManager.getCreature( _MpSourceId );
				// if nor more lootable items and no RM or no RM left we can close the temp inventory
				if( lastMaterial || !creature || (creature && creature->getMps().size()==0) )
				{
					if ( tempInventoryMode() == TEMP_INV_MODE::Loot )
					{
						// inform IA that everything was looted
						CCreatureDespawnMsg msg;
						msg.Entities.push_back(TheDataset.getDataSetRow(_EntityLoot));
						CWorldInstances::instance().msgToAIInstance(getInstanceNumber(), msg);
					}
				}	
				leaveTempInventoryMode();
			}
		}
		break;
	}
}

// ****************************************************************************
void CCharacter::getAllTempInventoryItems(bool sendCloseTempImpulsion)
{
	H_AUTO(CCharacter_getAllTempInventoryItems);

	for (uint i = 0 ; i < INVENTORIES::NbTempInvSlots; ++i)
		itemTempInventoryToBag(i, sendCloseTempImpulsion);
}

// ****************************************************************************
void CCharacter::clearTempInventory()
{
	switch (tempInventoryMode())
	{
		case TEMP_INV_MODE::Forage:
			endForageSession();
		break;

		case TEMP_INV_MODE::Quarter:
		case TEMP_INV_MODE::Harvest:
			_DepositHarvestInformation.Sheet = CSheetId::Unknown;
			_DepositHarvestInformation.EndCherchingTime = 0xffffffff;
			endHarvest();
		break;

		case TEMP_INV_MODE::Loot:
			// close the temp inv and restore all items to the looted container
			pickUpItemClose();
			harvestedEntity( CEntityId::Unknown );
			clearHarvestDB();
		break;
		
		default:
			_Inventory[INVENTORIES::temporary]->clearInventory();
		break;
	};

	leaveTempInventoryMode();
	sendCloseTempInventoryImpulsion();
}

// ****************************************************************************
bool CCharacter::tempInventoryEmpty()
{
	return (_Inventory[INVENTORIES::temporary]->getUsedSlotCount() == 0);
}

// ****************************************************************************
void CCharacter::incSlotVersion(INVENTORIES::TInventory invId, uint32 slot)
{
	if (!_InventoryUpdater.incInfoVersion(invId, slot))
	{
		STOP( NLMISC::toString("Invalid inventory %u for incSlotVersion()", invId) );
	}
}

// ****************************************************************************
void CCharacter::sendItemInfos( uint16 slotId )
{
	TLogNoContext_Item	noContext;
	try
	{
		CSmartPtr< CItemPtr > itemPtrSmart = new CItemPtr();

		// get pointer on static skills tree definition
		static CSheetId sheet("skills.skill_tree");
		const CStaticSkillsTree * SkillsTree = CSheets::getSkillsTreeForm( sheet );
		nlassert( SkillsTree );

		INVENTORIES::TInventory inventory = INVENTORIES::TInventory(slotId >> CItemInfos::SlotIdIndexBitSize);
		uint32 slot = ( slotId & CItemInfos::SlotIdIndexBitMask );
		CItemInfos infos;

		CGameItemPtr item = NULL;
		if (inventory == INVENTORIES::exchange)
		{
			if ( slot >= CExchangeView::NbExchangeSlots )
			{
				nlwarning("<sendItemInfos>for exchange %s tries slot %u count is %u",_Id.toString().c_str(), slot, CExchangeView::NbExchangeSlots);
				return;
			}

			if (_ExchangeView == NULL)
			{
				nlwarning("<sendItemInfos>for exchange %s tries to access exchange view, but no exchange pending",
					_Id.toString().c_str());
				return;
			}

			item = _ExchangeView->getExchangeItem(slot);
			if (item == NULL)
			{
				nlwarning("<sendItemInfos>for exchange %s tries slot %u. Slot is empty",_Id.toString().c_str(),slot );
				return;
			}
//			infos.versionInfo = (uint16) _PropertyDatabase.getProp( NLMISC::toString("EXCHANGE:GIVE:%u:INFO_VERSION",slot) );
			infos.versionInfo = CBankAccessor_PLR::getEXCHANGE().getGIVE().getArray(slot).getINFO_VERSION(_PropertyDatabase);
		}
		else if (inventory == INVENTORIES::exchange_proposition)
		{
			CCharacter * trader = PlayerManager.getChar( _CurrentInterlocutor );
			if ( !trader )
			{
				nlwarning("<sendItemInfos>%s tries exchange inventory but has no trader",_Id.toString().c_str());
				return;
			}

			if ( slot >= CExchangeView::NbExchangeSlots )
			{
				nlwarning("<sendItemInfos>for exchange %s tries slot %u count is %u",_Id.toString().c_str(),slot,CExchangeView::NbExchangeSlots);
				return;
			}

			item = trader->_ExchangeView->getExchangeItem(slot);
			if (item == NULL)
			{
				nlwarning("<sendItemInfos>for exchange_proposition %s tries slot %u. Slot is empty",_Id.toString().c_str(),slot );
				return;
			}
//			infos.versionInfo = (uint16) _PropertyDatabase.getProp( NLMISC::toString("EXCHANGE:RECEIVE:%u:INFO_VERSION",slot) );
			infos.versionInfo = CBankAccessor_PLR::getEXCHANGE().getRECEIVE().getArray(slot).getINFO_VERSION(_PropertyDatabase);
		}
		else if ( inventory == INVENTORIES::guild )
		{
			CMirrorPropValueRO<TYPE_CELL> mirrorValue( TheDataset, getEntityRowId(), DSPropertyCELL );
			const sint32 cell = mirrorValue;
			if ( !CBuildingManager::getInstance()->isRoomCell(cell) )
			{
				nlwarning("<sendItemInfos> user %s is not in a building",_Id.toString().c_str());
				return;
			}
			
			const CRoomInstanceGuild * room = dynamic_cast<CRoomInstanceGuild*>( CBuildingManager::getInstance()->getRoomInstanceFromCell( cell ) );
			if ( !room )
			{
				nlwarning("<sendItemInfos> user %s cell %d is invalid",_Id.toString().c_str(),cell);
				return;
			}
			CGuild * guild = CGuildManager::getInstance()->getGuildFromId( room->getGuildId() );
			if ( !guild )
			{
				nlwarning("<sendItemInfos> user %s cellId %d is not a guild room !",_Id.toString().c_str(),cell);
				return;
			}
			item = guild->getItem( slot );
			infos.versionInfo = guild->getAndSyncItemInfoVersion( slot, getId() );
		}
		else if ( inventory == INVENTORIES::player_room ) 
		{
			item = _PlayerRoom->getInventory()->getItem(slot);
			if ( item == NULL )
				return;
			infos.versionInfo = (uint16)_InventoryUpdater.getInfoVersion( (INVENTORIES::TInventory)inventory, slot );
		}
		else if ( inventory == INVENTORIES::trading )
		{
			if( _ShoppingList != 0 )
			{
				item = _ShoppingList->getItemIndexFromTradeList( slot );
				if( item == NULL )
				{
					CSheetId itemSheet;
					uint32 level = 0;
					if( _ShoppingList->getItemSheetAndQualityFromTradeList( slot, itemSheet, level ) )
					{
						item = GameItemManager.createItem( itemSheet, (uint16)level, true, true );
						if( item != NULL )
						{
							itemPtrSmart->setItem( item );
						}
						else
						{
							nlwarning("<CCharacter sendItemInfos> invalid trading slot %u for user %s",slot,_Id.toString().c_str());
							return;
						}
					}
				}
				else
				{
					if( _ShoppingList->isItemSoldedByNpc( slot ) )
					{
						CSheetId itemSheet;
						uint32 level = 0;
						if( _ShoppingList->getItemSheetAndQualityFromTradeList( slot, itemSheet, level ) )
						item->recommended( level );
					}
				}
				infos.versionInfo = 0;
			}
			else
			{
				return;
			}
		}
		else if ( inventory == INVENTORIES::reward_sharing )
		{
			CTeam * team = TeamManager.getRealTeam( _TeamId );
			if ( !team )
			{
				nlwarning("<CCharacter sendItemInfos> invalid team for user '%s'",_Id.toString().c_str());
				return;
			}
			if (!team->getReward() )
			{
				nlwarning("<CCharacter sendItemInfos> invalid team reward for user '%s'",_Id.toString().c_str());
				return;
			}

			item = team->getReward()->getItem( slot );
			if ( item == NULL )
			{
				nlwarning("<CCharacter sendItemInfos> invalid item for user '%s'",_Id.toString().c_str());
				return;
			}
//			infos.versionInfo = (uint16)_PropertyDatabase.getProp( NLMISC::toString("INVENTORY:SHARE:%u:INFO_VERSION",slot) );
			infos.versionInfo = CBankAccessor_PLR::getINVENTORY().getSHARE().getArray(slot).getINFO_VERSION(_PropertyDatabase);
		}
		else 
		{
			if ( inventory >= INVENTORIES::NUM_INVENTORY )
			{
				nlwarning("<CCharacter sendItemInfos> invalid inventory %u : there are %u inventory, for user %s",inventory, INVENTORIES::NUM_INVENTORY, _Id.toString().c_str());
				return;
			}
			if ( _Inventory[inventory] == NULL )
			{
				nlwarning("<CCharacter sendItemInfos> invalid inventory %u: NULL",inventory);
				return;
			}
			if ( slot >= _Inventory[inventory]->getSlotCount() )
			{
				nlwarning("<CCharacter sendItemInfos> invalid slot %u for inventory %u: only %u slots for user %s",slot,inventory,_Inventory[inventory]->getSlotCount(), _Id.toString().c_str());
				return;
			}
			item = _Inventory[inventory]->getItem(slot);
			if ( item == NULL )
				return;
			infos.versionInfo = (uint16)_InventoryUpdater.getInfoVersion( (INVENTORIES::TInventory)inventory, slot );
		}
		if ( item == NULL )
		{
			nlwarning("<CCharacter sendItemInfos> invalid slot %u for inventory %u: NULL, for user %s",slot,inventory, _Id.toString().c_str());
			return;
		}

		infos.Hp = item->durability();
		infos.HpMax = item->maxDurability();

		// Yoyo: HACK for trading: force item to be at max durability (because getCopy() copy the max durability!!)
		if(inventory == INVENTORIES::trading)
			infos.Hp = infos.HpMax;

		infos.CreatorName = CEntityIdTranslator::getInstance()->getEntityNameStringId( item->getCreator() );
/*
		CEntityBase* creator = CEntityBaseManager::getEntityBasePtr(item->getCreator());
		infos.CreatorName = 0;
		if( creator && TheDataset.isAccessible( creator->getEntityRowId() ) )
		{
			CMirrorPropValueRO<uint32> nameIndex( TheDataset, creator->getId(), "NameIndex" );
			 = nameIndex();
		}
*/
		infos.slotId = slotId;
		sint32 skillVal = 0;
		const CStaticItem * form = item->getStaticForm();
		if( form )
		{
			if( form->Skill != SKILLS::unknown )
			{
				SKILLS::ESkills skill = form->Skill;
				// Found compatible unlocked skill
				while( _Skills.getSkillStruct( skill )->Base == 0 && SkillsTree->SkillsTree[ skill ].ParentSkill != SKILLS::unknown )
				{
					skill = SkillsTree->SkillsTree[ skill ].ParentSkill;
				}
				skillVal = _Skills.getSkillStruct( skill )->Current;
			}
			infos.WearEquipmentMalus = form->WearEquipmentMalus;

			if( item->getStaticForm()->Family == ITEMFAMILY::SCROLL_R2 )
			{
				// sessionId() don't works with edition session
				const R2::TMissionItem * itemDesc = CR2MissionItem::getInstance().getR2ItemDefinition( currentSessionId(), item->getSheetId() );
				if( itemDesc != 0 )
				{
					infos.R2ItemDescription = itemDesc->Description;
					infos.R2ItemComment = itemDesc->Comment;
				}
			}
		}
		item->damage( skillVal, infos.CurrentDamage, infos.MaxDamage );

		infos.HitRate = (uint16)(item->hitRate() * 6); //nb hit per minute
		
		infos.SapLoadCurrent = item->sapLoad();
		infos.SapLoadMax = item->maxSapLoad();
		infos.Range = item->range();
		infos.ParryModifier = item->parryModifier();
		infos.DodgeModifier = item->dodgeModifier();
		infos.AdversaryParryModifier = item->adversaryParryModifier();
		infos.AdversaryDodgeModifier = item->adversaryDodgeModifier();

		infos.ProtectionFactor = item->protectionFactor();
		infos.MaxSlashingProtection = item->maxSlashingProtection();
		infos.MaxBluntProtection = item->maxBluntProtection();
		infos.MaxPiercingProtection = item->maxPiercingProtection();
		
		nlctassert(CItemInfos::MaxMagicProtectionByJewel==3);
		item->magicProtection( 1, infos.MagicProtection[0], infos.MagicProtectionFactor[0] );
		item->magicProtection( 2, infos.MagicProtection[1], infos.MagicProtectionFactor[1] );
		item->magicProtection( 3, infos.MagicProtection[2], infos.MagicProtectionFactor[2] );

		infos.DesertMagicResistance = item->magicResistance(RESISTANCE_TYPE::Desert);
		infos.ForestMagicResistance = item->magicResistance(RESISTANCE_TYPE::Forest);
		infos.LacustreMagicResistance = item->magicResistance(RESISTANCE_TYPE::Lacustre);
		infos.JungleMagicResistance = item->magicResistance(RESISTANCE_TYPE::Jungle);
		infos.PrimaryRootMagicResistance = item->magicResistance(RESISTANCE_TYPE::PrimaryRoot);
		
		infos.HpBuff = item->hpBuff();
		infos.SapBuff = item->sapBuff();
		infos.StaBuff = item->staBuff();
		infos.FocusBuff = item->focusBuff();
		infos.Enchantment.Bricks = item->getEnchantment();

		infos.CastingSpeedFactor[CItemInfos::OffensiveElemental] = item->getElementalCastingTimeFactor();
		infos.MagicPowerFactor[CItemInfos::OffensiveElemental] = item->getElementalPowerFactor();
		infos.CastingSpeedFactor[CItemInfos::OffensiveAffliction] = item->getOffensiveAfflictionCastingTimeFactor();
		infos.MagicPowerFactor[CItemInfos::OffensiveAffliction] = item->getOffensiveAfflictionPowerFactor();
		infos.CastingSpeedFactor[CItemInfos::DefensiveHeal] = item->getHealCastingTimeFactor();
		infos.MagicPowerFactor[CItemInfos::DefensiveHeal] = item->getHealPowerFactor();
		infos.CastingSpeedFactor[CItemInfos::DefensiveAffliction] = item->getDefensiveAfflictionCastingTimeFactor();
		infos.MagicPowerFactor[CItemInfos::DefensiveAffliction] = item->getDefensiveAfflictionPowerFactor();

		infos.RequiredSkill = item->getRequiredSkill();
		infos.RequiredSkillLevel = item->getRequiredSkillLevel();
		infos.RequiredSkill2 = item->getRequiredSkill2();
		infos.RequiredSkillLevel2 = item->getRequiredSkillLevel2();
		infos.RequiredCharac = item->getRequiredCharac();
		infos.RequiredCharacLevel = item->getRequiredCharacLevel();

		infos.TypeSkillMods = item->getTypeSkillMods();
		
		// Special case of web missions items
		if (item->getStaticForm()->Name == "Web Transaction")
		{
			string cText = item->getCustomText().toString();
			string::size_type sPos = cText.find(" ");
			string::size_type ePos = cText.find("\n---\n");
			if (sPos != string::npos && sPos != (cText.length()-1) && ePos != string::npos && ePos != (cText.length()-1))
			{
				string cUrl = cText.substr(sPos, ePos-sPos);
				infos.CustomText = ucstring("@WEBIG "+cUrl);
			}
		}
		else
		{
			infos.CustomText = item->getCustomText();
		}
		
		if (item->getPetIndex() < MAX_INVENTORY_ANIMAL)
		{
			infos.PetNumber = item->getPetIndex() + 1;
		}

		CMessage msgout( "IMPULSION_ID" );
		CBitMemStream bms;
		msgout.serial( _Id );
		if ( ! GenericMsgManager.pushNameToStream( "ITEM_INFO:SET", bms) )
		{
			nlwarning("<CCharacter sendItemInfos> Msg name ITEM_INFO:SET not found");
			return;
		}
		bms.serial(infos);
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		sendMessageViaMirror( NLNET::TServiceId(_Id.getDynamicId()), msgout );	

		// Call the onItemChanged only on character's inventories (trading is not an inventory)
		if ((inventory != INVENTORIES::guild) &&
			(inventory != INVENTORIES::player_room) &&
			(inventory != INVENTORIES::exchange_proposition) &&
			(inventory != INVENTORIES::exchange) &&
			(inventory != INVENTORIES::trading) &&
			(inventory != INVENTORIES::reward_sharing))
		{
			CInventoryPtr pInv = _Inventory[inventory];
			nlassert(pInv != NULL);
			pInv->onItemChanged(slot, INVENTORIES::itc_info_version);
		}
	}
	catch (const NLMISC::Exception &e)
	{
		nlwarning("<ITEM_INFOS> exception : '%s'",e.what() );
	}	
}

// ****************************************************************************
bool CCharacter::doesWear(const NLMISC::CSheetId & sheetId) const
{
	CGameItemPtr item;
	
	CInventoryPtr equipInv = getInventory(INVENTORIES::equipment);
	nlassert(equipInv != NULL);
	for (uint i = 0; i < equipInv->getSlotCount(); i++)
	{
		item = equipInv->getItem(i);
		if (item != NULL && item->getSheetId() == sheetId)
			return true;
	}

	CInventoryPtr handlingInv = getInventory(INVENTORIES::handling);
	nlassert(handlingInv != NULL);
	for (uint i = 0; i < handlingInv->getSlotCount(); i++)
	{
		item = handlingInv->getItem(i);
		if (item != NULL && item->getSheetId() == sheetId)
			return true;
	}

	return false;
}

// create a crystallized action item in temp inventory
// ****************************************************************************
void CCharacter::createCrystallizedActionItem(const std::vector<NLMISC::CSheetId> & action)
{
	static const CSheetId crystalSheetId("crystalized_spell.sitem");

	if (!EnchantSystemEnabled)
		return;

	if (!enterTempInventoryMode(TEMP_INV_MODE::Crystallize))
		return;

	CGameItemPtr item = GameItemManager.createItem(crystalSheetId, 1, true, true, _Id);
	if (item != NULL)
	{
		item->applyEnchantment(action);
		addItemToInventory(INVENTORIES::temporary, item);
	}
}

// create a sap reload item in temp inventory
// ****************************************************************************
void CCharacter::createRechargeItem(uint32 sapRecharge)
{
	if (!EnchantSystemEnabled)
		return;

	/*** OLD METHOD **********************************************
	if (!enterTempInventoryMode(TEMP_INV_MODE::Crystallize))
		return;

	CGameItemPtr item = GameItemManager.createItem(rechargeSheetId, uint16(sapRecharge), true, true, _Id);
	if (item != NULL)
	{
		item->setSapLoad(sapRecharge);
		addItemToInventory(INVENTORIES::temporary, item);
	}******/

	CInventoryPtr handlingInv = getInventory(INVENTORIES::handling);
	if (handlingInv == NULL)
		return;

	CGameItemPtr rightHandItem = handlingInv->getItem(INVENTORIES::right); // item to reload
	if (rightHandItem == NULL)
	{
		TVectorParamCheck params;
		sendDynamicSystemMessage(_EntityRowId, "RIGHT_HAND_EMPTY", params);
		return;
	}

	// check the right hand holds a weapon item
	const CStaticItem * form = rightHandItem->getStaticForm();
	if (form == NULL)
	{
		nlwarning("item %s has no static form", rightHandItem->getSheetId().toString().c_str());
		return;
	}

	if ((form->Family != ITEMFAMILY::MELEE_WEAPON) && (form->Family != ITEMFAMILY::RANGE_WEAPON))
	{
		TVectorParamCheck params;
		sendDynamicSystemMessage(_EntityRowId, "WEAPONS_ONLY_CAN_BEEN_RECHARGED", params);
		return;
	}

	rightHandItem->reloadSapLoad(sapRecharge);

	SM_STATIC_PARAMS_3(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer);
	params[0].SheetId = rightHandItem->getSheetId();
	params[1].Int = rightHandItem->sapLoad();
	params[2].Int = rightHandItem->maxSapLoad();
	sendDynamicSystemMessage(_EntityRowId, "ITEM_IS_RECHARGED", params);
}

// check if enchant or recharge an item 
// ****************************************************************************
void CCharacter::enchantOrRechargeItem(INVENTORIES::TInventory invId, uint32 slot)
{
	static const CSheetId crystalSheetId("crystalized_spell.sitem");
	static const CSheetId rechargeSheetId("item_sap_recharge.sitem");
	static const CSheetId lightRechargeSheetId("light_sap_recharge.sitem");

	if (!EnchantSystemEnabled)
		return;

	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return;

	if (slot >= inv->getSlotCount())
		return;

	CGameItemPtr item = inv->getItem(slot);
	if (item == NULL)
		return;

	if (item->getSheetId() == crystalSheetId)
	{
		enchantItem(invId, slot);
	}
	else if ((item->getSheetId() == rechargeSheetId) || (item->getSheetId() == lightRechargeSheetId))
	{
		rechargeItem(invId, slot);
	}
}

// this is a helper for enchantItem and rechargeItem which have a lot in common
// Check slot boundaries, item exists, item is an enchantment or a recharge and check there is an item
// in the right hand and this item is a weapon
// ****************************************************************************
bool CCharacter::checkSlotsForEnchantOrRecharge(INVENTORIES::TInventory invId, uint32 slot, bool enchant)
{
	static const CSheetId crystalSheetId("crystalized_spell.sitem");
	static const CSheetId rechargeSheetId("item_sap_recharge.sitem");
	static const CSheetId lightRechargeSheetId("light_sap_recharge.sitem");

	if (!EnchantSystemEnabled)
		return false;

	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return false;
	
	// check if slot is valid
	if (slot >= inv->getSlotCount())
	{
		nlwarning("slot %u is higher than size of inventory %s (size %u)", slot, INVENTORIES::toString(invId).c_str(), inv->getSlotCount());
		return false;
	}

	// check if slot is not empty
	CGameItemPtr item = inv->getItem(slot);
	if (item == NULL)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = slot;
		sendDynamicSystemMessage(_EntityRowId, "CANT_FOUND_ITEM", params);
		return false;
	}

	// check if item is not locked
	if (item->getNonLockedStackSize() == 0)
		return false;

	if (enchant)
	{
		// check if the item is an enchantment
		if (item->getSheetId() != crystalSheetId)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
			params[0].SheetId = item->getSheetId();
			sendDynamicSystemMessage(_EntityRowId, "ITEM_IS_NOT_CRYSTALLIZED_ACTION", params);
			return false;
		}
	}
	else
	{
		// check if the item is a recharge
		if ((item->getSheetId() != rechargeSheetId) && (item->getSheetId() != lightRechargeSheetId))
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
			params[0].SheetId = item->getSheetId();
			sendDynamicSystemMessage(_EntityRowId, "ITEM_IS_NOT_CRYSTALLIZED_SAPLOAD_RECHARGE", params);
			return false;
		}
	}

	// check there is something in the right hand
	CInventoryPtr handlingInv = getInventory(INVENTORIES::handling);
	nlassert(handlingInv != NULL);

	CGameItemPtr rightHandItem = handlingInv->getItem(INVENTORIES::right);
	if (rightHandItem == NULL)
	{
		TVectorParamCheck params;
		sendDynamicSystemMessage(_EntityRowId, "RIGHT_HAND_EMPTY", params);
		return false;
	}

	// check the right hand holds a weapon item
	const CStaticItem * form = rightHandItem->getStaticForm();
	if (form == NULL)
	{
		nlwarning("item %s has no static form", rightHandItem->getSheetId().toString().c_str());
		return false;
	}

	if ((form->Family != ITEMFAMILY::MELEE_WEAPON) && (form->Family != ITEMFAMILY::RANGE_WEAPON))
	{
		TVectorParamCheck params;
		if (enchant)
			sendDynamicSystemMessage(_EntityRowId, "WEAPONS_ONLY_CAN_BEEN_ENCHANTED", params);
		else
			sendDynamicSystemMessage(_EntityRowId, "WEAPONS_ONLY_CAN_BEEN_RECHARGED", params);
		return false;
	}

	// if recharging, check the right hand item is enchanted
	if (!enchant)
	{
		if (rightHandItem->getEnchantment().empty())
		{
			TVectorParamCheck params;
			sendDynamicSystemMessage(_EntityRowId, "ONLY_ENCHANTED_ITEMS_CAN_BE_RECHARGED", params);
			return false;
		}
	}

	return true;
}

// ****************************************************************************
void CCharacter::enchantItem(INVENTORIES::TInventory invId, uint32 slot)
{
	if (checkSlotsForEnchantOrRecharge(invId, slot, true))
	{
		CInventoryPtr inv = getInventory(invId);
		CInventoryPtr handlingInv = getInventory(INVENTORIES::handling);
		nlassert(inv != NULL);
		nlassert(handlingInv != NULL);

		CGameItemPtr crystalItem = inv->getItem(slot); // crystal item
		CGameItemPtr rightHandItem = handlingInv->getItem(INVENTORIES::right); // item to enchant with crystal
		nlassert(crystalItem != NULL);
		nlassert(rightHandItem != NULL);

		if (rightHandItem->maxSapLoad() == 0)
		{
			sendDynamicSystemMessage(_EntityRowId, "CANT_BE_ENCHANTED");
			return;
		}
		rightHandItem->applyEnchantment(crystalItem->getEnchantment());

		// consume crystal (destroy it)
		inv->deleteItem(slot);

		SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
		params[0].SheetId = rightHandItem->getSheetId();
		sendDynamicSystemMessage(_EntityRowId, "ITEM_IS_NOW_ENCHANTED", params);
	}
}

// recharge item with recharge sap load item
// ****************************************************************************
void CCharacter::rechargeItem(INVENTORIES::TInventory invId, uint32 slot)
{
	if (checkSlotsForEnchantOrRecharge(invId, slot, false))
	{
		CInventoryPtr inv = getInventory(invId);
		CInventoryPtr handlingInv = getInventory(INVENTORIES::handling);
		nlassert(inv != NULL);
		nlassert(handlingInv != NULL);

		CGameItemPtr sapRechargeItem = inv->getItem(slot); // sap recharge item
		CGameItemPtr rightHandItem = handlingInv->getItem(INVENTORIES::right); // item to reload
		nlassert(sapRechargeItem != NULL);
		nlassert(rightHandItem != NULL);

		// reload sap in right hand item
		if (sapRechargeItem->sapLoad() == 0)
			sapRechargeItem->setSapLoad(sapRechargeItem->quality());
		rightHandItem->reloadSapLoad(sapRechargeItem->sapLoad());

		// consume sap recharge (destroy it)
		inv->deleteStackItem(slot,1);

		SM_STATIC_PARAMS_3(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].SheetId = rightHandItem->getSheetId();
		params[1].Int = rightHandItem->sapLoad();
		params[2].Int = rightHandItem->maxSapLoad();
		sendDynamicSystemMessage(_EntityRowId, "ITEM_IS_RECHARGED", params);
	}
}

// proc enchantment of an item
// ****************************************************************************
void CCharacter::procEnchantment()
{
	if (!EnchantSystemEnabled)
		return;

	CInventoryPtr handlingInv = getInventory(INVENTORIES::handling);
	nlassert(handlingInv != NULL);

	CGameItemPtr rightHandItem = handlingInv->getItem(INVENTORIES::right);
	if (rightHandItem == NULL)
		return;

	if (CTickEventHandler::getGameCycle() < rightHandItem->getLatencyEndDate())
		return;

	const CStaticItem * form = rightHandItem->getStaticForm();
	if (form == NULL)
		return;

	if (form->Family == ITEMFAMILY::MELEE_WEAPON || form->Family == ITEMFAMILY::RANGE_WEAPON)
	{
		if (rightHandItem->getEnchantment().size() > 0)
		{
			CMagicPhrase phrase;
			if (!phrase.buildProc(_EntityRowId, rightHandItem->getEnchantment()))
			{
				nlwarning("could not build item phrase");
				return;
			}

			// check if item have enough sap
			if (rightHandItem->sapLoad() >= phrase.getSabrinaCost())
			{
				if (phrase.procItem())
				{
					// check again the right hand item because it may have been worned and destroyed
					rightHandItem = handlingInv->getItem(INVENTORIES::right);
					if (rightHandItem != NULL)
					{
						rightHandItem->consumeSapLoad( phrase.getSabrinaCost() );
						// store re proc time
						rightHandItem->setLatencyEndDate( phrase.getCastingTime() + CTickEventHandler::getGameCycle() );
					}
				}
				else
					nlwarning("user %s : no valid image for right weapon", _Id.toString().c_str());
			} 
			else
			{
				sendDynamicSystemMessage(_EntityRowId, "ITEM_IN_RIGHT_HAND_HAVE_NOT_EBOUGHT_SAP");
			}
		}
		else
			nlwarning("item in right hand is not an enchanted item. Normally is not possible to proc with it!");
	}
}

// ****************************************************************************
void CCharacter::useItem(uint32 slot)
{
	if ( slot >= _Inventory[INVENTORIES::bag]->getSlotCount() )
	{
		nlwarning("<useItem>%s invalid slot %u count = %u",_Id.toString().c_str(),slot,_Inventory[INVENTORIES::bag]->getSlotCount());
		return;
	}

	CGameItemPtr item = _Inventory[INVENTORIES::bag]->getItem(slot);
	if ( item == NULL )
	{
		nlwarning("<useItem>%s NULL item in slot %u count = %u",_Id.toString().c_str(),slot,_Inventory[INVENTORIES::bag]->getSlotCount());
		return;
	}

	const CStaticItem * form = item->getStaticForm();
	if ( !form )
	{
		nlwarning("<useItem>%s item in slot %u has a NULL form count = %u",_Id.toString().c_str(),slot,_Inventory[INVENTORIES::bag]->getSlotCount());
		return;
	}

	// cancel any previous static action
	cancelStaticActionInProgress();

	// if item is a consumable item, then call consumeItem, otherwise, just continue
	if (form->Consumable == true)
	{
		consumeItem(INVENTORIES::bag, slot);
		return;
	}
	
	// xp catalyser
	if (form->Family  == ITEMFAMILY::XP_CATALYSER)
	{
		useXpCatalyser( slot );
		if( form->XpCatalyser->IsRingCatalyser )
		{
//			_PropertyDatabase.setProp( "CHARACTER_INFO:RING_XP_CATALYSER:Level", item->quality() );
			CBankAccessor_PLR::getCHARACTER_INFO().getRING_XP_CATALYSER().setLevel(_PropertyDatabase, item->quality() );
//			_PropertyDatabase.setProp( "CHARACTER_INFO:RING_XP_CATALYSER:Count", item->getStackSize() );
			CBankAccessor_PLR::getCHARACTER_INFO().getRING_XP_CATALYSER().setCount(_PropertyDatabase, checkedCast<uint16>(item->getStackSize()) );
		}
		else
		{
//			_PropertyDatabase.setProp( "CHARACTER_INFO:XP_CATALYSER:Level", item->quality() );
			CBankAccessor_PLR::getCHARACTER_INFO().getXP_CATALYSER().setLevel(_PropertyDatabase, item->quality() );
//			_PropertyDatabase.setProp( "CHARACTER_INFO:XP_CATALYSER:Count", item->getStackSize() );
			CBankAccessor_PLR::getCHARACTER_INFO().getXP_CATALYSER().setCount(_PropertyDatabase, checkedCast<uint16>(item->getStackSize()) );
		}
		
		
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = item->quality();
		CCharacter::sendDynamicSystemMessage( _Id, "XP_CATALYSER_ACTIVE", params );
		return;
	}

	if ( form->Family  == ITEMFAMILY::TELEPORT )
	{
		pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegeance = getAllegiance();
		if ((form->TpType == TELEPORT_TYPES::KAMI) && (allegeance.first == PVP_CLAN::Karavan)
			|| (form->TpType == TELEPORT_TYPES::KARAVAN) && (allegeance.first == PVP_CLAN::Kami)
			|| getOrganization() == 5 ) //marauder
		{
			CCharacter::sendDynamicSystemMessage(_Id, "ALTAR_RESTRICTION");
			return;
		}

		if( CPVPManager2::getInstance()->isTPValid(this, item) && IsRingShard == false )
		{
			// teleport dont work in the same way if the user is dead or alive
			if ( _IsDead )
			{
				PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->playerRespawn(this);
				// apply respawn effects because user is dead
				applyRespawnEffects();
				// simply teleport the user
				useTeleport( *form );

				// destroy 1 tp ticket
				_Inventory[INVENTORIES::bag]->removeItem(slot, 1);
			}
			else
			{
				// cannot use a second teleport when teleporting
				if (_TpTicketSlot != INVENTORIES::INVALID_INVENTORY_SLOT)
				{
					// user is alive. Send System Messages
					CCharacter::sendDynamicSystemMessage( _Id, "ALREADY_TELEPORTING" );
					return;
				}

				// user is alive. Send System Messages
				SM_STATIC_PARAMS_1( params, STRING_MANAGER::integer );
				params[0].Int = DelayBeforeItemTP / 10;
				CCharacter::sendDynamicSystemMessage( _Id, "TELEPORT_USED",params );
				
				//CTeleportEffect *effect = new CTeleportEffect( _EntityRowId, _EntityRowId, EFFECT_FAMILIES::Teleport, 0, DelayBeforeItemTP + CTickEventHandler::getGameCycle(), *form );
				//addSabrinaEffect( effect );
				_TpTicketSlot = slot;
				lockItem( INVENTORIES::bag, slot, 1 );

				// add fx
				CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _EntityRowId, DSPropertyVISUAL_FX );
				CVisualFX fx;
				fx.unpack(visualFx.getValue());
				if (allegeance.first != PVP_CLAN::None
					&& allegeance.first != PVP_CLAN::Neutral
					&& CFameInterface::getInstance().getFameIndexed(_Id, PVP_CLAN::getFactionIndex(allegeance.first)) >= 600000)
				{
					if (allegeance.first == PVP_CLAN::Kami)
					{
						fx.Aura = MAGICFX::TeleportKami;
					}
					else if (allegeance.first == PVP_CLAN::Karavan)
					{
						fx.Aura = MAGICFX::TeleportKara;
					}
					else
					{
						fx.Aura = MAGICFX::NoAura;
					}
				}
				else
				{
					fx.Aura = MAGICFX::NoAura;
				}
				sint64 prop;
				fx.pack(prop);
				visualFx = (sint16)prop;

				// add tp phrase in manager
				static CSheetId tpBrick("bapa01.sbrick");
				vector<CSheetId> bricks;
				bricks.push_back(tpBrick);
				CPhraseManager::getInstance().executePhrase(_EntityRowId,_EntityRowId,bricks);
			}
		}
		else
		{
			if( _IsDead )
			{
				sendDynamicSystemMessage(_EntityRowId, "PVP_RESPAWN_FORBIDEN");
			}
			else if( getPvPRecentActionFlag() )
			{
				sendDynamicSystemMessage(_EntityRowId, "PVP_TP_FORBIDEN");
			}
			else if ( IsRingShard )
			{
				sendDynamicSystemMessage(_EntityRowId, "TP_FORBIDEN_IN_RING_INSTANCE");
			}
			else
			{
				sendDynamicSystemMessage(_EntityRowId, "PVP_TP_ENEMY_REGION_FORBIDEN");
			}
		}
	}
	else
	{
		nlwarning("<TELEPORT>%s item in slot %u has invalid family %d ('%s')",_Id.toString().c_str(),slot,item->getStaticForm()->Family,ITEMFAMILY::toString(item->getStaticForm()->Family).c_str() );
		return;
	}
}

// ****************************************************************************
void CCharacter::stopUseItem( bool isRingCatalyser )
{
	if( isRingCatalyser == false )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "XP_CATALYSER_NO_MORE_ACTIVE");
		_XpCatalyserSlot = INVENTORIES::INVALID_INVENTORY_SLOT;
//		_PropertyDatabase.setProp( "CHARACTER_INFO:XP_CATALYSER:Level", 0 );
		CBankAccessor_PLR::getCHARACTER_INFO().getXP_CATALYSER().setLevel(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( "CHARACTER_INFO:XP_CATALYSER:Count", 0 );
		CBankAccessor_PLR::getCHARACTER_INFO().getXP_CATALYSER().setCount(_PropertyDatabase, 0 );
	}
	else
	{
		CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( getId() ));
		BOMB_IF(p == NULL,"Failed to find player record for character: "<<getId().toString(),return);
		if (p->isTrialPlayer()) {		
			PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "XP_CATALYSER_NO_MORE_ACTIVE");
			_RingXpCatalyserSlot = INVENTORIES::INVALID_INVENTORY_SLOT;
	//		_PropertyDatabase.setProp( "CHARACTER_INFO:RING_XP_CATALYSER:Level", 0 );
			CBankAccessor_PLR::getCHARACTER_INFO().getRING_XP_CATALYSER().setLevel(_PropertyDatabase, 0 );
	//		_PropertyDatabase.setProp( "CHARACTER_INFO:RING_XP_CATALYSER:Count", 0 );
			CBankAccessor_PLR::getCHARACTER_INFO().getRING_XP_CATALYSER().setCount(_PropertyDatabase, 0 );
		}
	}
}

// ****************************************************************************
void CCharacter::useTeleport(const CStaticItem & form)
{
	_TpTicketSlot = INVENTORIES::INVALID_INVENTORY_SLOT;

	// teleport the user
	uint16 idx = CZoneManager::getInstance().getTpSpawnZoneIdByName( form.Destination);
	const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone( idx );
	if ( !zone )
	{
		nlwarning("<TELEPORT>%s item '%s'has a bad Dest zone %s",_Id.toString().c_str(),form.SheetId.toString().c_str(),form.Destination.c_str() );
		respawn(0);// force respawn to avoid exploits....
	}
	else
	{	
		CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _EntityRowId, DSPropertyVISUAL_FX );
		CVisualFX fx;
		fx.unpack(visualFx.getValue());
		fx.Aura = MAGICFX::NoAura;
		sint64 prop;
		fx.pack(prop);
		visualFx = (sint16)prop;
		sint32 x,y,z;
		float theta;
		zone->getRandomPoint( x,y,z,theta );
		forbidNearPetTp(); // tickets must no teleport mektoubs
		tpWanted(x,y,z,true,theta);
	}
}


// ****************************************************************************
void CCharacter::useXpCatalyser( uint32 slot )
{
	if( slot == INVENTORIES::INVALID_INVENTORY_SLOT )
	{
		nlwarning("<CCharacter::useXpCatalyser> Trying to use an xp catalyser from INVALID_INVENTORY_SLOT !");
		return;
	}
	
	CGameItemPtr item = _Inventory[INVENTORIES::bag]->getItem( slot );
	if ( item != NULL )
	{
		const CStaticItem * form = item->getStaticForm();
		if ( form != NULL )
		{
			if( form->XpCatalyser )
			{
				if( form->XpCatalyser->IsRingCatalyser )
				{
					_RingXpCatalyserSlot = slot;
				}
				else
				{
					_XpCatalyserSlot = slot;
				}
			}
			else
			{
				nlwarning("<CCharacter::useXpCatalyser> item in bag at slot %d is not an xp catalyser",slot);
			}
		}
		else
		{
			nlwarning("<CCharacter::useXpCatalyser> can't find static form for item in bag at slot %d",slot);
		}
	}
	else
	{
		nlwarning("<CCharacter::useXpCatalyser> can't find item in bag at slot %d",slot);
	}
}


// ****************************************************************************
TEMP_INV_MODE::TInventoryMode CCharacter::tempInventoryMode() const
{ 
	CTempInventory *tempInv = (CTempInventory*)(CInventoryBase*)_Inventory[INVENTORIES::temporary];
	nlassert(tempInv != NULL);
	return tempInv->getMode();
}

// ****************************************************************************
void CCharacter::applyItemModifiers(const CGameItemPtr &item)
{
	const CStaticItem * form = CSheets::getForm( item->getSheetId() );
	if( form != 0 )
	{
		if( form->Family == ITEMFAMILY::AMMO )
			return;
	}

	// init all modifiers due to equipment
	_DodgeModifier += item->dodgeModifier();
	_ParryModifier += item->parryModifier();
	_AdversaryDodgeModifier += item->adversaryDodgeModifier();
	_AdversaryParryModifier += item->adversaryParryModifier();

	// update DB for modifiers
	_CurrentDodgeLevel = max( sint32(0), _BaseDodgeLevel + _DodgeModifier);
	_CurrentParryLevel = max( sint32(0), _BaseParryLevel + _ParryModifier);
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.DodgeCurrent, _CurrentDodgeLevel );
	CBankAccessor_PLR::getCHARACTER_INFO().getDODGE().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentDodgeLevel) );
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryCurrent, _CurrentParryLevel );
	CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentParryLevel) );
	
	// modifiers on scores
	_PhysScores._PhysicalScores[ SCORES::hit_points ].Modifier += (item->hpBuff() + item->armorHpBuff());
	_PhysScores._PhysicalScores[ SCORES::stamina ].Modifier += item->staBuff();
	_PhysScores._PhysicalScores[ SCORES::sap ].Modifier += item->sapBuff();
	_PhysScores._PhysicalScores[ SCORES::focus ].Modifier += item->focusBuff();

	// specific modifiers
	if ( !item->getTypeSkillMods().empty() )
	{
		const vector<CTypeSkillMod> &mods = item->getTypeSkillMods();
		for (uint i = 0 ; i < mods.size() ; ++i)
		{
			if (mods[i].Type >= 0 && mods[i].Type < EGSPD::CClassificationType::Unknown)
			{
				_ClassificationTypesSkillModifiers[(uint)mods[i].Type] += mods[i].Modifier;
				++_NbNonNullClassificationTypesSkillMod;
			}
		}
	}
}

// ****************************************************************************
void CCharacter::removeItemModifiers(const CGameItemPtr &item)
{
	const CStaticItem * form = CSheets::getForm( item->getSheetId() );
	if( form != 0 )
	{
		if( form->Family == ITEMFAMILY::AMMO )
			return;
	}
	
	// init all modifiers due to equipment
	_DodgeModifier -= item->dodgeModifier();
	_ParryModifier -= item->parryModifier();
	_AdversaryDodgeModifier -= item->adversaryDodgeModifier();
	_AdversaryParryModifier -= item->adversaryParryModifier();

	// update DB for modifiers
	_CurrentDodgeLevel = max( sint32(0), _BaseDodgeLevel + _DodgeModifier);
	_CurrentParryLevel = max( sint32(0), _BaseParryLevel + _ParryModifier);
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.DodgeCurrent, _CurrentDodgeLevel );
	CBankAccessor_PLR::getCHARACTER_INFO().getDODGE().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentDodgeLevel) );
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryCurrent, _CurrentParryLevel );
	CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentParryLevel) );
	
	// modifiers on scores
	_PhysScores._PhysicalScores[ SCORES::hit_points ].Modifier -= (item->hpBuff() + item->armorHpBuff());
	_PhysScores._PhysicalScores[ SCORES::stamina ].Modifier -= item->staBuff();
	_PhysScores._PhysicalScores[ SCORES::sap ].Modifier -= item->sapBuff();
	_PhysScores._PhysicalScores[ SCORES::focus ].Modifier -= item->focusBuff();

	// specific modifiers
	if ( !item->getTypeSkillMods().empty() )
	{
		const vector<CTypeSkillMod> &mods = item->getTypeSkillMods();
		for (uint i = 0 ; i < mods.size() ; ++i)
		{
			if (mods[i].Type >= 0 && mods[i].Type < EGSPD::CClassificationType::Unknown)
			{
				_ClassificationTypesSkillModifiers[(uint)mods[i].Type] -= mods[i].Modifier;
				--_NbNonNullClassificationTypesSkillMod;
			}
		}
	}
}

// ****************************************************************************
bool CCharacter::canEnterTempInventoryMode(TEMP_INV_MODE::TInventoryMode mode)
{
	CTempInventory *tempInv = (CTempInventory*)(CInventoryBase*)_Inventory[INVENTORIES::temporary];
	nlassert(tempInv != NULL);
	return tempInv->canEnterMode(mode);
}

// ****************************************************************************
bool CCharacter::enterTempInventoryMode(TEMP_INV_MODE::TInventoryMode mode)
{
	CTempInventory *tempInv = (CTempInventory*)(CInventoryBase*)_Inventory[INVENTORIES::temporary];
	nlassert(tempInv != NULL);
	return tempInv->enterMode(mode);
}

// ****************************************************************************
void CCharacter::leaveTempInventoryMode()
{
	CTempInventory *tempInv = (CTempInventory*)(CInventoryBase*)_Inventory[INVENTORIES::temporary];
	nlassert(tempInv != NULL);
	tempInv->leaveMode();
}

// ****************************************************************************
bool CCharacter::wearRightHandItem(float wearFactor)
{
	CGameItemPtr item = getRightHandItem();
	if (item == NULL)
		return false;

	double wear = item->getWearPerAction() * wearFactor;
	item->removeHp(wear);

	// test if item must be destroyed
	if (item->getStaticForm() != NULL)
	{
		if (	ITEMFAMILY::destroyedWhenWorned(item->getStaticForm()->Family)
			&&	(item->getItemWornState() == ITEM_WORN_STATE::Worned))
		{
			/*SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
			params[0].SheetId = item->getSheetId();
			sendDynamicSystemMessage( _EntityRowId, "ITEM_WORNED_DESTROYED", params);
			
			destroyItem(item->getInventory()->getInventoryId(), item->getInventorySlot(), item->getStackSize(), false);
			*/
			unequipCharacter(INVENTORIES::handling, INVENTORIES::right);

			return true;
		}
	}

	return false;
}

// ****************************************************************************
bool CCharacter::wearLeftHandItem(float wearFactor)
{
	CGameItemPtr item = getLeftHandItem();
	if (item == NULL)
		return false;
	
	double wear = item->getWearPerAction() * wearFactor;
	item->removeHp(wear);
	
	// test if item must be destroyed
	if (item->getStaticForm() != NULL)
	{
		if (	ITEMFAMILY::destroyedWhenWorned(item->getStaticForm()->Family)
			&&	(item->getItemWornState() == ITEM_WORN_STATE::Worned))
		{
			/*SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
			params[0].SheetId = item->getSheetId();
			sendDynamicSystemMessage( _EntityRowId, "ITEM_WORNED_DESTROYED", params);
			
			destroyItem(item->getInventory()->getInventoryId(), item->getInventorySlot(), item->getStackSize(), false);
			*/
			unequipCharacter(INVENTORIES::handling, INVENTORIES::left);
			
			return true;
		}
	}
	
	return false;
}

// ****************************************************************************
void CCharacter::wearArmor(float wearFactor)
{
	static const uint8 NbArmorSlots = 6;
	static const SLOT_EQUIPMENT::TSlotEquipment armorSlots[NbArmorSlots]=
	{
		SLOT_EQUIPMENT::HEAD,
		SLOT_EQUIPMENT::CHEST,
		SLOT_EQUIPMENT::ARMS,
		SLOT_EQUIPMENT::HANDS,
		SLOT_EQUIPMENT::LEGS,
		SLOT_EQUIPMENT::FEET
	};
	
	for (uint i = 0; i < NbArmorSlots ; ++i)
	{
		wearItem(INVENTORIES::equipment, armorSlots[i], wearFactor);
	}
}

// ****************************************************************************
void CCharacter::wearShield(float wearFactor)
{
	CGameItemPtr shield = getLeftHandItem();
	if (shield == NULL)
		return;
	
	const CStaticItem * form = shield->getStaticForm();
	
	if (form == NULL || form->Family != ITEMFAMILY::SHIELD)
		return;
	
	if (shield->getItemWornState() != ITEM_WORN_STATE::Worned)
	{
		double wear = shield->getWearPerAction() * wearFactor;
		shield->removeHp(wear);
	}
	else
	{
		nlwarning("Player shield is worned, destroy it");
	}
	
	// if shield now worned out, destroy it and send a chat message to player
	if (shield->getItemWornState() == ITEM_WORN_STATE::Worned)
	{
		/*SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
		params[0].SheetId = shield->getSheetId();
		sendDynamicSystemMessage( _EntityRowId, "ITEM_WORNED_DESTROYED", params);
		
		destroyItem(shield->getInventory()->getInventoryId(), shield->getInventorySlot(), shield->getStackSize(), false);
		*/
		unequipCharacter(INVENTORIES::handling, INVENTORIES::left);
	}
}

// ****************************************************************************
void CCharacter::wearJewels(float wearFactor)
{
	/************************************************************************/
	/*  static values
	/************************************************************************/
	static const SLOT_EQUIPMENT::TSlotEquipment JewelSlots[] = 
	{
		SLOT_EQUIPMENT::ANKLEL,
		SLOT_EQUIPMENT::ANKLER,
		SLOT_EQUIPMENT::EARL,
		SLOT_EQUIPMENT::EARR,
		SLOT_EQUIPMENT::FINGERL,
		SLOT_EQUIPMENT::FINGERR,
		SLOT_EQUIPMENT::HEADDRESS,
		SLOT_EQUIPMENT::NECKLACE,
		SLOT_EQUIPMENT::WRISTL,
		SLOT_EQUIPMENT::WRISTR,
	};
	static const uint NbJewelSlots = sizeof(JewelSlots) / sizeof(SLOT_EQUIPMENT::TSlotEquipment);
	/************************************************************************/

	for (uint i = 0; i < NbJewelSlots ; ++i)
	{
		wearItem(INVENTORIES::equipment, JewelSlots[i], wearFactor);
	}
}

// ****************************************************************************
void CCharacter::wearItem(INVENTORIES::TInventory invId, uint32 slot, float wearFactor)
{
	BOMB_IF( (invId != INVENTORIES::equipment && invId != INVENTORIES::handling), "call wearItem for an unequipped item", return);

	// get equipped item
	CGameItemPtr item = getItem(invId, slot);
	if (item == NULL)
		return;
	
	const CStaticItem * form = item->getStaticForm();
	if (form == NULL)
		return;
	
	if (item->getItemWornState() != ITEM_WORN_STATE::Worned)
	{
		double wear = item->getWearPerAction() * wearFactor;
		item->removeHp(wear);
	}
	else
		nlwarning("player item is worned, destroy it");

	// if armor now worned out, destroy it and send a chat message to player
	if (item->getItemWornState() == ITEM_WORN_STATE::Worned)
	{
		/*SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
		params[0].SheetId = item->getSheetId();
		sendDynamicSystemMessage(_EntityRowId, "ITEM_WORNED_DESTROYED", params);

		destroyItem(item->getInventory()->getInventoryId(), item->getInventorySlot(), item->getStackSize(), false);
		*/
		unequipCharacter(invId, slot);
	}
}

/*
 * Find an item or a stack of items matching the specified family. If found, the slot is set in returnedSlot.
 * If not found, the return value is NULL.
 * Garanties:
 * - If not NULL, the returned item is either a stack or a food item.
 * - If single item: item family is requested, and getStaticForm() returns non-null.
 * - If stack: there is at least one child, and the first child is non-null and has the request item family, and getStaticForm() returns non-null on it.
 */
// ****************************************************************************
CGameItemPtr CCharacter::getItemByFamily(INVENTORIES::TInventory invId, ITEMFAMILY::EItemFamily family, uint32 & returnedSlot)
{
	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return NULL;

	for (uint32 i = 0; i < inv->getSlotCount(); i++)
	{
		CGameItemPtr item = inv->getItem(i);
		if (item == NULL)
			continue;

		if (item->getStaticForm() && item->getStaticForm()->Family == family)
		{
			returnedSlot = i;
			return item;
		}
	}

	return NULL;
}

// ***************************************************************************
uint CCharacter::selectItems(INVENTORIES::TInventory invId, CSheetId itemSheetId, uint32 quality, std::vector<CItemSlotId> *itemList)
{
	CInventoryPtr inv = getInventory(invId);
	if (inv == NULL)
		return 0;
	
	// For all items
	uint	quantitySelected= 0;
	for (uint32 i = 0; i < inv->getSlotCount(); i++)
	{
		CGameItemPtr item = inv->getItem(i);
		if (item == NULL)
			continue;
		
		// if match, append to the list
		if (item->getSheetId()==itemSheetId && item->quality()>=quality)
		{
			quantitySelected+= item->getStackSize();
			if(itemList)
			{
				CItemSlotId		entry;
				entry.InvId= invId;
				entry.Slot= i;
				entry.Quality= item->quality();
				itemList->push_back(entry);
			}
		}
	}

	return quantitySelected;
}

// ***************************************************************************
uint CCharacter::destroyItems(const std::vector<CItemSlotId> &itemSlotIns, uint32 maxQuantity)
{
	// none to destroy actually?
	if(maxQuantity==0 || itemSlotIns.empty())
		return 0;

	// If has to destroy only some of them, must sort to take first the ones of lowest quality
	const std::vector<CItemSlotId> *itemSlots= NULL;
	std::vector<CItemSlotId>	itemSlotSorted;
	if(maxQuantity!=uint32(-1))
	{
		itemSlotSorted= itemSlotIns;
		std::sort(itemSlotSorted.begin(), itemSlotSorted.end());
		itemSlots= &itemSlotSorted;
	}
	else
	{
		// just point to the original one
		itemSlots= &itemSlotIns;
	}

	// destroy items up to the maxquantity wanted
	uint	index= 0;
	uint	totalDestroyed= 0;
	while(maxQuantity>0 && index<itemSlotIns.size())
	{
		const CItemSlotId	&itemSlot= (*itemSlots)[index];
		// locate the item
		CGameItemPtr	pItem= getItem(itemSlot.InvId, itemSlot.Slot);
		if(pItem!=NULL)
		{
			// destroy
			uint32	quantityToDestroy= maxQuantity;
			quantityToDestroy= min(quantityToDestroy, pItem->getStackSize());
			destroyItem(itemSlot.InvId, itemSlot.Slot, quantityToDestroy, false);

			// decrease if not infinity
			if(maxQuantity!=-1)
				maxQuantity-= quantityToDestroy;

			// increase count
			totalDestroyed+= quantityToDestroy;
		}

		// next slot to destroy
		index++;
	}

	return totalDestroyed;
}


