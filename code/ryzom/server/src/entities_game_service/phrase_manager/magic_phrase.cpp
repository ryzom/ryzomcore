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
//
#include "game_share/brick_families.h"
#include "game_share/magic_fx.h"
#include "game_share/combat_flying_text.h"
//
#include "entity_structure/statistic.h"
#include "phrase_manager/magic_phrase.h"
#include "s_phrase_factory.h"
#include "phrase_manager/sabrina_area_debug.h"
#include "entity_manager/entity_manager.h"
#include "phrase_manager/phrase_manager.h"
#include "player_manager/player_manager.h"
#include "creature_manager/creature_manager.h"
#include "phrase_manager/s_effect.h"
#include "player_manager/character.h"
#include "creature_manager/creature.h"
#include "player_manager/player.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "redirect_attacks_effect.h"
#include "entities_game_service.h"
#include "mission_manager/mission_event.h"
#include "phrase_manager/shoot_again_effect.h"

#include "projectile_stats.h"

//////////////
//	USING	//
//////////////
using namespace std;
using namespace NLMISC;

//////////////
//	EXTERN	//
//////////////
extern NLMISC::CRandom		RandomGenerator;
extern CPlayerManager		PlayerManager;
extern CCreatureManager		CreatureManager;

DEFAULT_SPHRASE_FACTORY(CMagicPhrase, BRICK_TYPE::MAGIC)

NL_INSTANCE_COUNTER_IMPL(CMagicPhrase);


////////////
// STATIC //
////////////
float CMagicPhrase::_DefaultCastingTime = 1.0f;

string CMagicFocusItemFactor::_ElementalSkill = "SMOE";
string CMagicFocusItemFactor::_OffensiveAfflictionSkill = "SMOA";
string CMagicFocusItemFactor::_DefensiveAfflictionSkill = "SMDA";
string CMagicFocusItemFactor::_HealSkill = "SMDH";


//-----------------------------------------------
// CMagicPhrase dtor
//-----------------------------------------------
CMagicPhrase::~CMagicPhrase()
{
	H_AUTO(CMagicPhraseDestructor);
	
	for (uint  i = 0; i < _Actions.size(); i++ )
	{
		if (_Actions[i] != NULL)
			delete _Actions[i];
		else
			nlwarning("Found a NULL Action in magic phrase, position %u, size = %u",i, _Actions.size());
	}
	if (_Area)
		delete _Area;
}

//--------------------------------------------------------------
//	CMagicPhrase::initPhraseFromAiAction 
//--------------------------------------------------------------
bool CMagicPhrase::initPhraseFromAiAction( const TDataSetRow & actorRowId, const CStaticAiAction *aiAction )
{
	H_AUTO(CMagicPhrase_initPhraseFromAiAction);
	
	if ( !TheDataset.isAccessible(actorRowId) )
	{
		return false;
	}

	_ActorRowId = actorRowId;

	// read parameters
	if (aiAction == NULL)
	{
		nlwarning("MAGIC: initPhraseFromAiAction() called with a NULL pointer");
		return false;
	}

	IMagicAction *spellAction = IMagicAiActionFactory::buildActionFromAiAction(aiAction, this);
	if (spellAction == NULL)
		return false;

	AIACTIONLOG("AI actor %s execute Magic action %s ",TheDataset.getEntityId(actorRowId).toString().c_str(), aiAction->getSheetId().toString().c_str());

	const TAiActionParams &data = aiAction->getData();

	switch(aiAction->getType())
	{
	case AI_ACTION::HealSpell:
	case AI_ACTION::HoTSpell:
		_Nature = ACTNATURE::CURATIVE_MAGIC;
		break;
	case AI_ACTION::DamageSpell:
	case AI_ACTION::DoTSpell:
	case AI_ACTION::ToxicCloud:
		_Nature = ACTNATURE::OFFENSIVE_MAGIC;
		break;
		
	case AI_ACTION::EffectSpell:
	case AI_ACTION::EoTSpell:
		// TODO nature depends of effect type !!!
		_Nature = ACTNATURE::OFFENSIVE_MAGIC;
		break;
		
	default:
		// unknown type
		return false;
		break;
	};

	NLMISC::TGameCycle actionLatency = 0;

	switch(aiAction->getType())
	{
	case AI_ACTION::HealSpell:
	case AI_ACTION::DamageSpell:
	case AI_ACTION::ToxicCloud:
		{
			_SapCost += data.Spell.SapCost;
			_HPCost += data.Spell.HpCost;
			actionLatency = data.Spell.CastingTime;
			_PostCastTime += data.Spell.PostActionTime;
			
			// take behaviour only for creatures, npcs use cast behaviours
			if ( TheDataset.getEntityId(_ActorRowId).getType() == RYZOMID::creature)
				_CreatureBehaviour = data.Spell.Behaviour;
			else
				_CastBehaviour = data.Spell.Behaviour;
			
			if ( data.Spell.Skill != SKILLS::unknown )
			{
				_Skills.push_back(data.Spell.Skill);
				spellAction->setSkill(data.Spell.Skill);
			}
			else
			{
				_Skills.push_back(SKILLS::SM);
				spellAction->setSkill(SKILLS::SM);
			}
		}
		break;
	
	case AI_ACTION::DoTSpell:
	case AI_ACTION::HoTSpell:
		{
			_SapCost += data.OTSpell.SapCost;
			_HPCost += data.OTSpell.HpCost;
			actionLatency = data.OTSpell.CastingTime;
			_PostCastTime += data.OTSpell.PostActionTime;
			
			// take behaviour only for creatures, npcs use standard cast behaviours
			if ( TheDataset.getEntityId(_ActorRowId).getType() == RYZOMID::creature)
				_CreatureBehaviour = data.OTSpell.Behaviour;
			else
				_CastBehaviour = data.OTSpell.Behaviour;
			
			if ( data.OTSpell.Skill != SKILLS::unknown )
			{
				_Skills.push_back(data.OTSpell.Skill);
				spellAction->setSkill(data.OTSpell.Skill);
			}
			else
			{
				_Skills.push_back(SKILLS::SM);
				spellAction->setSkill(SKILLS::SM);
			}
		}
		break;

	case AI_ACTION::EffectSpell:
		{
			_SapCost += data.EffectSpell.SapCost;
			_HPCost += data.EffectSpell.HpCost;
			actionLatency = data.EffectSpell.CastingTime;
			_PostCastTime += data.EffectSpell.PostActionTime;
			
			// take behaviour only for creatures, npcs use standard cast behaviours
			if ( TheDataset.getEntityId(_ActorRowId).getType() == RYZOMID::creature)
				_CreatureBehaviour = data.EffectSpell.Behaviour;
			else
				_CastBehaviour = data.EffectSpell.Behaviour;
			
			if ( data.EffectSpell.Skill != SKILLS::unknown )
			{
				_Skills.push_back(data.EffectSpell.Skill);
				spellAction->setSkill(data.EffectSpell.Skill);
			}
			else
			{
				_Skills.push_back(SKILLS::SM);
				spellAction->setSkill(SKILLS::SM);
			}
		}
		break;
	case AI_ACTION::EoTSpell:
		{
			_SapCost += data.OTEffectSpell.SapCost;
			_HPCost += data.OTEffectSpell.HpCost;
			actionLatency = data.OTEffectSpell.CastingTime;
			_PostCastTime += data.OTEffectSpell.PostActionTime;
			
			// take behaviour only for creatures, npcs use standard cast behaviours
			if ( TheDataset.getEntityId(_ActorRowId).getType() == RYZOMID::creature)
				_CreatureBehaviour = data.OTEffectSpell.Behaviour;
			else
				_CastBehaviour = data.OTEffectSpell.Behaviour;
			
			if ( data.EffectSpell.Skill != SKILLS::unknown )
			{
				_Skills.push_back(data.OTEffectSpell.Skill);
				spellAction->setSkill(data.OTEffectSpell.Skill);
			}
			else
			{
				_Skills.push_back(SKILLS::SM);
				spellAction->setSkill(SKILLS::SM);
			}
		}
		break;

	default:
		// unknown type
		return false;
		break;
	};

	// if casting time and post action time are null, use creature base attack speed as post action delay
	if (actionLatency == CSpellParams::UseAttackSpeedForCastingTime)
	{
		CCreature  *creature = CreatureManager.getCreature(actorRowId);
		if (creature != NULL && creature->getForm() != NULL)
		{
			_CastingTime = creature->getForm()->getAttackLatency();
			_BaseCastingTime = _CastingTime;
		}
		else
		{
			_CastingTime = 30;
			_BaseCastingTime = _CastingTime;
		}
	}
	else
	{
		_CastingTime += actionLatency;
		_BaseCastingTime += actionLatency;
	}
		

	_Actions.push_back(spellAction);

	//compute area (not for toxic cloud)
	if (_Area == NULL && aiAction->getType() != AI_ACTION::ToxicCloud)
		_Area = CAreaEffect::buildArea(aiAction);

	// for debug, set a range in mm
	_Range = 50000;
	
	return true;
} // initPhraseFromAiAction //


