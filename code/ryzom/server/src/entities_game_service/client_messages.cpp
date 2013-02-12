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

//net
#include "nel/net/message.h"
#include "nel/misc/command.h"
#include "nel/misc/algo.h"
#include "nel/misc/common.h"
//game_share
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/sphrase_com.h"
#include "game_share/security_check.h"
#include "server_share/log_item_gen.h"
//egs
#include "client_messages.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "entity_manager/entity_callbacks.h"
#include "phrase_manager/phrase_manager_callbacks.h"
#include "zone_manager.h"
#include "phrase_manager/phrase_manager.h"
#include "mission_manager/mission_queue_manager.h"
#include "building_manager/building_manager.h"
#include "team_manager/team_manager.h"
#include "creature_manager/creature_manager.h"
#include "egs_sheets/egs_sheets.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_manager/pvp_manager_2.h"
#include "pvp_manager/pvp.h"
#include "building_manager/room_instance.h"
#include "player_manager/gear_latency.h"
#include "outpost_manager/outpost_manager.h"
#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"
#include "guild_manager/guild_member.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_manager.h"
#include "modules/r2_mission_item.h"
#include "modules/r2_give_item.h"
#include "game_share/scenario.h"
#include "server_share/r2_vision.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

extern CGenericXmlMsgHeaderManager	GenericMsgManager;

CVariable<bool> BuildSpireActive( "egs", "BuildSpireActive", "Activate build spire", true, 0, true );


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:ITEM
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
void cbClientItemDrop( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbItemDrop(msgin, serviceName, serviceId);
}

//

void cbClientItemPickUpClose( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	cbItemClosePickup(msgin, serviceName, serviceId);
}

//
void cbClientItemSwap( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbItemSwap(msgin, serviceName, serviceId);
}

//
void cbClientItemHarvest( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbHarvest(msgin, serviceName, serviceId);
}

//
void cbClientItemHarvestClose( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbHarvestClose(msgin, serviceName, serviceId);
}

//
void cbClientItemEquip( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientItemEquip);

	uint16 equippedInventory, equippedSlot, bagSlot;
	CEntityId id;

	msgin.serial( id );
	msgin.serial( equippedInventory );
	msgin.serial( equippedSlot );
	msgin.serial( bagSlot );

	CCharacter *c = (CCharacter * ) CEntityBaseManager::getEntityBasePtr( id );
	if( c )
	{
		c->setAfkState(false);
		c->incInterfaceCounter();

		// if player is stunned or dead cancel action
		if (c->isDead() || c->isStunned())
			return;
		c->equipCharacter( INVENTORIES::TInventory(equippedInventory), equippedSlot, bagSlot, true );
	}
}

//
void cbClientItemUnequip( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientItemUnequip);

	uint16 equippedInventory, equippedSlot;
	CEntityId id;

	msgin.serial( id );
	msgin.serial( equippedInventory );
	msgin.serial( equippedSlot );

	CCharacter *c = (CCharacter * ) CEntityBaseManager::getEntityBasePtr( id );
	if( c )
	{
		c->setAfkState(false);
		c->incInterfaceCounter();

		// if player is stunned or dead cancel action
		if (c->isDead() || c->isStunned())
			return;

		c->unequipCharacter( INVENTORIES::TInventory(equippedInventory), equippedSlot, true );
	}
}


//----------------------------
//	cbItemDestroy
//----------------------------
void cbClientItemDestroy( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientItemDestroy);

	CEntityId user;
	uint16 inventory,slot,quantity;

	msgin.serial(user);
	msgin.serial(inventory);
	msgin.serial(slot);
	msgin.serial(quantity);

	CCharacter *character = PlayerManager.getChar( user );
	if (character == NULL)
	{
		nlwarning("<cbItemDestroy> Invalid player Id %s", user.toString().c_str() );
		return;
	}

	// check character is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbItemDestroy> player Id %s not yet ready", user.toString().c_str() );
		return;
	}

	character->setAfkState(false);
	character->destroyItem( INVENTORIES::TInventory(inventory), slot, quantity, true );
}

// take an item from temp inventory to bag
void cbClientItemTempToBag( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientItemTempToBag);

	CEntityId user;
	uint16 slot;

	msgin.serial(user);
	msgin.serial(slot);

	CCharacter *character = PlayerManager.getChar( user );
	if (character == NULL)
	{
		nlwarning("<cbClientItemTempToBag> Invalid player Id %s", user.toString().c_str() );
		return;
	}

	// check character is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbClientItemTempToBag> player Id %s not yet ready", user.toString().c_str() );
		return;
	}

	character->setAfkState(false);
	character->itemTempInventoryToBag( slot );
}

// get all item from temp inventory
void cbClientItemAllTemp( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientItemAllTemp);

	CEntityId user;
	msgin.serial(user);

	CCharacter *character = PlayerManager.getChar( user );
	if (character == NULL)
	{
		nlwarning("<cbClientItemAllTemp> Invalid player Id %s", user.toString().c_str() );
		return;
	}
	// check character is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbClientItemAllTemp> player Id %s not yet ready", user.toString().c_str() );
		return;
	}

	character->setAfkState(false);
	character->sendCloseTempInventoryImpulsion();
}

// clear temp inventory
void cbClientItemNoTemp( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientItemNoTemp);

	CEntityId user;
	msgin.serial(user);

	CCharacter *character = PlayerManager.getChar( user );
	if (character == NULL)
	{
		nlwarning("<cbClientItemNoTemp> Invalid player Id %s", user.toString().c_str() );
		return;
	}
	// check character is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbClientItemAllTemp> player Id %s not yet ready", user.toString().c_str() );
		return;
	}

	TLogContext_Item_DropTempInventory dropTemp(user);

	character->setAfkState(false);
	character->clearTempInventory();
}

// Enchant or recharge an item
void cbClientItemEnchant( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientItemEnchant);

	CEntityId user;
	msgin.serial(user);

	uint8 inv;
	uint16 slot;
	msgin.serial(inv, slot);

	CCharacter *character = PlayerManager.getChar( user );
	if (character == NULL)
	{
		nlwarning("<cbClientItemEnchant> Invalid player Id %s", user.toString().c_str() );
		return;
	}
	// check character is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbClientItemEnchant> player Id %s not yet ready", user.toString().c_str() );
		return;
	}
	character->enchantOrRechargeItem( (INVENTORIES::TInventory) inv, slot );
	character->setAfkState(false);
}

// use a tp item
void cbClientItemUseItem( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientItemUseItem);

	CEntityId user;
	msgin.serial(user);

	uint16 slot;
	msgin.serial(slot);

	CCharacter *character = PlayerManager.getChar( user );
	if (character == NULL)
	{
		nlwarning("<USEITEM> Invalid player Id %s", user.toString().c_str() );
		return;
	}
	// check character is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<USEITEM> player Id %s not yet ready", user.toString().c_str() );
		return;
	}

	character->useItem(slot);
}

void cbClientItemStopUseXpCat( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientItemStopUseXpCat);

	CEntityId user;
	msgin.serial(user);

	bool isRingCatalyser;
	msgin.serial(isRingCatalyser);

	CCharacter *character = PlayerManager.getChar( user );
	if (character == NULL)
	{
		nlwarning("<cbClientItemStopUseXpCat> Invalid player Id %s", user.toString().c_str() );
		return;
	}
	// check character is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbClientItemStopUseXpCat> player Id %s not yet ready", user.toString().c_str() );
		return;
	}

	character->stopUseItem( isRingCatalyser );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:HARVEST
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void cbClientHarvestDeposit( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbHarvestDeposit(msgin, serviceName, serviceId);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:PHRASE
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// execute client sentence
void cbClientPhraseExecute( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientPhraseExecute);

	CEntityId	Id;
	msgin.serial( Id );

	uint8 set,slot;
	msgin.serial(set,slot);

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("<cbClientPhraseExecute> Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("<cbClientPhraseExecute> player Id %s not yet ready", Id.toString().c_str() );
		return;
	}

	ch->executeMemorizedPhrase( set, slot, false , false);
	ch->setAfkState(false);
}


//
void cbClientPhraseExecuteCyclic( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientPhraseExecuteCyclic);

	CEntityId	Id;
	msgin.serial( Id );

	uint8 set,slot;
	msgin.serial(set,slot);

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("<cbClientPhraseExecuteCyclic> Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("<cbClientPhraseExecuteCyclic> player Id %s not yet ready", Id.toString().c_str() );
		return;
	}

	ch->setAfkState(false);
	ch->executeMemorizedPhrase( set, slot, true,false );
}


//
void cbClientPhraseExecuteFaber( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientPhraseExecuteFaber);

	CEntityId	Id;
	msgin.serial( Id );

	NLMISC::CSheetId craftPlan;
	msgin.serial( craftPlan );

	uint8 set,slot;
	msgin.serial(set,slot);

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("player Id %s not yet ready", Id.toString().c_str() );
		return;
	}


	if ( ch->getKnownBricks().find( craftPlan ) == ch->getKnownBricks().end())
	{
		string name = CEntityIdTranslator::getInstance()->getByEntity(Id).toString();
		nlwarning("HACK: %s %s tries to craft a brick he doesn't know %s", Id.toString().c_str(), name.c_str(), craftPlan.toString().c_str());
		return;
	}

	ch->setCraftPlan( craftPlan );

	ch->clearFaberRms();
	msgin.serialCont( ch->getFaberRmsNoConst() );
	msgin.serialCont( ch->getFaberRmsFormulaNoConst() );

	ch->executeMemorizedPhrase( set, slot, false,false );
	ch->setAfkState(false);
}


//
void cbClientPhraseMemorize( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientPhraseMemorize);

	CEntityId	Id;
	msgin.serial( Id );

	uint8 set,slot;
	msgin.serial(set, slot);

	uint16 phraseId;
	msgin.serial( phraseId );

	CSPhraseCom phraseDesc;
	msgin.serial( phraseDesc );

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("<cbClientPhraseMemorize> Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("<cbClientPhraseMemorize> player Id %s not yet ready", Id.toString().c_str() );
		return;
	}

	// memorize
	ch->memorize(set, slot,phraseId,phraseDesc.Bricks);
	ch->setAfkState(false);
}

//
void cbClientPhraseForget( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientPhraseForget);

	CEntityId	Id;
	msgin.serial( Id );

	uint8 set,slot;
	msgin.serial(set,slot);

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("<cbClientPhraseForget> Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("<cbClientPhraseForget> player Id %s not yet ready", Id.toString().c_str() );
		return;
	}

	ch->forgetPhrase( set, slot);
	ch->setAfkState(false);
}

//
void cbClientPhraseDelete( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientPhraseDelete);

	CEntityId	Id;
	msgin.serial( Id );

	uint16 phraseId;
	msgin.serial( phraseId );

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("<cbClientPhraseDelete> Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("<cbClientPhraseDelete> player Id %s not yet ready", Id.toString().c_str() );
		return;
	}

	ch->deleteKnownPhrase(phraseId);
	ch->setAfkState(false);
}

//
void cbClientPhraseLearn( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientPhraseLearn);

	CEntityId	Id;
	msgin.serial( Id );

	uint16 phraseId;
	msgin.serial( phraseId );

	CSPhraseCom phrase;
	msgin.serial( phrase );

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("<cbClientPhraseLearn> Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("<cbClientPhraseLearn> player Id %s not yet ready", Id.toString().c_str() );
		return;
	}

	ch->learnPhrase( phrase.Bricks, phraseId, phrase.Name );
	ch->setAfkState(false);
}


void cbClientPhraseBuyByIndex( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientPhraseBuyByIndex);

	CEntityId	Id;
	msgin.serial( Id );

	uint8 botChatIndex;
	msgin.serial( botChatIndex );

	uint16 phraseId;
	msgin.serial( phraseId );

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("<cbClientPhraseBuyByIndex> Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("<cbClientPhraseBuyByIndex> player Id %s not yet ready", Id.toString().c_str() );
		return;
	}

	ch->incInterfaceCounter();
	ch->buyPhraseByIndex( botChatIndex, phraseId);
	ch->setAfkState(false);
}

void cbClientPhraseBuyBySheet( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientPhraseBuyBySheet);

	CEntityId	Id;
	msgin.serial( Id );

	CSheetId	phraseSheetId;
	uint32		val;
	msgin.serial( val );
	phraseSheetId = val;

	uint16		index;
	msgin.serial( index );

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("<cbClientPhraseBuyBySheet> Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("<cbClientPhraseBuyBySheet> player Id %s not yet ready", Id.toString().c_str() );
		return;
	}

	ch->incInterfaceCounter();
	ch->buyPhraseBySheet(phraseSheetId,index);
	ch->setAfkState(false);
}

