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
#include "timed_actions.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "entity_manager/entity_manager.h"
#include "phrase_utilities_functions.h"
#include "timed_action_phrase.h"
#include "player_manager/player.h"
#include "phrase_manager/s_effect.h"
#include "phrase_manager/phrase_manager.h"
#include "pvp_manager/pvp_manager_2.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "server_share/r2_vision.h"

#include "egs_sheets/egs_sheets.h"
//
#include "nel/net/message.h"

#include "game_share/visual_fx.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace	PHRASE_UTILITIES;

NL_INSTANCE_COUNTER_IMPL(CTimedAction);

extern CRandom RandomGenerator;

//-----------------------------------------------
bool CTimedAction::testCancelOnHit( sint32 attackSkillValue, CEntityBase * attacker, CEntityBase * defender)
{
	// get defender defense skill
	sint32 defenderValue;
	if ( defender->getId().getType() == RYZOMID::player )
	{
		CCharacter *pC = dynamic_cast<CCharacter*> (defender);
		if (!pC)
		{
			nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", defender->getId().toString().c_str());
			return false;
		}
		defenderValue = pC->getSkillValue(pC->getBestSkill());
	}
	else
	{
		const CStaticCreatures * form = defender->getForm();
		if ( !form )
		{
			nlwarning( "<MAGIC>invalid creature form %s in entity %s", defender->getType().toString().c_str(), defender->getId().toString().c_str() );
			return false;
		}	
		defenderValue = form->getAttackLevel();
	}
	
	//test if the spell is broken
	const uint8 chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::BreakCastResist, defenderValue - attackSkillValue);
	const uint8 roll = (uint8) RandomGenerator.rand(99);
	
	if ( roll >= chances )
		return true;
	else
		return false;
}

/*******************************************************************************/

//-----------------------------------------------
bool CTPTimedAction::validate(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	if (!actor || !actor->canEntityUseAction() )
		return false;

	return true;
}

//-----------------------------------------------
void CTPTimedAction::applyAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	BOMB_IF(!actor, "Actor is NULL", return);

	CGameItemPtr item = getAndUnlockTP(actor);
	if (item == NULL)
		return;

	// must be used by a player
	CCharacter *character = dynamic_cast<CCharacter *> (actor);
	if (!character)
	{
		nlwarning("Error, cannot cast actor in CCharacter *, actor Id %s", actor->getId().toString().c_str());
		return;
	}

	if( CPVPManager2::getInstance()->isTPValid(character, item) )
	{
		const uint32 slot = character->getTpTicketSlot();

		character->useTeleport(*item->getStaticForm());
		character->destroyItem(INVENTORIES::bag, slot, 1, true);
		character->resetTpTicketSlot();
	}
	else
	{
		if( character->getPvPRecentActionFlag() )
		{
			sendDynamicSystemMessage(character->getEntityRowId(), "PVP_TP_FORBIDEN");
		}
		else
		{
			sendDynamicSystemMessage(character->getEntityRowId(), "PVP_TP_ENEMY_REGION_FORBIDEN");
		}
	}
}

//-----------------------------------------------
void CTPTimedAction::stopAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	if (!actor)
		return;

	// must be used by a player
	CCharacter *character = dynamic_cast<CCharacter *> (actor);
	if (!character)
	{
		nlwarning("Error, cannot cast actor in CCharacter *, actor Id %s", actor->getId().toString().c_str());
		return;
	}

	getAndUnlockTP(actor);
	character->resetTpTicketSlot();

	// send message to player
	CCharacter::sendDynamicSystemMessage( actor->getId(), "TELEPORT_CANCELED" );
}

//-----------------------------------------------
CGameItemPtr CTPTimedAction::getAndUnlockTP(CEntityBase *actor)
{
	if (!actor) return NULL;
	
	// must be used by a player
	CCharacter *character = dynamic_cast<CCharacter *> (actor);
	if (!character)
	{
		nlwarning("Error, cannot cast actor in CCharacter *, actor Id %s", actor->getId().toString().c_str());
		return NULL;
	}
	
	// get used TP ticket
	uint32 ticketSlot = character->getTpTicketSlot();
	if (ticketSlot == INVENTORIES::INVALID_INVENTORY_SLOT)
	{
		//nlwarning("Error, player %s, TpTicketSlot should be != INVALID_INVENTORY_SLOT", character->getId().toString().c_str());
		return NULL;
	}
	
	// get ticket item
	CGameItemPtr item = character->getItem( INVENTORIES::bag, ticketSlot);
	if ( item == NULL)
	{
		nlwarning("No item found in slot %d in bag for player %s !! BUG", ticketSlot, character->getId().toString().c_str());
		return NULL;
	}
	// check item is a teleport item
	if ( item->getStaticForm() == NULL || item->getStaticForm()->Family != ITEMFAMILY::TELEPORT)
	{
		nlwarning("Item found in slot %d in bag for player %s  is NOT a tp ticket!! BUG", ticketSlot, character->getId().toString().c_str());
		return NULL;
	}
	
	// unlock item, Tp player and destroy item
	character->unLockItem(INVENTORIES::bag, ticketSlot,1);

	// Remove fx
	CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, character->getEntityRowId(), DSPropertyVISUAL_FX );
	CVisualFX fx;
	fx.unpack(visualFx.getValue());
	fx.Aura = MAGICFX::NoAura;
	sint64 prop;
	fx.pack(prop);
	visualFx = (sint16)prop;

	return item;
}