//-----------------------------------------------
// CMagicPhrase applyBrickParam
//-----------------------------------------------
void CMagicPhrase::applyBrickParam( const TBrickParam::IId * param, const CStaticBrick &brick, CBuildParameters &buildParams )
{
	H_AUTO(CMagicPhrase_applyBrickParam);
	
	nlassert(param);
	switch(param->id())
	{
	case TBrickParam::SET_BEHAVIOUR:
		// $*STRUCT CSBrickParamSetBehaviour: public TBrickParam::CId <TBrickParam::SET_BEHAVIOUR>
		// $*-s string Behaviour	// the new behaviour to use
		if ( (uint16)abs(brick.SabrinaValue) > _BehaviourWeight )
		{
			MBEHAV::EBehaviour behaviour = MBEHAV::stringToBehaviour(((CSBrickParamSetBehaviour*)param)->Behaviour);
			if (behaviour != MBEHAV::UNKNOWN_BEHAVIOUR)
			{
				_BehaviourWeight = (uint16)abs(brick.SabrinaValue);
				_CastBehaviour = behaviour;
			}
		}
		break;

	case TBrickParam::HP:
		INFOLOG("HP: %i",((CSBrickParamHp *)param)->Hp);
		_HPCost += ((CSBrickParamHp *)param)->Hp;
		break;				
	case TBrickParam::SAP:
		INFOLOG("SAP: %i",((CSBrickParamSap *)param)->Sap);
		_SapCost += ((CSBrickParamSap *)param)->Sap;
		break;
	case TBrickParam::MA_BREAK_RES:
		INFOLOG("MA_BREAK_RES: %u",((CSBrickParamMagicBreakResist *)param)->BreakResist);
		buildParams.BreakResistBrickPower = ((CSBrickParamMagicBreakResist*)param)->BreakResistPower;
		_BreakResist = ((CSBrickParamMagicBreakResist*)param)->BreakResist;
		break;
	case TBrickParam::MA_ARMOR_COMP:
		INFOLOG("MA_ARMOR_COMP: %u",((CSBrickParamMagicArmorComp *)param)->ArmorComp);
		_ArmorCompensation = ((CSBrickParamMagicArmorComp*)param)->ArmorComp;
		break;
	case TBrickParam::MA_VAMPIRISE:
		INFOLOG("MA_VAMPIRISE: %u",((CSBrickParamMagicVampirise *)param)->Vampirise );
		_Vampirise = ((CSBrickParamMagicVampirise *)param)->Vampirise;
		break;
	case TBrickParam::MA_VAMPIRISE_RATIO:
		INFOLOG("MA_VAMPIRISE_RATIO: %u",((CSBrickParamMagicVampiriseRatio *)param)->VampiriseRatio );
		_VampiriseRatio = ((CSBrickParamMagicVampiriseRatio *)param)->VampiriseRatio;
		break;
	case TBrickParam::AREA_TARGETS:
		// $*STRUCT CSBrickParamAreaTargets: public TBrickParam::CId <TBrickParam::AREA_TARGETS>
		// $*-f float	TargetFactor	// each target count as 'TargetFactor' for damage or heal division among targets
		// $*-i uint8	MaxTargets		// max nb targets
		_MaxTargets = ((CSBrickParamAreaTargets *)param)->MaxTargets;
#if !FINAL_VERSION
		nlassert( _MaxTargets * ((CSBrickParamAreaTargets *)param)->TargetFactor > 0.0f );
#endif
		if ( _MaxTargets * ((CSBrickParamAreaTargets *)param)->TargetFactor > 0.0f)
			_MultiTargetFactor = 1 / (_MaxTargets * ((CSBrickParamAreaTargets *)param)->TargetFactor );
		else
			_MultiTargetFactor = 1.0f;
		break;
	default:
		// try to build the area
		if (_Area == NULL)
		{
			if( MagicAreaEffectOn )
				_Area = CAreaEffect::buildArea(param);
		}
	}
	
}// CMagicPhrase applyBrickParam



//-----------------------------------------------
// computeRange :
// Compute the range for current phrase.
//-----------------------------------------------
void CMagicPhrase::computeRange(float rangeFactor, float wearMalus)
{
	H_AUTO(CMagicPhrase_computeRange3);
	
	if(rangeFactor<0.0f)
	{
		nlwarning("CMagicPhrase:computeRange: rangeFactor(%f) should never be < 0", rangeFactor);
		rangeFactor = 0.0f;
	}
	// Clamp
	if(rangeFactor > 1.0f)
		rangeFactor = 1.0f;
	// Check Min >= Max
	if(_MaxRange < _MinRange)
	{
		nlwarning("CMagicPhrase:computeRange: _MaxRange(%u) < _MinRange(%u)", _MaxRange, _MinRange);
		_MaxRange = _MinRange;
	}
	// Compute the Range.
	float maxRange = (float)(_MaxRange*1000);
	float minRange = (float)(_MinRange*1000);
	_Range = (sint32)((maxRange-minRange)*(1.0f-rangeFactor)/wearMalus+minRange);
}// computeRange //

//-----------------------------------------------
// computeCastingTime :
// Compute the minimum casting time.
//-----------------------------------------------
void CMagicPhrase::computeCastingTime(float castingTimeFactor, float wearMalus)
{
	H_AUTO(CMagicPhrase_computeCastingTime);
	
	if(castingTimeFactor<0.0f)
	{
		nlwarning("CMagicPhrase:computeCastingTime: castingTimeFactor(%f) should never be < 0", castingTimeFactor);
		castingTimeFactor = 0.0f;
	}
	// Clamp
	if(castingTimeFactor > 1.0f)
		castingTimeFactor = 1.0f;
	// Check Min >= Max
	if(_MaxCastTime < _MinCastTime)
	{
		nlwarning("CMagicPhrase:computeCastingTime: _MaxCastTime(%u) < _MinCastTime(%u)", _MaxCastTime, _MinCastTime);
		_MaxCastTime = _MinCastTime;
	}

	// apply magic focus item factor
	float meanFactor = 0.0f;
	for (uint i = 0 ; i < _Skills.size() ; ++i)
	{
		meanFactor += _UsedItemStats.getCastingTimeFactor(_Skills[i], _BrickMaxSabrinaCost);
	}
	if (!_Skills.empty())
		meanFactor /= _Skills.size();
	
#ifdef NL_DEBUG
	nlassert(meanFactor >= 0.0f);
#endif

	// Compute the casting time.
	float castingTime = (float)(_MaxCastTime-_MinCastTime)*castingTimeFactor*wearMalus+(float)_MinCastTime / (1 + meanFactor);
	float baseCastingTime = (float)_MinCastTime / (1 + meanFactor);
	
	//
	_CastingTime = NLMISC::TGameCycle( castingTime/(CTickEventHandler::getGameTimeStep()*10.0) );
	_BaseCastingTime = NLMISC::TGameCycle( baseCastingTime/(CTickEventHandler::getGameTimeStep()*10.0) );
}// computeCastingTime //

