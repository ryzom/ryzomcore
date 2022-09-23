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



//////////////
//	INCLUDE	//
//////////////
#include "stdpch.h"
#include "magic_phrase.h"
#include "game_share/brick_families.h"
#include "s_phrase_factory.h"
#include "entity_manager.h"
#include "phrase_manager.h"
#include "player_manager.h"
#include "game_share/entity_structure/statistic.h"
#include "s_effect.h"
#include "character.h"
#include "phrase_utilities_functions.h"

//////////////
//	USING	//
//////////////
using namespace RY_GAME_SHARE;
using namespace std;
using namespace NLMISC;

//////////////
//	EXTERN	//
//////////////
extern NLMISC::CRandom RandomGenerator;
extern CPlayerManager PlayerManager;

DEFAULT_SPHRASE_FACTORY( CMagicPhrase, BRICK_TYPE::MAGIC );

////////////
// STATIC //
////////////
float CMagicPhrase::_DefaultCastingTime = 1.0f;

//-----------------------------------------------
// CMagicPhrase dtor
//-----------------------------------------------
CMagicPhrase::~CMagicPhrase()
{
	for (uint  i = 0; i < _Actions.size(); i++ )
	{
		delete _Actions[i];
	}
}


//-----------------------------------------------
// CMagicPhrase applyBrickParam
//-----------------------------------------------
void CMagicPhrase::applyBrickParam( TBrickParam::IId * param )
{
	nlassert(param);
	switch(param->id())
	{
	case TBrickParam::MA_CASTING_TIME:
		INFOLOG("MA_CASTING_TIME: %f",((CSBrickParamCastingTime *)param)->CastingTime);
		_CastingTime += NLMISC::TGameCycle( ((CSBrickParamCastingTime *)param)->CastingTime / CTickEventHandler::getGameTimeStep() );
		break;
	case TBrickParam::HP:
		INFOLOG("HP: %i",((CSBrickParamHp *)param)->Hp);
		_HPCost += ((CSBrickParamHp *)param)->Hp;
		break;				
	case TBrickParam::SAP:
		INFOLOG("SAP: %i",((CSBrickParamSap *)param)->Sap);
		_SapCost += ((CSBrickParamSap *)param)->Sap;
		break;
	case TBrickParam::MA_RANGES:
		INFOLOG("MA_RANGES: %u",((CSBrickParamMagicRanges *)param)->RangeIndex);
		_RangeIndex += ((CSBrickParamMagicRanges *)param)->RangeIndex;
		break;
	case TBrickParam::MA_BREAK_RES:
		INFOLOG("MA_BREAK_RES: %u",((CSBrickParamMagicBreakResist *)param)->BreakResist);
		_BreakResist = ((CSBrickParamMagicBreakResist*)param)->BreakResist;
		break;
	case TBrickParam::MA_ARMOR_COMP:
		INFOLOG("MA_ARMOR_COMP: %u",((CSBrickParamMagicArmorComp *)param)->ArmorComp);
		_ArmorCompensation = ((CSBrickParamMagicArmorComp*)param)->ArmorComp;
		break;
	}
	
}// CMagicPhrase applyBrickParam