void cbClientPhraseCancelTop( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientPhraseCancelTop);

	if ( ! Mirror.mirrorIsReady() )
	{
		nlwarning("<cbClientPhraseCancelTop> Received from %s service but mirror not yet ready", serviceName.c_str() );
		return;
	}

	NLMISC::CEntityId entityId;
	msgin.serial( entityId );

	TDataSetRow	entityRowId = TheDataset.getDataSetRow(entityId);

	CPhraseManager::getInstance().cancelTopPhrase(entityRowId);
} // cbClientPhraseCancelTop //

void cbClientPhraseCancelAll( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientPhraseCancelAll);

	if ( ! Mirror.mirrorIsReady() )
	{
		nlwarning("<cbClientPhraseCancelAll> Received from %s service but mirror not yet ready", serviceName.c_str() );
		return;
	}

	NLMISC::CEntityId entityId;
	msgin.serial( entityId );

	TDataSetRow	entityRowId = TheDataset.getDataSetRow(entityId);

	CPhraseManager::getInstance().cancelAllPhrases(entityRowId);
} // cbClientPhraseCancelAll //

void cbClientPhraseCristalize( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientPhraseCristalize);

	CEntityId	Id;
	msgin.serial( Id );

	uint8 set,slot;
	msgin.serial(set,slot);

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( !ch )
	{
		nlwarning("<cbClientPhraseCristalize> Unknown character %s", Id.toString().c_str() );
		return;
	}
	// check character is ready
	if (!ch->getEnterFlag())
	{
		nlwarning("<cbClientPhraseCristalize> player Id %s not yet ready", Id.toString().c_str() );
		return;
	}

	ch->executeMemorizedPhrase( set, slot, false, true );
	ch->setAfkState(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:COMBAT
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
void cbClientCombatEngage( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbEngage(msgin, serviceName, serviceId);
}

//
void cbClientCombatDisengage( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbDisengage( msgin, serviceName, serviceId);
}

//
void cbClientCombatDefaultAttack( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbDefaultAttack(msgin, serviceName, serviceId);
}


void cbClientValidateMeleeCombat( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientValidateMeleeCombat);

	CEntityId idPlayer;
	msgin.serial( idPlayer );

	uint8 flag;
	msgin.serial(flag);

	CCharacter *ch = PlayerManager.getChar( idPlayer );
	if ( ch && ch->getEnterFlag() )
	{
		ch->validateMeleeCombat(flag != 0);
		//ch->setAfkState(false);
	}
}

void cbClientCombatProtectedSlot( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientCombatProtectedSlot);

	CEntityId idPlayer;
	msgin.serial( idPlayer );

	uint8 slot;
	msgin.serial( slot );

	CCharacter *ch = PlayerManager.getChar( idPlayer );
	if ( ch && ch->getEnterFlag() )
	{
		ch->protectedSlot( SLOT_EQUIPMENT::TSlotEquipment(slot) );
		ch->setAfkState(false);
	}
}

void cbClientCombatParry( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientCombatParry);

	CEntityId idPlayer;
	msgin.serial( idPlayer );

	CCharacter *ch = PlayerManager.getChar( idPlayer );
	if ( ch && ch->getEnterFlag() )
	{
		ch->dodgeAsDefense( false );
		ch->setAfkState(false);
	}
}

void cbClientCombatDodge( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientCombatDodge);

	CEntityId idPlayer;
	msgin.serial( idPlayer );

	CCharacter *ch = PlayerManager.getChar( idPlayer );
	if ( ch && ch->getEnterFlag() )
	{
		ch->dodgeAsDefense( true );
		ch->setAfkState(false);
	}
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:LEAGUE
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
void cbClientLeagueJoin( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbJoinLeague(msgin, serviceName, serviceId);
}

//
void cbClientLeagueJoinProposal( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbJoinLeagueProposal(msgin, serviceName, serviceId);
}

//
void cbClientLeagueJoinProposalDecline( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbJoinLeagueDecline(msgin, serviceName, serviceId);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:TEAM
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
void cbClientTeamLeave( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbLeaveTeam(msgin, serviceName, serviceId);
}

//
void cbClientTeamJoin( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbJoinTeam(msgin, serviceName, serviceId);
}

//
void cbClientTeamKick( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbKickTeammate(msgin, serviceName, serviceId);
}

//
void cbClientTeamJoinProposal( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbJoinTeamProposal(msgin, serviceName, serviceId);
}

//
void cbClientTeamJoinProposalDecline( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbJoinTeamDecline(msgin, serviceName, serviceId);
}

void cbClientTeamSetSuccessor( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientTeamSetSuccessor);

	CEntityId id;
	uint8 idx;
	msgin.serial(id);
	msgin.serial(idx);
	CCharacter* user = PlayerManager.getChar( id );
	if ( !user )
	{
		nlwarning("<TEAM> Invalid user %s",id.toString().c_str() );
		return;
	}
	CTeam * team = TeamManager.getTeam( user->getTeamId() );
	if ( !team )
	{
		nlwarning("<TEAM> Invalid team for user %s",id.toString().c_str() );
		return;
	}
	if ( team->getLeader() != id )
	{
		nlwarning("<TEAM> user %s is not leader: cant set successor",id.toString().c_str() );
		return;
	}
	if (team->getSuccessor() == id)
	{
		nlwarning("<TEAM> user %s already is successor", id.toString().c_str() );
		return;
	}

	// increment the target index as the leader is not in its team list
	++idx;
	team->setSuccessor( idx );

}

void cbClientTeamShareValidItem( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientTeamShareValidItem);

	CEntityId id;
	msgin.serial(id);
	CCharacter * user = PlayerManager.getChar(id);
	if ( !user )
	{
		nlwarning("<cbClientTeamShareValidItem>invalid char %s",id.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientTeamShareValidItem> player Id %s not yet ready", id.toString().c_str() );
		return;
	}

	user->setAfkState(false);
	CTeam * team = TeamManager.getRealTeam(user->getTeamId());
	if (!team)
	{
		nlwarning("<cbClientTeamShareValidItem>char %s has no valid team",id.toString().c_str());
		return;
	}
	if ( !team->getReward() )
	{
		nlwarning("<cbClientTeamShareValidItem>char %s: his team has no reward",id.toString().c_str());
		return;
	}

	uint8 pos,state;
	msgin.serial(pos);
	msgin.serial(state);
	team->getReward()->userItemSelect(user->getEntityRowId(),pos,state);
}

void cbClientTeamShareInvalidItem( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientTeamShareInvalidItem);

	CEntityId id;
	msgin.serial(id);
	CCharacter * user = PlayerManager.getChar(id);
	if ( !user )
	{
		nlwarning("<cbClientTeamShareInvalidItem>invalid char %s",id.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientTeamShareInvalidItem> player Id %s not yet ready", id.toString().c_str() );
		return;
	}
	user->setAfkState(false);
	CTeam * team = TeamManager.getRealTeam(user->getTeamId());
	if (!team)
	{
		nlwarning("<cbClientTeamShareInvalidItem>char %s has no valid team",id.toString().c_str());
		return;
	}
	if ( !team->getReward() )
	{
		nlwarning("<cbClientTeamShareInvalidItem>char %s: his team has no reward",id.toString().c_str());
		return;
	}

	uint8 pos,state;
	msgin.serial(pos);
	msgin.serial(state);
	team->getReward()->userItemSelect(user->getEntityRowId(),pos,state);
}

void cbClientTeamShareValid( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientTeamShareValid);

	CEntityId id;
	msgin.serial(id);
	CCharacter * user = PlayerManager.getChar(id);
	if ( !user )
	{
		nlwarning("<cbClientTeamShareValid>invalid char %s",id.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientTeamShareValid> player Id %s not yet ready", id.toString().c_str() );
		return;
	}
	user->setAfkState(false);
	CTeam * team = TeamManager.getRealTeam(user->getTeamId());
	if (!team)
	{
		nlwarning("<cbClientTeamShareValid>char %s has no valid team",id.toString().c_str());
		return;
	}
	if ( !team->getReward() )
	{
		nlwarning("<cbClientTeamShareValid>char %s: his team has no reward",id.toString().c_str());
		return;
	}

	uint8 state;
	msgin.serial(state);
	if ( team->getReward()->userValidSelect(user->getEntityRowId(),state) )
		team->clearReward();
}

//---------------------------------------------------
// Add a player to contact list or ignore list
//---------------------------------------------------
void cbClientAddToContactList( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientAddToContactList);

	CEntityId	charId;
	ucstring	playerName;
	uint8		list;

	msgin.serial(charId);
	msgin.serial(playerName);
	msgin.serial(list);

	CCharacter * c = PlayerManager.getChar( charId );
	if( c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		if (list == 0)
		{
			c->addPlayerToFriendList(playerName);
		}
		else
		{
			c->addPlayerToIgnoreList(playerName);
		}
	}
	else
		nlwarning("<cbAddToContactList> Unknown character %s", charId.toString().c_str() );
}

//---------------------------------------------------
// remove a player from contact list or ignore list
//---------------------------------------------------
void cbClientRemoveFromContactList( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientRemoveFromContactList);

	CEntityId	charId;
	uint32		contactId;
	uint8		list;

	msgin.serial(charId);
	msgin.serial(contactId);
	msgin.serial(list);

	CCharacter * c = PlayerManager.getChar( charId );
	if( c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		if (list == 0)
		{
			c->removePlayerFromFriendListByContactId(contactId);
		}
		else
		{
			c->removePlayerFromIgnoreListByContactId(contactId);
		}
	}
	else
		nlwarning("<cbAddToContactList> Unknown character %s", charId.toString().c_str() );
}