//-----------------------------------------------
// CMagicPhrase build
// \warning The bricks vector MUST NOT be EMPTY.
//-----------------------------------------------
bool CMagicPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute )
{
	H_AUTO(CMagicPhrase_build);
	
	if ( _Area )
	{
		delete _Area;
		_Area = NULL;
	}

	// we are sure there is at least one brick and that there are non NULL;
	nlassert(bricks.empty()==false);
	// Static Phrase.
	_IsStatic = true;
	// Set type to Magic
	_PhraseType = BRICK_TYPE::MAGIC;
	// Initialize the default casting time.
	_CastingTime = NLMISC::TGameCycle( CMagicPhrase::defaultCastingTime() / CTickEventHandler::getGameTimeStep() );
	_BaseCastingTime = _CastingTime;
	// Default range in meter
	_Range = 30000;

	_ActorRowId = actorRowId;

	CBuildParameters buildParams;

	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( actorRowId );
	if ( !caster )
	{
		nlwarning("<CMagicPhrase build> invalid caster %u", actorRowId.getIndex() );
		return false;
	}

	// compute cost, credit and aggro
	for( uint i = 0; i < bricks.size(); ++i )
	{
		// Get a reference on the brick.
		const CStaticBrick & brick = *bricks[i];
		INFOLOG("Brick Name:%s", brick.SheetId.toString().c_str());
		INFOLOG("SabrinaValue:%d", brick.SabrinaValue);
		INFOLOG("Family:%d(%s)", brick.Family, BRICK_FAMILIES::toString(brick.Family).c_str());
		INFOLOG("Skill:%d(%s)", brick.getSkill(0), SKILLS::toString(brick.getSkill(0)).c_str());
		//
		if ( brick.SabrinaValue < 0 )
			_SabrinaCredit -= brick.SabrinaValue;
		else
		{
			_SabrinaCost += brick.SabrinaValue;
			if (_BrickMaxSabrinaCost < brick.SabrinaValue)
				_BrickMaxSabrinaCost = brick.SabrinaValue;
		}
		
		if( brick.SabrinaRelativeValue < 0.0f )
			_SabrinaRelativeCredit -= brick.SabrinaRelativeValue;
		else
			_SabrinaRelativeCost += brick.SabrinaRelativeValue;

		// Get Casting Time Credit.
		for( uint j=0 ; j < brick.Params.size() ; ++j)
		{
			const TBrickParam::IId* param = brick.Params[j];
			if(param)
			{
				INFOLOG("Param(%d) ID : %d", j, param->id());
				// CREDIT : casting time
				if(brick.Params[j]->id() == TBrickParam::MA_CASTING_TIME)
					_CastingTimeCredit += (float)(-brick.SabrinaValue);
				// CREDIT : range
				if(brick.Params[j]->id() == TBrickParam::MA_RANGE)
				{
					if ( ((CSBrickParamMagicRanges *)param)->RangeIndex != 0)
						_RangeCredit += (float)(-brick.SabrinaValue);
				}
			}
		}
		// CASTING TIME //
		// Get the minimum casting Time
		if(_MinCastTime < brick.MinCastTime)
			_MinCastTime = brick.MinCastTime;
		// Get the maximum casting Time
		if(_MaxCastTime > brick.MaxCastTime)
			_MaxCastTime = brick.MaxCastTime;
		// RANGE //
		// Get the minimum casting Range
		if(_MinRange < brick.MinRange)
			_MinRange = brick.MinRange;
		// Get the maximum casting Range
		if(_MaxRange > brick.MaxRange)
			_MaxRange = brick.MaxRange;
		// Target Type
		if(brick.TargetRestriction == TARGET::SelfOnly)
			_TargetRestriction = brick.TargetRestriction;
	}

	// force enchant phrases to be self target ( for behaviour )
	if ( _EnchantPhrase )
		_TargetRestriction = TARGET::SelfOnly;

	// If the Phrase is a self target only, target = actor
	if(_TargetRestriction == TARGET::SelfOnly)
	{
		_Targets.resize(1);
		_Targets[0].setId(_ActorRowId);
	}


	// Parse other params
	std::vector<CSheetId> rangeTables;
	uint i = 0;
	while(i < bricks.size())
	{
		nlassert ( bricks[i] );
		const CStaticBrick & brick = *bricks[i];
		INFOLOG("Build brick % u. Name : %s",i, brick.SheetId.toString().c_str() );

		// determine the execution behaviour of the phrase through the effect
		// if different nature are found, choose the offensive one one
		if ( _Nature == ACTNATURE::UNKNOWN )
			_Nature = brick.Nature;
		else if ( brick.Nature != ACTNATURE::UNKNOWN && brick.Nature != ACTNATURE::NEUTRAL && brick.Nature != _Nature )
		{
			if (_Nature == ACTNATURE::NEUTRAL)
				_Nature = brick.Nature;
			else
				_Nature = ACTNATURE::NEUTRAL;
		}

		// add brick skills
		_Skills.insert(_Skills.end(), brick.Skills.begin(), brick.Skills.end());
		
		// if we are on an effect brick, process the effect
		if ( !brick.Params.empty()  && brick.Params[0]->id() == TBrickParam::MA )
		{
			INFOLOG("brick %u. Name : %s : first param is an effect type",i, brick.SheetId.toString().c_str() );

			// build the action
			IMagicAction * action  = IMagicActionFactory::buildAction(actorRowId,bricks,i,buildParams,this);
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
				applyBrickParam( brick.Params[j], brick, buildParams );
			}
			++i;
		}
		//
		INFOLOG("pos in phrase %u Brick name : %s : all param parsed",i, brick.SheetId.toString().c_str() );
	}

	// get used item stats
	initUsedMagicFocusStats();

	// compute real param values from build params
	if ( buildParams.BreakResistBrickPower < (uint16)getSabrinaCost() )
		_BreakResist = uint16(_BreakResist * float(buildParams.BreakResistBrickPower) / getSabrinaCost());
		
	// apply Wear equipment malus
	CCharacter * c = dynamic_cast< CCharacter * >( caster );
	float WearMalus = 1.0f;
	if( c )
	{
		WearMalus += c->wearMalus();
		_SapCost = (uint16)(_SapCost * WearMalus);
		_HPCost = (uint16)(_HPCost * WearMalus);
	}
		
	// Compute the casting time for the phrase.
	float castingTimeFactor;
	if(_SabrinaCost)
		castingTimeFactor = (float)_CastingTimeCredit/(float)getSabrinaCost();
	else
		castingTimeFactor = (float)_CastingTimeCredit;
	
	computeCastingTime(castingTimeFactor, WearMalus );

	// Compute the right range for this phrase.
	float rangeFactor;
	if(_SabrinaCost)
		rangeFactor = (float)_RangeCredit/(float)getSabrinaCost();
	else
		rangeFactor = (float)_RangeCredit;

	computeRange(rangeFactor, WearMalus);
	
	// FIX the Action Nature if unknown
	if(_Nature == ACTNATURE::UNKNOWN)
	{
		_Nature = ACTNATURE::NEUTRAL;
	}

	INFOLOG("Phrase built");
	return true;
}// CMagicPhrase build

//-----------------------------------------------
// CMagicPhrase evaluate
//-----------------------------------------------
bool CMagicPhrase::evaluate()
{
	return true;
}// CMagicPhrase evaluate


//-----------------------------------------------
// CMagicPhrase validate
//-----------------------------------------------
bool CMagicPhrase::validate()
{
	H_AUTO(CMagicPhrase_validate);
	
	CEntityBase * entity = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	if ( !entity )
	{
		nlwarning("<CMagicPhrase validate> Invalid caster %u",_ActorRowId.getIndex() );
		return false;
	}

	if (entity->isDead())
	{
		return false;
	}

	// test entity can use action
	if (entity->canEntityUseAction() == false)
	{
		return false;
	}

	// Must have at least one target
	if (_Targets.empty() && (!_EnchantPhrase))
	{
		return false;
	}	
		
	// tests only made for players
	if (entity->getId().getType() == RYZOMID::player)
	{
		// CHECK NB LINKS HERE


		// if spell isn't selfonly, the caster cannot cast it on himself
		if ((_TargetRestriction != TARGET::SelfOnly) && (!_EnchantPhrase))
		{
			if ( !_Targets.empty() && TheDataset.isAccessible(_Targets[0].getId()) && _Targets[0].getId() == _ActorRowId)
			{
				// TODO Send message to caster
				PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "MAGIC_CANNOT_SELFCAST");
				return false;
			}
		}
		
		// test caster scores (only for players)
		CCharacter *character = (CCharacter *) (entity);		
		
		const sint32 hp = entity->currentHp();
		if ( hp <= _HPCost  )
		{
			if ( entity->getId().getType() == RYZOMID::player )
				PHRASE_UTILITIES::sendDynamicSystemMessage(entity->getEntityRowId(),"EGS_MAGIC_LACK_HP" );
			return false;
		}
		const sint32 sap = entity->getScores()._PhysicalScores[ SCORES::sap ].Current;
		if ( sap < _SapCost  )
		{
			if ( entity->getId().getType() == RYZOMID::player )
				PHRASE_UTILITIES::sendDynamicSystemMessage(entity->getEntityRowId(),"EGS_MAGIC_LACK_SAP" );
			return false;
		}

		if ( _EnchantPhrase )
		{
			uint money = uint( getSabrinaCost() * CristalMoneyFactor );
			if ( !money )
				money = 1;
			if ( character->getMoney() < money )
			{
				PHRASE_UTILITIES::sendDynamicSystemMessage(entity->getEntityRowId(),"EGS_MAGIC_LACK_MONEY" );
				return false;
			}
//			entity->setActionFlag( RYZOMACTIONFLAGS::Attacks, true );
			return true;
		}		

		CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _Targets[0].getId() );
		if ( !target )
		{
			//nlwarning("<CMagicPhrase validate> Invalid target %u",_Targets[0].getId().getIndex() );
			return false;
		}
		// test target is still alive
		if( target->isDead() && (!( ( target->getId().getType() == RYZOMID::player ) && ( target->currentHp() > - target->maxHp() ) ) ) )
		{
			// target is dead
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "MAGIC_TARGET_DEAD");
			return false;
		}

		
		if( ! PHRASE_UTILITIES::testRange(*entity, *target, _Range) )
		{
			if ( entity->getId().getType() == RYZOMID::player )
				CCharacter::sendDynamicSystemMessage(entity->getId(),"EGS_MAGIC_TARGET_OUT_OF_RANGE" );
			return false;
		}

		// test if caster can cast a spell right now
		if (character && character->dateOfNextAllowedAction() > CTickEventHandler::getGameCycle() )
		{		
			PHRASE_UTILITIES::sendDynamicSystemMessage( entity->getEntityRowId(), "MAGIC_CANNOT_CAST_YET");
			// _BeingProcessed = false;
			return false;
		}
	}

	
	// at least one action must work on the main target
	{
		string errorCode;
		uint i = 0;
		for ( ; i < _Actions.size(); i++ )
		{
			if ( _Actions[i]->validate(this, errorCode) )
				break;
		}
		if ( i == _Actions.size() )
		{
			if ( entity->getId().getType() == RYZOMID::player )
			{
				if (errorCode.empty())
				{
					CCharacter::sendDynamicSystemMessage( entity->getId(),"EGS_MAGIC_BAD_TARGET" );
//					CCharacter::sendMessageToClient( entity->getId(),"EGS_MAGIC_BAD_TARGET" );
				}
				else
				{
					/// can use older STATIC_STRING or new string format, when all codes will be replaced, only send dyn msg
					if (errorCode.substr(0,6) == string("MAGIC_") )
						PHRASE_UTILITIES::sendDynamicSystemMessage( entity->getEntityRowId(), errorCode );
					else
						PHRASE_UTILITIES::sendSimpleMessage( entity->getId(), errorCode );
				}
			}
			return false;
		}
	}

	// update state
	return true;
}// CMagicPhrase validate