/*******************************************************************************/

//-----------------------------------------------
bool CDisconnectTimedAction::validate(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	if (!actor) return false;

	return true;
}

//-----------------------------------------------
void CDisconnectTimedAction::applyAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	// must be used by a player
	CCharacter *character = dynamic_cast<CCharacter *> (actor);
	if (!character)
	{
		nlwarning("Error, cannot cast actor in CCharacter *, actor Id %s", actor->getId().toString().c_str());
		return;
	}

	// disconnect player
	const uint32 userId = PlayerManager.getPlayerId(actor->getId());
	PlayerManager.disconnectPlayer( userId );
}

//-----------------------------------------------
void CDisconnectTimedAction::stopAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	// send message to player
	CCharacter::sendDynamicSystemMessage( actor->getId(), "DISCONNECT_CANCELED" );

	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( const_cast<CEntityId&> (actor->getId()) );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "CONNECTION:SERVER_QUIT_ABORT", bms) )
	{
		nlwarning("<disconnectPlayer> Msg name CONNECTION:SERVER_QUIT_ABORT not found");
		return;
	}
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(actor->getId().getDynamicId()), msgout );
}

//-----------------------------------------------
void CDisconnectTimedAction::stopBeforeExecution(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	stopAction(phrase,actor);
}


/*******************************************************************************/

//-----------------------------------------------
bool CMountTimedAction::validate(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	CBypassCheckFlags bypassCheckFlags;
	bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Invulnerability, true);

	if (!actor || !phrase || !actor->canEntityUseAction(bypassCheckFlags) )
		return false;

	CCharacter *playerChar = dynamic_cast<CCharacter*> (actor);
	if (!playerChar)
		return false;

	// test invisibility
	if ( !R2_VISION::isEntityVisibleToPlayers(playerChar->getWhoSeesMe()) )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( playerChar->getEntityRowId(), "CANT_MOUNT_WHILE_INVISIBLE" ); // not translated (only for GM)
		return false;
	}

	_EntityToMount = phrase->getTarget();
	
	CEntityBase * e = CEntityBaseManager::getEntityBasePtr( _EntityToMount );
	if( e )
	{
		const CStaticCreatures * form = e->getForm();
		if( form )
		{
			if( form->getProperties().mountable() )
			{
				if( e->getRiderEntity().isNull() )
				{
					// check distance from mount
					playerChar->setAfkState(false);
					sint32 petIndex = playerChar->getPlayerPet( _EntityToMount );
					if( (petIndex!=-1) )
					{
						COfflineEntityState state =  e->getState();
						CVector2d destination( state.X, state.Y );
						CVector2d start( playerChar->getState().X, playerChar->getState().Y );
						float distance = (float)(start - destination).sqrnorm();
						if( distance <= MaxTalkingDistSquare * 1000 * 1000 )
						{
							return true;
						}
					}
					return false;
				}
			}
			else
			{
				nlwarning("<cbAnimalMount> %d Entity %s %s is not moutable !! sheeter or client bug ?", CTickEventHandler::getGameCycle(), e->getId().toString().c_str(), e->getType().toString().c_str() );
			}
		}
		else
		{
			nlwarning("<cbAnimalMount> %d Can't found static form sheet for entity %s %s !!", CTickEventHandler::getGameCycle(), e->getId().toString().c_str(), e->getType().toString().c_str() );
		}
	}
	
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( const_cast<CEntityId&> (playerChar->getId()) );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "ANIMALS:MOUNT_ABORT", bms) )
	{
		nlwarning("<CEntityBase::tickUpdate> Msg name ANIMALS:MOUNT_ABORT not found");
		return false;
	}
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(playerChar->getId().getDynamicId()), msgout );

	return false;
}

//-----------------------------------------------
void CMountTimedAction::applyAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	BOMB_IF(!actor, "Actor is NULL", return);

	CCharacter *playerChar = dynamic_cast<CCharacter*> (actor);
	if (!playerChar)
		return;

	playerChar->mount(_EntityToMount);
}

//-----------------------------------------------
void CMountTimedAction::stopAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	if (!actor)
		return;

	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( const_cast<CEntityId&> (actor->getId()) );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "ANIMALS:MOUNT_ABORT", bms) )
	{
		nlwarning("<CEntityBase::tickUpdate> Msg name ANIMALS:MOUNT_ABORT not found");
		return;
	}
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(actor->getId().getDynamicId()), msgout );
}

//-----------------------------------------------
void CMountTimedAction::stopBeforeExecution(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	stopAction(phrase,actor);
}


/*******************************************************************************/