//---------------------------------------------------
// Mobe a player from a contact list to an another one
//---------------------------------------------------
void cbClientMoveInContactLists( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientMoveInContactLists);

	CEntityId	charId;
	uint32		contactIdOrigin;
	uint8		listOrigin;

	msgin.serial(charId);
	msgin.serial(contactIdOrigin);
	msgin.serial(listOrigin);

	CCharacter * c = PlayerManager.getChar( charId );
	if( c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		if (listOrigin == 0)
		{
			NLMISC::CEntityId	eid= c->getFriendByContactId(contactIdOrigin);
			// allows the whole operation or nothing if player does not exist
			if (eid != CEntityId::Unknown)
			{
				c->removePlayerFromFriendListByEntityId(eid);
				c->addPlayerToIgnoreList(eid);
			}
			else
			{
				// player not found => message
				PHRASE_UTILITIES::sendDynamicSystemMessage( c->getEntityRowId(), "OPERATION_NOTEXIST");
			}
		}
		else
		{
			NLMISC::CEntityId	eid= c->getIgnoreByContactId(contactIdOrigin);
			// allows the whole operation or nothing if player is not present
			if(eid != CEntityId::Unknown)
			{
				c->removePlayerFromIgnoreListByEntityId(eid);
				c->addPlayerToFriendList(eid);
			}
			else
			{
				// player not found => message
				PHRASE_UTILITIES::sendDynamicSystemMessage( c->getEntityRowId(), "OPERATION_NOTEXIST");
			}
		}
	}
	else
		nlwarning("<cbAddToContactList> Unknown character %s", charId.toString().c_str() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:TP
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// teleport near a bot
/*void cbClientTpBot( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId idPlayer;
	string name;
	CEntityId botId;
	msgin.serial( idPlayer );
	msgin.serial( name );

	CCharacter *ch = PlayerManager.getChar( idPlayer );
	if ( ch && ch->havePriv(PriviliegeGameMaster))
	{
		CEntityBase* entity = PlayerManager.getCharacterByName( name );
		if( ! entity )
		{
			TAIAlias alias = CAIAliasTranslator::getInstance()->getNPCAliasFromName(name);
			if ( ! CAIAliasTranslator::getInstance()->getEntityId(alias,botId) )
			{
				nlwarning("<cbClientTpBot> Unknown alias %d", alias );
				return;
			}
			entity = CreatureManager.getCreature( botId );
		}
		if ( entity )
		{
			sint32 x = entity->getState().X + sint32( cos(entity->getState().Heading) *  2000 );
			sint32 y = entity->getState().Y + sint32( sin(entity->getState().Heading) *  2000 );
			ch->allowNearPetTp();
			ch->tpWanted( x,y, entity->getState().Z );
		}
		else
		{
			nlwarning("<cbClientTpBot> Unknown bot %s", botId.toString().c_str() );
		}
	}
	else
	{
		nlwarning("<cbClientTpBot> Unknown character %s or don't have priv", idPlayer.toString().c_str() );
	}
}*/

//
/*void cbClientTpWanted( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId Id;
	sint32 x, y, z;

	msgin.serial( Id );
	msgin.serial( x );
	msgin.serial( y );
	msgin.serial( z );

	CCharacter *ch = PlayerManager.getChar( Id );
	if ( ch && ch->havePriv(PriviliegeGameMaster))
	{
		ch->forbidNearPetTp();
		ch->tpWanted( x, y , z );
	}
	else
	{
		nlwarning("<cbClientTpWanted> Unknown character %s or don't have priv", Id.toString().c_str() );
	}

}*/


// client want tp at respawn point (beta test features for work-arround stuck collide)
/*void cbClientTpRespawn( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId characterId;
	msgin.serial( characterId );

	CCharacter *ch = PlayerManager.getChar( characterId );
	if ( ch )
	{
		COfflineEntityState state = PlayerManager.getStartState( ch->getActiveRole() );
		ch->allowNearPetTp();
		ch->tpWanted( state.X, state.Y, state.Z );
	}
}*/


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:ANIMALS
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
void cbClientAnimalsBeast( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbAnimalCommand(msgin, serviceName, serviceId);
}

/*
 void cbClientAnimalsDisbandConvoy( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId id;
	msgin.serial( id );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		c->clearBeastTrain();
	}
	else
	{
		nlwarning("<cbClientAnimalsDisbandConvoy> received message from unknown client %s", id.toString().c_str() );
	}
}
*/


/*
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:TRADE
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


void cbClientTradeNextPage( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId userId;
	uint16 session;
	msgin.serial(userId);
	msgin.serial(session);
	CCharacter * c = PlayerManager.getChar( userId );
	if ( c )
		;//c->tradeNextPage(session);
	else
		nlwarning("<cbClientTradeNextPage> Invalid user %s",userId.toString().c_str());

}


void cbClientTradeMissionNextPage( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId userId;
	uint16 session;
	msgin.serial(userId);
	msgin.serial(session);
	CCharacter * c = PlayerManager.getChar( userId );
	if ( c )
		;//c->tradeMissionNextPage(session);
	else
		nlwarning("<cbClientTradeNextPage> Invalid user %s",userId.toString().c_str());
}

// Client want buy an item
void cbClientTradeBuy( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	cbTradeBuySomething(msgin, serviceName, serviceId);
}


// Client ask selling price of an item
void cbClientTradeQuerySellPrice( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId id;
	uint16 Inventory, Slot;

	msgin.serial( id );
	msgin.serial( Inventory );
	msgin.serial( Slot );

	CCharacter * c = PlayerManager.getChar( id );
	if( c )
	{
		c->askSellPrice( Inventory, Slot );
	}
	else
	{
		nlwarning("<cbClientTradeQuerySellPrice> received message from unknown client %s", id.toString().c_str() );
	}
}

// Client sell an item
void cbClientTradeSellItem( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId id;
	uint16 Inventory, Slot;
	uint16 Quantity;

	msgin.serial( id );
	msgin.serial( Inventory );
	msgin.serial( Slot );
	msgin.serial( Quantity );

	CCharacter * c = PlayerManager.getChar( id );
	if( c )
	{
		c->sellItem( Inventory, Slot, Quantity );
	}
	else
	{
		nlwarning("<cbClientTradeSellItem> received message from unknown client %s", id.toString().c_str() );
	}
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:EXCHANGE
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
void cbClientExchangeProposal( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbExchangeProposal(msgin, serviceName, serviceId);
}

//
void cbClientExchangeAcceptInvitation( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbAcceptExchangeInvitation(msgin, serviceName, serviceId);
}

//
void cbClientExchangeDeclineInvitation( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbDeclineExchangeInvitation(msgin, serviceName, serviceId);
}

//
void cbClientExchangeSeeds( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbExchangeSeeds(msgin, serviceName, serviceId);
}

//
void cbClientExchangeEnd( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbEndExchange(msgin, serviceName, serviceId);
}

//
void cbClientExchangeValidate( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	cbAcceptExchange(msgin, serviceName, serviceId);
}


void cbClientExchangeInvalidate( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientExchangeInvalidate);

	CEntityId id;
	msgin.serial(id);
	CCharacter * c = PlayerManager.getChar( id );
	if (c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		c->invalidateExchange();
	}
	else
		nlwarning("<cbClientExchangeInvalidate : invalid char %s>",id.toString().c_str());
}

void cbClientExchangeAdd( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientExchangeAdd);

	CEntityId id;
	msgin.serial(id);

	uint16 slotSrc, slotDest;
	uint16 quantity;

	msgin.serial(slotSrc);
	msgin.serial(slotDest);
	msgin.serial(quantity);

	CCharacter * c = PlayerManager.getChar( id );
	if (c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		c->incInterfaceCounter();
		c->itemBagToExchange(slotSrc, slotDest, quantity);
	}
	else
		nlwarning("<cbClientExchangeAdd : invalid char %s>",id.toString().c_str());
}

void cbClientExchangeRemove( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientExchangeRemove);

	CEntityId id;
	msgin.serial(id);

	uint16 slotSrc;
	msgin.serial(slotSrc);

	CCharacter * c = PlayerManager.getChar( id );
	if (c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		c->incInterfaceCounter();
		c->itemExchangeToBag(slotSrc);
	}
	else
		nlwarning("<cbClientExchangeRemove : invalid char %s>",id.toString().c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		BRICK/SENTENCE RELATED COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
/*void cbClientSentenceCancelCurrent( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	cbCancelCurrentSentence(msgin, serviceName, serviceId);
}

//
void cbClientSentenceCancel( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	cbCancelSentence(msgin, serviceName, serviceId);
}

//
void cbClientSentenceCancelAll( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	cbCancelAllSentences(msgin, serviceName, serviceId);
}
*/




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		bot chat missions commands
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/// start the static mission interface
void cbClientBotChatChooseStaticMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

/// ask for the next mission page
void cbClientBotChatNextMissionPage( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

/// pick a static mission
void cbClientBotChatPickStaticMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

///continue a mission by ending the current step
void cbClientContinueMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

///validate a gift to a bot
void cbClientValidateMissionGift( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

/// client ends a bot gift
void cbClientBotChatEndGift( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

/// client sends a dynamic bot chat nswer
void cbClientBotChatDynChatSend( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

// client abandons a mission
void cbClientAbandonMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

// client abandons a group mission
void cbClientGroupAbandonMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		bot chat trade
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


/// open interface for item trade
void cbClientBotChatStartTradeItem( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatStartTradeItem);

	uint16 session;
	CEntityId userId;
	msgin.serial(userId,session);
	CCharacter * user = PlayerManager.getChar(userId);

	if ( !user )
	{
		nlwarning("<cbClientBotChatStartTradeItem> Invalid char %s",userId.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientBotChatStartTradeItem> player Id %s not yet ready", userId.toString().c_str() );
		return;
	}

	// start a bot chat
	if ( !user->startBotChat( BOTCHATTYPE::TradeItemFlag ) )
	{
		return;
	}

	user->resetRawMaterialItemPartFilter();
	user->resetItemTypeFilter();
	user->startTradeItemSession(session);
	user->setAfkState(false);
}

/// open interface for teleport trade
void cbClientBotChatStartTradeTp( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatStartTradeTp);

	uint16 session;
	CEntityId userId;
	msgin.serial(userId,session);
	CCharacter * user = PlayerManager.getChar(userId);

	if ( !user )
	{
		nlwarning("<cbClientBotChatStartTradeTp> Invalid char %s",userId.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientBotChatStartTradeTp> player Id %s not yet ready", userId.toString().c_str() );
		return;
	}

	// start a bot chat
	if ( !user->startBotChat( BOTCHATTYPE::TradeTeleportFlag ) )
	{
		return;
	}
	user->startTradeItemSession(session);
	user->setAfkState(false);
}

/// open interface for faction item trade
void cbClientBotChatStartTradeFaction( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatStartTradeFaction);

	uint16 session;
	CEntityId userId;
	msgin.serial(userId,session);
	CCharacter * user = PlayerManager.getChar(userId);

	if ( !user )
	{
		nlwarning("<cbClientBotChatStartTradeFaction> Invalid char %s",userId.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientBotChatStartTradeFaction> player Id %s not yet ready", userId.toString().c_str() );
		return;
	}

	// start a bot chat
	if ( !user->startBotChat( BOTCHATTYPE::TradeFactionFlag ) )
	{
		return;
	}
	user->startTradeItemSession(session);
	user->setAfkState(false);
}

/// start trade guild options
void cbClientBotChatStartTradeGuildOptions( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatStartTradeGuildOptions);
	uint16 session;
	CEntityId userId;
	msgin.serial(userId,session);
	CBuildingManager::getInstance()->buildBuildingTradeList(userId,session);
}

/// buy a guild option
void cbClientBotChatBuyGuildOptions( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatBuyGuildOptions);
	uint8 idx;
	CEntityId userId;
	msgin.serial( userId,idx );
	CBuildingManager::getInstance()->buyBuildingOption( userId,idx );
}

/// start research with a guild bot
void cbClientBotChatStartGuildResearch( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatStartGuildResearch);

	uint16 session;
	CEntityId userId;
	msgin.serial(userId,session);

	/// todo appart
//	CGuildManager::getInstance()->buildGuildResearchList(userId,session);
}


/// trade next page
void cbClientBotChatTradeNextPage( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatTradeNextPage);

	uint16 session;
	CEntityId userId;
	msgin.serial(userId,session);
	CCharacter * user = PlayerManager.getChar(userId);

	if ( !user )
	{
		nlwarning("<cbClientBotChatTradeNextPage> Invalid char %s",userId.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientBotChatTradeNextPage> player Id %s not yet ready", userId.toString().c_str() );
		return;
	}
	user->fillTradePage(session);

}

/// buy an item
void cbClientBotChatTradeBuy( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatTradeBuy);

	uint16 idx;
	uint16 quantity;
	CEntityId userId;
	msgin.serial(userId,idx,quantity);
	CCharacter * user = PlayerManager.getChar(userId);

	if ( !user )
	{
		nlwarning("<cbClientBotChatTradeBuy> Invalid char %s",userId.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientBotChatTradeBuy> player Id %s not yet ready", userId.toString().c_str() );
		return;
	}
	user->buyItem( idx,quantity);
	user->setAfkState(false);
}

// destroy an item in sale store
void cbClientBotChatTradeDestroy( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatTradeDestroy);
	uint16 idx;
	uint16 quantity;
	CEntityId userId;
	msgin.serial(userId,idx,quantity);
	CCharacter * user = PlayerManager.getChar(userId);

	if ( !user )
	{
		nlwarning("<cbClientBotChatTradeDestroy> Invalid char %s",userId.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientBotChatTradeDestroy> player Id %s not yet ready", userId.toString().c_str() );
		return;
	}
	user->destroySaleItem( idx,quantity);
	user->setAfkState(false);
}

// sell item
void cbClientBotChatTradeSell( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatTradeSell);

	uint8 inv;
	uint8 idx;
	uint16 quantity;
	uint32 price;
	CEntityId userId;
	msgin.serial(userId,inv,idx,quantity,price);
	CCharacter * user = PlayerManager.getChar(userId);
	if ( !user )
	{
		nlwarning("<cbClientBotChatTradeSell> Invalid char %s",userId.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientBotChatTradeSell> player Id %s not yet ready", userId.toString().c_str() );
		return;
	}
	user->sellItem(INVENTORIES::TInventory(inv),idx,quantity,price);
	user->setAfkState(false);
}

// start a guild creation session
void cbClientBotChatStartGuildCreation( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatStartGuildCreation);

	CEntityId id;
	msgin.serial(id);
	CCharacter * user = PlayerManager.getChar(id);
	if ( !user )
	{
		nlwarning("<cbClientBotChatStartGuildCreation> Invalid char %s",id.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientBotChatStartGuildCreation> player Id %s not yet ready", id.toString().c_str() );
		return;
	}

	user->setAfkState(false);

	if ( !user->startBotChat( BOTCHATTYPE::CreateGuildFlag ) )
		return;
}


// start trade actions
void cbClientBotChatStartTradeAction( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatStartTradeAction);

	uint16 sessionId;
	CEntityId userId;

	msgin.serial(userId, sessionId);

	CCharacter * user = PlayerManager.getChar(userId);
	if ( !user )
	{
		nlwarning("<cbClientBotChatStartTradeAction> Invalid char %s",userId.toString().c_str());
		return;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientBotChatStartTradeAction> player Id %s not yet ready", userId.toString().c_str() );
		return;
	}
	user->setAfkState(false);

	// start a bot chat
	if ( !user->startBotChat( BOTCHATTYPE::TradePhraseFlag ) )
	{
		return;
	}

	user->startTradePhrases(sessionId);
}


void cbClientBotChatSetFilters( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatSetFilters);
	CEntityId userId;

	uint32 minQuality, maxQuality, minPrice, maxPrice;
	uint8 minClass, maxClass;
	uint8  itemPartFilter;
	uint8  itemTypeFilter;

	msgin.serial(userId, minQuality, maxQuality, minPrice, maxPrice);
	msgin.serial(minClass, maxClass, itemPartFilter, itemTypeFilter);

	CCharacter * user = PlayerManager.getChar(userId);
	if ( !user )
	{
		nlwarning("<cbClientBotChatSetFilters> Invalid char %s",userId.toString().c_str());
		return;
	}

	user->setAfkState(false);
	user->setFilters( minQuality, maxQuality, minPrice, maxPrice, (RM_CLASS_TYPE::TRMClassType)minClass, (RM_CLASS_TYPE::TRMClassType)maxClass, (RM_FABER_TYPE::TRMFType)itemPartFilter, (ITEM_TYPE::TItemType)itemTypeFilter );
}

