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
#include "game_share/brick_families.h"
#include "s_phrase_factory.h"
#include "entity_manager.h"
#include "phrase_manager.h"
#include "creature_manager.h"
#include "player_manager.h"
#include "character.h"
#include "phrase_utilities_functions.h"

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
	_SabrinaCredit = 0;
	_StaminaCost = 0;
	_HPCost = 0;
	_HarvestTime = 25; // 2.5s for DEBUG ONLY
//	_HarvestTime = 0; // 0s for DEBUG ONLY
	_IsStatic = true;
}


//-----------------------------------------------
// CHarvestPhrase build
//-----------------------------------------------
bool CHarvestPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks )
{
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
		
		switch( brick.Family )
		{
			/* TempYoyo: case BRICK_FAMILIES::RootHarvest:
				break;*/
			default:
			for ( uint i=0 ; i<brick.Params.size() ; ++i)
			{
				switch(brick.Params[i]->id())
				{	
					case TBrickParam::SAP:
						return false;
					case TBrickParam::HP:
						INFOLOG("HP: %i",((CSBrickParamHp *)brick.Params[i])->Hp);
						_HPCost += ((CSBrickParamHp *)brick.Params[i])->Hp;
						break;
					case TBrickParam::STA:
						INFOLOG("STA: %i",((CSBrickParamSta *)brick.Params[i])->Sta);
						_StaminaCost += ((CSBrickParamSta *)brick.Params[i])->Sta;
						break;
					default:
						// unused param ?
						break;
				}
			}
			break;
		}
	}
	
	return true;
}// CHarvestPhrase build


//-----------------------------------------------
// CHarvestPhrase evaluate
//-----------------------------------------------
bool CHarvestPhrase::evaluate(CEvalReturnInfos *msg)
{
	// update state
	_State = CSPhrase::Evaluated;
	return true;
}// CHarvestPhrase evaluate


//-----------------------------------------------
// CHarvestPhrase validate
//-----------------------------------------------
bool CHarvestPhrase::validate()
{
	// entities cant harvest if in combat
	TDataSetRow entityRowId = CPhraseManager::getInstance()->getEntityEngagedMeleeBy( _ActorRowId );
	if (entityRowId.isValid())
	{
		///\todo david : send message
		return false;
	}
	CEntityBase * entity = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	const sint32 hp = entity->getScores()._PhysicalScores[ SCORES::hit_points ].Current;
	if ( hp < _HPCost  )
	{
		///\todo david : send message
		return false;
	}
	const sint32 sta = entity->getScores()._PhysicalScores[ SCORES::stamina ].Current;
	if ( sta < _StaminaCost  )
	{
		///\todo david : send message
		return false;
	}
	if (hp <= 0	||	entity->getMode()==MBEHAV::DEATH)
	{
		///\todo david : send message
		return false;
	}

	/// todo david : test if on mount

	// update state
	if (_State == Evaluated)
		_State = Validated;
	else if (_State == ExecutionInProgress)
		_State = SecondValidated;
	return true;
}// CHarvestPhrase validate


//-----------------------------------------------
// CHarvestPhrase update
//-----------------------------------------------
bool  CHarvestPhrase::update()
{
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	
	// if the sentence execution delay time has ended, apply sentence effects
	if ( _State == SecondValidated && _ExecutionEndDate <= time && _NbWaitingRequests == 0)
	{
		apply();
	}
	else if ( _State == Latent /*&& _LatencyEndDate <= time*/ )
	{
		end();
	}
	
	return true;
}// CHarvestPhrase update