//-----------------------------------------------
// CMagicPhrase update
//-----------------------------------------------
bool  CMagicPhrase::update()
{
	return true;
}// CMagicPhrase update 

//-----------------------------------------------
// CMagicPhrase execute
//-----------------------------------------------
void  CMagicPhrase::execute()
{
	H_AUTO(CMagicPhrase_execute);
	
	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	if ( !caster )
	{
		nlwarning("<CMagicPhrase execute> Invalid entity %u",_ActorRowId.getIndex());
		// _BeingProcessed = false;
		return;
	}

	// _BeingProcessed = true;

	TDataSetRow mainTarget = _ActorRowId;
	bool self = true;
	if ( !_Targets.empty() && _Targets[0].getId() != _ActorRowId)
	{
		mainTarget = _Targets[0].getId();
		self = false;
	}
	
	// Item procs
	{
		CEntityBase * actingEntity = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
		std::vector<SItemSpecialEffect> effects, effects2;
		if ( actingEntity->getId().getType() == RYZOMID::player )
		{
			CCharacter* c = dynamic_cast<CCharacter*>(actingEntity);
			effects = c->lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::ISE_MAGIC_DIVINE_INTERVENTION);
		}
		if ( actingEntity->getId().getType() == RYZOMID::creature )
		{
			CGameItemPtr usedItem;
			CCreature* c = dynamic_cast<CCreature*>(actingEntity);
			usedItem = c->getRightHandItem();
			if (usedItem!=NULL && usedItem->getStaticForm() && usedItem->getStaticForm()->ItemSpecialEffects && !usedItem->getStaticForm()->ItemSpecialEffects->Effects.empty())
			{
				effects2 = usedItem->getStaticForm()->lookForEffects(ITEM_SPECIAL_EFFECT::ISE_MAGIC_DIVINE_INTERVENTION);
				effects.insert(effects.end(), effects2.begin(), effects2.end());
			}
			usedItem = c->getLeftHandItem();
			if (usedItem!=NULL && usedItem->getStaticForm() && usedItem->getStaticForm()->ItemSpecialEffects && !usedItem->getStaticForm()->ItemSpecialEffects->Effects.empty())
			{
				effects2 = usedItem->getStaticForm()->lookForEffects(ITEM_SPECIAL_EFFECT::ISE_MAGIC_DIVINE_INTERVENTION);
				effects.insert(effects.end(), effects2.begin(), effects2.end());
			}
		}
		std::vector<SItemSpecialEffect>::const_iterator it, itEnd;
		for (it=effects.begin(), itEnd=effects.end(); it!=itEnd; ++it)
		{
			float rnd = RandomGenerator.frand();
			if (rnd<it->EffectArgFloat[0])
			{
				_DivineInterventionOccured = true;
				PHRASE_UTILITIES::sendItemSpecialEffectProcMessage(ITEM_SPECIAL_EFFECT::ISE_MAGIC_DIVINE_INTERVENTION, actingEntity);
			}
		}
	}
	
	// Consume a shoot again buff if present
	{
		CEntityBase * actingEntity = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
		CSEffectPtr const effect = actingEntity->lookForActiveEffect(EFFECT_FAMILIES::ProcShootAgain);
		if (effect)
		{
		//	CShootAgainEffect const* shootAgain = static_cast<CShootAgainEffect const*>((CSEffect*)effect);
			_ShootAgainOccured = true;
			actingEntity->removeSabrinaEffect(effect);
		}
	}
	
	// determine the end of the cast
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();

	NLMISC::TGameCycle castingTime = _DivineInterventionOccured||_ShootAgainOccured?_BaseCastingTime:_CastingTime;

	// look for slow effects
	sint32 slowingParam = 0;
	
	// do not use smart pointers for local use only
	const CSEffect *slow = caster->lookForActiveEffect( EFFECT_FAMILIES::SlowMagic );
	if ( slow )
	{
		slowingParam += slow->getParamValue();
	}
	slow = caster->lookForActiveEffect( EFFECT_FAMILIES::SlowAttack);
	if ( slow )
	{
		slowingParam += slow->getParamValue();
	}
	slow = caster->lookForActiveEffect( EFFECT_FAMILIES::CombatSlow);
	if ( slow )
	{
		slowingParam += slow->getParamValue();
	}
	slow = caster->lookForActiveEffect( EFFECT_FAMILIES::CombatCastSlow);
	if ( slow )
	{
		slowingParam += slow->getParamValue();
	}
	castingTime = NLMISC::TGameCycle ( castingTime * (slowingParam / 100.0f + 1.0f ) );
	
	if (_Nature == ACTNATURE::RECHARGE)
		castingTime /= 2;

	_ExecutionEndDate  = time + castingTime;

	if (_IsStatic && caster->getId().getType() == RYZOMID::player )
	{
		CCharacter *player = dynamic_cast<CCharacter*> (caster);
		if (player)
			player->staticActionInProgress(true, STATIC_ACT_TYPES::Casting);
		if ( ! _EnchantPhrase )
			PHRASE_UTILITIES::sendSpellBeginCastMessages(_ActorRowId, mainTarget, _Nature);
	}

	// add PostCastLatency to action duration to indicate the real end of the cast
	caster->setCurrentAction(CLIENT_ACTION_TYPE::Spell, _ExecutionEndDate + PostCastLatency + _PostCastTime);

	// determine the behaviour
	if (_CreatureBehaviour == MBEHAV::UNKNOWN_BEHAVIOUR && (_DivineInterventionOccured||_ShootAgainOccured?_BaseCastingTime:_CastingTime) > 0)
	{
		MBEHAV::CBehaviour behav;

		if (_CastBehaviour == MBEHAV::UNKNOWN_BEHAVIOUR)
		{
			switch (_Nature)
			{
			case ACTNATURE::NEUTRAL:
				behav = MBEHAV::CAST_MIX;
				break;
			case ACTNATURE::CURATIVE_MAGIC:
				behav = MBEHAV::CAST_CUR;
				break;
			case ACTNATURE::OFFENSIVE_MAGIC:
				behav = MBEHAV::CAST_OFF;
				break;
			case ACTNATURE::RECHARGE:
				behav =  MBEHAV::CAST_MIX;
				break;
			}
		}
		else
		{
			behav = _CastBehaviour;
		}
		
		behav.Data = 0;
		behav.Data2 = 0;
		
		// TODO set dispersion mode if multitarget
		behav.Spell.SpellMode = MAGICFX::Bomb;
		// set time
		behav.Spell.Time = 0;
		// set intensity
		uint intensity = getSabrinaCost();	
		// NPC -> intensity depends of npc level
		if (caster->getId().getType() == RYZOMID::npc)
		{
			intensity = 249;
			CCreature *npcEntity = dynamic_cast<CCreature*> (caster);
			if (npcEntity)
			{
				const CStaticCreatures* form = npcEntity->getForm();
				if (form)
					intensity = form->getAttackLevel();
			}
		}
		if( intensity >= 250)
			intensity = 249;
		
		behav.Spell.SpellIntensity = 1 + (intensity/50);
		
		// set spell Id
		behav.Spell.SpellId = _MagicFxType;
		
		// set spell mode
		behav.Spell2.SelfSpell = (_TargetRestriction == TARGET::SelfOnly);
		
		if ( behav.Behaviour != MBEHAV::UNKNOWN_BEHAVIOUR )
			PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
		else
			nlwarning("<CMagicPhrase execute> Invalid behaviour");
	}

	// _BeingProcessed = false;
}// CMagicPhrase execute

// BRIANCODE - among other things, cleaned up the code to make it more readable,
// and took care of some weirdness, like making it easy to kill yourself with a cast,
// and properly handling it if you do.
bool CMagicPhrase::spendResources(CEntityBase* entity)
{
	// Entity is not a player? => creature/npc don't have sap/hp/sta for spell!
	if (entity->getId().getType() != RYZOMID::player)
		return true;

	// Dvine intervention? => Free!
	if(_DivineInterventionOccured)
		return true;

	// Spend Resources. Spend all reseources, or spend none.
	nlassert(SCORES::hit_points < entity->getScores()._PhysicalScores.size());
	nlassert(SCORES::sap < entity->getScores()._PhysicalScores.size());

	SCharacteristicsAndScores &hp = entity->getScores()._PhysicalScores[SCORES::hit_points];
	SCharacteristicsAndScores &sap = entity->getScores()._PhysicalScores[SCORES::sap];

	// do not allow player to die
	if ((sint32)_HPCost >= sint32(hp.Current))
		return false;
	if ((sint32)_SapCost > sint32(sap.Current))
		return false;

	
	if ( _HPCost != 0 )
	{
		// changeCurrentHp returns "true" if the change kills the entity. why yes, this IS stupid in the extreme.
		// because of the line above, this should never be relevant, but wtf. it's only two lines of code.
		if ( entity->changeCurrentHp( (_HPCost) * (-1) ) )
		{
			PHRASE_UTILITIES::sendDeathMessages( entity->getEntityRowId(), entity->getEntityRowId() );
			return false;
		}
	}
	
	if ( _SapCost != 0 )
	{
		entity->changeScore(SCORES::sap, -_SapCost);
	}

	return true;
}