//-----------------------------------------------
// CMagicPhrase build
//-----------------------------------------------
bool CMagicPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks )
{
	_IsStatic = true;

	//init with default values
	_RangeIndex = MagicDefaultRangeIndex;
	_CastingTime = NLMISC::TGameCycle( CMagicPhrase::defaultCastingTime() / CTickEventHandler::getGameTimeStep() );
	
	// we are sure there is at least one brick and that there are non NULL;
	nlassert( !bricks.empty() );

	_ActorRowId = actorRowId;

	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( actorRowId );
	if ( !caster )
	{
		nlwarning("<CMagicPhrase build> invalid caster %u", actorRowId.getIndex() );
		return false;
	}

	// compute cost, credit and aggro
	for ( uint i = 0; i < bricks.size(); ++i )
	{
		if ( bricks[i]->SabrinaValue < 0 )
			_SabrinaCredit -= bricks[i]->SabrinaValue;
		else
			_SabrinaCost += bricks[i]->SabrinaValue;
	}

	// Parse other params
	std::vector<CSheetId> ranges;
	for ( uint i = 0; i < bricks.size(); )
	{
		nlassert ( bricks[i] );
		const CStaticBrick & brick = *bricks[i];
		INFOLOG("Build brick % u. Name : %s",i, brick.SheetId.toString().c_str() );

		if ( brick.Skill != SKILLS::unknown )
			_Skills.push_back(brick.Skill);
		
		// if we are on an effect brick, treat the effect
		if ( !brick.Params.empty()  && brick.Params[0]->id() == TBrickParam::MA )
		{
			INFOLOG("brick %u. Name : %s : first param is an effect type",i, brick.SheetId.toString().c_str() );
			// get the action range table
			if ( brick.RangeTable != CSheetId::Unknown )
				ranges.push_back(brick.RangeTable);
		
			// determine the execution behaviour of the phrase through the effect
			// if different nature are found, choose the neutral one
			if ( _Nature == ACTNATURE::UNKNOWN )
				_Nature = brick.Nature;
			else if ( brick.Nature != _Nature )
				_Nature = ACTNATURE::NEUTRAL;


			// build the action
			IMagicAction * action  = IMagicActionFactory::buildAction(actorRowId,bricks,i,this);
			if ( !action )
			{
				nlwarning( "<CMagicPhrase build> could not build action in brick %s position in phrase %u", brick.SheetId.toString().c_str(),i );
				return false;
			}
			_Actions.push_back(action);
		}
		// if we are on a sentence global params
		else
		{
			INFOLOG("pos in phrase %u brick name : %s : first param is a global sentence param or is empty",i, brick.SheetId.toString().c_str() );
			for ( uint j=0 ; j < brick.Params.size() ; ++j)
			{
				applyBrickParam( brick.Params[j] );
			}
			++i;
		}
		INFOLOG("pos in phrase %u Brick name : %s : all param parsed",i, brick.SheetId.toString().c_str() );
	}
	

	if ( _RangeIndex < 0 )
	{
		nlwarning( "<CMagicPhrase build> range index %d is < 0", _RangeIndex );
		_RangeIndex = 0;
	}

	// compute final range : it is the shortst action range
	if (  ranges.empty()  )
	{
		nlwarning("using default range value");
		ranges.push_back( CSheetId( "default.sbrick_magic_range"  ) );
	}
	for (uint i = 0; i < ranges.size(); i++ )
	{
		const CStaticMagicRange * sheet = CSheets::getMagicRangeTable(ranges[i]);
		if ( sheet )
		{
			if ( _RangeIndex > (sint)sheet->Ranges.size() )
			{
				nlwarning( "<CMagicPhrase build> range index %d too high: max is %u", _RangeIndex, sheet->Ranges.size() );
				return false;
			}
			if ( _Range < sheet->Ranges[_RangeIndex])
				_Range = sheet->Ranges[_RangeIndex];
		}
		else
		{
			nlwarning( "<CMagicPhrase build> invalid range in effect %u", i);
		}
	}
	if ( _Range == 0 )
	{
		nlwarning( "<CMagicPhrase build> no valid range found setting it to 100");
		_Range = 100;
	}
	_Range = _Range * 1000;
	INFOLOG("Phrase built");
	return true;
}// CMagicPhrase build

//-----------------------------------------------
// CMagicPhrase evaluate
//-----------------------------------------------
bool CMagicPhrase::evaluate( CEvalReturnInfos *msg )
{
	// update state
	_State = CSPhrase::Evaluated;
	return true;
}// CMagicPhrase evaluate