void cbRefreshTradeList( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbRefreshTradeList);
	CEntityId userId;

	msgin.serial( userId );
	CCharacter * user = PlayerManager.getChar(userId);
	if ( !user )
	{
		nlwarning("<cbClientBotChatSetFilters> Invalid char %s",userId.toString().c_str());
		return;
	}
	user->refreshTradeList();
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		Client bot chat end
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbClientBotChatEnd( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientBotChatEnd);

	CEntityId userId;
	msgin.serial(userId);
	CCharacter* user = PlayerManager.getChar( userId );
	if (user && user->getEnterFlag())
	{
		user->endBotChat();
		user->incInterfaceCounter();
		user->setAfkState(false);
	}
	else
		nlwarning("<cbClientBotChatEnd> invalid character %s", userId.toString().c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		PLAYER COMMAND
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// client send an emote
void cbClientSendEmote( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientSendEmote);

	CEntityId id;
	MBEHAV::EBehaviour behaviour = MBEHAV::IDLE;
	uint16 emoteTextId;
	try
	{
		msgin.serial( id );
		msgin.serialEnum( behaviour );
		msgin.serial( emoteTextId );

	}
	catch(const Exception &e)
	{
		nlwarning("Bad emote serialisation '%s'", e.what());
		return;
	}

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->getEnterFlag() )
	{
		// check if client can launch this emote from his UI
		const CStaticTextEmotes & emotes = CSheets::getTextEmoteList();
		const CStaticTextEmotes::CTextEmotePhrases * phraseCont = emotes.getPhrase( emoteTextId );
		if ( phraseCont != NULL )
		{
			if( phraseCont->UsableFromClientUI == false )
			{
				nlwarning("<cbClientSendEmote> client %s sent a forbidden emote : %u",c->getId().toString().c_str(),emoteTextId );
				return;
			}
		}

		c->sendEmote( id, behaviour, emoteTextId );
	}
	else
	{
		nlwarning("<cbClientSendEmote> received message from unknown client %s", id.toString().c_str() );
	}
}


// client send a custom emote
void cbClientSendCustomEmote( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientSendCustomEmote);

	CEntityId id;
	MBEHAV::EBehaviour behaviour = MBEHAV::IDLE;
	ucstring emoteCustomText;
	try
	{
		msgin.serial( id );
		msgin.serialEnum( behaviour );
		msgin.serial( emoteCustomText );

	}
	catch(const Exception &e)
	{
		nlwarning("Bad custom emote serialisation '%s'", e.what());
		return;
	}

	if(behaviour >= 140 && behaviour <= 169)
	{
		string name = CEntityIdTranslator::getInstance()->getByEntity(id).toString();
		nlwarning("HACK: %s %s tries to launch a firework %d", id.toString().c_str(), name.c_str(), behaviour);
		return;
	}

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->getEnterFlag() )
	{
		c->sendCustomEmote( id, behaviour, emoteCustomText );
	}
	else
	{
		nlwarning("<cbClientSendCustomEmote> received message from unknown client %s", id.toString().c_str() );
	}
}

// client send a command where for known where is it
void cbClientSendWhere( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientSendWhere);

	CEntityId id;
	msgin.serial( id );
	CZoneManager::getInstance().answerWhere(id);
}

// client send a command where for known where is it
void cbClientSendAfk( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientSendAfk);

	CEntityId id;
	bool afk;
	msgin.serial( id );
	msgin.serial( afk );
	CCharacter * user = PlayerManager.getChar( id );
	if ( !user )
	{
		nlwarning("<AFK>'%s' is not a valid user",id.toString().c_str() );
		return;
	}
	user->setAfkState( afk );
}

// client send a command to roll a dice
void cbClientRollDice( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientRollDice);

	CEntityId entityId;
	sint16 min;
	sint16 max;
	sint16 roll;
	msgin.serial( entityId );
	msgin.serial( min );
	msgin.serial( max );

	static NLMISC::CRandom* dice = (NLMISC::CRandom*)NULL;
	if (!dice)
	{
		dice = new NLMISC::CRandom;
		dice->srand(CTickEventHandler::getGameCycle());
	}

	roll = min + (sint16)dice->rand(max-min);

	SM_STATIC_PARAMS_4(params, STRING_MANAGER::player, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
	params[0].setEIdAIAlias(entityId, CAIAliasTranslator::getInstance()->getAIAlias(entityId));
	params[1].Int = min;
	params[2].Int = max;
	params[3].Int = roll;

	TDataSetRow playerRowId = TheDataset.getDataSetRow( entityId );
	if ( !TheDataset.isAccessible(playerRowId) )
		return;

	PHRASE_UTILITIES::sendDynamicSystemMessage(playerRowId, "ROLL_DICE", params);
	STRING_MANAGER::sendSystemStringToClientAudience(playerRowId, std::vector<NLMISC::CEntityId>(), CChatGroup::shout, "ROLL_DICE", params);
}

// client send command sit (for sit/up)
void cbClientSit( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientSit);

	CEntityId id;
	bool sit;
	msgin.serial( id );
	msgin.serial( sit );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		if( sit )
		{
			if( !c->isInWater() && c->getMode()!=MBEHAV::MOUNT_NORMAL && c->getMode()!=MBEHAV::DEATH )
			{
				c->setMode( MBEHAV::SIT );
				// only if player is'nt equipping an item
				//( to prevent exploit "sit to reduce equip latency time")
				if( !c->getGearLatency().isLatent() )
				{
					c->cancelStaticActionInProgress(STATIC_ACT_TYPES::Neutral, false, false);
				}
				c->cancelStaticEffects();
			}
		}
		else
		{
			c->setMode( MBEHAV::NORMAL );
		}
	}
	else
	{
		nlwarning("<cbClientSendWhere> received message from unknown client %s", id.toString().c_str() );
	}
}


// client send a command to change guild motd
void cbClientGuildMotd( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildMotd);

	CEntityId entityId;
	string motd;
	msgin.serial( entityId );
	msgin.serial( motd );

	CCharacter * user = PlayerManager.getChar( entityId );
	if (!user)
	{
		nlwarning("<cbClientGuildMotd>'%s' is not a valid char",entityId.toString().c_str());
		return;
	}
	if (!user->getEnterFlag())
	{
		nlwarning("<cbClientGuildMotd>'%s' is not entered", entityId.toString().c_str());
		return;
	}
	if (!TheDataset.isAccessible(user->getEntityRowId()))
	{
		nlwarning("<cbClientGuildMotd>'%s' is not valid in mirror", entityId.toString().c_str());
		return;
	}

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
	if( guild )
	{
		guild->setMOTD(motd, entityId);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:CHEAT
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// turn character to god mod
//void cbGodMode( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId);
/*void cbClientCheatGod( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId id;
	msgin.serial( id );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->havePriv(PriviliegeGameMaster))
	{
		c->ToggleGodMode();
	}

//	cbGodMode(msgin, serviceName, serviceId);
}*/

/*
void cbClientCreateItemInBag( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId charId;
	msgin.serial( charId );

	CSheetId itemSheetId;
	msgin.serial( itemSheetId );

	uint16 quantity;
	msgin.serial( quantity );

	uint16 quality;
	msgin.serial( quality );

	// get character
	CCharacter *c = PlayerManager.getChar( charId );
	if (c&& c->havePriv(PriviliegeGameMaster))
	{
		nlwarning("<cbClientCreateItemInBag> Invalid player Id %s", charId.toString().c_str() );
		return;
	}

	c->createItemInBag( quality, quantity, itemSheetId );
}*/

/*// character learn all existing bricks
void cbClientLearnAllBricks( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId id;
	msgin.serial( id );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->havePriv(PriviliegeGameMaster))
		GameItemManager.learnAllBricks( c );
	else
		nlwarning("cbClientLearnAllBricks, unknown player '%s'", id.toString().c_str() );
}
*/

// client ask to add money to his character
/*void cbClientMoney( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CEntityId id;
	msgin.serial ( id );

	sint32 vb, b, m, s;

	msgin.serial ( vb );
	msgin.serial ( b );
	msgin.serial ( m );
	msgin.serial ( s );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->havePriv(PriviliegeGameMaster))
	{
		c->giveSeed( CSeeds( vb, b, m, s ) );
	}
}*/

/*// Client want set Ryzom Time
void cbClientRyzomTime( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CEntityId	Id;
	msgin.serial( Id );

	if (!PlayerManager.havePriv(Id, PriviliegeGameMaster))
		return;

	float		Time;
	msgin.serial( Time );

	CMessage msgout( "SET_RYZOM_TIME" );
	msgout.serial( Time );
	sendMessageViaMirror( "WOS", msgout );
}

// Client want set Ryzom Day
void cbClientRyzomDay( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CEntityId	Id;
	msgin.serial( Id );

	if (!PlayerManager.havePriv(Id, PriviliegeGameMaster))
		return;

	uint32		Day;
	msgin.serial( Day );

	CMessage msgout( "SET_RYZOM_DAY" );
	msgout.serial( Day );
	sendMessageViaMirror( "WOS", msgout );
}
*/
// Client want gain xp in indicated skill
/*void cbClientGainXp( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CEntityId Id;
	string	Skill, Speciality;
	uint32	Xp;

	msgin.serial( Id );
	msgin.serial( Xp );
	msgin.serial( Skill );

	CCharacter * c = PlayerManager.getChar( Id );
	if( c && c->havePriv(PriviliegeGameMaster))
	{
		c->addXpToSkill( (double) Xp, Skill );
	}
}
*/
// Client create a new character
/*void cbClientCreateCharacter( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CEntityId id;
	string characterName;
	EGSPD::CPeople::TPeople people;
	GSGENDER::EGender gender;
//Deprecated
//	ROLES::ERole role;
	uint16 level;

	msgin.serial( id );
	msgin.serial( characterName );
	msgin.serialEnum( people );
	msgin.serialEnum( gender );
//	msgin.serialEnum( role );
	msgin.serial( level );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->havePriv(PriviliegeGameMaster))
	{
		uint32 playerId = PlayerManager.getPlayerId( id );
		CPlayer * player = PlayerManager.getPlayer( playerId );
		if( player )
		{
			player->replaceCharacter( characterName, people, gender, level );
			PlayerManager.savePlayer( playerId );
		}
	}
}
*/
// Client create a new character
/*void cbClientAddRole( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CEntityId id;
	ROLES::ERole role;
	uint16 level;

	msgin.serial( id );
	msgin.serialEnum( role );
	msgin.serial( level );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->havePriv(PriviliegeGameMaster))
	{
		c->getRoles().addNewJob( JOBS::getJobForRace( role, (EGSPD::CPeople::TPeople) c->getRace() ) );
	}
}
*/
// Client want to learn bricks
/*void cbClientLearnBrick( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CEntityId id;
	ROLES::ERole role;
	uint16 level;

	msgin.serial( id );
	msgin.serialEnum( role );
	msgin.serial( level );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->havePriv(PriviliegeGameMaster))
	{
		GameItemManager.learnAllBricksForRoleAndRace( c, role, (EGSPD::CPeople::TPeople) c->getRace(), level );
	}
}*/

/// cient want to learn all faber plans
/*void cbClientLearnAllFaberPlans( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CEntityId id;

	msgin.serial( id );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->havePriv(PriviliegeGameMaster))
	{
		c->learnAllFaberPlans();
	}
} // cbClientLearnAllFaberPlans //
*/

/*
 * CLIENT:DEBUG
 */

// Send server client coordinate
void cbClientWhere( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientWhere);

	CEntityId id;
	msgin.serial( id );
	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		CMessage msgout( "IMPULSION_ID" );
		msgout.serial( id );
		CBitMemStream bms;
		if ( ! GenericMsgManager.pushNameToStream( "DEBUG:REPLY_WHERE", bms) )
		{
			nlwarning("<cbClientWhere> Msg name DEBUG:REPLY_WHERE not found");
			return;
		}
		bms.serial( const_cast< sint32& > (c->getState().X.getValue()) );
		bms.serial( const_cast< sint32& > (c->getState().Y.getValue()) );
		bms.serial( const_cast< sint32& > (c->getState().Z.getValue()) );
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(id.getDynamicId()), msgout );
	}
}


