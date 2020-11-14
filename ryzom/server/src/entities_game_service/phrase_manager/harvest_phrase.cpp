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
#include "harvest_phrase.h"
#include "s_phrase_factory.h"
#include "entity_manager/entity_manager.h"
#include "phrase_manager/phrase_manager.h"
#include "creature_manager/creature_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "zone_manager.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_utilities_functions.h"
// game share
#include "game_share/brick_families.h"
#include "game_share/string_manager_sender.h"

#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_brick.h"

extern CPlayerManager	PlayerManager;
extern CCreatureManager CreatureManager;

DEFAULT_SPHRASE_FACTORY( CHarvestPhrase, BRICK_TYPE::HARVEST );



using namespace std;
using namespace NLMISC;
using namespace NLNET;

//-----------------------------------------------
// ctor
//-----------------------------------------------
CHarvestPhrase::CHarvestPhrase()
{
	_SabrinaCost = 0;
	_SabrinaRelativeCost = 1.0f;
	_SabrinaCredit = 0;
	_SabrinaRelativeCredit = 1.0f;
	_StaminaCost = 0;
	_HPCost = 0;
	//_HarvestTime = 25; // 2.5s for DEBUG ONLY
	_HarvestTime = 0; 
	_IsStatic = true;
	_PhraseType = BRICK_TYPE::HARVEST;
}

//-----------------------------------------------
// dtor
//-----------------------------------------------
CHarvestPhrase::~CHarvestPhrase()
{
	// ANTI BUG
/*	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		if (player->getBehaviour().Behaviour == MBEHAV::LOOT_INIT)
		{
			nlwarning("HARVEST : BUG, harvest phrase deleted while actor still has LOOT_INIT behaviour, call stop method");
			stop();
		}
	}
	*/
	// NO : now behaviour is reset when closing interface
}

//-----------------------------------------------
// CHarvestPhrase build
//-----------------------------------------------
bool CHarvestPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute )
{
	H_AUTO(CHarvestPhrase_build);
	
	// we are sure there is at least one brick and that there are non NULL;
	nlassert( !bricks.empty() );

	_ActorRowId = actorRowId;

	// compute cost and credit and parse other params
	for ( uint i = 0; i < bricks.size(); ++i )
	{
		const CStaticBrick & brick = *bricks[i];

		if ( i == 0)
			_RootSheetId = brick.SheetId;

		if ( brick.SabrinaValue < 0 )
			_SabrinaCredit -= brick.SabrinaValue;
		else
			_SabrinaCost += brick.SabrinaValue;
		
		if( brick.SabrinaRelativeValue < 0.0f )
			_SabrinaRelativeCredit -= brick.SabrinaRelativeValue;
		else
			_SabrinaRelativeCost += brick.SabrinaRelativeValue;

//		switch( brick.Family )
//		{
			/* TempYoyo: case BRICK_FAMILIES::RootHarvest:
				break;*/
//			default:
			for ( uint i=0 ; i<brick.Params.size() ; ++i)
			{
				const TBrickParam::IId* param = brick.Params[i];

				switch(brick.Params[i]->id())
				{	
					case TBrickParam::SAP:
						return false;
					case TBrickParam::HP:
						_HPCost += ((CSBrickParamHp *)param)->Hp;
						break;
					case TBrickParam::STA:
						_StaminaCost += ((CSBrickParamSta *)param)->Sta;
						break;
					default:
						// unused param ?
						break;
				}
			}
//			break;
//		}		
	}
	
	return true;
}// CHarvestPhrase build


//-----------------------------------------------
// CHarvestPhrase evaluate
//-----------------------------------------------
bool CHarvestPhrase::evaluate()
{
	// update state
	return true;
}// CHarvestPhrase evaluate