//-----------------------------------------------
// CHarvestPhrase execute
//-----------------------------------------------
void  CHarvestPhrase::execute()
{
	if( _NbWaitingRequests != 0)
		return;
	
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	_State = CSPhrase::ExecutionInProgress;
	_ExecutionEndDate  = time + _HarvestTime ;

	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		player->setCurrentAction(CLIENT_ACTION_TYPE::Harvest,_ExecutionEndDate);

		const CStaticItem *item = CSheets::getForm(_RawMaterialId);
		if (item != 0)
		{
		/*	string msgName = "BS_HARVEST_BEGIN_S";
			CMessage msg("STATIC_STRING");
			msg.serial( const_cast<CEntityId&> (player->getId()) );
			set<CEntityId> excluded;
			msg.serialCont( excluded );
			msg.serial( msgName );
			msg.serial( const_cast<string&> (item->Name));

			sendMessageViaMirror ("IOS", msg);
		*/
			
			string msgName = "WOS_HARVEST_SEARCHING";
			CMessage msg("STATIC_STRING");
			msg.serial( const_cast<CEntityId&> (player->getId()) );
			set<CEntityId> excluded;
			msg.serialCont( excluded );
			msg.serial( msgName );
			sendMessageViaMirror ("IOS", msg);
		}

		player->_PropertyDatabase.setProp( "EXECUTE_PHRASE:SHEET", _RootSheetId.asInt() );
	}

	// set behaviour
	PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, MBEHAV::HARVESTING );
}// CHarvestPhrase execute


//-----------------------------------------------
// CHarvestPhrase apply
//-----------------------------------------------
void CHarvestPhrase::apply()
{
	_State = CSPhrase::Latent;

	///\todo behaviour

	// spend energies
	CEntityBase* entity = PHRASE_UTILITIES::entityPtrFromId( _ActorRowId );
	if (entity == NULL)
	{
		nlwarning("<CHarvestPhrase::apply> Invalid entity Id %s", TheDataset.getEntityId(_ActorRowId).toString().c_str() );		
		return;
	}
	RY_GAME_SHARE::SCharacteristicsAndScores &sta = entity->getScores()._PhysicalScores[SCORES::stamina];
	if ( sta.Current != 0)
	{
		sta.Current = sta.Current - _StaminaCost;
		if (sta.Current < 0)
			sta.Current = 0;
	}

	RY_GAME_SHARE::SCharacteristicsAndScores &hp = entity->getScores()._PhysicalScores[SCORES::hit_points];
	if ( hp.Current != 0)
	{
		hp.Current = hp.Current - _HPCost;
		if (hp.Current < 0)
			hp.Current = 0;
	}

// Harvest the raw materials
	

}//CHarvestPhrase apply


//-----------------------------------------------
// CHarvestPhrase end
//-----------------------------------------------
void CHarvestPhrase::end()
{
	_State = CSPhrase::LatencyEnded;

	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		player->clearCurrentAction();
	}

	vector<uint16> qualities;

	// set behaviour
	PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, MBEHAV::HARVESTING_END );

	if (_Deposit)
		//player->harvestResultDeposit( _MinQuality, false);
		player->harvestDepositResult( _MinQuality );
	else
		//harvestCorpseResult();
		player->harvestCorpseResult( qualities );

} // end //

//-----------------------------------------------
// CHarvestPhrase stop
//-----------------------------------------------
void CHarvestPhrase::stop()
{
	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		player->clearCurrentAction();
//		if (_Deposit)
//			PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "EGS_FORAGE_INTERRUPTED");
//		else
//			PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "EGS_QUARTER_INTERRUPTED");
	}

	_State = CSPhrase::LatencyEnded;

	// set behaviour
	PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, MBEHAV::HARVESTING_END );
} // stop //


//-----------------------------------------------
// CHarvestPhrase harvestCorpseResult
//-----------------------------------------------
void CHarvestPhrase::harvestCorpseResult()
{
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
		if ( character->createItemInBag( (uint8) quality, character->harvestedMpQuantity(), _RawMaterialId) == false)
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
				CMissionEventHarvest event(_RawMaterialId ,character->harvestedMpQuantity(),quality);
				character->processMissionEvent( event );
				string msgName = "OPS_HARVEST_SUCESS_ISI";

				CMessage msg("STATIC_STRING");
				msg.serial( const_cast<CEntityId&>(character->getId()) );
				set<CEntityId> excluded;
				msg.serialCont( excluded );
				msg.serial( msgName );

				uint32 value = uint32(character->harvestedMpQuantity());
				msg.serial( value );//qty
				msg.serial( const_cast<string&> (item->Name) );//mp name

				value = uint32(quality);
				msg.serial( value );//quality

				sendMessageViaMirror ("IOS", msg);
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