//-----------------------------------------------
// CMagicPhrase validate
//-----------------------------------------------
bool CMagicPhrase::validate()
{
	CEntityBase * entity = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _Targets[0] );

	if ( !entity )
	{
		nlwarning("<CMagicPhrase validate> Invalid caster %u",_ActorRowId.getIndex() );
		return false;
	}

	if ( !target )
	{
		nlwarning("<CMagicPhrase validate> Invalid target %u",_Targets[0].getIndex() );
		return false;
	}

	// test caster scores
	const sint32 hp = entity->getScores()._PhysicalScores[ SCORES::hit_points ].Current;
	if (hp <= 0	||	entity->getMode()==MBEHAV::DEATH)
	{
		return false;
	}
	
	if ( hp < _HPCost  )
	{
		if ( entity->getId().getType() == RYZOMID::player )
			CCharacter::sendMessageToClient( entity->getId(),"MAGIC_LACK_HP" );
		return false;
	}
	const sint32 sap = entity->getScores()._PhysicalScores[ SCORES::sap ].Current;
	if ( sap < _SapCost  )
	{
		if ( entity->getId().getType() == RYZOMID::player )
			CCharacter::sendMessageToClient( entity->getId(),"MAGIC_LACK_SAP" );
		return false;
	}

	// test range
	const double dx = entity->getState().X - target->getState().X;
	const double dy = entity->getState().Y - target->getState().Y;

	// get range debuff
	sint32 range = _Range;
	const CSEffect * debuff = entity->lookForSEffect( EFFECT_FAMILIES::RangeCap, false );
	if ( debuff && debuff->getParamValue() < range )
		range = debuff->getParamValue();
	
	if( dx* dx + dy*dy > (double)range* (double) range )
	{
		if ( entity->getId().getType() == RYZOMID::player )
			CCharacter::sendMessageToClient( entity->getId(),"MAGIC_TARGET_OUT_OF_RANGE" );
		return false;
	}

	
	/// todo nico : test if on mount
	/// todo nico : test specific effects preventing spell cast

	// at least one action must work on the main target
	uint i = 0;
	for ( ; i < _Actions.size(); i++ )
	{
		if ( _Actions[i]->validate(this) )
			break;
	}
	if ( i == _Actions.size() )
	{
		if ( entity->getId().getType() == RYZOMID::player )
			CCharacter::sendMessageToClient( entity->getId(),"MAGIC_BAD_TARGET" );
		return false;
	}


	// update state
	if (_State == Evaluated)
		_State = Validated;
	else if (_State == ExecutionInProgress)
		_State = SecondValidated;
	return true;
}// CMagicPhrase validate

//-----------------------------------------------
// CMagicPhrase update
//-----------------------------------------------
bool  CMagicPhrase::update()
{
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	// if the sentence execution delay time has ended, apply sentence effects
	if ( _State == SecondValidated && _ExecutionEndDate <= time && _NbWaitingRequests == 0 )
	{
		apply();
		_State = LatencyEnded;
	}
	return true;
}// CMagicPhrase update

//-----------------------------------------------
// CMagicPhrase execute
//-----------------------------------------------
void  CMagicPhrase::execute()
{
	if( _NbWaitingRequests != 0)
		return;

	TDataSetRow mainTarget = _ActorRowId;
	bool self = true;
	if ( !_Targets.empty() && _Targets[0] != _ActorRowId)
	{
		mainTarget = _Targets[0];
		self = false;
	}	

	// determine the final behaviour
	MBEHAV::CBehaviour behav;
	switch (_Nature)
	{
	case ACTNATURE::NEUTRAL:
		behav = MBEHAV::CAST_MIX;
		break;
	case ACTNATURE::DEFENSIVE:
		behav = MBEHAV::CAST_CUR;
		break;
	case ACTNATURE::OFFENSIVE:
		behav = MBEHAV::CAST_OFF;
		break;
	}

	behav.Data = 0;
	if ( behav.Behaviour != MBEHAV::UNKNOWN_BEHAVIOUR )
		PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
	else
		nlwarning("<CMagicPhrase execute> Invalid behaviour");

	// determine the end of the cast
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	_State = CSPhrase::ExecutionInProgress;
	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	if ( !caster )
	{
		nlwarning("<CMagicPhrase execute> Invalid entity %u",_ActorRowId.getIndex());
	}

	NLMISC::TGameCycle castingTime = _CastingTime ;
	const CSEffect * slow = caster->lookForSEffect( EFFECT_FAMILIES::SlowMagic );
	if ( slow )
	{
		castingTime = NLMISC::TGameCycle ( castingTime * (slow->getParamValue() / 100.0f ) );
	}
	_ExecutionEndDate  = time + castingTime;

	if ( caster->getId().getType() == RYZOMID::player )
		PHRASE_UTILITIES::sendSpellBeginCastMessages(_ActorRowId, mainTarget, _Nature);
}// CMagicPhrase execute