//-----------------------------------------------
// CMagicPhrase launch
//-----------------------------------------------
bool CMagicPhrase::launch()
{
	H_AUTO(CMagicPhrase_launch);
	
	bool autoSuccess = EntitiesNoActionFailure || _IsProc;

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	if (entity == NULL)
	{
		nlwarning("<CCombatPhrase::launch> Invalid entity Id %s", TheDataset.getEntityId(_ActorRowId).toString().c_str() );		
		return false;
	}

	// if actor use a magician staff with a low required, warn him
	if (_UsedItemStats.isMagicFocus() && _UsedItemStats.requiredLevel() > 0 && _UsedItemStats.requiredLevel() < _BrickMaxSabrinaCost)
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "MAGIC_MAGICIAN_STAFF_LOW_REQ");
	}
	
	// spend sap, hp
	// returns false if the spell would either kill the caster, or any cost is more than the caster has available.
	if (spendResources(entity) == false)
		return false;

	// proc item special effect
	{
		CEntityBase * actingEntity = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
		std::vector<SItemSpecialEffect> effects, effects2;
		if ( actingEntity->getId().getType() == RYZOMID::player )
		{
			CCharacter* c = dynamic_cast<CCharacter*>(actingEntity);
			effects = c->lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::ISE_MAGIC_SHOOT_AGAIN);
		}
		if ( actingEntity->getId().getType() == RYZOMID::creature )
		{
			CGameItemPtr usedItem;
			CCreature* c = dynamic_cast<CCreature*>(actingEntity);
			usedItem = c->getRightHandItem();
			if (usedItem!=NULL && usedItem->getStaticForm() && usedItem->getStaticForm()->ItemSpecialEffects && !usedItem->getStaticForm()->ItemSpecialEffects->Effects.empty())
			{
				effects2 = usedItem->getStaticForm()->lookForEffects(ITEM_SPECIAL_EFFECT::ISE_MAGIC_SHOOT_AGAIN);
				effects.insert(effects.end(), effects2.begin(), effects2.end());
			}
			usedItem = c->getLeftHandItem();
			if (usedItem!=NULL && usedItem->getStaticForm() && usedItem->getStaticForm()->ItemSpecialEffects && !usedItem->getStaticForm()->ItemSpecialEffects->Effects.empty())
			{
				effects2 = usedItem->getStaticForm()->lookForEffects(ITEM_SPECIAL_EFFECT::ISE_MAGIC_SHOOT_AGAIN);
				effects.insert(effects.end(), effects2.begin(), effects2.end());
			}
		}
		std::vector<SItemSpecialEffect>::const_iterator it, itEnd;
		for (it=effects.begin(), itEnd=effects.end(); it!=itEnd; ++it)
		{
			float rnd = RandomGenerator.frand();
			if (rnd<it->EffectArgFloat[0])
			{
				TGameCycle const endDate = CTickEventHandler::getGameCycle() + (uint32)(it->EffectArgFloat[1]*10.f);
				CShootAgainEffect* effect = new CShootAgainEffect(_ActorRowId, _ActorRowId, EFFECT_FAMILIES::ProcShootAgain, /*_ParamValue*/0, /*power*/0, endDate);
				if (effect)
				{
					actingEntity->addSabrinaEffect(effect);
					PHRASE_UTILITIES::sendItemSpecialEffectProcMessage(ITEM_SPECIAL_EFFECT::ISE_MAGIC_SHOOT_AGAIN, actingEntity);
				}
			}
		}
	}

	// check for magic madness effect
	bool isMad = false;
	TDataSetRow madnessCaster;
	CSEffectPtr madness;
	madness = entity->lookForActiveEffect(EFFECT_FAMILIES::MadnessMagic);
	if (madness == NULL)
		madness = entity->lookForActiveEffect(EFFECT_FAMILIES::Madness);
	if ( madness )
	{
		const uint8 roll = (uint8) RandomGenerator.rand(99);
		if ( roll < madness->getParamValue() )
		{
			isMad = true;
			madnessCaster = madness->getCreatorRowId();
		}
	}


	if (isMad)
	{
		_Targets.resize(1);
		nlassert(!_Targets.empty());
		_Targets[0].setId(_ActorRowId);
	}
	// check for redirect attack effects
	else if (!_EnchantPhrase &&  _Nature == ACTNATURE::OFFENSIVE_MAGIC)
	{
		if( _Targets.empty() )
		{
			nlwarning("BUG: Tick %d Actor %s <CMagicPhrase::launch> Target normaly not empty here !", CTickEventHandler::getGameCycle(), entity->getId().toString().c_str() );
			return false;
		}

		CEntityBase *mainTarget = CEntityBaseManager::getEntityBasePtr(_Targets[0].getId());
		if (!mainTarget)
			return false;
		
		const CSEffectPtr effect = mainTarget->lookForActiveEffect(EFFECT_FAMILIES::RedirectAttacks);
		if ( effect )
		{
			CRedirectAttacksEffect *rEffect = dynamic_cast<CRedirectAttacksEffect *> (&(*effect));
			if (!rEffect)
			{
				nlwarning("Found an effect with type RedirectAttacks but dynamic_cast in CRedirectAttacksEffect * returns NULL ?!");
			}
			else
			{
				CEntityBase *newTarget = rEffect->getTargetForRedirection();
				if (newTarget)
				{
					_Targets.resize(1);
					nlassert(!_Targets.empty());
					_Targets[0].setId(newTarget->getEntityRowId());
				}
			}
		}
	}
	

	TDataSetRow mainTarget = _ActorRowId;
	if ( !_Targets.empty() && _Targets[0].getId() != _ActorRowId)
	{
		mainTarget = _Targets[0].getId();
	}

	float successFactor = 1.0f;
	TReportAction report;
	sint deltaLvl = 0;
	sint skillValue = 0;
	sint skillBaseValue = 0;

	if (autoSuccess)
	{
		report.ActorRowId = entity->getEntityRowId();
		report.ActionNature = _Nature;
		report.Skill = SKILLS::unknown;					// no xp gain but damage must be registered
		report.SkillLevel = getBrickMaxSabrinaCost();	// use the real level of the enchantment
		report.factor = 1.0f;
	}
	else
	{
		// test forced failure
		if (PHRASE_UTILITIES::forceActionFailure(entity) )
		{
			successFactor = 0.0;
		}
		else
		{
			if ( entity->getId().getType() == RYZOMID::player )
			{
				CCharacter *pC = dynamic_cast<CCharacter*> (entity);
				if (!pC)
				{
					nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", entity->getId().toString().c_str());
					return false;
				}
				
				// compute average skill value
				for ( uint i = 0; i < _Skills.size(); i++ )
				{
					skillValue += pC->getSkillValue(_Skills[i]);
					skillBaseValue += pC->getSkillBaseValue(_Skills[i]);
				}
				if (!_Skills.empty())
				{
					skillValue /= (sint)_Skills.size();
					skillBaseValue /= (sint)_Skills.size();
				}
			}
			else
			{
				const CStaticCreatures * form = entity->getForm();
				if ( !form )
				{
					nlwarning( "<MAGIC>invalid creature form %s in entity %s", entity->getType().toString().c_str(), entity->getId().toString().c_str() );
					return false;
				}	
				skillBaseValue = skillValue = form->getAttackLevel();
			}
			
			const CSEffect* debuff = entity->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMagic );
			if ( debuff)
				skillValue -= debuff->getParamValue();
			const CSEffect * outPostBuff = entity->lookForActiveEffect( EFFECT_FAMILIES::OutpostMagic );
			if ( outPostBuff )
				skillValue += outPostBuff->getParamValue();

			sint32 armorMalus = (sint32) ( entity->getArmorCastingMalus() * (float)skillValue );
			armorMalus -= _ArmorCompensation;
			if ( armorMalus <0 )
				armorMalus = 0;
			skillValue -= armorMalus;

			// get the success factor
			const sint16 relativeLevel = sint16(skillValue + sint32(_SabrinaCredit*_SabrinaRelativeCredit) - sint32(getSabrinaCost()) - sint32(_BrickMaxSabrinaCost));
			const uint8 roll = (uint8) RandomGenerator.rand(99);
			if (_Nature == ACTNATURE::OFFENSIVE_MAGIC)
			{
//				const uint8 chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::OffensiveMagicCast, relativeLevel);
				successFactor = CStaticSuccessTable::getSuccessFactor( SUCCESS_TABLE_TYPE::OffensiveMagicCast, relativeLevel, roll);
			}
			else
			{
				//const uint8 chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::CurativeMagicCast, relativeLevel );
				successFactor = CStaticSuccessTable::getSuccessFactor( SUCCESS_TABLE_TYPE::CurativeMagicCast, relativeLevel, roll);
			}
			if ( successFactor > 0.0f && entity->getId().getType() == RYZOMID::player && ! _EnchantPhrase)
			{
				CMissionEventCast event(_BrickSheets);
				((CCharacter*)entity)->processMissionEvent(event);
			}

			// delta level used for Xp Gain
			deltaLvl = skillBaseValue - (sint32)(_BrickMaxSabrinaCost);

			if (!_Skills.empty())
			{
				/// compute XP gain
				///\todo nico multi skill progress
				///\todo nico multi target XP
				if (isMad)
				{
					report.ActorRowId = madnessCaster;
					report.ActionNature = _Nature;
					report.Skill = _Skills[ 0 ];
					report.factor = 0.0f;
				}
				else
				{
					report.ActorRowId = entity->getEntityRowId();
					report.ActionNature = _Nature;
					report.Skill = _Skills[ 0 ];
					report.factor = successFactor;
				}
			}
			else
			{
				nlwarning("MAGIC: executing a phrase with no valid skill !");
			}
		}
	}

	// if the user enchants a phrase, process it and abort execution
	if ( _EnchantPhrase )
	{
		enchantPhrase((CCharacter*)entity,successFactor);
		// _BeingProcessed = false;
		return false;
	}