// local macro definition ( undef below )
#define SEND_GM_WHO_ANSWER( _code_ , _msg_) {\
	gms = PlayerManager.getSpecialUsers(_code_);\
	if ( gms && !gms->empty() )\
	{\
		for ( uint i = 0; i < gms->size(); i++ )\
		{\
			TDataSetRow row = TheDataset.getDataSetRow( (*gms)[i] );\
			if  ( TheDataset.isAccessible( row ) )\
			{\
				CMirrorPropValue<TYPE_WHO_SEES_ME> whoSeesMe(TheDataset, row, DSPropertyWHO_SEES_ME );\
				if ( R2_VISION::isEntityVisibleToPlayers(whoSeesMe) )\
				{\
					params[0].setEId((*gms)[i]);\
					CCharacter::sendDynamicSystemMessage( id,_msg_,params );\
					nbAnswers++;\
				}\
			}\
		}\
	}\
}

// local macro definition ( undef below )
#define SEND_GM_WHO_ANSWER_INVIS( _code_ , _msg_) {\
	gms = PlayerManager.getSpecialUsers(_code_);\
	if ( gms && !gms->empty() )\
	{\
		for ( uint i = 0; i < gms->size(); i++ )\
		{\
			TDataSetRow row = TheDataset.getDataSetRow( (*gms)[i] );\
			if  ( TheDataset.isAccessible( row ) )\
			{\
				CMirrorPropValue<TYPE_WHO_SEES_ME> whoSeesMe(TheDataset, row, DSPropertyWHO_SEES_ME );\
				if ( !R2_VISION::isEntityVisibleToPlayers(whoSeesMe) )\
				{\
					params[0].setEId( (*gms)[i] );\
					CCharacter::sendDynamicSystemMessage( id,_msg_,params );\
					nbAnswers++;\
				}\
			}\
		}\
	}\
}

// Send list of connected character name / account to client
void cbClientWho( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientWho);

	CEntityId id;
	std::string opt;
	msgin.serial( id,opt );
	const std::vector<CEntityId> * gms = NULL;

	// Make sure opt is not like "A(c)" for e acute
	ucstring ucopt;
	ucopt.fromUtf8(opt);
	opt = ucopt.toString();

	uint nbAnswers = 0;

	// standard who
	if ( opt.empty() )
	{
		CCharacter * user = PlayerManager.getChar( id );
		if ( !user )
		{
			nlwarning("<WHO>'%s' is invalid", id.toString().c_str() );
			return;
		}
		CRegion* region = dynamic_cast<CRegion*>( CZoneManager::getInstance().getPlaceFromId( user->getCurrentRegion() ) );
		if ( region == NULL )
		{
			nlwarning("<WHO>'%s' Invalid region %u", id.toString().c_str(), user->getCurrentRegion() );
			return;
		}

		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::place);
			params[0].Identifier = region->getName();
			CCharacter::sendDynamicSystemMessage( id,"WHO_REGION_INTRO",params );
		}

		// send all users in region
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
			for ( set<CEntityId>::const_iterator it = region->getPlayersInside().begin(); it != region->getPlayersInside().end(); ++it )
			{
				TDataSetRow row = TheDataset.getDataSetRow( (*it) );
				if  ( TheDataset.isAccessible( row ) )
				{
					CCharacter *ch = PlayerManager.getChar( *it );

					// check that user are visible to each other and in the same instance
					if ( R2_VISION::isEntityVisibleToPlayers(ch->getWhoSeesMe())
						&& user->getInstanceNumber() == ch->getInstanceNumber())
					{
						params[0].setEId( (*it) );
						CCharacter::sendDynamicSystemMessage( id,"WHO_REGION_LIST", params );
						nbAnswers++;
					}
				}
			}
		}

	}
	// GM who
	else if ( NLMISC::nlstricmp( opt.c_str(), "GM" ) == 0)
	{
		TVectorParamCheck params(1);
		params[0].Type = STRING_MANAGER::player;
		CCharacter::sendDynamicSystemMessage( id,"WHO_GM_INTRO" );
		SEND_GM_WHO_ANSWER( ":SGM:","WHO_SGM_LIST" );
		SEND_GM_WHO_ANSWER( ":GM:","WHO_GM_LIST" );
		SEND_GM_WHO_ANSWER( ":VG:","WHO_VG_LIST" );
		SEND_GM_WHO_ANSWER( ":SG:","WHO_SG_LIST" );
		SEND_GM_WHO_ANSWER( ":G:","WHO_G_LIST" );

		CCharacter * user = PlayerManager.getChar( id );
		if ( user )
		{
			CPlayer *p = PlayerManager.getPlayer(PlayerManager.getPlayerId(user->getId()));
			if (p != NULL)
			{
				bool allowIt = p->havePriv(":DEV:");
				if( allowIt || p->havePriv(":SGM:GM:VG:SG:G:") )
				{
					nbAnswers = 0;
					TVectorParamCheck params(1);
					params[0].Type = STRING_MANAGER::player;
					CCharacter::sendDynamicSystemMessage( id,"WHO_INVISIBLE_GM_INTRO" );
					if ( allowIt || p->havePriv(":SGM:") )
					{
						SEND_GM_WHO_ANSWER_INVIS( ":SGM:","WHO_SGM_LIST" );
						allowIt = true;
					}
					if ( allowIt || p->havePriv(":SGM:GM:") )
					{
						SEND_GM_WHO_ANSWER_INVIS( ":GM:","WHO_GM_LIST" );
						allowIt = true;
					}
					if ( allowIt || p->havePriv(":SGM:GM:VG:") )
					{
						SEND_GM_WHO_ANSWER_INVIS( ":VG:","WHO_VG_LIST" );
					}
					if ( allowIt || p->havePriv(":SGM:GM:VG:SG:") )
					{
						SEND_GM_WHO_ANSWER_INVIS( ":SG:","WHO_SG_LIST" );
					}
					SEND_GM_WHO_ANSWER_INVIS( ":G:","WHO_G_LIST" );
				}
			}
		}
	}
	else
	{
		TChanID chanID;

		CCharacter * user = PlayerManager.getChar( id );
		if ( !user )
		{
			nlwarning("<WHO>'%s' is invalid", id.toString().c_str() );
			return;
		}
		CPlayer *p = PlayerManager.getPlayer(PlayerManager.getPlayerId(user->getId()));
		
		if (NLMISC::nlstricmp( opt.c_str(), "league" ) == 0)
		{
			chanID = user->getLeagueId();
		}
		else
		{
			chanID = DynChatEGS.getChanIDFromName(opt);
		}

		if (chanID == DYN_CHAT_INVALID_CHAN)
		{
			CCharacter::sendDynamicSystemMessage( id, "WHO_CHANNEL_NOT_FOUND" );
			return;
		}

		bool havePriv = p->havePriv(":DEV:SGM:GM:EM:");
		bool hasChannel = false;
		nbAnswers = 0;

		vector<NLMISC::CEntityId> players;
		DynChatEGS.getPlayersInChan(chanID, players);
		ucstring playerNames("");
		uint32 shardId = CEntityIdTranslator::getInstance()->getEntityShardId(id);

		for (uint i = 0; i < players.size(); i++)
		{
			if (players[i] == id)
				hasChannel = true;

			ucstring name = CEntityIdTranslator::getInstance()->getByEntity(players[i]);
			if (shardId == CEntityIdTranslator::getInstance()->getEntityShardId(players[i]))
			{
				// Same shard, remove shard from name
				CEntityIdTranslator::removeShardFromName(name);
			}
			playerNames += ((i > 0) ? "\n" : "") + name ;
		}

		if (!hasChannel && !havePriv)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal = opt;
			CCharacter::sendDynamicSystemMessage( id, "WHO_CHANNEL_NOT_CONNECTED", params );

			return;
		}

		if (!playerNames.empty())
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal = opt;
			CCharacter::sendDynamicSystemMessage( id, "WHO_CHANNEL_INTRO", params);
			//playerNames = "Players in channel \"" + opt + "\":" + playerNames;
			params[0].Literal = playerNames;
			CCharacter::sendDynamicSystemMessage( id, "LITERAL", params );
			return;
		}
	}

	if ( nbAnswers == 0 )
	{
		CCharacter::sendDynamicSystemMessage( id,"WHO_NO_ANSWER" );
	}
}

#undef SEND_GM_WHO_ANSWER_INVIS
#undef SEND_GM_WHO_ANSWER

// received a ping from a client, write it to client database
void cbClientPing( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId id;
	msgin.serial( id );

	TGameCycle time;
	msgin.serial( time );

	CCharacter * c = PlayerManager.getChar( id );
	if( c )
	{
//		c->_PropertyDatabase.setProp( c->getDataIndexReminder()->DATABASE_PING.PING, time );
		CBankAccessor_PLR::getDEBUG_INFO().setPing(c->_PropertyDatabase, time );
	}
}