//-----------------------------------------------
// CMagicPhrase apply
//-----------------------------------------------
void CMagicPhrase::apply()
{
	// spend sap, hp
	CEntityBase* entity = PHRASE_UTILITIES::entityPtrFromId( _ActorRowId );
	if (entity == NULL)
	{
		nlwarning("<CCombatPhrase::apply> Invalid entity Id %s", TheDataset.getEntityId(_ActorRowId).toString().c_str() );		
		return;
	}
	RY_GAME_SHARE::SCharacteristicsAndScores &sap = entity->getScores()._PhysicalScores[SCORES::sap];
	if ( _SapCost )
	{
		sap.Current = sap.Current - _SapCost;
		if (sap.Current < 0)
			sap.Current = 0;
	}

	RY_GAME_SHARE::SCharacteristicsAndScores &hp = entity->getScores()._PhysicalScores[SCORES::hit_points];
	if ( _HPCost != 0)
	{
		entity->changeCurrentHp(-_HPCost);
	}

	// compute average skill value
	sint skillValue = 0;
	for ( uint i = 0; i < _Skills.size(); i++ )
	{
		SSkill * skill = entity->getSkills().getSkillStruct( _Skills[i] );
		if ( skill )
		{
			skillValue+= skill->Current;
		}
		else
		{
			nlwarning("<CMagicPhrase apply> invalid skill %d",_Skills[i]);
			return;
		}
	}
	skillValue /= _Skills.size();
	const CSEffect * debuff = entity->lookForSEffect( EFFECT_FAMILIES::DebuffSkillMagic );
	if ( debuff)
		skillValue -= debuff->getParamValue();

	sint32 armorMalus = (sint32) ( entity->getArmorCastingMalus() * (float)skillValue );
	armorMalus -= _ArmorCompensation;
	if ( armorMalus <0 )
		armorMalus = 0;
	skillValue -= armorMalus;

	

	// check for magic madness effect
	bool isMad = false;
	const CSEffect * madness = entity->lookForSEffect(EFFECT_FAMILIES::MadnessMagic);
	if ( madness )
	{
		const uint8 roll = (uint8) RandomGenerator.rand(99);
		if ( roll < madness->getParamValue() )
		{
			_Targets.resize(1);
			_Targets[0] = _ActorRowId;
			isMad = true;
		}
	}

	// get the success factor (divide delta level by 10 because a level is 10 skill points
	sint deltaLvl = ( skillValue + (sint32)_SabrinaCredit - (sint32)(_SabrinaCost<<1) )/10;
	const uint8 chances = PHRASE_UTILITIES::getSuccessChance( deltaLvl );
	const uint8 roll = (uint8) RandomGenerator.rand(99);
	float successFactor = PHRASE_UTILITIES::getSucessFactor(chances, roll);

	/// compute XP gain
	if ( entity->getId().getType() == RYZOMID::player && successFactor > 0.0f )
	{
		for (uint i = 0; i < _Skills.size(); i++ )
		{
			///\todo nico successFactor is the quality factor of the action for xp
			///\todo nico multi target XP
			CEntityBase * mainTarget = CEntityBaseManager::getEntityBasePtr( _Targets[0] );
			((CCharacter*) entity)->actionReport( mainTarget,deltaLvl, ACTNATURE::OFFENSIVE, SKILLS::toString( _Skills[i] ) );
		}
	}


	// send behaviour
	MBEHAV::CBehaviour behav;

	///\todo links
	/*	
		CAST_OFF_LINK,				
		CAST_CUR_LINK,				
		CAST_MIX_LINK,	*/	
	
	TDataSetRow mainTarget = _ActorRowId;
	if ( !_Targets.empty() && _Targets[0] != _ActorRowId)
	{
		mainTarget = _Targets[0];
	}

	if ( successFactor < 0.0f )
	{
		PHRASE_UTILITIES::sendSpellFumbleMessages(_ActorRowId, mainTarget);
		switch (_Nature)
		{
		case ACTNATURE::NEUTRAL:
			behav = MBEHAV::CAST_MIX_FUMBLE;
			break;
		case ACTNATURE::DEFENSIVE:
			behav = MBEHAV::CAST_CUR_FUMBLE;
			break;
		case ACTNATURE::OFFENSIVE:
			behav = MBEHAV::CAST_OFF_FUMBLE;
			break;
		}
	}
	else if ( successFactor > 0.0f )
	{
		PHRASE_UTILITIES::sendSpellSuccessMessages(_ActorRowId, mainTarget);
		switch (_Nature)
		{
		case ACTNATURE::NEUTRAL:
			behav = MBEHAV::CAST_MIX_SUCCESS;
			break;
		case ACTNATURE::DEFENSIVE:
			behav = MBEHAV::CAST_CUR_SUCCESS;
			break;
		case ACTNATURE::OFFENSIVE:
			behav = MBEHAV::CAST_OFF_SUCCESS;
			break;
		}
	}
	else
	{
		PHRASE_UTILITIES::sendSpellFailedMessages(_ActorRowId, mainTarget);
		switch (_Nature)
		{
		case ACTNATURE::NEUTRAL:
			behav = MBEHAV::CAST_MIX_FAIL;
			break;
		case ACTNATURE::DEFENSIVE:
			behav = MBEHAV::CAST_CUR_FAIL;
			break;
		case ACTNATURE::OFFENSIVE:
			behav = MBEHAV::CAST_OFF_FAIL;
			break;
		}
	}

	// apply each effect of the spell
	for ( uint i = 0; i < _Actions.size(); i++ ) 
	{
		_Actions[i]->apply(this,successFactor,behav, isMad );
	}
	behav.Spell.Time = CTickEventHandler::getGameCycle();

	if ( behav.Behaviour != MBEHAV::UNKNOWN_BEHAVIOUR )
		PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
	else
		nlwarning("<CMagicPhrase apply> Invalid behaviour");
	
	// clear temporary execution data
	_Targets.reserve(1);
	_Targets.resize(1);
}//CMagicPhrase apply 

//-----------------------------------------------
// CMagicPhrase stop
//-----------------------------------------------
void CMagicPhrase::stop()
{
	if ( _State >= CSPhrase::ExecutionInProgress )
	{
		CCharacter* player = PlayerManager.getChar(_ActorRowId);
		if (player)
		{
			player->clearCurrentAction();
			PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "EGS_ACTOR_CASTING_INTERUPT");			
		}			

		// send behaviour
		MBEHAV::CBehaviour behav;
		switch (_Nature)
		{
		case ACTNATURE::NEUTRAL:
			behav = MBEHAV::CAST_MIX_FAIL;
			break;
		case ACTNATURE::DEFENSIVE:
			behav = MBEHAV::CAST_CUR_FAIL;
			break;
		case ACTNATURE::OFFENSIVE:
			behav = MBEHAV::CAST_OFF_FAIL;
			break;
		}
		// set behaviour
		PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
	}
} // stop //