/*				
	if ( entity->getId().getType() == RYZOMID::player && successFactor > 0.0f )
	{
		float xpFactor = 1.0f / float( _Skills.size() );
		for (uint i = 0; i < _Skills.size(); i++ )
		{
			///\todo nico multi skill progress
			///\todo nico multi target XP
		}
	}
*/
	if ( _Area )
	{
		if ( DumpRangeAnalysis)
			AreaDebug.init(mainTarget);

		CEntityRangeSelector areaSelector;
		areaSelector.buildTargetList(_ActorRowId,mainTarget,_Area, _Nature );

		// validate targets, only keep valid ones, and up to _MaxTargets
		uint8 nbValidatedTargets = 0;
		string errorCode;
		_ApplyParams.DistanceToTarget.clear();
		_ApplyParams.TargetPowerFactor.clear();
		_Targets.clear();
		_Targets.reserve( areaSelector.getEntities().size() );
		uint8 nbTargets = _Area->Bomb.MaxTargets + _Area->Spray.MaxTargets + _Area->Chain.MaxTargets;
		if(_MaxTargets == 1 && nbTargets > 1 )
			_MaxTargets = nbTargets;
		for ( uint i = 0 ; i < areaSelector.getEntities().size() && nbValidatedTargets < _MaxTargets ; ++i )
		{
			if( _ActorRowId != areaSelector.getEntities()[i]->getEntityRowId() ) // caster not affected by his area spell
			{
				if( PHRASE_UTILITIES::validateSpellTarget(_ActorRowId,areaSelector.getEntities()[i]->getEntityRowId(),_Nature, errorCode, areaSelector.getEntities()[i]->getEntityRowId() == mainTarget) )
				{
					_ApplyParams.DistanceToTarget.push_back(areaSelector.getDistances()[i]);
					_ApplyParams.TargetPowerFactor.push_back(areaSelector.getFactor(i));

					_Targets.push_back( CSpellTarget(areaSelector.getEntities()[i]->getEntityRowId()) );
					++nbValidatedTargets;
				}
			}
		}

		// we no warn anymore in this case, perhaps the bug come from AIS who despawn creature too fast after there death
		// we never found any bug in area selection.
		// if no target found, there is a nasty bug in area selection, warn and exit
		if (nbValidatedTargets == 0)
		{
/*			CEntityBase *mainTargetPtr = CEntityBaseManager::getEntityBasePtr(_Targets[0].getId());
			if (!mainTargetPtr)
			{
				nlwarning("MAGIC: bug, Actor %s (position %d %d), area effect found no target (main target = %s), area =%s", 
					entity->getId().toString().c_str(), entity->getX(), entity->getY(),
					mainTarget.toString().c_str(),
					_Area->toString().c_str()
					);
			}
			else
			{
				nlwarning("MAGIC: bug, Actor %s (position %d %d) on target %s (position %d, %d), area effect found no target, area =%s", 
					entity->getId().toString().c_str(), entity->getX(), entity->getY(),
					mainTargetPtr->getId().toString().c_str(), mainTargetPtr->getX(), mainTargetPtr->getY(),
					_Area->toString().c_str()
					);
			}
*/
			return false;
		}
	}
	else
	{
		_ApplyParams.DistanceToTarget.clear();
		_ApplyParams.TargetPowerFactor.clear();

		_ApplyParams.DistanceToTarget.push_back( (float) PHRASE_UTILITIES::getDistance(_ActorRowId, mainTarget));
		_ApplyParams.TargetPowerFactor.push_back(1.0f);
	}

	MBEHAV::CBehaviour behav;
	if (_CreatureBehaviour == MBEHAV::UNKNOWN_BEHAVIOUR && (_DivineInterventionOccured||_ShootAgainOccured?_BaseCastingTime:_CastingTime) > 0)
	{
		if ( successFactor > 0.0f )
		{
			PHRASE_UTILITIES::sendSpellSuccessMessages(_ActorRowId, mainTarget);
			switch (_Nature)
			{
			case ACTNATURE::NEUTRAL:
				behav = MBEHAV::CAST_MIX_SUCCESS;
				// tmp nico : stats about projectiles
				projStatsIncrement();
				break;
			case ACTNATURE::CURATIVE_MAGIC:
				behav = MBEHAV::CAST_CUR_SUCCESS;
				// tmp nico : stats about projectiles
				projStatsIncrement();
				break;
			case ACTNATURE::OFFENSIVE_MAGIC:
				behav = MBEHAV::CAST_OFF_SUCCESS;
				// tmp nico : stats about projectiles
				projStatsIncrement();
				break;
			case ACTNATURE::RECHARGE:
				behav =  MBEHAV::CAST_MIX_SUCCESS;
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
			case ACTNATURE::CURATIVE_MAGIC:
				behav = MBEHAV::CAST_CUR_FAIL;
				break;
			case ACTNATURE::OFFENSIVE_MAGIC:
				behav = MBEHAV::CAST_OFF_FAIL;
				break;
			case ACTNATURE::RECHARGE:
				behav =  MBEHAV::CAST_MIX_FAIL;
				break;

			}
		}
	}
	else
	{
		behav = _CreatureBehaviour;
	}

	if (behav.isCreatureAttack())
	{
		// set time
		behav.CreatureAttack.Time = CTickEventHandler::getGameCycle();
		// set intensity
		uint intensity = getSabrinaCost();
		// Creature -> intensity depends of creature level
		if (entity->getId().getType() == RYZOMID::creature)
		{
			intensity = 249;
			CCreature *creature = dynamic_cast<CCreature*> (entity);
			if (creature)
			{
				const CStaticCreatures* form = creature->getForm();
				if (form)
					intensity = form->getAttackLevel();
			}
			else
				nlwarning("Entity %s type is creature but dynamic_cast in CCreature * returns NULL ?!", entity->getId().toString().c_str());
		}

		if( intensity >= 250)
			intensity = 249;
		behav.CreatureAttack.MagicImpactIntensity = 1 + (intensity/50);
	}
	else
	{
		// set fx type
		behav.Spell.SpellId = _MagicFxType;
		
		if ( _Area )
			behav.Spell.SpellMode = _Area->Type;
		else
			behav.Spell.SpellMode = MAGICFX::Chain;
		
		// set time
		behav.Spell.Time = CTickEventHandler::getGameCycle();
		
		// set intensity
		uint intensity = getSabrinaCost();
		// NPC -> intensity depends of npc level
		if (entity->getId().getType() == RYZOMID::npc)
		{
			intensity = 249;
			CCreature *npcEntity = dynamic_cast<CCreature*> (entity);
			if (npcEntity)
			{
				const CStaticCreatures* form = npcEntity->getForm();
				if (form)
					intensity = form->getAttackLevel();
			}
		}
		if( intensity >= 250)
			intensity = 249;
		
		behav.Spell.SpellIntensity = 1 + (intensity/50);
	}

	/// test Invulnerabilty of each target !
	const uint nbTargets = (uint)_Targets.size();

	NLMISC::CBitSet invulnerabilityOffensive(nbTargets);
	NLMISC::CBitSet invulnerabilityAll(nbTargets);
	
	testTargetsInvulnerabilities(invulnerabilityOffensive, invulnerabilityAll);

	// apply each effect of the spell
	NLMISC::CBitSet resists(nbTargets);
	resists.setAll();
	CBitSet affectedTargets(nbTargets);
	
	for ( uint i = 0; i < _Actions.size(); i++ )
	{
		_Actions[i]->launch(this,deltaLvl,skillValue, successFactor,behav,_ApplyParams.TargetPowerFactor,affectedTargets, invulnerabilityOffensive,invulnerabilityAll,isMad,resists,report);
	}

	// build affected target list (must be done before behaviour)
	// update caster visual property with target list.
	CMirrorPropValueList<uint32>	targetList(TheDataset, _ActorRowId, DSPropertyTARGET_LIST);
	targetList.clear();
	

	if ( _Area )
	{
		const sint size = (sint)_Targets.size();		
		nlassertex( size == (sint32)affectedTargets.size(), ("%d %d", size, affectedTargets.size() ) );
		nlassertex( size == (sint32)invulnerabilityAll.size(), ("%d %d", size, invulnerabilityAll.size() ) );
		nlassertex( size == (sint32)invulnerabilityOffensive.size(), ("%d %d", size, invulnerabilityOffensive.size() ) );
		for (sint i = size-1 ; i >= 0 ; --i)
		{
			if ( affectedTargets[i] )
			{
				if(i < (sint)resists.size())
					PHRASE_UTILITIES::updateMirrorTargetList( targetList, _Targets[i].getId(), _ApplyParams.DistanceToTarget[i],resists[i]);
			}
		}
		//targetList.testList( size*2 ); // wrong now because of the if
	}
	else if (!_Targets.empty())
	{
		const sint size = (sint)_Targets.size();		
		nlassertex( size == (sint32)invulnerabilityAll.size(), ("%d %d", size, invulnerabilityAll.size() ) );
		nlassertex( size == (sint32)invulnerabilityOffensive.size(), ("%d %d", size, invulnerabilityOffensive.size() ) );

		if(resists.size() > 0)
			PHRASE_UTILITIES::updateMirrorTargetList( targetList, _Targets[0].getId(), (float)PHRASE_UTILITIES::getDistance( _Targets[0].getId(),_ActorRowId),resists[0]);
		//targetList.testList( 1*2 ); // wrong now because of the if
	}
	
	// update caster behaviour (must be done after target list)
	if ( behav.Behaviour != MBEHAV::UNKNOWN_BEHAVIOUR )
		PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
	
	// add post cast latency, only for non instant cast
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	if (_DivineInterventionOccured||_ShootAgainOccured?_BaseCastingTime:_CastingTime)
		_LatencyEndDate = time + PostCastLatency + _PostCastTime;
	else
		_LatencyEndDate = 0 + _PostCastTime;

	// compute the apply date
	if ( !_Targets.empty()  && _ActorRowId != _Targets[0].getId())
	{
		const double distance = PHRASE_UTILITIES::getDistance(_ActorRowId, _Targets[0].getId()); // in meters
		const double launchTime = (distance / MAGICFX::PROJECTILE_SPEED) / CTickEventHandler::getGameTimeStep();
		_ApplyDate = time + NLMISC::TGameCycle( launchTime );

		// apply immediately if the launch time is too big (> 100 seconds)
		if (_ApplyDate - time > 1000)
		{
			CEntityBase * target = CEntityBaseManager::getEntityBasePtr(_Targets[0].getId());
			if (target)
			{
				nlwarning("<CMagicPhrase::launch> launch time is too big (%u seconds), maybe due to a teleport. Actor: %s, target: %s",
					_ApplyDate - time,
					entity->getId().toString().c_str(),
					target->getId().toString().c_str()
					);
			}
			_ApplyDate = 0;
		}
	}
	else
	{
		// apply immediately if the main target is the actor or no defined target
		_ApplyDate = 0;	
	}

	// Display stat
	CCharacter * c = dynamic_cast<CCharacter*>(CEntityBaseManager::getEntityBasePtr(_ActorRowId));
	if( c )
	{
		CSheetId hl, hr;
		uint32 qualityl, qualityr;
		
		CGameItemPtr item = c->getItem( INVENTORIES::handling, INVENTORIES::left );
		if( item == 0 )
		{
			qualityl = 0;
		}
		else
		{
			hl = item->getSheetId();
			qualityl = item->quality();
		}
		item = c->getItem( INVENTORIES::handling, INVENTORIES::right );
		if( item == 0 )
		{
			qualityr = 0;
		}
		else
		{
			hr = item->getSheetId();
			qualityr = item->quality();
		}
		//Bsi.append( StatPath, NLMISC::toString("[EAM] %s %s %d %s %d %1.2f", c->getId().toString().c_str(), hl.toString().c_str(), qualityl, hr.toString().c_str(), qualityr, successFactor) );
		//EgsStat.displayNL("[EAM] %s %s %d %s %d %1.2f", c->getId().toString().c_str(), hl.toString().c_str(), qualityl, hr.toString().c_str(), qualityr, successFactor);
//		EGSPD::executeActionMagic(c->getId(), hl.toString(), qualityl, hr.toString(), qualityr, successFactor);
	}

	// save params needed by apply()
	_ApplyParams.AffectedTargets.resize(affectedTargets.size());
	_ApplyParams.InvulnerabilityAll.resize(invulnerabilityAll.size());
	_ApplyParams.InvulnerabilityOffensive.resize(invulnerabilityOffensive.size());
	_ApplyParams.Resists.resize(resists.size());

	_ApplyParams.DeltaLevel = deltaLvl;
	_ApplyParams.SkillLevel = skillValue;
	_ApplyParams.SuccessFactor = successFactor;
	_ApplyParams.Behav = behav;
	//DistanceToTarget & TargetPowerFactor already filled
	_ApplyParams.AffectedTargets = affectedTargets;
	_ApplyParams.InvulnerabilityOffensive = invulnerabilityOffensive;
	_ApplyParams.InvulnerabilityAll = invulnerabilityAll;
	_ApplyParams.IsMad = isMad;
	_ApplyParams.Resists = resists;
	_ApplyParams.ActionReport = report;