//-----------------------------------------------
// CHarvestPhrase validate
//-----------------------------------------------
bool CHarvestPhrase::validate()
{
	H_AUTO(CHarvestPhrase_validate);
	
	_BeingProcessed = true;

	// entities cant harvest if in combat
	TDataSetRow entityRowId = CPhraseManager::getInstance().getEntityEngagedMeleeBy( _ActorRowId );
	if (TheDataset.isAccessible(entityRowId))
	{
		///\todo david : send message
		_BeingProcessed = false;
		return false;
	}
	entityRowId = CPhraseManager::getInstance().getEntityEngagedRangeBy( _ActorRowId );
	if (TheDataset.isAccessible(entityRowId))
	{
		///\todo david : send message
		_BeingProcessed = false;
		return false;
	}
	
	CCharacter * player = PlayerManager.getChar(_ActorRowId);
	if (!player)
	{
		_BeingProcessed = false;
		return false;
	}

	const sint32 hp = player->currentHp();
	if ( hp < _HPCost  )
	{
		///\todo david : send message
		_BeingProcessed = false;
		return false;
	}
	const sint32 sta = player->getScores()._PhysicalScores[ SCORES::stamina ].Current;
	if ( sta < _StaminaCost  )
	{
		///\todo david : send message
		_BeingProcessed = false;
		return false;
	}
	if (hp <= 0	|| player->isDead())
	{
		///\todo david : send message
		_BeingProcessed = false;
		return false;
	}	

	/// todo david : test if on mount

	// on first validate, get the harvested item and check it's validity
	if ( state() == Evaluated)
	{
	// TEMPORARY CHANGE THIS WHEN THE BRICK WILL HAVE THE RIGHT PARAMS-------
		if (_RootSheetId == CSheetId("bhf01.sbrick"))
		{
			// this is the forage action
			// end harvest but doesn't close the interface
			/*player->endHarvest(false);

			// begin harvest
			player->staticActionInProgress( true );
			player->harvestDeposit(true);	
			player->depositSearchSkill(SKILLS::SH);
			player->openHarvest();
			player->tempInventoryMode(TEMP_INV_MODE::HarvestDeposit);
			CZoneManager::getInstance().harvestDeposit(player);

			if (player->getHarvestInfos().Sheet != CSheetId::Unknown)
			{
				//player->harvestAsked(0);
				_Deposit = true;
				_RawMaterialId = player->getHarvestInfos().Sheet;
				_MinQuality = player->getHarvestInfos().MinQuality;
				_MaxQuality = player->getHarvestInfos().MaxQuality;
				_Quantity = player->getHarvestInfos().Quantity;
				player->harvestedMpQuantity((uint8)_Quantity);	
			}
			else
			{
				player->sendMessageToClient( player->getId(), "WOS_HARVEST_FOUND_NOTHING");
				player->sendCloseTempInventoryImpulsion();
				player->getHarvestInfos().Sheet = CSheetId::Unknown;
				player->endHarvest();
				_BeingProcessed = false;
				return false;
			}*/
		}
	// TEMPORARY ---------------------------------------------
		const CStaticItem *item = CSheets::getForm(_RawMaterialId);
		if (item != 0)
		{
			CCharacter::sendDynamicSystemMessage(player->getId(), "WOS_HARVEST_SEARCHING");
/*				string msgName = "WOS_HARVEST_SEARCHING";
			CMessage msg("STATIC_STRING");
			msg.serial( const_cast<CEntityId&> (player->getId()) );
			set<CEntityId> excluded;
			msg.serialCont( excluded );
			msg.serial( msgName );
			sendMessageViaMirror ("IOS", msg);
*/			}
		else
		{
			nlwarning("HARVEST : Cannot find form for raw material %s, cancel harvest", _RawMaterialId.toString().c_str());
			CCharacter::sendDynamicSystemMessage(player->getId(), "WOS_HARVEST_FOUND_NOTHING");
//				player->sendMessageToClient( player->getId(), "WOS_HARVEST_FOUND_NOTHING");
			player->sendCloseTempInventoryImpulsion();
			player->getHarvestInfos().Sheet = CSheetId::Unknown;
			player->endHarvest();
			_BeingProcessed = false;
			return false;
		}
	}

	return true;
}// CHarvestPhrase validate


//-----------------------------------------------
// CHarvestPhrase update
//-----------------------------------------------
bool  CHarvestPhrase::update()
{
	return true;
}// CHarvestPhrase update


//-----------------------------------------------
// CHarvestPhrase execute
//-----------------------------------------------
void  CHarvestPhrase::execute()
{
	H_AUTO(CHarvestPhrase_execute);
	
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();

	_ExecutionEndDate  = time + _HarvestTime ;

// TODO
#if 0
	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		player->setCurrentAction(CLIENT_ACTION_TYPE::Harvest,_ExecutionEndDate);

		player->_PropertyDatabase.setProp( "EXECUTE_PHRASE:SHEET", _RootSheetId.asInt() );
		player->_PropertyDatabase.setProp( "EXECUTE_PHRASE:PHRASE", 0);
	}
#endif
	// set behaviour
	PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, MBEHAV::LOOT_INIT );
}// CHarvestPhrase execute


//-----------------------------------------------
// CHarvestPhrase launch
//-----------------------------------------------
bool CHarvestPhrase::launch()
{
	// apply immediatly
	_ApplyDate = 0;
	return true;
}// CHarvestPhrase launch