extern void cbClientAdmin( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
extern void cbClientAdminOffline( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
extern void cbClientRemoteAdmin( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);


/************************************************************************/
/*							GUILD CALLBACKS								*/
/*				implemented in guild_client_callbacks.cpp				*/
/************************************************************************/

// client wants to create a guild
void cbClientGuildCreate( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
/// set the leader
void cbClientGuildSetLeader( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
/// set playergrade
void cbClientGuildSetGrade( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
//client asked another to join his guild
void cbClientGuildJoinInvitation( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
//client accept invitation
void cbClientGuildAcceptJoinInvitation( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
//client refused invitation
void cbClientGuildRefuseJoinInvitation( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
/// kick a member
void cbClientGuildKickMember( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
/// quit the guild
void cbClientGuildQuit( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
/// item : bag to guild
//void cbClientBagToGuild( NLNET::CMessage& msgin, const std::string & serviceName, uint16 serviceId );
/// item : guild to bag
//void cbClientGuildToBag( NLNET::CMessage& msgin, const std::string & serviceName, uint16 serviceId );
/// money : bag to guild
void cbClientGuildPutMoney( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
/// money : guild to bag
void cbClientGuildTakeMoney( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );

/// outpost callbacks (implemented in guild_client_callbacks.cpp)
void cbClientOutpostSetSquad(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostSetSquadSpawnZone(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostInsertSquad(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostRemoveSquad(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostSetExpenseLimit(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
//void cbClientOutpostBuyBuilding(NLNET::CMessage & msgin, const std::string & serviceName, uint16 serviceId);
//void cbClientOutpostDestroyBuilding(NLNET::CMessage & msgin, const std::string & serviceName, uint16 serviceId);
void cbClientOutpostSideChosen( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );
void cbClientOutpostSelect(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostUnselect(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostWarStart(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostWarValidate(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostGiveup(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostBanishPlayer(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostBanishGuild(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
void cbClientOutpostSetDefensePeriod(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);

// client asked for a lift destination
void cbClientGuildLiftDestAsked( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eId;
	msgin.serial( eId );
	uint16 session;
	msgin.serial(session);
	CBuildingManager::getInstance()->fillTriggerPage(eId, session,true );
}

// client asked for a lift next page
void cbClientGuildLiftNextPage( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eId;
	msgin.serial( eId );
	uint16 session;
	msgin.serial(session);
	CBuildingManager::getInstance()->fillTriggerPage(eId, session, false );
}

// client asked for a lift teleport
void cbClientGuildLiftTeleport( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eId;
	msgin.serial( eId );
	uint16 index;
	msgin.serial(index);
	CCharacter * user = PlayerManager.getChar( eId );
	if ( user )
		CBuildingManager::getInstance()->triggerTeleport(user,index);
}

void cbClientSetCharacterTitle( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eId;
	uint8 title;
	msgin.serial( eId );
	msgin.serial( title );

	CCharacter * c = PlayerManager.getChar(eId);
	if ( !c )
	{
		nlwarning("<cbClientSetCharacterTitle> Invalid user %s",eId.toString().c_str());
		return;
	}
	c->setAfkState(false);

	const uint32 userId = PlayerManager.getPlayerId(eId);
	CPlayer * p = PlayerManager.getPlayer(userId);
	if (p == NULL)
	{
		nlwarning("<cbClientSetCharacterTitle> Invalid player %u", userId);
		return;
	}

	if ( title == uint8(CHARACTER_TITLE::FBT) )
	{
		if ( !p->isBetaTester() )
			return;
	}
	else if ( title == uint8(CHARACTER_TITLE::WIND) )
	{
		if ( !p->isWindermeerCommunity() )
			return;
	}
	else if ( title >= uint8(CHARACTER_TITLE::BeginGmTitle) && title <= uint8(CHARACTER_TITLE::EndGmTitle) )
	{
		return;
	}

	// kxu: TODO: check validity of title chosen by player
	c->setNewTitle(CHARACTER_TITLE::toString((CHARACTER_TITLE::ECharacterTitle)title));
	c->registerName();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:ITEM_INFO
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbClientItemInfos( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	userId;
	uint16		slotId;
	msgin.serial(userId,slotId);
	CCharacter * user = PlayerManager.getChar(userId);
	if ( user && user->getEnterFlag() && TheDataset.isAccessible( user->getEntityRowId() ) )
		user->sendItemInfos( slotId );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:MISSION_PREREQ
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbClientMissionPrereq( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	userId;
	uint8		slotId;
	msgin.serial(userId, slotId);
	CCharacter * user = PlayerManager.getChar(userId);
	if ( user && user->getEnterFlag() && TheDataset.isAccessible( user->getEntityRowId() ) )
		user->sendMissionPrerequisitInfos( slotId );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:DEATH:ASK_RESPAWN
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbClientAskRespawn( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	userId;
	uint16		index;
	msgin.serial(userId,index);

	CCharacter * user = PlayerManager.getChar(userId);
	if ( user && user->getEnterFlag() )
	{
		user->respawn( index );
		user->setAfkState(false);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:TARGET
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbClientCompassEnableDynamic( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	userId;
	//TDataSetIndex	compressedIndex;
	uint32 compressedIndex;

	msgin.serial(userId);
	msgin.serial(compressedIndex);

	TDataSetRow index = TheDataset.getCurrentDataSetRow( compressedIndex );
	if ( ! TheDataset.isAccessible(index) )
	{
		nlwarning("<cbClientCompassEnableDynamic>: compressed index results in an invalid datasetrow");
		return;
	}

	CCharacter * user = PlayerManager.getChar(userId);
	if ( user && user->getEnterFlag() )
	{
		user->setCompassTarget( index );
	}
}

void cbClientCompassDisableDynamic( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	static const TDataSetRow invalidRow;

	CEntityId	userId;
	msgin.serial(userId);

	CCharacter * user = PlayerManager.getChar(userId);
	if ( user && user->getEnterFlag() )
	{
		user->setCompassTarget( invalidRow );
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT: PVP choose clan
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
void cbClientPvpChooseClan( NLNET::CMessage& msgin, const std::string & serviceName, uint16 serviceId )
{
	CEntityId userId;
	uint8 clan;

	msgin.serial(userId,clan);

	CCharacter * user = PlayerManager.getChar(userId);
	if ( user && user->getEnterFlag() )
	{
		if ( user->getPVPInterface().isValid() )
			user->getPVPInterface().setUserClan( clan );
	}
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:MISSION
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbClientMissionEnterCritical( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);

	bool accept;
	msgin.serial(accept);

	CCharacter *player = PlayerManager.getChar(userId);
	if (player)
		CMissionQueueManager::getInstance()->playerEntersCriticalArea(userId, player->getEnterCriticalZoneProposalQueueId(), accept);


}

void cbClientMissionWake( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );

void cbClientMissionGroupWake( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId );

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:DUEL
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void cbClientDuelAsked( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	CPVPManager2::getInstance()->askForDuel( userId );
}

void cbClientDuelAccept( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	CPVPManager2::getInstance()->acceptDuel( userId );
}

void cbClientDuelRefuse( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	CPVPManager2::getInstance()->refuseDuel( userId );
}

void cbClientDuelAbandon( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	CPVPManager2::getInstance()->abandonDuel( userId );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:PVP CHALLENGE
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void cbClientPVPChallengeAsked( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	CPVPManager::getInstance()->askForPVPChallenge( userId );
}

void cbClientPVPChallengeAccept( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	CPVPManager::getInstance()->acceptPVPChallenge( userId );
}

void cbClientPVPChallengeRefuse( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	CPVPManager::getInstance()->refusePVPChallenge( userId );
}

void cbClientPVPChallengeAbandon( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	CCharacter * user = PlayerManager.getChar( userId );
	if ( !user )
	{
		nlwarning("<PVP>%s is invalid",userId.toString().c_str());
		return;
	}
	CTeam * team = TeamManager.getTeam( user->getTeamId() );
	if ( team && team->getLeader() != userId )
	{
		CCharacter::sendDynamicSystemMessage( userId,"CHALLENGE_ABANDON_NOT_LEADER" );
		return;
	}

	if ( user->getPVPInterface().isValid() )
		user->getPVPInterface().leavePVP( IPVP::AbandonChallenge );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:PVP FACTION / GUILDE / ALLEGIANCE
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbClientPvPSetTag( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId charId;
	msgin.serial(charId);

	bool tagPVP = false;
	uint8 flag;
	msgin.serial(flag);
	if(flag) tagPVP = true;

	CCharacter * c = PlayerManager.getOnlineChar(charId);
	if( c != 0)
	{
		c->setPVPFlag( tagPVP );
	}
	else
	{
		nlwarning("<cbClientPvPSetTag> %s is invalid", charId.toString().c_str());
	}
}

void cbClientPvPSetNeutralAllegiance( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId charId;
	msgin.serial(charId);

	uint8 allegianceU8;
	msgin.serial(allegianceU8);

	PVP_CLAN::TPVPClan allegiance = (PVP_CLAN::TPVPClan)allegianceU8;

	CCharacter * c = PlayerManager.getOnlineChar(charId);
	if( c != 0)
	{
		c->setAllegianceFromIndeterminedStatus(allegiance);
	}
	else
	{
		nlwarning("<cbClientPvPSetNeutralAllegiance> %s is invalid", charId.toString().c_str());
	}
}

void cbClientPvPSetNeutralAllegianceGuild( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId charId;
	msgin.serial(charId);

	uint8 allegianceU8;
	msgin.serial(allegianceU8);

	PVP_CLAN::TPVPClan allegiance = (PVP_CLAN::TPVPClan)allegianceU8;

	CCharacter * c = PlayerManager.getOnlineChar(charId);
	if( c != 0)
	{
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId(c->getGuildId());
		if( guild )
		{
			CGuildMember * gm = guild->getMemberFromEId(charId);
			if( gm->getGrade() == EGSPD::CGuildGrade::Leader )
			{
				guild->setAllegianceFromIndeterminedStatus(allegiance);
			}
			else
			{
				CCharacter::sendDynamicSystemMessage( charId,"GUILD_INSUFFICIENT_GRADE" );
			}
		}
		else
		{
			nlwarning("<cbClientPvPSetNeutralAllegianceGuild> Player Character % have no guild", charId.toString().c_str());
		}
	}
	else
	{
		nlwarning("<cbClientPvPSetNeutralAllegiance> %s is invalid", charId.toString().c_str());
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void cbClientQuitGameRequest( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId charId;
	bool bypassDisconnectionTimer = false;
	try
	{
		msgin.serial(charId);
		if (IsRingShard)
			bypassDisconnectionTimer = true;
		else
		{
			msgin.serial(bypassDisconnectionTimer);
			if (bypassDisconnectionTimer)
			{
				// For fast-disconnection on a mainland shard, one needs to produce credentials!
				H_AUTO(BypassDiscoTimer);
				CSecurityCheckForFastDisconnection securityCheck;
				securityCheck.receiveSecurityCode(msgin);
				CPlayer *player = PlayerManager.getPlayer(uint32(charId.getShortId()) >> 4);
				if (player)
					securityCheck.setCookie(player->getLoginCookie()); // if not set (null player), the check won't pass

				securityCheck.check("");
			}
		}
	}
	catch (const Exception &e) // will catch any serialization/security exception
	{
		GIVEUP_IF(charId.isUnknownId(), "cbClientQuitGameRequest: unknown char", return);
		nldebug("BypassDisconnectionTimer denied for %s: %s", charId.toString().c_str(), e.what());
		bypassDisconnectionTimer = false;
	}

	TDataSetRow rowId = TheDataset.getDataSetRow(charId);
	const uint32 playerId = PlayerManager.getPlayerId(charId);

	if (rowId.isNull() || !rowId.isValid())
	{
		PlayerManager.disconnectPlayer(playerId);
	}
	else
	{
		// stop all phrases in execution for character
		CPhraseManager::getInstance().cancelAllPhrases(rowId);

		if (bypassDisconnectionTimer)
		{
			// send the disconnection acknowledgement at once
			PlayerManager.disconnectPlayer(playerId);
		}
		else
		{
			// add disconnection phrase in manager
			static CSheetId decoBrick("bapa02.sbrick");
			vector<CSheetId> bricks;
			bricks.push_back(decoBrick);
			if ( CPhraseManager::getInstance().executePhrase(rowId, rowId, bricks) == NULL )
			{
				// error while building the phrase, can happen when character isn't really online (connection failed)
				PlayerManager.userDisconnected( playerId ); // still waits for the timer before really disconnecting
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void cbClientReturnToMainland( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	uint32 userId;
	msgin.serial(userId);
	uint8 charIndex;
	msgin.serial(charIndex);
	TSessionId rejectedSessionId;
	msgin.serial(rejectedSessionId);

	CCharacter *character = PlayerManager.getChar( userId, (uint32)charIndex );
	if (character)
		character->returnToPreviousSession( userId, (sint32)charIndex, rejectedSessionId );
	else
		nlwarning( "Char not found for %u/%u", userId, (uint)charIndex );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:EVENT
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/// returns 0 on success, anything else is error:
/// -1: Invalid inventory
/// -2: Invalid slot
/// -3: Empty slot
sint32 clientEventSetItemCustomText(CCharacter* character, INVENTORIES::TInventory inventory, uint32 slot, ucstring const& text)
{
	if (inventory==INVENTORIES::UNDEFINED)
	{
		return -1;
	}
	CInventoryPtr invent = character->getInventory(inventory);
	if (slot >= invent->getSlotCount())
	{
		return -2;
	}
	if (invent->getItem(slot) == NULL)
	{
		return -3;
	}

	CGameItemPtr item = invent->getItem(slot);
	item->setCustomText(text);
	// Following line was commented out by trap, reason unknown
	character->incSlotVersion(inventory, slot);

	return 0;
}

void cbClientEventSetItemCustomText( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eid;
	INVENTORIES::TInventory inventory;
	uint32 uiInventory;
	uint32 slot;
	ucstring text;
	msgin.serial(eid);
	msgin.serial(uiInventory);
	inventory = (INVENTORIES::TInventory)uiInventory;
	msgin.serial(slot);
	msgin.serial(text);

	CCharacter* character = PlayerManager.getChar(eid);
	if(!character) return;

	if (!character->havePriv(":DEV:SGM:GM:EM:"))
	{
		// it should be the crafter of the item, check
		if (inventory==INVENTORIES::UNDEFINED) return;
		CInventoryPtr invent = character->getInventory(inventory);
		if (slot >= invent->getSlotCount()) return;
		if (invent->getItem(slot) == NULL) return;
		CGameItemPtr item = invent->getItem(slot);

		const NLMISC::CEntityId &crafterEId = item->getCreator();
		const NLMISC::CEntityId &userEId = character->getId();

		if(crafterEId != userEId)
		{
			string name = CEntityIdTranslator::getInstance()->getByEntity(userEId).toString();
			nlwarning("HACK: %s %s tries to set custom text on an item he didn't crafted", userEId.toString().c_str(), name.c_str());
			return;
		}

		// text must not be too big
		if(text.size() > 256)
		{
			string name = CEntityIdTranslator::getInstance()->getByEntity(userEId).toString();
			nlwarning("HACK: %s %s tries to set custom text of a size > 256 (%d)", userEId.toString().c_str(), name.c_str(), text.size());
			return;
		}

		// the item must have the good family
		const CStaticItem * form = item->getStaticForm();
		if (!form) return;
		ITEMFAMILY::EItemFamily family = form->Family;
		if (!ITEMFAMILY::isTextCustomizable(family))
		{
			string name = CEntityIdTranslator::getInstance()->getByEntity(userEId).toString();
			nlwarning("HACK: %s %s tries to set custom text on a item that is not text customizable (%s)", userEId.toString().c_str(), name.c_str(), ITEMFAMILY::toString(family).c_str());
			return;
		}

		// prevent use of @WEB at begin
		if (text.size() >= 4 && text[0]=='@' && text[1]=='W' && text[2]=='E' && text[3]=='B')
			text = text.substr(4, text.size() - 4);

		// force that the begin of the text for non admin is %mfc
		if(!text.empty() && text.substr(0, 4) != ucstring("%mfc"))
		{
			text = ucstring("%mfc") + text;
		}
	}

	clientEventSetItemCustomText(character, inventory, slot, text);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:TOTEM
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbTotemBuild( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	if( BuildSpireActive == false )
		return;
	CEntityId	Id;
	msgin.serial( Id );

	CCharacter *ch = PlayerManager.getChar( Id );

	if ( !ch )
	{
		nlwarning("Totem build aborted: no character");
		return;
	}
	if ( !ch->getPVPFlag() )
	{
		nlwarning("Totem build aborted: character has no pvp flag");
		return;
	}

	CGuild::TAllegiances allegiance = ch->getAllegiance();
	if ( allegiance.first == PVP_CLAN::Neutral )
	{
		nlwarning("Totem build aborted: character has no cult allegiance");
		return;
	}
	NLMISC::CVector pos( (float)ch->getX() / 1000.0f,  (float)ch->getY() / 1000.0f, 0 );
	CRegion* region = CZoneManager::getInstance().getRegion( pos );

	if ( !region )
	{
		nlwarning("Totem build aborted: no region where character is");
		return;
	}
	if ( !CPVPFactionRewardManager::getInstance().canBuildTotem( ch ) )
	{
		nlwarning("Totem build aborted: can't build totem");
		return;
	}
	CPVPFactionRewardManager::getInstance().startTotemBuilding( region->getId(), ch );
	nlinfo( "<cbTotemBuild> : %s is building a totem in region %s", Id.toString().c_str(), region->getName().c_str() );
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:DM_GIFT
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbDMGiftBegin( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eid;
	msgin.serial( eid );

	if(!IsRingShard)
	{
		string name = CEntityIdTranslator::getInstance()->getByEntity(eid).toString();
		nlwarning("HACK: %s %s tries to start a GM gift on a non ring shard", eid.toString().c_str(), name.c_str());
		return;
	}

	CR2MissionItem::getInstance().dmGiftBegin( eid );
}

void cbDMGiftValidate( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eid;
	msgin.serial( eid );

	if(!IsRingShard)
	{
		string name = CEntityIdTranslator::getInstance()->getByEntity(eid).toString();
		nlwarning("HACK: %s %s tries to validate a GM gift on a non ring shard", eid.toString().c_str(), name.c_str());
		return;
	}

	std::vector< R2::TItemAndQuantity > items;
	CSheetId sheet;
	uint8 quantity;

	while( msgin.getPos() < (sint32) msgin.length() )
	{
		msgin.serial( sheet );
		msgin.serial( quantity );
		if( sheet != CSheetId::Unknown && quantity != 0)
		{
			R2::TItemAndQuantity item;
			item.SheetId = sheet;
			item.Quantity = quantity;

			const CStaticItem * form = CSheets::getForm(sheet);
			if (form == NULL)
			{
				string name = CEntityIdTranslator::getInstance()->getByEntity(eid).toString();
				nlwarning("HACK: %s %s tries to create an item that has a null form", eid.toString().c_str(), name.c_str());
				return;
			}
			if(form->Family != ITEMFAMILY::SCROLL_R2)
			{
				string name = CEntityIdTranslator::getInstance()->getByEntity(eid).toString();
				nlwarning("HACK: %s %s tries to create an item that is not a plot item", eid.toString().c_str(), name.c_str());
				return;
			}
			items.push_back( item );
		}
	}
	CR2MissionItem::getInstance().dmGiftValidate( eid, items );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:RING_MISSION
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbRingMissionSelectAction( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eid;
	uint32 actionId;

	msgin.serial( eid );
	msgin.serial( actionId );

	CCharacter *ch = PlayerManager.getChar( eid );
	if( ch )
	{
		CR2GiveItem::getInstance().giveItemGranted( ch->getTargetDataSetRow(), actionId );
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//		CLIENT:NPC_ICON
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void cbGetNpcIconDesc( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eid;
	vector<uint32> npcKeys;
	
	msgin.serial( eid );
	msgin.serialCont( npcKeys );

	CCharacter *ch = PlayerManager.getChar( eid );
	if( ch )
	{
		ch->sendNpcMissionGiverIconDesc( npcKeys );
	}
}


//----------------------------
//	CbClientArray
//----------------------------
TUnifiedCallbackItem CbClientArray[]=
{
	{ "CLIENT:CONNECTION:CLIENT_QUIT_REQUEST",	cbClientQuitGameRequest },
	{ "RET_MAINLAND",							cbClientReturnToMainland },

	{ "CLIENT:ITEM:DROP",					cbClientItemDrop },
	{ "CLIENT:ITEM:PICK_UP_CLOSE",			cbClientItemPickUpClose },
	{ "CLIENT:ITEM:SWAP",					cbClientItemSwap },
	{ "CLIENT:ITEM:HARVEST",				cbClientItemHarvest },
	{ "CLIENT:ITEM:HARVEST_CLOSE",			cbClientItemHarvestClose },
	{ "CLIENT:ITEM:DESTROY",				cbClientItemDestroy },
	{ "CLIENT:ITEM:EQUIP",					cbClientItemEquip },
	{ "CLIENT:ITEM:UNEQUIP",				cbClientItemUnequip },
	{ "CLIENT:ITEM:TEMP_TO_BAG",			cbClientItemTempToBag },
	{ "CLIENT:ITEM:ALL_TEMP",				cbClientItemAllTemp },
	{ "CLIENT:ITEM:NO_TEMP",				cbClientItemNoTemp },
	{ "CLIENT:ITEM:ENCHANT",				cbClientItemEnchant },
	{ "CLIENT:ITEM:USE_ITEM",				cbClientItemUseItem },
	{ "CLIENT:ITEM:STOP_USE_XP_CAT",		cbClientItemStopUseXpCat },


	{ "CLIENT:ITEM_INFO:GET",				cbClientItemInfos },
	{ "CLIENT:MISSION_PREREQ:GET",			cbClientMissionPrereq },

	{ "CLIENT:HARVEST:DEPOSIT",				cbClientHarvestDeposit },

	{ "CLIENT:PHRASE:MEMORIZE",				cbClientPhraseMemorize },
	{ "CLIENT:PHRASE:FORGET",				cbClientPhraseForget },
	{ "CLIENT:PHRASE:LEARN",				cbClientPhraseLearn },
	{ "CLIENT:PHRASE:DELETE",				cbClientPhraseDelete },
	{ "CLIENT:PHRASE:EXECUTE",				cbClientPhraseExecute },
	{ "CLIENT:PHRASE:EXECUTE_CYCLIC",		cbClientPhraseExecuteCyclic },
	{ "CLIENT:PHRASE:EXECUTE_FABER",		cbClientPhraseExecuteFaber },
	{ "CLIENT:PHRASE:BUY",					cbClientPhraseBuyByIndex },
	{ "CLIENT:PHRASE:BUY_SHEET",			cbClientPhraseBuyBySheet },

	{ "CLIENT:PHRASE:CANCEL_TOP",			cbClientPhraseCancelTop },
	{ "CLIENT:PHRASE:CANCEL_ALL",			cbClientPhraseCancelAll },
	{ "CLIENT:PHRASE:CRISTALIZE",			cbClientPhraseCristalize },


	{ "CLIENT:COMBAT:ENGAGE",				cbClientCombatEngage },
	{ "CLIENT:COMBAT:DISENGAGE",			cbClientCombatDisengage },
	{ "CLIENT:COMBAT:DEFAULT_ATTACK",		cbClientCombatDefaultAttack },
	{ "CLIENT:COMBAT:VALIDATE_MELEE",		cbClientValidateMeleeCombat },
	{ "CLIENT:COMBAT:PARRY",				cbClientCombatParry },
	{ "CLIENT:COMBAT:DODGE",				cbClientCombatDodge },
	{ "CLIENT:COMBAT:PROTECTED_SLOT",		cbClientCombatProtectedSlot },

	{ "CLIENT:TEAM:LEAVE",					cbClientTeamLeave },
	{ "CLIENT:TEAM:JOIN",					cbClientTeamJoin },
	{ "CLIENT:TEAM:KICK",					cbClientTeamKick },
	{ "CLIENT:TEAM:JOIN_PROPOSAL",			cbClientTeamJoinProposal },
	{ "CLIENT:TEAM:JOIN_PROPOSAL_DECLINE",	cbClientTeamJoinProposalDecline },
	{ "CLIENT:TEAM:SET_SUCCESSOR",			cbClientTeamSetSuccessor },

	{ "CLIENT:TEAM:SHARE_VALID_ITEM",		cbClientTeamShareValidItem },
	{ "CLIENT:TEAM:SHARE_INVALID_ITEM",		cbClientTeamShareInvalidItem },
	{ "CLIENT:TEAM:SHARE_VALID",			cbClientTeamShareValid },

	{ "CLIENT:TEAM:CONTACT_ADD",			cbClientAddToContactList },
	{ "CLIENT:TEAM:CONTACT_DEL",			cbClientRemoveFromContactList },
	{ "CLIENT:TEAM:CONTACT_MOVE",			cbClientMoveInContactLists },


//	{ "CLIENT:TP:BOT",						cbClientTpBot },
//	{ "CLIENT:TP:WANTED",					cbClientTpWanted },
	{ "CLIENT:TP:ACK",						cbClientTpAck },
//	{ "CLIENT:TP:RESPAWN",					cbClientTpRespawn },

//	{ "CLIENT:ANIMALS:DISBAND_CONVOY",		cbClientAnimalsDisbandConvoy },
	{ "CLIENT:ANIMALS:BEAST",				cbClientAnimalsBeast },
//	{ "CLIENT:ANIMALS:MOUNT",				cbAnimalMount },
//	{ "CLIENT:ANIMALS:UNSEAT",				cbAnimalUnseat },

/*
	{ "CLIENT:TRADE:NEXT_PAGE_ITEM",		cbClientTradeNextPage },
	{ "CLIENT:TRADE:NEXT_PAGE_MISSION_ITEM",cbClientTradeMissionNextPage },
	{ "CLIENT:TRADE:BUY",					cbClientTradeBuy },
	{ "CLIENT:TRADE:QUERY_PRICE",			cbClientTradeQuerySellPrice },
	{ "CLIENT:TRADE:SELL",					cbClientTradeSellItem },
*/

	{ "CLIENT:EXCHANGE:PROPOSAL",			cbClientExchangeProposal },
	{ "CLIENT:EXCHANGE:ACCEPT_INVITATION",	cbClientExchangeAcceptInvitation },
	{ "CLIENT:EXCHANGE:DECLINE_INVITATION",	cbClientExchangeDeclineInvitation },
	{ "CLIENT:EXCHANGE:SEEDS",				cbClientExchangeSeeds },
	{ "CLIENT:EXCHANGE:END",				cbClientExchangeEnd },
	{ "CLIENT:EXCHANGE:VALIDATE",			cbClientExchangeValidate },
	{ "CLIENT:EXCHANGE:INVALIDATE",			cbClientExchangeInvalidate },
	{ "CLIENT:EXCHANGE:ADD",				cbClientExchangeAdd },
	{ "CLIENT:EXCHANGE:REMOVE",				cbClientExchangeRemove },


//	{ "CLIENT:SENTENCE:CANCEL_CURRENT",		cbClientSentenceCancelCurrent },
//	{ "CLIENT:SENTENCE:CANCEL",				cbClientSentenceCancel },
//	{ "CLIENT:SENTENCE:CANCEL_ALL",			cbClientSentenceCancelAll },

	{ "CLIENT:COMMAND:EMOTE",				cbClientSendEmote },
	{ "CLIENT:COMMAND:CUSTOM_EMOTE",		cbClientSendCustomEmote },
	{ "CLIENT:COMMAND:WHERE",				cbClientSendWhere },
	{ "CLIENT:COMMAND:ADMIN",				cbClientAdmin },
	{ "CLIENT:COMMAND:ADMIN_OFFLINE",		cbClientAdminOffline },
	{ "CLIENT:COMMAND:REMOTE_ADMIN_ANSWER",	cbClientRemoteAdmin },
	{ "CLIENT:COMMAND:SIT",					cbClientSit },
	{ "CLIENT:COMMAND:AFK",					cbClientSendAfk },
	{ "CLIENT:COMMAND:RANDOM",				cbClientRollDice },
	{ "CLIENT:COMMAND:GUILDMOTD",			cbClientGuildMotd },


// For all these commented commands, now you have to use the CLIENT:COMMAND:ADMIN message using /a client command

//	{ "CLIENT:CHEAT:GOD",					cbClientCheatGod },
//	{ "CLIENT:CHEAT:CREATE_ITEM_IN_BAG",	cbClientCreateItemInBag	},
//	{ "CLIENT:CHEAT:LEARN_ALL_BRICKS",		cbClientLearnAllBricks },
//	{ "CLIENT:CHEAT:MONEY",					cbClientMoney },
//	{ "CLIENT:CHEAT:SET_TIME",				cbClientRyzomTime },
//	{ "CLIENT:CHEAT:SET_DAY",				cbClientRyzomDay },
//	{ "CLIENT:CHEAT:XP",					cbClientGainXp },
//	{ "CLIENT:CHEAT:CREATE_CHARACTER",		cbClientCreateCharacter },
//	{ "CLIENT:CHEAT:ADD_ROLE",				cbClientAddRole },
//	{ "CLIENT:CHEAT:LEARN_BRICK",			cbClientLearnBrick },
//	{ "CLIENT:CHEAT:LEARN_ALL_FABER_PLANS",	cbClientLearnAllFaberPlans },

	{ "CLIENT:DEBUG:WHERE",					cbClientWhere },
	{ "CLIENT:DEBUG:WHO",					cbClientWho },
	{ "CLIENT:DEBUG:PING",					cbClientPing },

	{ "CLIENT:BOTCHAT:START_CHOOSE_MISSION",	cbClientBotChatChooseStaticMission },
	{ "CLIENT:BOTCHAT:NEXT_PAGE_MISSION",		cbClientBotChatNextMissionPage },
	{ "CLIENT:BOTCHAT:PICK_MISSION",			cbClientBotChatPickStaticMission },
	{ "CLIENT:BOTCHAT:CONTINUE_MISSION",		cbClientContinueMission },
	{ "CLIENT:BOTCHAT:VALIDATE_PLAYER_GIFT",	cbClientValidateMissionGift },


	{ "CLIENT:BOTCHAT:START_TRADE_ITEM",		cbClientBotChatStartTradeItem },
	{ "CLIENT:BOTCHAT:START_TRADE_TELEPORT",	cbClientBotChatStartTradeTp },
	{ "CLIENT:BOTCHAT:START_TRADE_FACTION",		cbClientBotChatStartTradeFaction },

	{ "CLIENT:BOTCHAT:START_TRADE_GUILD_OPTIONS",	cbClientBotChatStartTradeGuildOptions },
	{ "CLIENT:BOTCHAT:BUY_GUILD_OPTION",			cbClientBotChatBuyGuildOptions },
	{ "CLIENT:BOTCHAT:START_GUILD_RESEARCH",		cbClientBotChatStartGuildResearch },


/*	{ "CLIENT:BOTCHAT:START_TRADE_SKILL",		cbClientBotChatStartTradeSkill },
	{ "CLIENT:BOTCHAT:START_TRADE_PACT",		cbClientBotChatStartTradePact },*/
	{ "CLIENT:BOTCHAT:START_TRADE_ACTION",		cbClientBotChatStartTradeAction },

	{ "CLIENT:BOTCHAT:SET_FILTERS",				cbClientBotChatSetFilters },


	{ "CLIENT:BOTCHAT:NEXT_PAGE_ITEM",			cbClientBotChatTradeNextPage },
	{ "CLIENT:BOTCHAT:BUY",						cbClientBotChatTradeBuy },
	{ "CLIENT:BOTCHAT:DESTROY_ITEM",			cbClientBotChatTradeDestroy },
	{ "CLIENT:BOTCHAT:SELL",					cbClientBotChatTradeSell },
	{ "CLIENT:BOTCHAT:REFRESH_TRADE_LIST",		cbRefreshTradeList },

	{ "CLIENT:BOTCHAT:START_CREATE_GUILD",		cbClientBotChatStartGuildCreation },

	{ "CLIENT:BOTCHAT:END",						cbClientBotChatEnd },
	{ "CLIENT:BOTCHAT:END_GIFT",				cbClientBotChatEndGift },

	{ "CLIENT:BOTCHAT:DYNCHAT_SEND",			cbClientBotChatDynChatSend },


	// mission journal related commands
	{ "CLIENT:JOURNAL:MISSION_ABANDON",			cbClientAbandonMission },
	{ "CLIENT:JOURNAL:GROUP_MISSION_ABANDON",	cbClientGroupAbandonMission },

	// guild related messages
	{ "CLIENT:GUILD:CREATE",					cbClientGuildCreate },
	{ "CLIENT:GUILD:INVITATION",				cbClientGuildJoinInvitation },
	{ "CLIENT:GUILD:ACCEPT_INVITATION",			cbClientGuildAcceptJoinInvitation },
	{ "CLIENT:GUILD:REFUSE_INVITATION",			cbClientGuildRefuseJoinInvitation },

	{ "CLIENT:GUILD:SET_GRADE",					cbClientGuildSetGrade },
	{ "CLIENT:GUILD:KICK_MEMBER",				cbClientGuildKickMember },


	{ "CLIENT:GUILD:QUIT",						cbClientGuildQuit },
	{ "CLIENT:GUILD:TELEPORT",					cbClientGuildLiftTeleport },
	{ "CLIENT:GUILD:FIRST_ASCENSOR_PAGE",		cbClientGuildLiftDestAsked },
	{ "CLIENT:GUILD:NEXT_ASCENSOR_PAGE",		cbClientGuildLiftNextPage },
	{ "CLIENT:GUILD:SET_PLAYER_TITLE",			cbClientSetCharacterTitle },
	//{ "CLIENT:GUILD:BAG_TO_GUILD",			cbClientBagToGuild },
	//{ "CLIENT:GUILD:GUILD_TO_BAG",			cbClientGuildToBag },
	{ "CLIENT:GUILD:PUT_MONEY",					cbClientGuildPutMoney },
	{ "CLIENT:GUILD:TAKE_MONEY",				cbClientGuildTakeMoney },

	// outpost related messages
	{ "CLIENT:OUTPOST:SET_SQUAD",				cbClientOutpostSetSquad },
	{ "CLIENT:OUTPOST:SET_SQUAD_SPAWN",			cbClientOutpostSetSquadSpawnZone },
	{ "CLIENT:OUTPOST:INSERT_SQUAD",			cbClientOutpostInsertSquad },
	{ "CLIENT:OUTPOST:REMOVE_SQUAD",			cbClientOutpostRemoveSquad },
	{ "CLIENT:OUTPOST:SET_SQUAD_CAPITAL",		cbClientOutpostSetExpenseLimit },
//	{ "CLIENT:OUTPOST:BUY_BUILDING",			cbClientOutpostBuyBuilding },
//	{ "CLIENT:OUTPOST:DESTROY_BUILDING",		cbClientOutpostDestroyBuilding },
	{ "CLIENT:OUTPOST:SIDE_CHOSEN",				cbClientOutpostSideChosen },
	{ "CLIENT:OUTPOST:SELECT",					cbClientOutpostSelect },
	{ "CLIENT:OUTPOST:UNSELECT",				cbClientOutpostUnselect },
	{ "CLIENT:OUTPOST:DECLARE_WAR_START",		cbClientOutpostWarStart },
	{ "CLIENT:OUTPOST:DECLARE_WAR_VALIDATE",	cbClientOutpostWarValidate },
	{ "CLIENT:OUTPOST:GIVEUP_OUTPOST",			cbClientOutpostGiveup },
	{ "CLIENT:OUTPOST:BANISH_PLAYER",			cbClientOutpostBanishPlayer },
	{ "CLIENT:OUTPOST:BANISH_GUILD",			cbClientOutpostBanishGuild },
	{ "CLIENT:OUTPOST:SET_DEF_PERIOD",			cbClientOutpostSetDefensePeriod },

	// death management messages
	{ "CLIENT:DEATH:ASK_RESPAWN",				cbClientAskRespawn },

	// set compass target
	{ "CLIENT:TARGET:COMPASS_DYNAMIC",			cbClientCompassEnableDynamic },
	{ "CLIENT:TARGET:COMPASS_NOT_DYNAMIC",		cbClientCompassDisableDynamic },

	{ "CLIENT:DUEL:ASK",						cbClientDuelAsked },
	{ "CLIENT:DUEL:ACCEPT",						cbClientDuelAccept },
	{ "CLIENT:DUEL:REFUSE",						cbClientDuelRefuse },
	{ "CLIENT:DUEL:ABANDON",					cbClientDuelAbandon },

	{ "CLIENT:PVP_CHALLENGE:ASK",				cbClientLeagueJoinProposal },
	{ "CLIENT:PVP_CHALLENGE:ACCEPT",			cbClientLeagueJoin },
	{ "CLIENT:PVP_CHALLENGE:REFUSE",			cbClientLeagueJoinProposalDecline },
	{ "CLIENT:PVP_CHALLENGE:ABANDON",			cbClientPVPChallengeAbandon },

//	{ "CLIENT:PVP_VERSUS:CLAN",					cbClientPvpChooseClan },
	{ "CLIENT:PVP:PVP_TAG",						cbClientPvPSetTag },
	{ "CLIENT:PVP:SET_NEUTRAL_ALLEGIANCE",		cbClientPvPSetNeutralAllegiance },
	{ "CLIENT:PVP:SET_NEUTRAL_ALLEGIANCE_GUILD",cbClientPvPSetNeutralAllegianceGuild },

	{ "CLIENT:MISSION:ENTER_CRITICAL",			cbClientMissionEnterCritical },
	{ "CLIENT:MISSION:WAKE",					cbClientMissionWake },
	{ "CLIENT:MISSION:GROUP_WAKE",				cbClientMissionGroupWake },

	{ "CLIENT:EVENT:SET_ITEM_CUSTOM_TEXT",		cbClientEventSetItemCustomText },

	{ "CLIENT:TOTEM:BUILD",						cbTotemBuild },

	{ "CLIENT:DM_GIFT:BEGIN",					cbDMGiftBegin },
	{ "CLIENT:DM_GIFT:VALIDATE",				cbDMGiftValidate },

	{ "CLIENT:RING_MISSION:MISSION_RING_SELECT",cbRingMissionSelectAction },

	{ "CLIENT:NPC_ICON:GET_DESC",				cbGetNpcIconDesc },
}; 



void CClientMessages::init()
{
	// setup the callback array
	CUnifiedNetwork::getInstance()->addCallbackArray( CbClientArray, sizeof(CbClientArray)/sizeof(CbClientArray[0]) );
}

void CClientMessages::release()
{
}