#if !FINAL_VERSION
	nlassert(_Targets.size() == _ApplyParams.DistanceToTarget.size());
	nlassert(_Targets.size() == _ApplyParams.TargetPowerFactor.size());
	nlassert(_Targets.size() == _ApplyParams.AffectedTargets.size());
	nlassert(_Targets.size() == _ApplyParams.InvulnerabilityAll.size());
	nlassert(_Targets.size() == _ApplyParams.InvulnerabilityOffensive.size());
	nlassert(_Targets.size() == _ApplyParams.Resists.size());
#endif

	return true;
}// CMagicPhrase launch

//-----------------------------------------------
// CMagicPhrase apply
//-----------------------------------------------
void CMagicPhrase::apply()
{
	H_AUTO(CMagicPhrase_apply);
	
	bool autoSuccess = EntitiesNoActionFailure || _IsProc;

	// get back the params saved in launch()
	sint & deltaLvl = _ApplyParams.DeltaLevel;
	sint & skillValue = _ApplyParams.SkillLevel;
	float & successFactor = _ApplyParams.SuccessFactor;
	MBEHAV::CBehaviour & behav = _ApplyParams.Behav;
	//CEntityRangeSelector & areaSelector = _ApplyParams.Ranges;
	std::vector<float> &distancetoTarget = _ApplyParams.DistanceToTarget;
	std::vector<float> &targetPowerFactor = _ApplyParams.TargetPowerFactor;
	NLMISC::CBitSet & affectedTargets = _ApplyParams.AffectedTargets;
	NLMISC::CBitSet & invulnerabilityOffensive = _ApplyParams.InvulnerabilityOffensive;
	NLMISC::CBitSet & invulnerabilityAll = _ApplyParams.InvulnerabilityAll;
	bool & isMad = _ApplyParams.IsMad;
	NLMISC::CBitSet & resists = _ApplyParams.Resists;
	TReportAction & report = _ApplyParams.ActionReport;

	// do not gain Xp if auto success = true (enchant phrase)
	const bool gainXp = (!_EnchantPhrase && !_IsProc);

	{
		H_AUTO(CMagicPhrase_apply_applyActions);
		for ( uint i = 0; i < _Actions.size(); i++ )
		{
			if( _Actions[i] )
				_Actions[i]->apply(this,deltaLvl,skillValue, successFactor,behav,targetPowerFactor,affectedTargets, invulnerabilityOffensive,invulnerabilityAll,isMad,resists,report,_Vampirise,_VampiriseRatio, gainXp);
		}
	}
	
	// wear armor, shield, jewels..
	CCharacter *character = PlayerManager.getChar(_ActorRowId);
	if (character)
	{
		H_AUTO(CMagicPhrase_apply_wearEquipment);
		// wear focus item is it has been used
		if ( _UsedItemStats.wearItem() )
		{
			character->wearRightHandItem();
		}

		character->wearArmor();
		character->wearShield();
		character->wearJewels();
	}

	if ( _Area )
	{
		const sint size = (sint)_Targets.size();
		for (sint i = size-1 ; i >= 0 ; --i)
		{
			if ( affectedTargets[i] )
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
				if (invulnerabilityAll[i])
				{
					params[0].setEIdAIAlias( CEntityBaseManager::getEntityId(_Targets[i].getId()), CAIAliasTranslator::getInstance()->getAIAlias(CEntityBaseManager::getEntityId(_Targets[i].getId())) );
					PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "MAGIC_TARGET_INVULNERABLE_ALL_MAGIC", params);
				}
				else if (invulnerabilityOffensive[i])
				{
					params[0].setEIdAIAlias( CEntityBaseManager::getEntityId(_Targets[i].getId()), CAIAliasTranslator::getInstance()->getAIAlias(CEntityBaseManager::getEntityId(_Targets[i].getId())) );
					PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "MAGIC_TARGET_INVULNERABLE_OFFENSIVE", params);
				}
			}
		}
	}
	else if (!_Targets.empty())
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		if (invulnerabilityAll[0])
		{
			params[0].setEIdAIAlias( CEntityBaseManager::getEntityId(_Targets[0].getId()), CAIAliasTranslator::getInstance()->getAIAlias(CEntityBaseManager::getEntityId(_Targets[0].getId())) );

			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "MAGIC_TARGET_INVULNERABLE_ALL_MAGIC", params);
		}
		else if (invulnerabilityOffensive[0])
		{
			params[0].setEIdAIAlias( CEntityBaseManager::getEntityId(_Targets[0].getId()), CAIAliasTranslator::getInstance()->getAIAlias(CEntityBaseManager::getEntityId(_Targets[0].getId())) );
			
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "MAGIC_TARGET_INVULNERABLE_OFFENSIVE", params);
		}
	}

	// clear temporary execution data
	_Targets.resize(1);

	// _BeingProcessed = false;
}//CMagicPhrase apply