//-----------------------------------------------
// CHarvestPhrase apply
//-----------------------------------------------
void CHarvestPhrase::apply()
{
	H_AUTO(CHarvestPhrase_apply);
	
	// spend energies
	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	if (entity == NULL)
	{
		nlwarning("<CHarvestPhrase::apply> Invalid entity Id %s", TheDataset.getEntityId(_ActorRowId).toString().c_str() );		
		return;
	}
	SCharacteristicsAndScores &sta = entity->getScores()._PhysicalScores[SCORES::stamina];
	if ( sta.Current != 0)
	{
		sta.Current = sta.Current - _StaminaCost;
		if (sta.Current < 0)
			sta.Current = 0;
	}

	SCharacteristicsAndScores &hp = entity->getScores()._PhysicalScores[SCORES::hit_points];
	if ( hp.Current != 0)
	{
		hp.Current = hp.Current - _HPCost;
		if (hp.Current < 0)
			hp.Current = 0;
	}
}//CHarvestPhrase apply


//-----------------------------------------------
// CHarvestPhrase end
//-----------------------------------------------
void CHarvestPhrase::end()
{
	H_AUTO(CHarvestPhrase_end);
	
	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		player->clearCurrentAction();
	}

	vector<uint16> qualities;

	// set behaviour no! -> set it when closing interface
//	PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, MBEHAV::LOOT_END );

	/*if (_Deposit) // obsolete (now done by the foraging actions)
	{
		//player->harvestResultDeposit( _MinQuality, false);
		//player->harvestDepositResult( max(_MinQuality,uint16(1)) );
		player->harvestDepositResult( max( (uint16)( rand() * ( _MaxQuality * 10  - _MinQuality * 10) / RAND_MAX + _MinQuality * 10 ), uint16(1)) );
	}
	else*/
		//harvestCorpseResult();
		player->harvestCorpseResult( qualities );

} // end //

//-----------------------------------------------
// CHarvestPhrase stop
//-----------------------------------------------
void CHarvestPhrase::stop()
{
	H_AUTO(CHarvestPhrase_stop);
	
	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		player->clearCurrentAction();
		/*if (_Deposit)
			PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "EGS_FORAGE_INTERRUPTED");
		else*/
			PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "EGS_QUARTER_INTERRUPTED");
	}

	// set behaviour
	PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, MBEHAV::LOOT_END );
} // stop //


//-----------------------------------------------
// CHarvestPhrase harvestCorpseResult
//-----------------------------------------------
void CHarvestPhrase::harvestCorpseResult()
{
	H_AUTO(CHarvestPhrase_harvestCorpseResult);
	
	// get harvester character
	CCharacter *character = PlayerManager.getChar( _ActorRowId );
	if (character == NULL)
	{
		//nlwarning("<cbHarvestResult> Invalid player Id %s", playerId.toString().c_str() );
		return;
	}

	// get harvested corpse
	const CEntityId &harvestedEntity = character->harvestedEntity();

	CCreature *creature = CreatureManager.getCreature( harvestedEntity );
	if (creature == NULL)
	{
		nlwarning("<cbHarvestResult> Invalid creature Id %s", harvestedEntity.toString().c_str() );
		// reset harvest info
		character->resetHarvestInfos();
		character->endHarvest();
		return;
	}

	const vector< CCreatureRawMaterial> &mps = creature->getMps();
	if ( character->harvestedMpIndex() >= mps.size() || character->harvestedMpQuantity() > mps[character->harvestedMpIndex()].Quantity )
	{
		// reset harvest info
		character->resetHarvestInfos();
		return;
	}

	uint16 quality = _MaxQuality;

	// create the mp items if any
	if (quality > 0)
	{
		if ( !character->createItemInInventory(INVENTORIES::bag, quality, character->harvestedMpQuantity(), _RawMaterialId, character->getId()) )
		{
	//		CMissionEventItem event(CMissionEvent::Harvest,playerId,harvestedEntity,_RawMaterialId,quality,character->harvestedMpQuantity());
	//		character->processMissionEvent(event);
			// error creating the object, hand probably not empty
		//	character->resetHarvestInfos();
		//	return;
		}
		else
		{
			const CStaticItem *item = CSheets::getForm(_RawMaterialId);
			if (item)
			{
				///\todo nico: check if this event exists
//				CMissionEventHarvest event(_RawMaterialId ,character->harvestedMpQuantity(),quality);
//				character->processMissionEvent( event );

				SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
				params[0].Int = (sint32)character->harvestedMpQuantity();
				params[1].SheetId = _RawMaterialId;
				params[2].Int = (sint32)quality;

				STRING_MANAGER::sendStringToClient( character->getEntityRowId(), "HARVEST_SUCCESS", params );
			}
		}
	}
	// the mp have been destroyed -> do nothing
	else
	{
	}

	// remove the quantity of mp harvested from the ressource
	creature->removeMp( character->harvestedMpIndex(), character->harvestedMpQuantity() );
		
	// reset harvest info
	character->resetHarvestInfos();
} // harvestCorpseResult //