//-----------------------------------------------
bool CUnmountTimedAction::validate(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	CBypassCheckFlags bypassCheckFlags;
	bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::OnMount, true);
	bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Invulnerability, true);

	if (!actor || !phrase || !actor->canEntityUseAction(bypassCheckFlags) )
		return false;

	return true;
}

//-----------------------------------------------
void CUnmountTimedAction::applyAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	BOMB_IF(!actor, "Actor is NULL", return);

	CCharacter *playerChar = dynamic_cast<CCharacter*> (actor);
	if (!playerChar)
		return;

	playerChar->unmount();
}

//-----------------------------------------------
void CUnmountTimedAction::stopAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
}

//-----------------------------------------------
void CUnmountTimedAction::stopBeforeExecution(CTimedActionPhrase *phrase, CEntityBase *actor)
{
}

/*******************************************************************************/


//-----------------------------------------------
bool CConsumeItemTimedAction::validate(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	CCharacter *player = dynamic_cast<CCharacter *> (actor);
	// get consumed item
	BOMB_IF(player == NULL, "actor is null", return false);

	CGameItemPtr consumedItem = player->getConsumedItem();
	if (consumedItem ==NULL)
		return false;

	const CStaticItem *form = consumedItem->getStaticForm();
	if (!form || !form->ConsumableItem)
		return false;

	CBypassCheckFlags bypassCheckFlags;

	if (form->ConsumableItem->Flags.Mektoub != 0)
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::OnMount, true);

	if (form->ConsumableItem->Flags.Sit != 0)
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::WhileSitting, true);

	if (form->ConsumableItem->Flags.Swim != 0)
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::InWater, true);
	
	if (form->ConsumableItem->Flags.StandUp == false)
	{
		// check player is sit, on a mektoub or swimming otherwise return false
		const MBEHAV::EMode mode = player->getMode();
		if (mode != MBEHAV::SIT && mode != MBEHAV::MOUNT_NORMAL && mode != MBEHAV::MOUNT_SWIM && mode != MBEHAV::SWIM && !player->isInWater())
		{
			CCharacter::sendDynamicSystemMessage(player->getId(),"CONSUMABLE_NOT_STAND_UP");
			return false;
		}
	}

	if (!actor || !phrase || !player->canEntityUseAction(bypassCheckFlags) )
		return false;

	//NB: overdose timer is checked before lauching this action
	
	return true;
}

//-----------------------------------------------
void CConsumeItemTimedAction::applyAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	BOMB_IF(!actor, "Actor is NULL", return);
	
	CCharacter *playerChar = dynamic_cast<CCharacter*> (actor);
	if (!playerChar)
		return;

	// get consumed item form
	CGameItemPtr consumedItem = playerChar->getConsumedItem();
	if (consumedItem ==NULL)
		return;

	uint16 quality = consumedItem->quality();
	
	_Form = consumedItem->getStaticForm();
	if (!_Form || !_Form->ConsumableItem)
		return;

	// destroy consumed item
	playerChar->destroyConsumedItem();
	playerChar->disableConsumableFamily(_Form->ConsumableItem->Family, CTickEventHandler::getGameCycle() + TGameCycle(_Form->ConsumableItem->OverdoseTimer /  CTickEventHandler::getGameTimeStep()) );
	
	// execute power
	CPhraseManager::getInstance().useConsumableItem(actor->getEntityRowId(), _Form, quality);
	
}

//-----------------------------------------------
void CConsumeItemTimedAction::stopAction(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	BOMB_IF(actor == NULL, "actor is null", return);
	CCharacter *playerChar = dynamic_cast<CCharacter*> (actor);
	if (!playerChar)
		return;

	CCharacter::sendDynamicSystemMessage(playerChar->getId(),"CONSUMABLE_CANCEL");
//	Fix: item are already desalocated, and _Form is NULL
	playerChar->resetConsumedItem(true);
}

//-----------------------------------------------
void CConsumeItemTimedAction::stopBeforeExecution(CTimedActionPhrase *phrase, CEntityBase *actor)
{
	BOMB_IF(actor == NULL, "actor is null", return);
	CCharacter *playerChar = dynamic_cast<CCharacter*> (actor);
	if (!playerChar)
		return;

	CCharacter::sendDynamicSystemMessage(playerChar->getId(),"CONSUMABLE_CANCEL");

	playerChar->resetConsumedItem(true);
}

//-----------------------------------------------
bool CConsumeItemTimedAction::testCancelOnHit( sint32 attackSkillValue, CEntityBase * attacker, CEntityBase * defender)
{
	// get consumed item
	CCharacter *player = dynamic_cast<CCharacter *> (defender);
	// get consumed item
	BOMB_IF(player == NULL, "actor is null", return false);

	CGameItemPtr consumedItem = player->getConsumedItem();
	if (consumedItem ==NULL)
		return true;

	const CStaticItem *form = consumedItem->getStaticForm();
	if (!form || !form->ConsumableItem)
		return true;

	return form->ConsumableItem->Flags.BreakWhenHit;
}