//-----------------------------------------------
// CMagicPhrase stop
//-----------------------------------------------
void CMagicPhrase::stop()
{
	H_AUTO(CMagicPhrase_stop);
	
	// _BeingProcessed = true;

	if ( state() == CSPhrase::ExecutionInProgress )
	{
		PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "EGS_ACTOR_CASTING_INTERUPT");
		PlayerManager.sendImpulseToClient(TheDataset.getEntityId(_ActorRowId), std::string("COMBAT:FLYING_TEXT"), _ActorRowId.getCompressedIndex(), (uint8)COMBAT_FLYING_TEXT::SelfInterrupt);
		
		// send behaviour
		MBEHAV::CBehaviour behav;
		switch (_Nature)
		{
		case ACTNATURE::NEUTRAL:
			behav = MBEHAV::CAST_MIX_FAIL;
			break;
		case ACTNATURE::CURATIVE_MAGIC:
			behav = MBEHAV::CAST_CUR_FAIL;
			break;
		case ACTNATURE::OFFENSIVE_MAGIC:
			behav = MBEHAV::CAST_OFF_FAIL;
			break;
		case ACTNATURE::RECHARGE:
			behav = MBEHAV::CAST_MIX_FAIL;
			break;

		}
		// set behaviour
		PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
	}

	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_ActorRowId);
	if (entity)
	{
		entity->clearCurrentAction();
		entity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );

		if ( _IsStatic && entity->getId().getType() == RYZOMID::player)
		{
			CCharacter *player = dynamic_cast<CCharacter*> (entity);
			if (player)
				player->staticActionInProgress(false);
		}
	}

	// _BeingProcessed = false;
} // stop //

//-----------------------------------------------
// CMagicPhrase end
//-----------------------------------------------
void CMagicPhrase::end()
{
	H_AUTO(CMagicPhrase_end);
	
	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_ActorRowId);
	if (entity)
	{
		entity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
		entity->clearCurrentAction();

		if (_IsStatic && entity->getId().getType() == RYZOMID::player)
		{
			CCharacter *player = dynamic_cast<CCharacter*> (entity);
			if (player)
				player->staticActionInProgress(false);
		}
	}
} // end //

//-----------------------------------------------
// setPrimaryTarget :
// Change the primary target (if not a self only spell).
//-----------------------------------------------
void CMagicPhrase::setPrimaryTarget( const TDataSetRow &entityRowId )	// virtual
{
	H_AUTO(CMagicPhrase_setPrimaryTarget);
	
	// Change the primary target (if not a self only spell).
	if(_TargetRestriction != TARGET::SelfOnly)
	{
		if (_Targets.empty())
			_Targets.resize(1);

		_Targets[0].setId(entityRowId);
	}
}// setPrimaryTarget //


//-----------------------------------------------
// enchantPhrase :
// process an enchanting effect
//-----------------------------------------------
void CMagicPhrase::enchantPhrase(CCharacter * user,float successFactor)
{
	H_AUTO(CMagicPhrase_enchantPhrase);
	
	MBEHAV::CBehaviour behav;
	uint moneyCost = uint( getSabrinaCost() * CristalMoneyFactor );
	if ( ! moneyCost )
		moneyCost = 1;
	user->spendMoney( moneyCost );

	/// todo fumbles
	if ( successFactor >= 0.0f )
	{
		PHRASE_UTILITIES::sendSpellSuccessMessages(_ActorRowId, _ActorRowId);
		behav = MBEHAV::CAST_CUR_SUCCESS;
		user->createCrystallizedActionItem( _BrickSheets );
	}
	else
	{
		PHRASE_UTILITIES::sendSpellFailedMessages(_ActorRowId, _ActorRowId);
		behav = MBEHAV::CAST_CUR_FAIL;
	}

	// set fx type
	behav.Spell.SpellId = MAGICFX::Cur;
	behav.Spell.SpellMode = MAGICFX::Chain;

	// set time
	behav.Spell.Time = CTickEventHandler::getGameCycle();

	// set intensity
	uint intensity = getSabrinaCost();
	if( intensity >= 250)
		intensity = 249;
	behav.Spell.SpellIntensity = 1 + (intensity/50);

	// update caster behaviour
	if ( behav.Behaviour != MBEHAV::UNKNOWN_BEHAVIOUR )
		PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
	else
		nlwarning("<CMagicPhrase apply> Invalid behaviour");

	_Targets.resize(1);
//	user->setActionFlag( RYZOMACTIONFLAGS::Attacks, true );
	_LatencyEndDate = 0;//time + _HitRateModifier + weapon.LatencyInTicks + ammo.SpeedInTicks ;
	// _BeingProcessed = false;

} // enchantPhrase //

//-----------------------------------------------
// CMagicPhrase buildProcItem
//-----------------------------------------------
bool CMagicPhrase::buildProc( const TDataSetRow & actorRowId, const std::vector<NLMISC::CSheetId>& brickIds )
{
	H_AUTO(CMagicPhrase_buildProc);
	
	// get the brick forms
	const uint size = (uint)brickIds.size();
	vector< const CStaticBrick* > bricks(size);
	for( uint i = 0; i < size; i++ )
	{
		bricks[i] = CSheets::getSBrickForm( brickIds[i] );
		if( bricks[i] == NULL )
		{
			nlwarning("<CMagicPhrase procItem> Can't found form for brick '%s'",brickIds[i].toString().c_str());
			return false ;
		}
	}
	// build the phrase
	return build( actorRowId, bricks );
}// CMagicPhrase procItem

//-----------------------------------------------
// CMagicPhrase procItem
//-----------------------------------------------
bool CMagicPhrase::procItem()
{
	H_AUTO(CMagicPhrase_procItem);
	
	CCharacter * user = PlayerManager.getChar(_ActorRowId);
	if ( user )
	{
		_IsProc = true;

		setPrimaryTarget( user->getTargetDataSetRow() );
		if ( validate() )
		{
			execute();
			if (!launch())
				return false;
			
			apply();
			end();
			return true;
		}
	}
	return false;
}// CMagicPhrase procItem


//-----------------------------------------------
// CMagicPhrase testTargetsInvulnerabilities
//-----------------------------------------------
void CMagicPhrase::testTargetsInvulnerabilities( CBitSet &invulnerabilityOffensive, CBitSet &invulnerabilityAll)
{
	H_AUTO(CMagicPhrase_testTargetsInvulnerabilities);
	
	const uint nbTargets = (uint)_Targets.size();
	for (uint i = 0 ; i < nbTargets ; ++i)
	{
		CEntityBase *target = CEntityBaseManager::getEntityBasePtr(_Targets[i].getId());
		if (!target)
			continue;

		// anti magic shield prevents offensive magic
		CSEffect *effect = NULL;
		// invulnerability power prevents ALL magic (offensive AND curative)
		effect = target->lookForActiveEffect( EFFECT_FAMILIES::PowerInvulnerability );
		if (effect)
		{
			invulnerabilityAll.set(i);
		}
		else
		{
			effect = target->lookForActiveEffect( EFFECT_FAMILIES::PowerAntiMagicShield );
			if (effect)
			{
				invulnerabilityOffensive.set(i);
			}
		}
	}	
} // testTargetsInvulnerabilities //


//-----------------------------------------------
// CMagicPhrase initUsedMagicFocusStats
//-----------------------------------------------
void CMagicPhrase::initUsedMagicFocusStats()
{
	H_AUTO(CMagicPhrase_initUsedMagicFocusStats);
	
	//_UsedItemStats
	CCharacter *player = PlayerManager.getChar(_ActorRowId);
	if (!player)
		return;

	CGameItemPtr item = player->getRightHandItem();
	if (item != NULL)
	{
		_UsedItemStats.init(item);
	}
} // initUsedMagicFocusStats //

