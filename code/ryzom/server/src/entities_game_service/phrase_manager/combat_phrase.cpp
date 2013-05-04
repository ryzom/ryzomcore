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
#include "combat_phrase.h"
// net
#include "nel/net/message.h"
// misc
#include "nel/misc/common.h"
// game share
#include "game_share/mode_and_behaviour.h"
#include "server_share/combat_state.h"
#include "game_share/hit_type.h"
#include "game_share/time_weather_season/time_date_season_manager.h"
#include "game_share/magic_fx.h"
#include "game_share/intensity_types.h"
//
#include "entity_structure/statistic.h"
#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_ai_action.h"
#include "weapon_damage_table.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "entity_manager/entity_base.h"
#include "entities_game_service.h"
#include "player_manager/character.h"
#include "creature_manager/creature.h"
#include "egs_mirror.h"
#include "phrase_manager/phrase_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "s_phrase_factory.h"
#include "phrase_manager/sabrina_area_debug.h"
#include "progression/progression_pve.h"
#include "progression/progression_pvp.h"

#include "combat_action_stun.h"
#include "combat_action_bleed.h"
#include "combat_action_simple_effect.h"
#include "combat_action_simple_dynamic_effect.h"
#include "combat_action_slow_move.h"
#include "combat_action_regen_modifier.h"
#include "combat_action_special_damage.h"

#include "bounce_effect.h"
#include "power_shielding_effect.h"
#include "redirect_attacks_effect.h"
#include "enchant_weapon_effect.h"

#include "projectile_stats.h"

#include "magic_action_attack.h"

#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"

//////////////
//	USING	//
//////////////
using namespace std;
using namespace NLNET;
using namespace NLMISC;


//////////////
//	EXTERN	//
//////////////
extern CRandom				RandomGenerator;
extern CPlayerManager		PlayerManager;

// skill used when no weapon in hand (hand to hand combat)
extern SKILLS::ESkills		BarehandCombatSkill;
extern CVariable<uint16>	HandToHandReachValue;
extern CVariable<uint16>	MinTwoWeaponsLatency;

#if !FINAL_VERSION
CVariable<float> CombatLatencyFactor("egs","CombatLatencyFactor", "Factor applied on the duration of latency for combat actions", 1.f, 0, true);
CVariable<bool> CombatForceCritical("egs","CombatForceCritical", "Force critical hit", false, 0, true);
CVariable<bool> CombatForceMiss("egs","CombatForceMiss", "Force Miss", false, 0, true);
CVariable<bool> CombatForceDodgeParry("egs","CombatForceDodgeParry", "Force Dodge Parry", false, 0, true);
#endif // !FINAL_VERSION


DEFAULT_SPHRASE_FACTORY( CCombatPhrase, BRICK_TYPE::COMBAT );


/// get the sabrina value of a weapon, given the required skill and the acting entity skill
uint16 getWeaponSabrinaValue(const CCombatWeapon &weapon, sint32 skill)
{
	return min((uint16)skill,weapon.Quality);
}

/// get the ActionDuration value (for behaviour) for given action length in ticks
uint8 getActionDuration( TGameCycle length )
{
	// length in seconds
	const float s = (float)(length * CTickEventHandler::getGameTimeStep());
	if (s <= 1.5f)
		return 0; // 1s
	if (s <= 2.0f)
		return 1; // 1.5s
	if (s <= 2.5f)
		return 2; // 2s
	// 
	return 3; // 2.5s
}

/// get impact intensity
INTENSITY_TYPE::TImpactIntensity getImpactIntensity(uint16 refLevel)
{
	uint intensity = 1 + (refLevel * INTENSITY_TYPE::NB_IMPACT) / 250;
	if ( intensity >= INTENSITY_TYPE::NB_IMPACT)
		intensity = INTENSITY_TYPE::NB_IMPACT - 1;
	return (INTENSITY_TYPE::TImpactIntensity) intensity;
}


//--------------------------------------------------------------
//			CDamageFactor::entityMatchRequirements  
//--------------------------------------------------------------
bool CDamageFactor::entityMatchRequirements(CEntityBase *entity)
{
	H_AUTO(CDamageFactor_entityMatchRequirements);
	
	if (!entity) return false;
	if (Race != EGSPD::CPeople::EndPeople && Race != entity->getRace())
		return false;
	if (Classification != EGSPD::CClassificationType::EndClassificationType && EGSPD::testClassificationType(entity->getRace(), Classification) == false)
		return false;
	if (Ecosystem != ECOSYSTEM::unknown )
	{
		if (entity->getId().getType() != RYZOMID::player)
		{
			const CStaticCreatures * form = entity->getForm();
			if (!form || form->getEcosystem() != Ecosystem) 
				return false;
		}
	}
	if (Season != EGSPD::CSeason::EndSeason && EGSPD::CSeason::Invalid && CTimeDateSeasonManager::getRyzomTimeReference().getRyzomSeason() != Season)
		return false;

	return true;
} // CDamageFactor::entityMatchRequirements //


//--------------------------------------------------------------
//					build  
//--------------------------------------------------------------
bool CCombatPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute )
{
	H_AUTO(CCombatPhrase_build);
	
	if (TheDataset.isAccessible(actorRowId) )
	{
		// create attacker structure
		CEntityBase *attacker = CEntityBaseManager::getEntityBasePtr(actorRowId);
		if (attacker == NULL)
			return false;

		// check _attacker is null
		nlassert(_Attacker == NULL);

		if (attacker->getId().getType() == RYZOMID::player )
		{
			_Attacker = new CCombatAttackerPlayer(actorRowId);
		}
		else if (attacker->getId().getType() == RYZOMID::npc)
		{
			_Attacker = new CCombatAttackerNpc(actorRowId);
		}
		else //creature
		{
			_Attacker = new CCombatAttackerAI(actorRowId);
		}
	}

	if (!_Attacker)
		return false;

	// create defender structure -> wait first validation

	// add bricks
	for (uint i = 0; i < bricks.size(); i++)
	{
		addBrick( *bricks[i] );
	}

	return true;
} // build //

//--------------------------------------------------------------
//					initPhraseFromAiAction
//--------------------------------------------------------------
bool CCombatPhrase::initPhraseFromAiAction( const TDataSetRow & actorRowId, const CStaticAiAction *aiAction, float damageCoeff, float speedCoeff )
{
	H_AUTO(CCombatPhrase_initPhraseFromAiAction);
	
	if ( !TheDataset.isAccessible(actorRowId) )
	{
		return false;
	}

	if (!aiAction)
		return false;

	if ( aiAction->getType() != AI_ACTION::Melee && aiAction->getType() != AI_ACTION::Range )
		return false;

	// read parameters
	const TAiActionParams &data = aiAction->getData();

	// create attacker structure
	CEntityBase *attacker = CEntityBaseManager::getEntityBasePtr(actorRowId);
	if (attacker == NULL)
		return false;

	AIACTIONLOG("AI actor %s execute action %s on target %s",attacker->getId().toString().c_str(), aiAction->getSheetId().toString().c_str(), attacker->getTarget().toString().c_str());

	if (attacker->getId().getType() == RYZOMID::player )
	{
		_Attacker = new CCombatAttackerPlayer(actorRowId);
	}
	else if (attacker->getId().getType() == RYZOMID::npc)
	{
		_Attacker = new CCombatAttackerNpc(actorRowId, aiAction);
	}
	else //creature
	{
		_Attacker = new CCombatAttackerAI(actorRowId, aiAction);
	}
	
	if (!_Attacker)
		return false;
		

	// simple modifiers and factor 
	_DamageFactor = (1.0f + data.Combat.DamageFactor) * damageCoeff;
	_DamageModifier = data.Combat.DamageModifier;
	if (data.Combat.ArmorFactor != 1.0f)
	{
		_ArmorAbsorptionFactor.CreatureType = EGSPD::CClassificationType::EndClassificationType;
		_ArmorAbsorptionFactor.ArmorType = ARMORTYPE::ALL;
		_ArmorAbsorptionFactor.MinValue = data.Combat.ArmorFactor;
		_ArmorAbsorptionFactor.MaxValue = data.Combat.ArmorFactor;
		_ArmorAbsorptionFactor.PowerValue = 1;
	}

	if ( data.Combat.SpeedFactor > -1.0f)
	{
		if ( speedCoeff > 0 )
			_LatencyFactor = 1.0f / ( (1.0f + data.Combat.SpeedFactor) * speedCoeff );
		else
			_LatencyFactor = 1.0f / (1.0f + data.Combat.SpeedFactor);
	}

	// force critical
	if ( fabs(data.Combat.Critic) < 1.0f)
	{
		_CriticalHitChancesModifier.MaxValue = (sint8) (100 * data.Combat.Critic - CriticalHitChances);
		_CriticalHitChancesModifier.MinValue = _CriticalHitChancesModifier.MaxValue;
	}
	else
	{
		_CriticalHitChancesModifier.MaxValue = 100 - CriticalHitChances;
		_CriticalHitChancesModifier.MinValue = 100 - CriticalHitChances;
	}
	

	// set behaviour if !=  unknown
	if (data.Combat.Behaviour != MBEHAV::UNKNOWN_BEHAVIOUR)
	{
		_Behaviour = data.Combat.Behaviour;

		// patch : if attacker is an npc, use default attack instead of creature attacks
	//	if (attacker->getId().getType() == RYZOMID::npc && _Behaviour.isCreatureAttack())
	//	{
	//		_Behaviour = MBEHAV::DEFAULT_ATTACK;
	//	}
		// :NOTE: I (vuarand) commented out this code coz it leads to a
		// behaviour bug with npc having a creature sheet. The commit note for
		// this block is not related to it, so there's no explanation for its
		// presence there (and fleury is no longer there).
	}

	// aiming
	_AiAimingType = data.Combat.AimingType;

	// if special effect
	if (data.Combat.EffectFamily != AI_ACTION::UnknownEffect)
	{
		CCombatAction *combatAiAction = CCombatAIActionFactory::buildAiAction(aiAction,this);
		if (combatAiAction)
		{
			_CombatActions.push_back(combatAiAction);
		}
	}

	// if add special damage
	if (data.Combat.SpecialDamageFactor > 0.0f && data.Combat.SpecialDamageType != DMGTYPE::UNDEFINED)
	{
		// create appropriated effect
		CCombatActionSpecialDamage *action = new CCombatActionSpecialDamage(actorRowId, this, data.Combat.SpecialDamageType);
		if (action)
		{
			action->setDynValues(data.Combat.SpecialDamageFactor,data.Combat.SpecialDamageFactor,1);
			_CombatActions.push_back(action);
		}
	}

	return true;
} // initPhraseFromAiAction //

//--------------------------------------------------------------
//					destructor  
//--------------------------------------------------------------
CCombatPhrase::~CCombatPhrase()
{
	H_AUTO(CCombatPhraseDestructor);
	
	clearTargets();
	
	const uint size = (uint)_CombatActions.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		if (_CombatActions[i] != NULL)
			delete _CombatActions[i];
	}

	if (_Attacker != NULL)
		delete _Attacker;
};

//--------------------------------------------------------------
//					init()  
//--------------------------------------------------------------
void CCombatPhrase::init()
{
	setState(CSPhrase::New);
	
	_Behaviour					= MBEHAV::DEFAULT_ATTACK;
	_BehaviourWeight				= 0;
	
	_RootSheetId					= CSheetId::Unknown;

	_PhraseType						= BRICK_TYPE::COMBAT;

	_Attacker						= NULL;

	_RootSkill						= SKILLS::unknown;
	_AttackSkill					= SKILLS::unknown;

	_AiAimingType					= AI_AIMING_TYPE::Random;

	_CyclicPhrase					= false;
	_Idle							= false;
	_TargetTooFarMsg				= false;
	_NotEnoughStaminaMsg			= false;
	_NotEnoughHpMsg					= false;
	_NoAmmoMsg						= false;
	_BadOrientationMsg				= false;
	_CurrentTargetIsValid			= false;
	_SpecialHit						= false;
	_MeleeCombat					= true;	
	_HitAllMeleeAggressors			= false;
	_CriticalHit					= false;
	
	_ExecutionEndDate				= 0;
	_LatencyEndDate					= 0;

	_SabrinaCost					= 0;
	_SabrinaRelativeCost			= 1.0f;
	_SabrinaCredit					= 0;
	_SabrinaRelativeCredit			= 1.0f;
	_BrickMaxSabrinaCost			= 0;

	_HPCost							= 0;
	_StaminaCost					= 0;
	_StaminaWeightFactorCost		= 0.0f;

	_ExecutionLengthModifier		= 0;
	_HitRateModifier				= 0;
	_DamageModifier					= 0;

	_TotalStaminaCost				= 0;
	_TotalHPCost					= 0;
	_TotalSabrinaCost				= 0;

	_LatencyFactor					= 1.0f;
	_PhraseSuccessDamageFactor		= 1.0f;
	
	_AggroMultiplier				= 1.0f;
	_AggroModifier					= 0;

	_DamageFactor					= 1.0f;
	_WeaponSabrinaValue				= 0;

	_DamagePointBlank				= 1.0f;
	_DamageShortRange				= 1.0f;
	_DamageMediumRange				= 1.0f;
	_DamageLongRange				= 1.0f;
	_MadSkill						= SKILLS::unknown;
	_IsMad							= false;
	_MadnessCaster					= TDataSetRow();

	_MissFlyingTextTriggered		= false;
//	clearTargets();

} // init //

//--------------------------------------------------------------
//					addBrick()  
//--------------------------------------------------------------
void CCombatPhrase::addBrick( const CStaticBrick &brick )
{
	H_AUTO(CCombatPhrase_addBrick);
	
	// check attacker is still valid
	if (!_Attacker || !_Attacker->isValid())
	{
		DEBUGLOG("COMBAT : (addBrick) Invalid attacker (null or not in mirror)");
		return;
	}

	if ( brick.ForcedLocalisation != SLOT_EQUIPMENT::UNDEFINED )
	{
		if ( _AimedSlot.Slot == SLOT_EQUIPMENT::UNDEFINED )
		{
			_AimedSlot.Slot = brick.ForcedLocalisation;
		}
		else if (brick.ForcedLocalisation != _AimedSlot.Slot)
		{
			nlwarning("<CCombatPhrase::addBrick> Phrase as a forced localisation to %s, but the new brick (name %s) has localisation %s",SLOT_EQUIPMENT::toString(_AimedSlot.Slot).c_str(), brick.Name.c_str(), SLOT_EQUIPMENT::toString(brick.ForcedLocalisation).c_str());
		}
	}

	if ( brick.SabrinaValue <= 0 )
	{
		_SabrinaCredit -= brick.SabrinaValue;
	}
	else
	{
		_SabrinaCost += brick.SabrinaValue;
		if (_BrickMaxSabrinaCost < brick.SabrinaValue)
			_BrickMaxSabrinaCost = brick.SabrinaValue;
	}

	if( brick.SabrinaRelativeValue <= 0 )
	{
		_SabrinaRelativeCredit -= brick.SabrinaRelativeValue;
	}
	else
	{
		_SabrinaRelativeCost += brick.SabrinaRelativeValue;
	}
	
	// process params
	unsigned i;
	for (i=0 ; i<brick.Params.size() ; ++i)
	{
		const TBrickParam::IId* param = brick.Params[i];
		switch(param->id())
		{
		case TBrickParam::HP:
//			INFOLOG("HP: %i",((CSBrickParamHp *)param)->Hp);
			_HPCost += ((CSBrickParamHp *)param)->Hp;
			break;

		case TBrickParam::STA:
//			INFOLOG("STA: %i",((CSBrickParamSta *)param)->Sta);
			_StaminaCost += ((CSBrickParamSta *)param)->Sta;
			break;

		case TBrickParam::STA_WEIGHT_FACTOR:
			_StaminaCost += ((CSBrickParamStaWeightFactor *)param)->StaConst;
			_StaminaWeightFactorCost += ((CSBrickParamStaWeightFactor *)param)->StaFactor;
			break;

		case TBrickParam::SET_BEHAVIOUR:
			// $*STRUCT CSBrickParamSetBehaviour: public TBrickParam::CId <TBrickParam::SET_BEHAVIOUR>
			// $*-s string Behaviour	// the new behaviour to use
			if ( (uint16)abs(brick.SabrinaValue) > _BehaviourWeight )
			{
				MBEHAV::EBehaviour behaviour = MBEHAV::stringToBehaviour(((CSBrickParamSetBehaviour*)param)->Behaviour);
				if (behaviour != MBEHAV::UNKNOWN_BEHAVIOUR)
				{
					_BehaviourWeight = (uint16)abs(brick.SabrinaValue);
					_Behaviour = behaviour;
				}
			}
			break;

		case TBrickParam::INC_DMG:
		// $*STRUCT CSBrickParamIncreaseDamage: public TBrickParam::CId <TBrickParam::INC_DMG>
		// $*-f float	MinFactor = 1.0f //min factor on damage
		// $*-f float	MaxFactor = 2.0f //max factor on damage
			_DamageFactorOnSuccess.PowerValue = brick.PowerValue;
			_DamageFactorOnSuccess.MinValue = ((CSBrickParamIncreaseDamage *)param)->MinFactor;
			_DamageFactorOnSuccess.MaxValue = ((CSBrickParamIncreaseDamage *)param)->MaxFactor;
			break;

		case TBrickParam::INC_DMG_TYPE_RSTR:
			{
			// $*STRUCT CSBrickParamIncDmgTypeRestriction: public TBrickParam::CId <TBrickParam::INC_DMG_TYPE_RSTR>
			// $*-s std::string	TypeRestriction //type restriction
			// $*-f float		FactorModifier = 0.0f //bonus on damage factor
			_DamageFactorOnSuccess.Classification = EGSPD::CClassificationType::fromString( ((CSBrickParamIncDmgTypeRestriction *)param)->TypeRestriction );
			/// TODO : Factor modif
			}
			break;

		case TBrickParam::INC_DMG_RACE_RSTR:
			{
			// $*STRUCT CSBrickParamIncDmgRaceRestriction: public TBrickParam::CId <TBrickParam::INC_DMG_RACE_RSTR>
			// $*-s std::string	RaceRestriction //race restriction
			// $*-f float		FactorModifier = 0.0f //bonus on damage factor
				_DamageFactorOnSuccess.Race = EGSPD::CPeople::fromString( ((CSBrickParamIncDmgRaceRestriction *)param)->RaceRestriction );
			/// TODO : Factor modif
			}
			break;

		case TBrickParam::INC_DMG_ECOS_RSTR:
			{
			// $*STRUCT CSBrickParamIncDmgEcosystemRestriction: public TBrickParam::CId <TBrickParam::INC_DMG_ECOS_RSTR>
			// $*-s std::string	EcosystemRestriction //Ecosystem restriction
			// $*-f float		FactorModifier = 0.0f //bonus on damage factor
			_DamageFactorOnSuccess.Ecosystem = ECOSYSTEM::stringToEcosystem( ((CSBrickParamIncDmgEcosystemRestriction *)param)->EcosystemRestriction );
			/// TODO : Factor modif
			}
			break;

		case TBrickParam::INC_DMG_SEASON_RSTR:
			{
			// $*STRUCT CSBrickParamIncDmgSeasonRestriction: public TBrickParam::CId <TBrickParam::INC_DMG_SEASON_RSTR>
			// $*-s std::string	SeasonRestriction //Season restriction
			// $*-f float		FactorModifier = 0.0f //bonus on damage factor
				_DamageFactorOnSuccess.Season = EGSPD::CSeason::fromString( ((CSBrickParamIncDmgSeasonRestriction *)param)->SeasonRestriction );
			/// TODO : Factor modif
			}
			break;

		case TBrickParam::SPECIAL_DAMAGE:
			{
				// $*STRUCT CSBrickParamSpecialDamage: public TBrickParam::CId <TBrickParam::SPECIAL_DAMAGE>
				// $*-s std::string	DamageType // damage type
				// $*-f float		MinFactor = 0.0f //min factor of damage
				// $*-f float		MaxFactor = 1.0f //max factor of damage
				DMGTYPE::EDamageType damageType = DMGTYPE::stringToDamageType(((CSBrickParamSpecialDamage *)param)->DamageType);
				if (damageType != DMGTYPE::UNDEFINED)
				{
					CCombatActionSpecialDamage *action = new CCombatActionSpecialDamage(_Attacker->getEntityRowId(), this, damageType);
					if (action)
					{
						action->setDynValues(((CSBrickParamSpecialDamage *)param)->MinFactor,((CSBrickParamSpecialDamage *)param)->MaxFactor,brick.PowerValue);
						_CombatActions.push_back(action);
					}
				}
			}
			break;

		case TBrickParam::LATENCY_FACTOR:
			// $*STRUCT CSBrickParamLatencyFactor: public TBrickParam::CId <TBrickParam::LATENCY_FACTOR>
			// $*-f float MinLatencyFactor = 1	// min factor on weapon latency
			// $*-f float MaxLatencyFactor = 0.5	// max factor on weapon latency
			_LatencyFactorDyn.PowerValue = brick.PowerValue;
			_LatencyFactorDyn.MinValue = ((CSBrickParamLatencyFactor *)param)->MinLatencyFactor;
			_LatencyFactorDyn.MaxValue = ((CSBrickParamLatencyFactor *)param)->MaxLatencyFactor;
			break;
			
		case TBrickParam::STA_LOSS_FACTOR:
			_StaminaLossDynFactor.PowerValue = brick.PowerValue;
			_StaminaLossDynFactor.MinValue = ((CSBrickParamStaLossFactor *)param)->MinFactor;
			_StaminaLossDynFactor.MaxValue = ((CSBrickParamStaLossFactor *)param)->MaxFactor;
			break;
					
		case TBrickParam::SAP_LOSS_FACTOR:
			_SapLossDynFactor.PowerValue = brick.PowerValue;
			_SapLossDynFactor.MinValue = ((CSBrickParamSapLossFactor *)param)->MinFactor;
			_SapLossDynFactor.MaxValue = ((CSBrickParamSapLossFactor *)param)->MaxFactor;
			break;

		case TBrickParam::ATT_SKILL_MOD:			
			_AttackSkillModifier.PowerValue = brick.PowerValue;
			_AttackSkillModifier.MinValue = ((CSBrickParamAttackSkillModifier *)param)->MinModifier;
			_AttackSkillModifier.MaxValue = ((CSBrickParamAttackSkillModifier *)param)->MaxModifier;
			break;

		case TBrickParam::DEFENSE_MOD:
			{
//			INFOLOG("DEFENSE_MOD: min %f, max %f",((CSBrickParamDefenseModifier *)param)->MinModifier, ((CSBrickParamDefenseModifier *)param)->MaxModifier);
			CCombatActionDynamicEffect *action = new CCombatActionDynamicEffect( _Attacker->getEntityRowId(), this, 0, EFFECT_FAMILIES::CombatDefenseModifier);
			if (action)
			{
				action->setDynValues( (float)((CSBrickParamDefenseModifier *)param)->MinModifier, (float)((CSBrickParamDefenseModifier *)param)->MaxModifier, brick.PowerValue);
				action->usePhraseLatencyAsDuration(true);
				action->applyOnTargets(false);
				_CombatActions.push_back(action);
			}
			else
				nlwarning("Failed to allocate new CCombatActionDynamicEffect");
			}
			break;


		case TBrickParam::COMBAT_SLOW_ATTACK:
			{
			// $*STRUCT CSBrickParamCombatSlowAttack: public TBrickParam::CId <TBrickParam::COMBAT_SLOW_ATTACK>
			// $*-f float Duration	// duration of the effect in seconds
			// $*-i sint32 MinFactor	// min factor applied on target attack latency (+50 = +50%)
			// $*-i sint32 MaxFactor	// max factor applied on target attack latency (+50 = +50%)
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( ((CSBrickParamCombatSlowAttack *)param)->Duration / CTickEventHandler::getGameTimeStep() );
			CCombatActionDynamicEffect *action = new CCombatActionDynamicEffect( _Attacker->getEntityRowId(), this, duration, EFFECT_FAMILIES::CombatAttackSlow);
			if (action)
			{
				action->setDynValues( (float)((CSBrickParamCombatSlowAttack *)param)->MinFactor, (float)((CSBrickParamCombatSlowAttack *)param)->MaxFactor, brick.PowerValue);
				_CombatActions.push_back(action);
			}
			else
				nlwarning("Failed to allocate new CCombatActionDynamicEffect");
			}
			break;

		case TBrickParam::COMBAT_SLOW:
			{
				// $*STRUCT CSBrickParamCombatSlow: public TBrickParam::CId <TBrickParam::COMBAT_SLOW>
				// $*-f float Duration	// duration of the effect in seconds
				// $*-i sint32 MinFactor	// min factor applied on target attack latency or casting time (+50 = +50%)
				// $*-i sint32 MaxFactor	// max factor applied on target attack latency or casting time(+50 = +50%)
				NLMISC::TGameCycle duration = NLMISC::TGameCycle( ((CSBrickParamCombatSlow *)param)->Duration / CTickEventHandler::getGameTimeStep() );
				CCombatActionDynamicEffect *action = new CCombatActionDynamicEffect( _Attacker->getEntityRowId(), this, duration, EFFECT_FAMILIES::CombatSlow);
				if (action)
				{
					action->setDynValues( (float)((CSBrickParamCombatSlow *)param)->MinFactor, (float)((CSBrickParamCombatSlow *)param)->MaxFactor, brick.PowerValue);
					_CombatActions.push_back(action);
				}
				else
					nlwarning("Failed to allocate new CCombatActionDynamicEffect");
			}
			break;

		case TBrickParam::SLOW_CAST:
			{
			// STRUCT CSBrickParamSlowCast: public TBrickParam::CId <TBrickParam::SLOW_CAST>
			// -f float Duration	// duration of the slow in seconds
			// -f float MinFactor = 1.0f // min factor applied on cast time
			// -f float MaxFactor = 2.0f // max factor applied on cast time
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( ((CSBrickParamSlowCast *)param)->Duration / CTickEventHandler::getGameTimeStep() );
			CCombatActionDynamicEffect *effect = new CCombatActionDynamicEffect( _Attacker->getEntityRowId(), this, duration, EFFECT_FAMILIES::CombatCastSlow);
			if (effect)
			{
				effect->setDynValues( ((CSBrickParamSlowCast *)param)->MinFactor, ((CSBrickParamSlowCast *)param)->MaxFactor, brick.PowerValue );
				_CombatActions.push_back( effect );
			}
			}
			break;

//		case TBrickParam::COMBAT_SLOW_MOVE:
			//			{
			
			// $*STRUCT CSBrickParamCombatSlowMove: public TBrickParam::CId <TBrickParam::COMBAT_SLOW_MOVE>
			// $*-f float Duration	// duration of the effect in seconds
			// $*-i sint32 MinFactor  // min factor on move speed (both walk and run, -20 = -20%)
			// $*-i sint32 MaxFactor  // max factor on move speed (both walk and run, -20 = -20%)
//			NLMISC::TGameCycle duration = NLMISC::TGameCycle( ((CSBrickParamCombatSlowMove *)param)->Duration / CTickEventHandler::getGameTimeStep() );
//			_CombatActions.push_back( new CCombatActionSlowMove( _Attacker->getEntityRowId(), this, duration, (sint16)((CSBrickParamCombatSlowMove *)param)->Factor));
//			}
//			break;


		case TBrickParam::OPENING_1:
			{
			INFOLOG("OPENING_1 : %s",((CSBrickParamOpening1 *)param)->EventFlag.c_str());
			_OpeningNeededFlags.resize(1);
			_OpeningNeededFlags[0] = BRICK_FLAGS::toBrickFlag(((CSBrickParamOpening1 *)param)->EventFlag);
			}
			break;

		case TBrickParam::OPENING_2:
			{
			INFOLOG("OPENING_2 : %s : %s",((CSBrickParamOpening2 *)param)->EventFlag1.c_str(), ((CSBrickParamOpening2 *)param)->EventFlag2.c_str());
			_OpeningNeededFlags.resize(2);
			_OpeningNeededFlags[0] = BRICK_FLAGS::toBrickFlag(((CSBrickParamOpening2 *)param)->EventFlag1);
			_OpeningNeededFlags[1] = BRICK_FLAGS::toBrickFlag(((CSBrickParamOpening2 *)param)->EventFlag2);
			}
			break;

		case TBrickParam::OPENING_3:
			{
			INFOLOG("OPENING_3 : %s : %s : %s",((CSBrickParamOpening3 *)param)->EventFlag1.c_str(), ((CSBrickParamOpening3 *)param)->EventFlag2.c_str(), ((CSBrickParamOpening3 *)param)->EventFlag3.c_str());
			_OpeningNeededFlags.resize(3);
			_OpeningNeededFlags[0] = BRICK_FLAGS::toBrickFlag(((CSBrickParamOpening3 *)param)->EventFlag1);
			_OpeningNeededFlags[1] = BRICK_FLAGS::toBrickFlag(((CSBrickParamOpening3 *)param)->EventFlag2);
			_OpeningNeededFlags[2] = BRICK_FLAGS::toBrickFlag(((CSBrickParamOpening3 *)param)->EventFlag3);
			}

		case TBrickParam::BLEED_FACTOR:
			{
			// $*STRUCT CSBrickParamBleedFactor: public TBrickParam::CId <TBrickParam::BLEED_FACTOR>
			// $*-f float Duration		// duration of the effect in seconds
			// $*-f float MinFactor	// max factor of dealt damage also lost in bleed
			// $*-f float MaxFactor	// max factor of dealt damage also lost in bleed			
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( ((CSBrickParamBleedFactor *)param)->Duration / CTickEventHandler::getGameTimeStep() );
			NLMISC::TGameCycle cycle = NLMISC::TGameCycle( 2.0f / CTickEventHandler::getGameTimeStep() );
			if (cycle && duration)
			{
				uint16 nbCycles = uint16(duration/cycle);
				if (nbCycles)
				{
					CCombatActionBleed *action = new CCombatActionBleed( _Attacker->getEntityRowId(), this, duration, cycle);
					if (!action)
						nlwarning("COMBAT : failed to allocate new CCombatActionBleed object");
					else
					{
						action->setDynValues(((CSBrickParamBleedFactor *)param)->MinFactor, ((CSBrickParamBleedFactor *)param)->MaxFactor, brick.PowerValue);
						_CombatActions.push_back(action);
					}
				}
			}
			}
			break;

		case TBrickParam::DEBUFF_REGEN:
			{
			// $*STRUCT CSBrickParamDebuffRegen: public TBrickParam::CId <TBrickParam::DEBUFF_REGEN>
			// $*-s std::string Score // affected score regen (Sap, Stamina, HitPoints, Focus)
			// $*-f float Duration = 0.0	// duration in seconds
			// $*-f float MinFactor = 0.0	// min factor of regen debuff
			// $*-f float MaxFactor = 0.0	// max factor of regen debuff
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( ((CSBrickParamDebuffRegen *)param)->Duration / CTickEventHandler::getGameTimeStep() );
			if (duration>0)
			{
				SCORES::TScores score = SCORES::toScore(((CSBrickParamDebuffRegen *)param)->Score);
				if (score == SCORES::unknown)
				{
					nlwarning("COMBAT : unknown score %s found in brick %s", ((CSBrickParamDebuffRegen *)param)->Score.c_str(), brick.Name.c_str());
				}
				else
				{
					EFFECT_FAMILIES::TEffectFamily family = EFFECT_FAMILIES::Unknown;
					switch(score)
					{
					case SCORES::sap:
						family = EFFECT_FAMILIES::CombatDebuffSapRegen;
						break;
					case SCORES::stamina:
						family = EFFECT_FAMILIES::CombatDebuffStaminaRegen;
						break;
					case SCORES::hit_points:
						family = EFFECT_FAMILIES::CombatDebuffHitPointsRegen;
						break;
					case SCORES::focus:
						family = EFFECT_FAMILIES::CombatDebuffFocusRegen;
						break;
					default:
						break;
					};
					if (family != EFFECT_FAMILIES::Unknown)
					{
						CCombatActionRegenModifier *action = new CCombatActionRegenModifier(_Attacker->getEntityRowId(), this, duration, score, family);
						if (!action)
							nlwarning("COMBAT : failed to allocate new CCombatActionRegenModifier object");
						else
						{
							action->setDynValues(((CSBrickParamDebuffRegen *)param)->MinFactor, ((CSBrickParamDebuffRegen *)param)->MaxFactor, brick.PowerValue);
							_CombatActions.push_back( action );
						}
					}
				}
			}
			}
			break;
	
		case TBrickParam::ARMOR_MOD:
			{
			// $*STRUCT CSBrickParamArmorMod: public TBrickParam::CId <TBrickParam::ARMOR_MOD>
			// $*-s std::string	ArmorType // affected armor type (light, medium, heavy, kitin etc..)
			// $*-f float		MinFactor = 1.0f //min factor applied on armor absorption
			// $*-f float		MaxFactor = 0.5f //max factor applied on armor absorption (< min as smaller is better)
			_ArmorAbsorptionFactor.PowerValue = brick.PowerValue;
			_ArmorAbsorptionFactor.MinValue = ((CSBrickParamArmorMod *)param)->MinFactor;
			_ArmorAbsorptionFactor.MaxValue = ((CSBrickParamArmorMod *)param)->MaxFactor;
			_ArmorAbsorptionFactor.ArmorType = ARMORTYPE::toArmorType(((CSBrickParamArmorMod *)param)->ArmorType);
			if (_ArmorAbsorptionFactor.ArmorType == ARMORTYPE::UNKNOWN)
				_ArmorAbsorptionFactor.CreatureType = EGSPD::CClassificationType::fromString(((CSBrickParamArmorMod *)param)->ArmorType);
			}
			break;

		case TBrickParam::AIM:
			{
			// $*STRUCT CSBrickParamAim: public TBrickParam::CId <TBrickParam::AIM>
			// $*-s std::string BodyType // homin, kitin (=land kitin), bird, flying_kitin....
			// $*-s std::string AimedSlot // head, body, arms...
			/// aim ALWAYS have min chances = 0 and max chances = 1.0f (100%)
			// remove beginning space from strings
			// TEMP fix
			while ( !((CSBrickParamAim *)param)->BodyType.empty() && ((CSBrickParamAim *)param)->BodyType[0] ==' ')
				((CSBrickParamAim *)param)->BodyType.erase(0,1);
			while ( !((CSBrickParamAim *)param)->AimedSlot.empty() && ((CSBrickParamAim *)param)->AimedSlot[0] ==' ')
				((CSBrickParamAim *)param)->AimedSlot.erase(0,1);
			/// end temp fix

			INFOLOG("AIM: body type %s, aimed slot %s", ((CSBrickParamAim *)param)->BodyType.c_str(),((CSBrickParamAim *)param)->AimedSlot.c_str() );
			_AimedSlot.Slot = SLOT_EQUIPMENT::stringToSlotEquipment( ((CSBrickParamAim *)param)->AimedSlot );
			_AimedSlot.BodyType = BODY::toBodyType( ((CSBrickParamAim *)param)->BodyType );
			_AimedSlot.PowerValue = brick.PowerValue;
			INFOLOG("_AimedSlot.BodyType = %s, Slot = %s", BODY::toString(_AimedSlot.BodyType).c_str(), SLOT_EQUIPMENT::toString(_AimedSlot.Slot).c_str());
			}
			break;

		case TBrickParam::THROW_OFF_BALANCE:
			{
				// $*STRUCT CSBrickParamThrowOffBalance: public TBrickParam::CId <TBrickParam::THROW_OFF_BALANCE>
				// $*-f float	MinDuration = 0.0f	// effect min duration
				// $*-f float	MaxDuration = 5.0f	// effect max duration 

				//////////////////////////////////////////////////////////////////////////
				// TODO
				//////////////////////////////////////////////////////////////////////////
			}
			break;

		case TBrickParam::SPECIAL_HIT:
			{
				_SpecialHit = true;
			}
			break;

		case TBrickParam::HIT_ALL_AGGRESSORS:
			// $*STRUCT CSBrickParamHitAllAggressors: public TBrickParam::CId <TBrickParam::HIT_ALL_AGGRESSORS>
			// $*-f float MinFactor		// min factor on dealt damage (total damage divided among targets)
			// $*-f float MaxFactor		// max factor on dealt damage (total damage divided among targets)
			_HitAllMeleeAggressors = true;
			_MultiTargetGlobalDamageFactor.MinValue = ((CSBrickParamHitAllAggressors *)param)->MinFactor;
			_MultiTargetGlobalDamageFactor.MaxValue = ((CSBrickParamHitAllAggressors *)param)->MaxFactor;
			_MultiTargetGlobalDamageFactor.PowerValue = brick.PowerValue;
			break;

		case TBrickParam::WEAPON_WEAR_MOD:
			// $*STRUCT CSBrickParamWeaponWearMod: public TBrickParam::CId <TBrickParam::WEAPON_WEAR_MOD>
			// $*-f float MinModifier		// min weapon wear modifier
			// $*-f float MaxModifier		// max weapon wear modifier
			_WeaponWearModifier.MinValue = ((CSBrickParamWeaponWearMod *)param)->MinModifier;
			_WeaponWearModifier.MaxValue = ((CSBrickParamWeaponWearMod *)param)->MaxModifier;
			_WeaponWearModifier.PowerValue = brick.PowerValue;
			break;

		case TBrickParam::DEFINE_FLAG:
			{
				BRICK_FLAGS::TBrickFlag flag = BRICK_FLAGS::toBrickFlag(((CSBrickParamDefineFlag *)param)->Flag);
				if (flag != BRICK_FLAGS::UnknownFlag)
					_BrickDefinedFlags.push_back(flag);
			}
			break;

		case TBrickParam::CRITICAL_HIT_MOD:
			_CriticalHitChancesModifier.MinValue = ((CSBrickParamCriticalHitMod *)param)->MinModifier;
			_CriticalHitChancesModifier.MaxValue = ((CSBrickParamCriticalHitMod *)param)->MaxModifier;
			_CriticalHitChancesModifier.PowerValue = brick.PowerValue;
			break;
			
		}
	}
} // addBrick //


//--------------------------------------------------------------
//					evaluate()  
//--------------------------------------------------------------
bool CCombatPhrase::evaluate()
{
	_Idle				= false;
	_PhraseSuccessDamageFactor = 0.0f;
	_AttackSkill		= SKILLS::unknown;
	_Validated			= false;
	_TargetTooFarMsg	= false;
	_NotEnoughStaminaMsg = false;
	_NotEnoughHpMsg		= false;
	_DisengageOnEnd		= false;
	_LatencyEndDate		= 0;
	_ExecutionEndDate	= 0;

	return true;
} // evaluate //


//--------------------------------------------------------------
//					validate()  
//--------------------------------------------------------------
bool CCombatPhrase::validate()
{
	H_AUTO(CCombatPhrase_validate);
	
	if (_Attacker == NULL || !_Attacker->isValid())
	{
		nlwarning("COMBAT Found NULL  or Invalid attacker.");
		return false;
	}

	CEntityBase *actingEntity = _Attacker->getEntity();
	if (actingEntity == NULL)
	{
		nlwarning("COMBAT : Cannot find entity ptr for acting entity %u", _Attacker->getEntityRowId().getIndex());
		return false;
	}

	// if actor is dead, returns
	if (actingEntity->isDead())
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looking for a memory problem
	//check types consistency 
	switch( actingEntity->getId().getType() )
	{
	case RYZOMID::player:
		if ( !dynamic_cast<CCombatAttackerPlayer*> (_Attacker) )
		{
			nlwarning("SERIOUS ERROR !! Entity %s type 'player' and _Attacker type does not match ! very serious error", actingEntity->getId().toString().c_str());
				return false;
		}
		break;
	case RYZOMID::npc:
		if ( !dynamic_cast<CCombatAttackerNpc*> (_Attacker) )
		{
			nlwarning("SERIOUS ERROR !! Entity %s type 'npc' and _Attacker type does not match ! very serious error", actingEntity->getId().toString().c_str());
			return false;
		}
		break;
	default: //creature
		if ( !dynamic_cast<CCombatAttackerAI*> (_Attacker) )
		{
			nlwarning("SERIOUS ERROR !! Entity %s type 'creature' and _Attacker type does not match ! very serious error", actingEntity->getId().toString().c_str());
			return false;
		}
	};
	//////////////////////////////////////////////////////////////////////////
	

	_Idle = false;

	bool engaged = false;
	string errorCode;
	CEntityBase *defender = NULL;

	CCombatDefenderPtr combatDefender;
	if ( !_Targets.empty())
		combatDefender = _Targets[0].Target;

	// get target
	TDataSetRow targetRowId = actingEntity->getTargetDataSetRow();
	if ( !combatDefender || targetRowId != combatDefender->getEntityRowId() )
	{
		combatDefender = createDefender(targetRowId);
		clearTargets();
		if (combatDefender)
		{
			_Targets.resize(1);
			_Targets[0].Target = combatDefender;
		}
	}

	if (!combatDefender)
	{
		_CurrentTargetIsValid = false;
		errorCode = "INVALID_TARGET";
	}
	else
	{
		// check the attacker has engaged a target if not auto engage the target
		TDataSetRow entityRowId = CPhraseManager::getInstance().getEntityEngagedMeleeBy( _Attacker->getEntityRowId() );
		if ( !TheDataset.isAccessible( entityRowId ) )
		{
			entityRowId = CPhraseManager::getInstance().getEntityEngagedRangeBy( _Attacker->getEntityRowId() );
		}
		if (TheDataset.isAccessible(entityRowId) )
		{
//			CEntityId entityId = TheDataset.getEntityId(entityRowId);
			if ( targetRowId == entityRowId )
			{
				engaged = true;
			}
			else
			{
				engaged = false;
			}		
		}

		defender = combatDefender->getEntity();
		if (defender == NULL)
		{
			_CurrentTargetIsValid = false;
			errorCode = "INVALID_TARGET";
		}
		else
		{
			// check opening
			if (!_Validated && !_OpeningNeededFlags.empty())
			{
				if (!checkOpening())
					return false;
			}
			
			_CurrentTargetIsValid = checkTargetValidity( combatDefender->getEntityRowId(), errorCode);
		}
	}

	if (!_CurrentTargetIsValid)
	{
		// if target is invalid and attacker isn't a player then return false and reset attack flag
		if (actingEntity->getId().getType() != RYZOMID::player)
		{
//			nldebug("Bot %s had a invalid target for it's combat action", actingEntity->getId().toString().c_str());
			return false;
		}
		else
		{
			PHRASE_UTILITIES::sendSimpleMessage(_Attacker->getEntityRowId(), errorCode);

			if (_CyclicPhrase)
			{
				//idle( true );
				return false;
			}
			else
			{
				return false;
			}
		}
	}
	
	// get the weapon used by the acting entity and check it
	_RightWeapon = CCombatWeapon();
	_LeftWeapon = CCombatWeapon();
	_Ammo = CCombatWeapon();
	
	if ( _Attacker->getItem( CCombatAttacker::RightHandItem, _RightWeapon) )
	{

		// forbid use of multi target with an area effect weapon !!!!
		if ( FightAreaEffectOn && _RightWeapon.Area && _HitAllMeleeAggressors )
			return false;

		if (_RightWeapon.Family == ITEMFAMILY::MELEE_WEAPON )
		{
			_MeleeCombat = true;

			// get left hand weapon if any
			_Attacker->getItem( CCombatAttacker::LeftHandItem, _LeftWeapon);
		}
		else if (_RightWeapon.Family == ITEMFAMILY::RANGE_WEAPON )
		{
			_AttackSkill = _RightWeapon.Skill;
			_MeleeCombat = false;
			// test ammo			
			if ( _Attacker->getItem( CCombatAttacker::Ammo, _Ammo) )
			{
				// check ammo qty
				if ( !_Attacker->checkAmmoAmount() )
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "EGS_NOT_ENOUGH_AMMO");
					_NoAmmoMsg = true;
					if (_CyclicPhrase)
						_Idle = true;
					else
						return false;
				}

				// lock ammos
				_Attacker->lockAmmos();
			}
			else
			{
				PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_NO_AMMO");
				_NoAmmoMsg = true;
				if (_CyclicPhrase)
					_Idle = true;
				else
					return false;
			}
		}
		else
		{
			DEBUGLOG("COMBAT : Entity %u, item in right hand is not a weapon", _Attacker->getEntityRowId().getIndex() );
			// ERROR -> Not a weapon
			//PHRASE_UTILITIES::sendSimpleMessage(_Attacker->getEntityRowId(), "BS_ITEM_INCOMPATIBLE");
			PHRASE_UTILITIES::sendDynamicSystemMessage(_Attacker->getEntityRowId(), "COMBAT_NOT_A_WEAPON");

			return false;
		}
	}
	else
	{
		_MeleeCombat = true; // hand to hand
	}

	// test target is alive
	if (_CurrentTargetIsValid && defender != NULL)
	{
		if ( defender->isDead() )
		{						
			DEBUGLOG("COMBAT : Entity %s is dead", TheDataset.getEntityId(combatDefender->getEntityRowId()).toString().c_str());

			if (actingEntity->getId().getType() == RYZOMID::player)
			{
				PHRASE_UTILITIES::sendSimpleMessage(_Attacker->getEntityRowId(), "COMBAT_TARGET_DEAD");
			}
			else
			{
//				nldebug("Bot %s had a invalid target for it's combat action", actingEntity->getId().toString().c_str());
			}			

			return false;
		}
	}

	if ( _CurrentTargetIsValid && defender != 0 && actingEntity->getId().getType() == RYZOMID::player)
	{
		if(_MeleeCombat)
		{
			// check combat float mode
			CCharacter *character = PlayerManager.getChar(_Attacker->getEntityRowId());
			if (character && !character->meleeCombatIsValid())
			{
				if (!_TargetTooFarMsg)
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_TARGET_TOO_FAR_OR");
					_TargetTooFarMsg = true;
				}

				if (_CyclicPhrase)
					idle( true );
				else
				{
					return false;
				}
				INFOLOG("COMBAT : Entity %s is isn't engaged IDLE mode",TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str());
			}
		}
		else
		{
			// get range in mm and test range
			uint32 range = uint32(_RightWeapon.Range + _Ammo.Range);
			if ( ! PHRASE_UTILITIES::testRange(*actingEntity, *defender, range ) )
			{
				if (!_TargetTooFarMsg)
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_TARGET_TOO_FAR");
					_TargetTooFarMsg = true;
				}

				if (_CyclicPhrase)
				{
					idle( true );
				}
				else
				{
					return false;
				}
				INFOLOG("COMBAT : Entity %s is too far from it's target %s",actingEntity->getId().toString().c_str(), defender->getId().toString().c_str());
			}

			// test orientation
			if ( !checkOrientation(actingEntity,defender) )
			{
				if (_CyclicPhrase)
				{
					idle( true );
				}
				else
				{
					return false;
				}
			}
		}
	}

	// test acting entity can act
	if (actingEntity->canEntityUseAction() == false)
	{
		if (_CyclicPhrase)
		{
			idle( true );
		}
		else
		{
			return false;
		}
	}
	
	// Test entity can use an action right now
	if (actingEntity->getId().getType() == RYZOMID::player)
	{
		CCharacter *character = dynamic_cast<CCharacter *> (actingEntity);
		if (character && character->dateOfNextAllowedAction() > CTickEventHandler::getGameCycle() )
		{		
			//errorCode = "EGS_CANNOT_ENGAGE_COMBAT_YET";
			//PHRASE_UTILITIES::sendSimpleMessage( actingEntity->getId(), "EGS_CANNOT_ENGAGE_COMBAT_YET");
			PHRASE_UTILITIES::sendDynamicSystemMessage( actingEntity->getEntityRowId(), "COMBAT_CANNOT_USE_ACTION_YET");
			DEBUGLOG("<CCombatPhrase::validate> Entity %s cannot engage combat yet", actingEntity->getId().toString().c_str() );
			return false;
		}
	}

	// if a feint, check opponent engaged current attacker in melee combat
	if (isFeint())
	{
		if ( CPhraseManager::getInstance().getEntityEngagedMeleeBy(combatDefender->getEntityRowId()) != actingEntity->getEntityRowId() )
		{
			// send message to attacker
			if ( actingEntity->getId().getType() == RYZOMID::player)
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);	
				NLMISC::CEntityId eId(TheDataset.getEntityId(combatDefender->getEntityRowId()));
				params[0].setEIdAIAlias( eId, CAIAliasTranslator::getInstance()->getAIAlias( eId ) );
				PHRASE_UTILITIES::sendDynamicSystemMessage(actingEntity->getEntityRowId(), "COMBAT_FEINT_INVALID", params);	
			}
			return false;
		}
	}
	
	// check the player has engaged a target if not auto engage the target
	if  (!engaged && _CurrentTargetIsValid)
	{
		if (_MeleeCombat)
		{
			if ( ! PHRASE_UTILITIES::engageTargetInMelee( actingEntity->getId(), TheDataset.getEntityId(combatDefender->getEntityRowId()) ) )
			{
				DEBUGLOG("COMBAT : Entity %s Failed to engage its target in melee combat", actingEntity->getId().toString().c_str() );
				return false;
			}
		}
		else
		{
			if ( ! PHRASE_UTILITIES::engageTargetRange( actingEntity->getId(), TheDataset.getEntityId(combatDefender->getEntityRowId()) ) )
			{
				DEBUGLOG("COMBAT : Entity %s Failed to engage its target in range combat", actingEntity->getId().toString().c_str());
				return false;
			}
		}
	}

	_TotalStaminaCost = _StaminaCost + (uint32)(_StaminaWeightFactorCost * _Attacker->getEntity()->getWeightOfEquippedWeapon() / 1000 );
	_TotalHPCost = _HPCost;

	// if attacker is a player, use equipment wear malus on stamina and Hp cost
	if (actingEntity->getId().getType() == RYZOMID::player)
	{
		CCharacter *character = dynamic_cast<CCharacter*> (actingEntity);
		if (!character)
		{
			nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", actingEntity->getId().toString().c_str());
		}
		else
		{
			const float factor = 1.0f + 0.5f * character->wearMalus();
			_TotalStaminaCost = sint32( _TotalStaminaCost * factor );
			_TotalHPCost = sint32( _TotalHPCost * factor );
		}
	}

	if(!checkPhraseCost(errorCode))
	{
		if (!_NotEnoughStaminaMsg && errorCode == "EGS_TOO_EXPENSIVE_STAMINA") 
		{
			_NotEnoughStaminaMsg = true;
			PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), errorCode );
		}
		else if (!_NotEnoughHpMsg && errorCode == "EGS_TOO_EXPENSIVE_HP")
		{
			_NotEnoughHpMsg = true;
			PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), errorCode );
		}
	
		if (_CyclicPhrase)
		{
			idle( true );
		}
		else
		{
			return false;
		}
	}

	if (!validateCombatActions(errorCode))
	{
		PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), errorCode );
		DEBUGLOG("COMBAT : Entity %s Failed to validate combat actions, error = %s", actingEntity->getId().toString().c_str(), errorCode.c_str() );
		return false;
	}

	// if no item in hand check whether all bricks can be used
	if ( !_Attacker->getItem( CCombatAttacker::RightHandItem, _RightWeapon) )
	{
		for (uint s=0; s<_BrickSheets.size(); ++s)
		{
			const CStaticBrick * brick = CSheets::getSBrickForm( _BrickSheets[s] );
			if( brick )
			{
				if( brick->UsableWithEmptyHands == false )
				{
					return false;
				}
			}
		}
	}
	
	_Validated = true;

	return true;
} // validate //

//--------------------------------------------------------------
//					update()  
//--------------------------------------------------------------
bool  CCombatPhrase::update()
{
	H_AUTO(CCombatPhrase_update);
	
	uint debugStep = 0;
	try
	{
		if (!_Attacker || !_Attacker->isValid())
		{
			debugStep = 1;
			DEBUGLOG("COMBAT : (update) attacker null or invalid (no longer in mirror)");
			return false;
		}

		if ( state() == Validated || state() == SecondValidated || state() == ExecutionInProgress )
		{
			debugStep = 2;
			_Idle = false;

			CEntityBase *actor = _Attacker->getEntity();
			debugStep = 3;
			if (!actor)
			{
				// cannot reset the action flag, the actor cannot be found...
				return false;
			}

			// check if actor can use action
			if (actor->canEntityUseAction() == false)
			{
				if (_CyclicPhrase)
					_Idle = true;
				else
				{
					return false;
				}
			}		
			debugStep = 4;

			// get main target
			CCombatDefenderPtr combatDefender;
			if ( !_Targets.empty())
			{
				if (_Targets[0].Target->isValid())
					combatDefender = _Targets[0].Target;
			}
			debugStep = 5;
			
			// check target validity
			TDataSetRow target = actor->getTargetDataSetRow();
			debugStep = 6;
			if	( TheDataset.isAccessible(target) 
				&& (!combatDefender || !combatDefender->isValid() || target != combatDefender->getEntityRowId()) 
				)
			{
				debugStep = 7;
				combatDefender = createDefender(target);
				debugStep = 8;
				
				if (!combatDefender)
				{
					_CurrentTargetIsValid =false;
					_Idle = true;
				}
				else
				{
					CEntityBase *defender = combatDefender->getEntity();
					debugStep = 9;
					if (!defender)
					{
						_CurrentTargetIsValid =false;
						//_Idle = true;
						return false;
					}
					else
					{
						clearTargets();
						debugStep = 10;
						_Targets.resize(1);
						_Targets[0].Target = combatDefender;
						debugStep = 11;

						string errorCode;
						_CurrentTargetIsValid = checkTargetValidity( combatDefender->getEntityRowId(), errorCode);
						debugStep = 12;
						if ( !_CurrentTargetIsValid )
						{
							PHRASE_UTILITIES::sendSimpleMessage(actor->getId(), errorCode);
							//_Idle = true;
							return false;
						}
						else
						{
							debugStep = 13;
							if (_MeleeCombat)
							{
								if ( ! PHRASE_UTILITIES::engageTargetInMelee( actor->getId(), defender->getId() ) )
								{
									DEBUGLOG("COMBAT : Entity %s Failed to engage its target in melee combat", actor->getId().toString().c_str());
									return false;
								}
							}
							else
							{
								if ( ! PHRASE_UTILITIES::engageTargetRange( actor->getId(),  defender->getId() ) )
								{
									DEBUGLOG("COMBAT : Entity %s Failed to engage its target in range combat", actor->getId().toString().c_str() );
									return false;
								}
							}
						}
					}
				}
			}
			else
			{
				if (!_CurrentTargetIsValid)
				{
					//_Idle = true;
					return false;
				}
			}

			// if target is invalid and attacker isn't a player then return false and reset attack flag
			debugStep = 14;
			if (!_CurrentTargetIsValid && actor->getId().getType() != RYZOMID::player)
			{
	//			nldebug("Bot %s had a invalid target for it's combat action, could have stucked", actor->getId().toString().c_str());
				return false;
			}

			// check costs for players
			debugStep = 15;
			string errorCode;
			if(!checkPhraseCost(errorCode))
			{
				debugStep = 16;
				if (!_NotEnoughStaminaMsg && errorCode == "EGS_TOO_EXPENSIVE_STAMINA" && !_Idle )
				{
					_NotEnoughStaminaMsg = true;
					PHRASE_UTILITIES::sendSimpleMessage( actor->getId(), errorCode );
				}
				else if (!_NotEnoughHpMsg && errorCode == "EGS_TOO_EXPENSIVE_HP" && !_Idle )
				{
					_NotEnoughHpMsg = true;
					PHRASE_UTILITIES::sendSimpleMessage( actor->getId(), errorCode );
				}
				_Idle = true;
			}


			// check distance if attacker is a player
			debugStep = 17;
			if ( _CurrentTargetIsValid && actor->getId().getType() == RYZOMID::player)
			{
				if (_MeleeCombat )
				{
					debugStep = 18;
					CCharacter *character = dynamic_cast<CCharacter *> (actor);
					if (character && !character->meleeCombatIsValid())
					{
						debugStep = 19;
						if (!_TargetTooFarMsg && !_Idle)
						{
							PHRASE_UTILITIES::sendSimpleMessage( actor->getId(), "BS_TARGET_TOO_FAR_OR");
							_TargetTooFarMsg = true;
						}

						_Idle = true;
					}
				}
				else
				{
	// keep weapons and ammos found in last call to validate
	//				_RightWeapon.init();
	//				_Ammo.init();
	//				_Attacker->getItem( CCombatAttacker::RightHandItem, _RightWeapon);
	//				_Attacker->getItem( CCombatAttacker::Ammo, _Ammo);

					debugStep = 20;
					if (!combatDefender || !combatDefender->getEntity())
						return false;
					
					// test range in mm
					debugStep = 21;
					const uint32 range = uint32(_RightWeapon.Range + _Ammo.Range);
					if ( ! PHRASE_UTILITIES::testRange(*actor, *combatDefender->getEntity(), range ) )
					{
						debugStep = 21;
						if (!_TargetTooFarMsg)
						{
							PHRASE_UTILITIES::sendSimpleMessage( actor->getId(), "BS_TARGET_TOO_FAR");
							_TargetTooFarMsg = true;
						}

						debugStep = 22;
						idle( true );
						INFOLOG("COMBAT : Entity %s is too far from it's target %s, distance %f, range %f",actor->getId().toString().c_str(), combatDefender->getEntity()->getId().toString().c_str(), PHRASE_UTILITIES::getDistance(actor->getEntityRowId(), combatDefender->getEntityRowId()),range/1000.0f);
					}

					// test orientation
					debugStep = 23;
					if ( !checkOrientation( actor, combatDefender->getEntity() ) )
					{
						idle(true);
					}
				}
			}

			// check ammo if player and range combat
			debugStep = 24;
			if ( !_MeleeCombat && actor->getId().getType() == RYZOMID::player)
			{
				debugStep = 25;
				if ( _Attacker->getItem( CCombatAttacker::Ammo, _Ammo) )
				{
					// check ammo qty
					debugStep = 26;
					if ( !_Attacker->checkAmmoAmount() )
					{
						debugStep = 27;
						if (!_NoAmmoMsg)
						{
							PHRASE_UTILITIES::sendSimpleMessage( actor->getId(), "EGS_NOT_ENOUGH_AMMO");
							_NoAmmoMsg = true;
						}
						debugStep = 28;
						if (_CyclicPhrase)
							idle(true);
						else
							return false;
					}
				}
				else
				{
					debugStep = 29;
					if (!_NoAmmoMsg)
					{
						PHRASE_UTILITIES::sendSimpleMessage( actor->getId(), "BS_NO_AMMO");
						_NoAmmoMsg = true;
					}
					debugStep = 30;
					if (_CyclicPhrase)
						idle(true);
					else
						return false;
				}
			}
		}
	}
	catch (...)
	{
		try
		{
			nlwarning("DBGRING: Exception catched in CCombatPhrase::update(), after step %u", debugStep);
			nldebug("DBGRING: _Attacker=%s", _Attacker->getEntityRowId().toString().c_str());
			nldebug("DBGRING: _Targets.size=%u", _Targets.size());
			for (uint i=0; i!=_Targets.size(); ++i)
			{
				nldebug("DBGRING: _Targets[%u]=%s", i, _Targets[i].Target->getEntityRowId().toString().c_str());
				
			}
			nldebug("DBGRING: _CyclicPhrase=%u _MeleeCombat=%u", (uint)_CyclicPhrase, (uint)_MeleeCombat);
			nldebug("DBGRING: _Attacker:%s", _Attacker->getEntity()->getId().toString().c_str());
			nldebug("DBGRING: actor.target=%s ", _Attacker->getEntity()->getTargetDataSetRow().toString().c_str());
			for (uint i=0; i!=_Targets.size(); ++i)
			{
				nldebug("DBGRING: _Targets[%u]:%s", i, _Targets[i].Target->getEntity()->getId().toString().c_str());
				
			}
		}
		catch (...)
		{
			nlwarning("DBGRING: Exception in catch handler!");
		}
		return false;
	}

	return true;
} // update //

//--------------------------------------------------------------
//					execute()  
//--------------------------------------------------------------
void  CCombatPhrase::execute()
{
	H_AUTO(CCombatPhrase_execute);
	
	// check attacker is still valid
	if (!_Attacker || !_Attacker->isValid())
	{
		DEBUGLOG("COMBAT : (execute) Invalid attacker (null or not in mirror)");
		return;
	}

	if( _Idle )
		return;

	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();

	_ExecutionEndDate  = time + _ExecutionLengthModifier ;

	if (_Attacker)
	{
		CEntityBase *entity = _Attacker->getEntity();
		if (entity)
			entity->cancelStaticEffects();

		if (entity->getId().getType() == RYZOMID::player)
		{
			CCharacter* player = dynamic_cast<CCharacter*> (entity);
			if (player)
			{
				// if player has any static action in progress, cancel it
				// cancel entity static action
				player->cancelStaticActionInProgress();			

				player->setCurrentAction(CLIENT_ACTION_TYPE::Combat,_ExecutionEndDate);
				if (_RootSheetId != CSheetId::Unknown)
				{
//					player->_PropertyDatabase.setProp( "EXECUTE_PHRASE:SHEET", _RootSheetId.asInt() );
					CBankAccessor_PLR::getEXECUTE_PHRASE().setSHEET(player->_PropertyDatabase, _RootSheetId);
//					player->_PropertyDatabase.setProp( "EXECUTE_PHRASE:PHRASE", 0 );
					CBankAccessor_PLR::getEXECUTE_PHRASE().setPHRASE(player->_PropertyDatabase, 0);

					//Bsi.append( StatPath, NLMISC::toString("[UAA] %s %s %s", player->getId().toString().c_str(), "default attack", _RootSheetId.toString().c_str()) );
					//EgsStat.displayNL("[UAA] %s %s %s", player->getId().toString().c_str(), "default attack", _RootSheetId.toString().c_str());
//					EGSPD::useActionAchete(player->getId(), "default attack", _RootSheetId.toString());
				}
			}
		}
	}
} // execute //


//--------------------------------------------------------------
//					launch()  
//--------------------------------------------------------------
bool CCombatPhrase::launch()
{
	H_AUTO(CCombatPhrase_launch);
	
	_LatencyEndDate = 0;
	_ApplyDate = 0;

	if ( !_Attacker ) 
	{
		nlwarning("<CCombatPhrase::launch> found NULL attacker or defender.");
		return false;
	}

	if (_Targets.empty())
	{
		nlwarning("<CCombatPhrase::launch> combat action has no target.");
		return false;
	}

	// Reset flying text counters
	_MissFlyingTextTriggered= false;
	for(uint i=0;i<_Targets.size();i++)
	{
		_Targets[i].NbParryDodgeFlyingTextRequired= 0;
	}

	// reserve common space for parry/dodge/miss delayed messages
	_DelayedEvents.reserve(_Targets.size());

	// get the main target race
	EGSPD::CPeople::TPeople targetRace = EGSPD::CPeople::Unknown;
	if (!_Targets.empty() && _Targets[0].Target != NULL)
	{
		CEntityBase *target = _Targets[0].Target->getEntity();
		if (target)
			targetRace = target->getRace();
	}

	CEntityBase * actingEntity = CEntityBaseManager::getEntityBasePtr( _Attacker->getEntityRowId() );
	if (actingEntity == NULL)
	{
		nlwarning("<CCombatPhrase::launch> invalid entity Id %s", TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str() );
		return false;
	}

	_TargetTooFarMsg = false;	
	_Behaviour.Data = 0;
	_Behaviour.Data2 = 0;
	_Behaviour.DeltaHP = 0;

	// spend stamina, hp
	if (_TotalStaminaCost != 0)
	{
		SCharacteristicsAndScores &stamina = actingEntity->getScores()._PhysicalScores[SCORES::stamina];
		if ( stamina.Current != 0 )
		{
			stamina.Current = stamina.Current - _TotalStaminaCost;
			if (stamina.Current < 0)
				stamina.Current = 0;
		}
	}

	if ( _TotalHPCost != 0)
	{
		// acting entity should not be able to kill itself
		const sint32 lostHp = min( _TotalHPCost, sint32(actingEntity->currentHp() - 1) );
		
		if ( actingEntity->changeCurrentHp( (lostHp) * (-1) ) )
		{
			PHRASE_UTILITIES::sendDeathMessages( actingEntity->getEntityRowId(), actingEntity->getEntityRowId() );
			return false;
		}
	}

	_LeftWeaponSabrinaValue = 0;

	if ( _Attacker->getItem( CCombatAttacker::RightHandItem, _RightWeapon) )
	{
		if (_RightWeapon.Family == ITEMFAMILY::MELEE_WEAPON )
		{
			_MeleeCombat = true;

			// get left hand weapon if any
			_Attacker->getItem( CCombatAttacker::LeftHandItem, _LeftWeapon);

			if (_LeftWeapon.Family == ITEMFAMILY::MELEE_WEAPON)
				_LeftWeaponSabrinaValue = getWeaponSabrinaValue(_LeftWeapon, _Attacker->getSkillValue(_LeftWeapon.Skill, targetRace));
		}
		else if (_RightWeapon.Family == ITEMFAMILY::RANGE_WEAPON )
		{
			if ( _Attacker->getItem( CCombatAttacker::Ammo, _Ammo) )
			{
				// check ammo qty
				if ( !_Attacker->checkAmmoAmount() )
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "EGS_NOT_ENOUGH_AMMO");
					return false;
				}
				if (!_Behaviour.isCreatureAttack())
				{
					_Behaviour.Range.WeaponType = _RightWeapon.AreaType;
					_Behaviour.Range.Time = CTickEventHandler::getGameCycle()>>2;
					_Behaviour.Behaviour = MBEHAV::RANGE_ATTACK;
					// tmp nico : stats about projectiles
					projStatsIncrement();					
				}
				
				// unlock ammos
				_Attacker->unlockAmmos(1);
				// consume ammos
				_Attacker->consumeAmmos(1);
			}
			else
			{
				PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_NO_AMMO");
				return false;
			}  
		}
		else
		{
			// ERROR -> Not a weapon
			PHRASE_UTILITIES::sendSimpleMessage(_Attacker->getEntityRowId(), "BS_ITEM_INCOMPATIBLE");
			return false;
		}
	}
	else
	{
		_RightWeapon.Damage = HandToHandDamage;
		_RightWeapon.DmgType = DMGTYPE::BLUNT;
		_RightWeapon.LatencyInTicks = HandToHandLatency;

		_RightWeapon.Quality = (uint16)_Attacker->getSkillValue(BarehandCombatSkill, targetRace);
		_RightWeapon.Skill = BarehandCombatSkill;
	}
	
	_WeaponSabrinaValue = getWeaponSabrinaValue(_RightWeapon, _Attacker->getSkillValue(_RightWeapon.Skill, targetRace));

	// apply weapon wear if needed
	if (_WeaponWearModifier.MaxValue > 0.0f)
	{
		// use total sabrina credit as reference for this modifier
		_Attacker->damageItem(_WeaponWearModifier.applyValue((uint16)(_SabrinaCredit * _SabrinaRelativeCredit)), true);
	}
	
	// get weapon latency
	float latency;
	if(_LeftWeapon.LatencyInTicks != 0)
	{
		latency = float(_HitRateModifier + std::max( MinTwoWeaponsLatency.get(), std::max(_RightWeapon.LatencyInTicks, _LeftWeapon.LatencyInTicks)) + _Ammo.LatencyInTicks);
	}
	else
	{
		latency = float(_HitRateModifier + std::max(_RightWeapon.LatencyInTicks, _LeftWeapon.LatencyInTicks) + _Ammo.LatencyInTicks);
	}

	// check for madness effect
	// save these values to _IsMad and _MadnessCaster needed by apply()
	_IsMad = false;
	TDataSetRow & madnessCaster = _MadnessCaster;
	EFFECT_FAMILIES::TEffectFamily family;
	if (_MeleeCombat)
	{
		family = EFFECT_FAMILIES::MadnessMelee;
	}
	else
	{
		family = EFFECT_FAMILIES::MadnessRange;
	}
	
	const CSEffectPtr madness = actingEntity->lookForActiveEffect(family);
	if ( madness )
	{
		const uint8 roll = (uint8) RandomGenerator.rand(99);
		if ( roll < madness->getParamValue() )
		{
			DEBUGLOG("<CCombatPhrase::launch> entity %s hit itself (madness effect)", actingEntity->getId().toString().c_str());
			_IsMad = true;
			madnessCaster = madness->getCreatorRowId();
			_MadSkill = madness->getSkill();
		}
	}
	// check other madness effect
	if (!_IsMad)
	{
		const CSEffectPtr madness = actingEntity->lookForActiveEffect(EFFECT_FAMILIES::Madness);
		if ( madness )
		{
			const uint8 roll = (uint8) RandomGenerator.rand(99);
			if ( roll < madness->getParamValue() )
			{
				DEBUGLOG("<CCombatPhrase::launch> entity %s hit itself (madness effect)", actingEntity->getId().toString().c_str());
				_IsMad = true;
				madnessCaster = madness->getCreatorRowId();
				_MadSkill = madness->getSkill();
			}
		}
	}
	// check reflect damage effect on main target
	if (!_IsMad)
	{
		if (!_Targets.empty() && _Targets[0].Target != NULL)
		{
			CEntityBase *target = _Targets[0].Target->getEntity();
			if (target)
			{
				const CSEffectPtr reflectEffect = target->lookForActiveEffect(EFFECT_FAMILIES::ReflectDamage);
				if ( reflectEffect )
				{
					const uint8 roll = (uint8) RandomGenerator.rand(99);
					if ( roll < reflectEffect->getParamValue() )
					{
						DEBUGLOG("<CCombatPhrase::launch> entity %s hit itself (ReflectDamage effect)", actingEntity->getId().toString().c_str());
						_IsMad = true;
						_MadSkill = reflectEffect->getSkill();
					}
				}
			}
		}
	}
	// if mad then change target to attacker
	if (_IsMad)
	{
		clearTargets();
		CCombatDefenderPtr defender = createDefender(_Attacker->getEntityRowId());
		if (defender != NULL)
		{
			_Targets.resize(1);
			_Targets[0].Target = defender;
		}
		else
		{
			nlwarning("<CCombatPhrase::launch> failed to create CCombatDefender from _Attacker (is mad) !");
			return false;
		}
	}

	// right hand and left hand successes
	bool success = false;
	bool successLeft = false;

	// if the attack is a feint, does no damage and no effect on targets
	if (!_IsMad && isFeint())
	{
		// send a message to both attacker and defender
		if (!_Targets.empty() && _Targets[0].Target != NULL)
		{
			CEntityBase *target = _Targets[0].Target->getEntity();
			if (target)
			{		
				TVectorParamCheck params;
				if ( actingEntity->getId().getType() == RYZOMID::player)
				{
					params.resize(1);
					params[0].Type = STRING_MANAGER::entity;
					params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias( target->getId() ) );
					PHRASE_UTILITIES::sendDynamicSystemMessage(actingEntity->getEntityRowId(), "COMBAT_FEINT_ACTOR", params);
				}
				
				if ( target->getId().getType() == RYZOMID::player)
				{
					params.resize(1);
					params[0].Type = STRING_MANAGER::entity;					
					params[0].setEIdAIAlias( actingEntity->getId(), CAIAliasTranslator::getInstance()->getAIAlias( actingEntity->getId() ) );
					PHRASE_UTILITIES::sendDynamicSystemMessage(target->getEntityRowId(), "COMBAT_FEINT_TARGET", params);
				}
			}
		}

		buildTargetList(true, false);

		const uint32 nbTargets = (uint32)_Targets.size();

		// update behaviour
		if ( _Behaviour.isCombat() )
		{
			if ( nbTargets > 1)
				_Behaviour.Combat2.TargetMode = 1;
			else
				_Behaviour.Combat2.TargetMode = 0;
		}
		else
		{
			if ( nbTargets > 1)
				_Behaviour.CreatureAttack2.TargetMode = 1;
			else
				_Behaviour.CreatureAttack2.TargetMode = 0;
		}
	}
	else
	{
		// check redirect attack effect
		if (!_Targets.empty() && _Targets[0].Target != NULL)
		{
			CEntityBase *target = _Targets[0].Target->getEntity();
			if (target)
			{
				const CSEffectPtr effect = target->lookForActiveEffect( EFFECT_FAMILIES::RedirectAttacks );
				if ( effect )
				{
					CRedirectAttacksEffect *rEffect = dynamic_cast<CRedirectAttacksEffect *> (&(*effect));
					if (!rEffect)
					{
						nlwarning("Found an effect of type RedirectAttacks but failed to dynamic_cast it in  CRedirectAttacksEffect !");
					}
					else
					{
						CEntityBase *newTarget = rEffect->getTargetForRedirection();
						if (newTarget)
						{
							clearTargets();
							CCombatDefenderPtr defender = createDefender(newTarget->getEntityRowId());
							if (defender != NULL)
							{
								_Targets.resize(1);
								_Targets[0].Target = defender;
							}
							else
							{
								nlwarning("<CCombatPhrase::launch> failed to create CCombatDefender from redirectAttackTarget !");
								return false;
							}
						}
					}
				}
			}
		}
		
		// build target lists - used for both hands if dual wield
		buildTargetList(true, _IsMad);

		// resize datas
		const uint32 nbTargets = (uint32)_Targets.size();

		// update behaviour
		if ( _Behaviour.isCombat() )
		{
			if ( nbTargets > 1)
				_Behaviour.Combat2.TargetMode = 1;
			else
				_Behaviour.Combat2.TargetMode = 0;
		}
		else
		{
			if ( nbTargets > 1)
				_Behaviour.CreatureAttack2.TargetMode = 1;
			else
				_Behaviour.CreatureAttack2.TargetMode = 0;
		}

		// 1 action per hand and per target
		_RightApplyActions.reserve(nbTargets);
		_LeftApplyActions.reserve(nbTargets);

		// launch right hand attack
		success = launchAttack(actingEntity, true, _IsMad, madnessCaster);

		// launch left hand attack if a weapon is in left hand and right attack isn't a miss
		successLeft = false;
		if (_LeftWeapon.Family == ITEMFAMILY::MELEE_WEAPON)
		{
			successLeft = launchAttack(actingEntity, false, _IsMad, madnessCaster);
		}

		if (!success && !successLeft)
		{
			// update behaviour
			if(_Behaviour.isCreatureAttack())
			{
				_Behaviour.CreatureAttack2.HitType = HITTYPE::Failed;
				_Behaviour.CreatureAttack.ImpactIntensity = 0;
			}
			else
			{
				if (_MeleeCombat)
				{
					_Behaviour.Combat.HitType = HITTYPE::Failed;
					_Behaviour.Combat.ImpactIntensity = 0;
					_Behaviour.Combat.KillingBlow = 0;
				}
				else
				{
					_Behaviour.Range.ImpactIntensity = 0;
				}
			}

			_Targets.resize(1);
			nlassert(_Targets[0].Target != NULL);
			_Targets[0].DamageFactor	= 0.f;
			_Targets[0].Distance		= (float) PHRASE_UTILITIES::getDistance(_Targets[0].Target->getEntityRowId(), _Attacker->getEntityRowId());
			_Targets[0].DodgeFactor		= 1.f;
		}
	}

	// update actor visual property with target list (must be done before behaviour)
	CMirrorPropValueList<uint32>	targetList(TheDataset, _Attacker->getEntityRowId(), DSPropertyTARGET_LIST);
	targetList.clear();

	const sint size = (sint)_Targets.size();
	for (sint i = size-1 ; i >= 0 ; --i)
	{
		PHRASE_UTILITIES::updateMirrorTargetList(targetList, _Targets[i].Target->getEntityRowId(), _Targets[i].Distance, false);
	}

	// compute latency end date
	// apply latency factor
	latency *= _LatencyFactor;
	latency *= _LatencyFactorDyn.applyValue(_WeaponSabrinaValue);
	
	// look for attack slow effects and add it to latency
	sint32 combatAttackSlowMod = 0;
	sint32 slowMeleeMod = 0;
		
	sint32 latencyMod = 0;
	// do not use smart pointers for local use only
	const CSEffect *effect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::CombatAttackSlow );
	if ( effect )
	{
		latencyMod += effect->getParamValue();
		combatAttackSlowMod = effect->getParamValue();
	}
	effect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::SlowAttack);
	if ( effect )
	{
		latencyMod += effect->getParamValue();
	}
	effect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::CombatSlow);
	if ( effect )
	{
		latencyMod += effect->getParamValue();
	}

	if (_MeleeCombat)
	{
		effect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::SlowMelee );
		if( effect )
		{
			slowMeleeMod = effect->getParamValue();
			latencyMod += effect->getParamValue();
		}
	}
	else
	{
		effect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::SlowRange );
	}
	
	if ( effect )
		latencyMod += effect->getParamValue();

	// add spire effect ( melee/range speed )
	if ( actingEntity->getId().getType() == RYZOMID::player )
	{
		const CSEffect* pEffect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::TotemCombatMRSpd );
		if ( pEffect != NULL )
		{
			latencyMod -= pEffect->getParamValue();
		}
	}
	
	latency *= (1.0f + latencyMod / 100.0f);

#if !FINAL_VERSION
	latency *= CombatLatencyFactor;
#endif // !FINAL_VERSION

	if (latency>600.0f || latency < 0.0f)
	{
		nlwarning("<CCombatPhrase::launch> entity %s, combat action latency = %f ticks ! set it to 30 ticks only", actingEntity->getId().toString().c_str(), latency);
		nlwarning("\t\tweapon latency : %u",_RightWeapon.LatencyInTicks);
		nlwarning("\t\t_LatencyFactorOnSuccess : %f",_LatencyFactorDyn.applyValue(_WeaponSabrinaValue));
		nlwarning("\t\tlatencyMod : %u",latencyMod);
		nlwarning("\t\t_HitRateModifier : %u",_HitRateModifier);
		nlwarning("\t\tcombatAttackSlow effect mod value : %d",combatAttackSlowMod);
		nlwarning("\t\tslowMelee effect value : %d",slowMeleeMod);
					
		latency = 30.0f;
	}
	
	// set latency end date
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	_LatencyEndDate = time + (NLMISC::TGameCycle)latency;

	// compute the apply date
	if (_Targets[0].Target!=NULL && actingEntity->getEntityRowId() == _Targets[0].Target->getEntityRowId())
	{
		// apply immediately if the main target is the actor
		_ApplyDate = 0;
	}
	else if ( _MeleeCombat )
	{
		// If it is a melee attack, Consider an average of 0.4 second of delay (time for the weapon to rich the target body)
		/* To do better, we should rely on the animation played, which rely on lot of parameters
			- whether the weapon used is in right or left hand (???)
			- the type of hit (default, power, area)
			- the client-side VISUAL localisation (a player hitting a yubo's head still does a Low-Attack)
		*/
		_ApplyDate = time + 4;
	}
	else
	{
		// Yoyo: I don't want to get into the complex client system that associate attack to attack list sheets.
		// Just make a simple hardcode that should work in 90% of cases: if the item type is pistol, bowpistol etc...
		// then just set a short average apply date of 0.2 seconds
		if(_RightWeapon.IsDirectRangeAttack)
		{
			_ApplyDate = time + 2;
		}
		// else consider it is a range attack that cast a projectile
		else
		{
			// Apply according to the time of the missile to reach its ennemy
			const double distance = PHRASE_UTILITIES::getDistance(actingEntity->getEntityRowId(), _Targets[0].Target->getEntityRowId()); // in meters
			double launchTime = (distance / MAGICFX::PROJECTILE_SPEED) / CTickEventHandler::getGameTimeStep();

			_ApplyDate = time + NLMISC::TGameCycle( launchTime );

			// apply immediately if the launch time is too big (> 100 seconds)
			if (_ApplyDate - time > 1000)
			{
				CEntityBase * target = CEntityBaseManager::getEntityBasePtr(_Targets[0].Target->getEntityRowId());
				if (target)
				{
					nlwarning("<CCombatPhrase::launch> launch time is too big (%u seconds), maybe due to a teleport. Actor: %s, target: %s",
						_ApplyDate - time,
						actingEntity->getId().toString().c_str(),
						target->getId().toString().c_str()
						);
				}
				_ApplyDate = 0;
			}
		}
	}

	// Action duration
	if (_Behaviour.isCombat())
	{
		if (_MeleeCombat)
			_Behaviour.Combat.ActionDuration = getActionDuration((NLMISC::TGameCycle)latency);
//		else
//			_Behaviour.Range.ActionDuration = getActionDuration((NLMISC::TGameCycle)latency);
	}
	else
	{
		_Behaviour.CreatureAttack.ActionDuration = getActionDuration((NLMISC::TGameCycle)latency);
	}

	// update the actor behaviour (must be done after target list)
	if ( _Behaviour.Behaviour != MBEHAV::UNKNOWN_BEHAVIOUR )
		PHRASE_UTILITIES::sendUpdateBehaviour( _Attacker->getEntityRowId(), _Behaviour );

	actingEntity->setCurrentAction(CLIENT_ACTION_TYPE::Combat,_LatencyEndDate);

	// set all the special combat event flags
	if ( !_BrickDefinedFlags.empty() && actingEntity->getId().getType() == RYZOMID::player )
	{
		CCharacter *player = dynamic_cast<CCharacter*> (actingEntity);
		if (!player)
		{
			nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", actingEntity->getId().toString().c_str());
		}
		else
		{
			for (uint i = 0 ; i < _BrickDefinedFlags.size() ; ++i)
			{
				player->setCombatEventFlag(_BrickDefinedFlags[i]);
			}
		}
	}

	// Display stat
	CCharacter * c = dynamic_cast<CCharacter*>(CEntityBaseManager::getEntityBasePtr(_Attacker->getEntityRowId()));
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
		//Bsi.append( StatPath, NLMISC::toString("[EAC] %s %s %d %s %d %s %s %d", c->getId().toString().c_str(), hl.toString().c_str(), qualityl, hr.toString().c_str(), qualityr, (success?"success":"miss"), SKILLS::toString(_AttackSkill).c_str(), uint(_AttackSkill) >= c->getSkills()._Skills.size()?-1:c->getSkills()._Skills[_AttackSkill].Current) );
		//EgsStat.displayNL("[EAC] %s %s %d %s %d %s %s %d", c->getId().toString().c_str(), hl.toString().c_str(), qualityl, hr.toString().c_str(), qualityr, (success?"success":"miss"), SKILLS::toString(_AttackSkill).c_str(), uint(_AttackSkill) >= c->getSkills()._Skills.size()?-1:c->getSkills()._Skills[_AttackSkill].Current);
//		EGSPD::executeActionFight(c->getId(), hl.toString(), qualityl, hr.toString(), qualityr, success, SKILLS::toString(_AttackSkill).c_str(), (uint(_AttackSkill) >= c->getSkills()._Skills.size()?-1:c->getSkills()._Skills[_AttackSkill].Current) );
	}
	return true;
} // launch //


//--------------------------------------------------------------
//					launchAttack()  
//--------------------------------------------------------------
bool CCombatPhrase::launchAttack(CEntityBase * actingEntity, bool rightHand, bool isMad, TDataSetRow madnessCaster)
{
	H_AUTO(CCombatPhrase_launchAttack);
	
#ifdef NL_DEBUG
	nlassert(actingEntity);
#endif

	// wear equipment if actor is a player
	if (actingEntity->getId().getType() == RYZOMID::player)
	{
		H_AUTO(CCombatPhrase_launchAttack_wearEquipment);
		CCharacter *playerChar = dynamic_cast<CCharacter*> (actingEntity);
		if (playerChar != NULL)
		{
			// now we use the weapon speed factor as a divisor of wear per action 
			// (a weapon twice as fast will wear twice as slow)
			nlassert(ReferenceWeaponLatencyForWear > 0);
			const uint16 latency = (rightHand ? _RightWeapon.LatencyInTicks : _LeftWeapon.LatencyInTicks);
			const float wearFactor = (float)latency / (float)ReferenceWeaponLatencyForWear;
			
			if (rightHand)
			{
				playerChar->wearRightHandItem(wearFactor);
			}
			else
			{
				playerChar->wearLeftHandItem(wearFactor);
			}
			
			// wear armor, shield and jewels
			playerChar->wearArmor(wearFactor);
			playerChar->wearShield(wearFactor);
			playerChar->wearJewels(wearFactor);
		}
		else
			nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", actingEntity->getId().toString().c_str());
	}

	const bool success = testPhraseSuccess(rightHand);
	if (!success)
	{
		if ( _Targets.empty() || _Targets[0].Target == NULL)
		{
			return false;
		}
		TDataSetRow targetRowId = _Targets[0].Target->getEntityRowId();

		// set event flag is defender is a player and is targeting current attacker
		CCharacter* playerDefender = PlayerManager.getChar(targetRowId);
		if(playerDefender && (playerDefender->getTargetDataSetRow() == actingEntity->getEntityRowId()))
		{
			playerDefender->setCombatEventFlag(BRICK_FLAGS::Miss);
		}

		// failed message
		if(_MeleeCombat)
		{
			// send a delayed flying text (so the Evade text appear when the weapon hit the body)
			// send only one time, to avoid 2 display of "Evade" flying text if right and left weapon missed
			addDelayedEvent(targetRowId, EventEvade, !_MissFlyingTextTriggered);
			_MissFlyingTextTriggered= true;
		}
		else
		{
			// in case of miss ranged attack, missile/bullets are even not launched => display a special message now
			PHRASE_UTILITIES::sendCombatFailedMessages( _Attacker->getEntityRowId(), targetRowId, PHRASE_UTILITIES::FailRange );
		}
		
		// send an ai event report for main target
		_AiEventReport.init();
		_AiEventReport.Originator = _Attacker->getEntityRowId();
		_AiEventReport.Target = targetRowId;
		_AiEventReport.Type = ACTNATURE::FIGHT;
		_AiEventReport.AggroAdd = -0.2f;

		CPhraseManager::getInstance().addAiEventReport(_AiEventReport);
	}
	else
	{
		uint32 nbTargets = (uint32)_Targets.size();
#if !FINAL_VERSION
		nlassert(nbTargets);
#endif
		bool needComputeBaseDamage = true;
		// optim if acting player has no race specific skill bonus
		if (actingEntity->getId().getType() == RYZOMID::player)
		{
			CCharacter *playerChar = dynamic_cast<CCharacter*> (actingEntity);
			if (playerChar != NULL || (playerChar->getNbNonNullClassificationTypesSkillMod() == 0))
			{
				// compute base damage
				computeBaseDamage(rightHand, EGSPD::CPeople::Unknown);
				needComputeBaseDamage = false;
			}
		}

		// launch on all targets
		for (uint i = 0 ; i < nbTargets ; ++i)
		{
			launchAttackOnTarget(i, rightHand, isMad, needComputeBaseDamage);
		}
	}

	return success;
} // launchAttack //


//--------------------------------------------------------------
//					launchAttackOnTarget()
//
//  BRIANCODE - 6:42pm 10/18/05
//  Added code to end (after all positive checks concluded and actions processed)
//  to enable mission tracking on combat phrases and stanzas
//--------------------------------------------------------------
void CCombatPhrase::launchAttackOnTarget(uint8 targetIndex, bool rightHand, bool isMad, bool needComputeBaseDamage)
{
	H_AUTO(CCombatPhrase_launchAttackOnTarget);
	
	const bool mainTarget = (targetIndex == 0);

	if (targetIndex >= _Targets.size())
	{
		nlwarning("<CCombatPhrase::launchAttackOnTarget> tried to launch action on target %u but there is %u targets", targetIndex, (uint8)_Targets.size());
		return;
	}

	CCombatDefenderPtr & combatDefender = _Targets[targetIndex].Target;
	if (combatDefender == NULL)
	{
		nlwarning("<CCombatPhrase::launchAttackOnTarget> target index %u is NULL", targetIndex);
		return;
	}

	CEntityBase * defender = combatDefender->getEntity();
	if (defender == NULL)
	{
		nlwarning("<CCombatPhrase::launchAttackOnTarget> cannot get entity base pointer");
		return;
	}
	
	// if defender is dead return doing nothing (can happen when using dual strikes or two weapons)
	if (defender->isDead())
		return;

	// if need to compute base damage, do it
	if (needComputeBaseDamage)
		computeBaseDamage(rightHand, defender->getRace());

	CEntityBase * actingEntity = CEntityBaseManager::getEntityBasePtr( _Attacker->getEntityRowId() );
	if (actingEntity == NULL)
	{
		nlwarning("<CCombatPhrase::launchAttackOnTarget> invalid entity Id %s", TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str() );
		return;
	}

	// compute delta level for xp gain
	const sint16 deltaLevel = computeDeltaLevel(combatDefender, rightHand);

	CEntityId targetId = defender->getId();
	EGSPD::CPeople::TPeople targetRace = defender->getRace();

	sint32 damage = 0;
	sint32 lostStamina = 0;
	sint32 lostSap = 0;

	bool killTarget = false;

	// get shield 
	CCombatShield shield;
	combatDefender->getShield(shield);

	// get weapon used
	CCombatWeapon & weapon = ( rightHand ? _RightWeapon : _LeftWeapon );

	// dmg type
	DMGTYPE::EDamageType dmgType = (_Ammo.Damage != 0) ? _Ammo.DmgType : weapon.DmgType;
	
	// determine localisation, only for main target, secondary targets use same localisation
	PHRASE_UTILITIES::TPairSlotShield localisation;
	if ( mainTarget )
	{
		localisation = getHitLocalisation( combatDefender, shield, dmgType );
		_HitLocalisation = localisation.first;
	}
	else
	{
		localisation.first = _HitLocalisation;
		localisation.second = true;
	}

	// add fight skill bonus given by consumable effect
	sint32 fightSkillBonus = 0;
	if ( actingEntity->getId().getType() == RYZOMID::player )
	{
		CCharacter * c = dynamic_cast<CCharacter*>(actingEntity);
		if( _MeleeCombat )	
		{
			fightSkillBonus = c->meleeSuccessModifier();
		}
		else
		{
			fightSkillBonus = c->rangeSuccessModifier();
		}
	}

	// now target can defend in range combat too
	// if attacker is mad, he cannot dodge/parry his own attack
	float dodgeFactor;
	if (!_IsMad)
		dodgeFactor = testOpponentDefense(combatDefender, rightHand, _HitLocalisation, deltaLevel+(sint16)fightSkillBonus, _Targets[targetIndex].NbParryDodgeFlyingTextRequired);
	else
		dodgeFactor = 0.0f;

	if ( _MeleeCombat && !isMad && dodgeFactor >= 1.0f )
	{
		// target has dodged
		DEBUGLOG("<CCombatPhrase::launchAttackOnTarget> %s Actor %u, target %u has dodged", (rightHand?"RightHand":"LeftHand"), _Attacker->getEntityRowId().getIndex(), defender->getEntityRowId().getIndex() );

		// set the dodge flag
		//_TargetHasDodged.set(targetIndex);

		// update the actor behaviour if main target and right hand
		if (mainTarget && rightHand)
		{
			if ( _Behaviour.isCreatureAttack() )
			{
				_Behaviour.CreatureAttack.ImpactIntensity = 0;
				_Behaviour.CreatureAttack2.HitType = HITTYPE::Failed;					
				_Behaviour.CreatureAttack2.DamageType = dmgType;
			}
			else
			{	
				if ( _MeleeCombat )
				{
					_Behaviour.Combat.HitType = HITTYPE::Failed;
					_Behaviour.Combat.ImpactIntensity = 0;
					_Behaviour.Combat.KillingBlow = 0;
					_Behaviour.Combat2.DamageType = dmgType;
				}
				else
				{
					_Behaviour.Range.ImpactIntensity = 0;
				}
			}
		}

		// update combat flags for target, ONLY if target is a player and is targeting the attacker !
		if ( (defender->getId().getType() == RYZOMID::player) &&
			 (defender->getTargetDataSetRow() == actingEntity->getEntityRowId()) )
		{
			CCharacter * character = dynamic_cast<CCharacter *>(defender);
			if (character)
			{
				// resolve this at apply time (clearer if the dodge/parry is displayed when the weapon really hits)
				if (character->dodgeAsDefense())
					addDelayedEvent(character->getEntityRowId(), EventMeleeDodgeOpening);
				else
					addDelayedEvent(character->getEntityRowId(), EventMeleeParryOpening);
			}
			else
			{
				nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", defender->getId().toString().c_str());
			}
		}

		// add aggro even if the target has dodged
		_AiEventReport.init();
		_AiEventReport.Originator = _Attacker->getEntityRowId();
		_AiEventReport.Target = defender->getEntityRowId();
		_AiEventReport.Type = ACTNATURE::FIGHT;
		_AiEventReport.AggroAdd = -0.2f;

		CPhraseManager::getInstance().addAiEventReport(_AiEventReport);
	}
	else
	{
		if (mainTarget)
		{
			if (_Behaviour.isCreatureAttack())
			{
				if (_CriticalHit)
				{
					if (_SpecialHit)
						_Behaviour.CreatureAttack2.HitType = HITTYPE::CriticalHitResidual;
					else
						_Behaviour.CreatureAttack2.HitType = HITTYPE::CriticalHit;
				}
				else
				{
					if (_SpecialHit)
						_Behaviour.CreatureAttack2.HitType = HITTYPE::HitResidual;
					else
						_Behaviour.CreatureAttack2.HitType = HITTYPE::Hit;
				}
			}
			else if ( _MeleeCombat )
			{
				if (_CriticalHit)
				{
					if (_SpecialHit)
						_Behaviour.Combat.HitType = HITTYPE::CriticalHitResidual;
					else
						_Behaviour.Combat.HitType = HITTYPE::CriticalHit;
				}
				else
				{
					if (_SpecialHit)
						_Behaviour.Combat.HitType = HITTYPE::HitResidual;
					else
						_Behaviour.Combat.HitType = HITTYPE::Hit;
				}
			}
		}

		float factor = 1.0f;

		// apply area damage reduction
		factor += _Targets[targetIndex].DamageFactor - 1.0f;

		// brick damage modifiers (only on right hand attack)
		if (rightHand)
			factor += _DamageFactorOnSuccess.applyValue(_WeaponSabrinaValue) - 1.0f;

		// if PVP, apply PVP damage factor
		if (defender->getId().getType() == RYZOMID::player && actingEntity->getId().getType() == RYZOMID::player && defender != actingEntity)
		{
			if (_MeleeCombat)
			{
				factor *= PVPMeleeCombatDamageFactor.get();
			}
			else
				factor *= PVPRangeCombatDamageFactor.get();
		}

		// add spire effect ( melee/range attack )
		if ( actingEntity->getId().getType() == RYZOMID::player )
		{
			const CSEffect* pEffect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::TotemCombatMROff );
			if ( pEffect != NULL )
			{
				factor *= ( 1.0f + pEffect->getParamValue() / 100.0f );
			}
		}
		
		damage = sint32( (_BaseDamage * factor) * (1.0f - dodgeFactor) * GlobalDebugDamageFactor.get());

		CCreature * npc = dynamic_cast<CCreature*>(defender);
		CCharacter * c = dynamic_cast<CCharacter*>(actingEntity);
		if(npc && c && !PHRASE_UTILITIES::testRange(*actingEntity, *defender, (uint32)npc->getMaxHitRangeForPC()*1000) )
		{
			damage = 0;
		}

		sint32 damageBeforeArmor = damage;
		
		DEBUGLOG("<CCombatPhrase::launchAttackOnTarget> (%s) Attacker %s, Defender %s, dodge factor = %f, _PhraseSuccessDamageFactor = %f, Total Damage before armor = %d", 
		(rightHand?"RightHand":"LeftHand"),
		_Attacker->getEntity()->getId().toString().c_str(),
		defender->getId().toString().c_str(),
		dodgeFactor,
		_PhraseSuccessDamageFactor,
		damage
		);

		sint32 attackerLevel = _Attacker->getSkillValue(_RightWeapon.Skill, targetRace);

		// check if target has a bodyguard (unless target is mad)
		if (!isMad)
		{
			const CSEffectPtr effect = defender->lookForActiveEffect( EFFECT_FAMILIES::PowerShielding );
			if ( effect )
			{
				damage = applyDamageOnBodyguard(actingEntity, defender->getEntityRowId(), attackerLevel,effect,damage,dmgType);
			}
		}

		// apply armor and shield protections
		BODY::TBodyPart hitBodyPart = BODY::getBodyPart(EGSPD::getBodyType(defender->getRace()), localisation.first);		

		bool shieldIsEffective = testShieldEfficiency(combatDefender, shield);

		// compute part of base damage that inflicts damage, then compute total damage
		sint32 inflictedNaturalDamage = applyArmorProtections(combatDefender, sint32(_NaturalDamage * _PhraseSuccessDamageFactor), dmgType, _HitLocalisation, shieldIsEffective);
		damage = applyArmorProtections(combatDefender, damage, dmgType, _HitLocalisation, shieldIsEffective);

		// Water Wall Aura
		{
			const CSEffectPtr effect = defender->lookForActiveEffect( EFFECT_FAMILIES::PowerWaterWall );
			if( effect )
			{
				sint32 damageAbsorbtion = effect->getParamValue();
				damage -=  (sint32)( (float)damage * (float)damageAbsorbtion/100.f);
			}
		}

		if (mainTarget)
		{
			if (_Behaviour.isCreatureAttack())
			{
				_Behaviour.CreatureAttack.Localisation = hitBodyPart;
				_Behaviour.CreatureAttack2.DamageType = dmgType;

				uint16 intensity = getImpactIntensity(_WeaponSabrinaValue);
				if (intensity > _Behaviour.CreatureAttack.ImpactIntensity)
				{
					_Behaviour.CreatureAttack.ImpactIntensity = intensity;
				}

				intensity = 0;
				// if creature behaviour, set MagicImpactIntensity to force missile display if any
				if (actingEntity->getId().getType() == RYZOMID::creature)
				{
					// Creature -> intensity depends of creature level
					const CCreature *creature = dynamic_cast<CCreature*> (actingEntity);
					if (!creature)
					{
						nlwarning("Entity %s type is creature but dynamic_cast in CCreature * returns NULL ?!", actingEntity->getId().toString().c_str());
					}
					else
					{
						const CStaticCreatures* form = creature->getForm();
						if (form)
							intensity = form->getAttackLevel();
					}
				}
				_Behaviour.CreatureAttack.MagicImpactIntensity = 1 + (intensity/50);
			}
			else
			{
				if ( _MeleeCombat )
				{
					_Behaviour.Combat2.DamageType = dmgType;
					_Behaviour.Combat.Localisation = hitBodyPart;

					uint16 intensity = getImpactIntensity(_WeaponSabrinaValue);
					if (intensity > _Behaviour.Combat.ImpactIntensity)
					{
						_Behaviour.Combat.ImpactIntensity = intensity;
					}
				}
				else
				{
					_Behaviour.Range.Localisation = hitBodyPart;

					uint16 intensity = getImpactIntensity(_WeaponSabrinaValue);
					if (intensity > _Behaviour.Range.ImpactIntensity)
					{
						_Behaviour.Range.ImpactIntensity = intensity;
					}
				}
			}

			if (damage > 0)
			{
				if (_MeleeCombat)
				{
					// in melee combat, post action delay is null: we know the hp the defender will have at the apply time
					if (defender->currentHp() + _Behaviour.DeltaHP > 0)
						_Behaviour.DeltaHP -= (sint16)damage;
				}
				else
				{
					_Behaviour.DeltaHP -= (sint16)damage;
				}

				killTarget = ( defender->currentHp() <= -_Behaviour.DeltaHP );
				if (killTarget && mainTarget)
				{
					_Behaviour.Combat.KillingBlow = 1;
				}
			}
		}

		TApplyAction action;
		action.Target						= combatDefender;
		action.MainTarget					= mainTarget;
		action.DeltaLevel					= deltaLevel;
		action.DodgeFactor					= dodgeFactor;
		action.HitLocalisation				= localisation.first;
		action.InflictedDamageBeforeArmor	= damageBeforeArmor;
		action.InflictedNaturalDamage		= inflictedNaturalDamage;
		action.InflictedDamage				= damage;

		if (rightHand)
			_RightApplyActions.push_back(action);
		else
			_LeftApplyActions.push_back(action);

	}

} // launchAttackOnTarget //


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CCombatPhrase::apply()
{
	H_AUTO(CCombatPhrase_apply);
	
	if (!_Attacker)
	{
		nlwarning("<CCombatPhrase::apply> found NULL attacker.");
		return;
	}

	if (_Targets.empty())
	{
		nlwarning("<CCombatPhrase::apply> combat action has no target.");
		return;
	}

	CEntityBase * actingEntity = CEntityBaseManager::getEntityBasePtr( _Attacker->getEntityRowId() );
	if (actingEntity == NULL)
	{
		nlwarning("<CCombatPhrase::apply> invalid entity Id %s", TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str() );
		return;
	}

	// apply any special events now
	flushDelayedEvents();
	
	// ??? must be done after flushDelayedEvents
	_Behaviour.DeltaHP -= 10;

	// first apply right hand attack, then left
	applyAttack(actingEntity, true);
	applyAttack(actingEntity, false);
	
	// clear all actions
	_RightApplyActions.clear();
	_LeftApplyActions.clear();

	// clear all targets but main target
	_Targets.resize(1);

} // apply //


//--------------------------------------------------------------
//					applyAttack()
//--------------------------------------------------------------
void CCombatPhrase::applyAttack(CEntityBase * actingEntity, bool rightHand)
{
	H_AUTO(CCombatPhrase_applyAttack);
	
	std::vector<TApplyAction> & actions = rightHand ? _RightApplyActions : _LeftApplyActions;

	const uint nbActions = (uint)actions.size();

	// if there is no action for this hand, the attack failed
	if (nbActions == 0)
		return;

	std::vector<TReportAction> actionReports;
	actionReports.reserve(nbActions);

	for (uint i = 0; i < nbActions; i++)
	{
		applyAction(actions[i], actionReports, rightHand);
	}

	const uint nbReports = (uint)actionReports.size();

	// if attacker is mad, change reports
	if (_IsMad)
	{
		for (uint i = 0; i < nbReports; i++)
		{
			if (_MadSkill != SKILLS::unknown)
				actionReports[i].Skill = _MadSkill;
			actionReports[i].ActorRowId = _MadnessCaster;
			actionReports[i].factor = 0.f;
		}
	}

	// send event report for xp gain, do it before killing targets
	for (uint i = 0; i < nbReports; i++)
	{
		bool mainTarget = (actionReports[i].TargetRowId == _Targets[0].Target->getEntityRowId());
		PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport(actionReports[i], mainTarget);
		PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(actionReports[i]);
	}

	// apply damage
	{
		H_AUTO(CCombatPhrase_applyAttack_applyDamage);
		for (uint i = 0; i < nbActions; i++)
		{
			H_AUTO(CCombatPhrase_applyDamagePerAction);
			if (actions[i].InflictedDamage > 0)
			{
				BOMB_IF( (actions[i].Target == NULL), "<CCombatPhrase::applyAttack> target should not be NULL", continue );

				CEntityBase * defender = actions[i].Target->getEntity();
				if (defender == NULL)
					continue;

				defender->changeCurrentHp( -actions[i].InflictedDamage, actingEntity->getEntityRowId() );

				// update target infos
				const uint targetIndex = actions[i].Target->getTargetIndex();
				BOMB_IF( (targetIndex >= _Targets.size()), "<CCombatPhrase::applyAttack> invalid target index", continue );

				_Targets[targetIndex].InflictedNaturalDamage	= actions[i].InflictedNaturalDamage;
				_Targets[targetIndex].InflictedDamage			= actions[i].InflictedDamage;
				_Targets[targetIndex].DodgeFactor				= actions[i].DodgeFactor;
			}
		}
	}

	if (rightHand)
	{
		// apply combat special actions
		applyCombatActions();
	}

} // applyAttack //


//--------------------------------------------------------------
//					applyAction()
//--------------------------------------------------------------
void CCombatPhrase::applyAction(TApplyAction & action, std::vector<TReportAction> & actionReports, bool rightHand)
{
	H_AUTO(CCombatPhrase_applyAction);
	
	CCombatDefenderPtr & combatDefender = action.Target;
	if (combatDefender == NULL)
	{
		nlwarning("<CCombatPhrase::applyAction> target is NULL");
		return;
	}

	CEntityBase * defender = combatDefender->getEntity();
	if (defender == NULL)
	{
		nlwarning("<CCombatPhrase::applyAction> cannot get entity base pointer of the defender");
		return;
	}

	// if defender is dead return doing nothing (can happen when using dual strikes or two weapons)
	if (defender->isDead())
		return;

	CEntityBase * actingEntity = CEntityBaseManager::getEntityBasePtr( _Attacker->getEntityRowId() );
	if (actingEntity == NULL)
	{
		nlwarning("<CCombatPhrase::applyAction> invalid entity Id %s", TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str() );
		return;
	}

	bool sendAggro = true;
	CCreature * npc = dynamic_cast<CCreature*>(defender);
	if(npc)
	{
		CCharacter * c = dynamic_cast<CCharacter*>(actingEntity);
		if( c )
		{
			if(!PHRASE_UTILITIES::testRange(*actingEntity, *defender, (uint32)npc->getMaxHitRangeForPC()*1000))
			{
				action.InflictedDamageBeforeArmor = 0;
				action.InflictedDamage = 0;
				sendAggro = false;
				_Behaviour.DeltaHP = 0;
				c->sendDynamicSystemMessage(c->getId(), "UNEFFICENT_RANGE");
			}
		}
	}

	CEntityId targetId = defender->getId();
	EGSPD::CPeople::TPeople targetRace = defender->getRace();

	_AiEventReport.init();
	_AiEventReport.Originator = _Attacker->getEntityRowId();
	_AiEventReport.Target = combatDefender->getEntityRowId();
	_AiEventReport.Type = ACTNATURE::FIGHT;

	const sint32 & damageBeforeArmor = action.InflictedDamageBeforeArmor;
	sint32 &damage = action.InflictedDamage;

	sint32 lostStamina = 0;
	sint32 lostSap = 0;

	bool killTarget = false;

	// get weapon used
	CCombatWeapon & weapon = ( rightHand ? _RightWeapon : _LeftWeapon );

	// dmg type
	DMGTYPE::EDamageType dmgType = (_Ammo.Damage != 0) ? _Ammo.DmgType : weapon.DmgType;

	if ( _MeleeCombat && !_IsMad && action.DodgeFactor >= 1.0f )
	{
		nlwarning("<CCombatPhrase::applyAction> dodged combat attacks should not be added to apply actions.");
		return;
	}

	if (action.MainTarget && _CriticalHit)
	{
		if (_Behaviour.isCreatureAttack())
		{
			PHRASE_UTILITIES::sendCriticalHitMessage( _Attacker->getEntityRowId(), defender->getEntityRowId() );
		}
		else if (_MeleeCombat)
		{
			PHRASE_UTILITIES::sendCriticalHitMessage( _Attacker->getEntityRowId(), defender->getEntityRowId() );
		}
	}

	const sint32 attackerLevel = _Attacker->getSkillValue(_RightWeapon.Skill, targetRace);

	// check bounce effect (only for main target)
	if (!_MeleeCombat && action.MainTarget)
	{
		const CSEffectPtr effect = defender->lookForActiveEffect( EFFECT_FAMILIES::Bounce );
		if ( effect )
		{
			applyBounceEffect(actingEntity, attackerLevel, effect, damage, dmgType);
		}
	}
	
	if( targetId.getType() == RYZOMID::player )
	{
		damage = defender->applyDamageOnArmor( dmgType, damage );
	}

	// apply damage
	if (damage > 0)
	{
		// get hit body part
		BODY::TBodyPart hitBodyPart = BODY::getBodyPart(EGSPD::getBodyType(defender->getRace()), action.HitLocalisation);

		// test death but do not remove HP right now as report must be sent before killing target
		killTarget = ( defender->currentHp() <= damage );
		// target do not die, check special effect and stamina and sap loss
		if ( !killTarget )
		{
			// trigger special effect and break cast
			CPhraseManager::getInstance().breakCast(attackerLevel,actingEntity,defender);
			if( hitBodyPart != BODY::UnknownBodyPart )
			{
				applyLocalisationSpecialEffect(combatDefender, action.HitLocalisation, damage, lostStamina);
			}

			// sta and sap loss
			if (_StaminaLossDynFactor.PowerValue > 0 )
			{
				lostStamina += (sint32)( damage * _StaminaLossDynFactor.applyValue(_WeaponSabrinaValue) );
			}
			
			if (_SapLossDynFactor.PowerValue > 0 )
			{
				lostSap += (sint32)( damage * _SapLossDynFactor.applyValue(_WeaponSabrinaValue) );
			}
			
			if ( lostStamina != 0 )
			{
				INFOLOG("<CCombatPhrase::applyAction> entity %s lose %d stamina", TheDataset.getEntityId(defender->getEntityRowId()).toString().c_str(),lostStamina );
				defender->getPhysScores()._PhysicalScores[SCORES::stamina].Current = defender->getPhysScores()._PhysicalScores[SCORES::stamina].Current - lostStamina;
				
				// clip score to 0
				if ( defender->getPhysScores()._PhysicalScores[SCORES::stamina].Current < 0 )
					defender->getPhysScores()._PhysicalScores[SCORES::stamina].Current = 0;
				
				// add modifier to sentence AI event reports
				_AiEventReport.addDelta(AI_EVENT_REPORT::Stamina, (-1)*lostStamina);
			}
			if ( lostSap != 0 )
			{
				//value = toString( _BaseSapAbsorption[i] );
				INFOLOG("<CCombatPhrase::applyAction> entity %s lose %d sap", TheDataset.getEntityId(defender->getEntityRowId()).toString().c_str(), lostSap );
				defender->getPhysScores()._PhysicalScores[SCORES::sap].Current = defender->getPhysScores()._PhysicalScores[SCORES::sap].Current - lostSap;
				
				// clip score to 0
				if ( defender->getPhysScores()._PhysicalScores[SCORES::sap].Current < 0 )
					defender->getPhysScores()._PhysicalScores[SCORES::sap].Current = 0;
				
				// add modifier to sentence AI event reports
				_AiEventReport.addDelta(AI_EVENT_REPORT::Sap, (-1)*lostSap);
			}
			
			// send hit message
			PHRASE_UTILITIES::sendHitMessages(actingEntity->getId(),targetId, _IsMad, damage, damageBeforeArmor, lostStamina, lostSap, hitBodyPart);
		}
		// target die from hit, only display HP damage and do not check special effect because of damage loc and critical hit
		else
		{
			PHRASE_UTILITIES::sendHitMessages(actingEntity->getId(),targetId, _IsMad, damage, damageBeforeArmor, 0, 0, hitBodyPart);
			PHRASE_UTILITIES::sendDeathMessages( actingEntity->getId(), defender->getId() );
		}

		if(sendAggro)
		{
			// add report
			TReportAction report;
			report.ActorRowId = _Attacker->getEntity()->getEntityRowId();
			if( defender ) report.TargetRowId = defender->getEntityRowId();
			report.ActionNature = ACTNATURE::FIGHT;
			report.DeltaLvl = action.DeltaLevel;
			report.Skill = weapon.Skill;
			report.factor = 1.0f - action.DodgeFactor;
			report.Hp = std::min(damage, defender->currentHp());
			report.Sta = lostStamina;
			report.Sap = lostSap;
			actionReports.push_back(report);
		}
	}
	else
		PHRASE_UTILITIES::sendHitMessages( actingEntity->getId(), targetId, _IsMad, 0, 0, 0, 0);

	INFOLOG("<CCombatPhrase::applyAction> %s Entity %s hits entity %s does %u damage",
			(rightHand?"RightHand":"LeftHand"),
			actingEntity->getId().toString().c_str(),
			defender->getId().toString().c_str(), 
			damage 
			);

	// manage combat event flags for players
	if ( actingEntity->getId().getType() == RYZOMID::player && action.MainTarget && !killTarget ) 
	{
		CCharacter * player = dynamic_cast<CCharacter *>(actingEntity);
		if (player)
		{
			if (_CriticalHit)
			{
				player->setCombatEventFlag(BRICK_FLAGS::CriticalHit);
//				sint8 prop = sint8(player->_PropertyDatabase.getProp("FLAGS:CRITICAL") + 1);
				sint8 prop = sint8(CBankAccessor_PLR::getFLAGS().getCRITICAL(player->_PropertyDatabase)+1);
				if (prop == 0) prop++; // avoid 0 to bugfix flying text poping at startup
				//player->_PropertyDatabase.setProp("FLAGS:CRITICAL", prop);
				CBankAccessor_PLR::getFLAGS().setCRITICAL(player->_PropertyDatabase, prop);
				// Force update of the BRICK_FLAGS DB. important to make BRICK_FLAGS and CRITICAL synchronized
				player->updateBrickFlagsDBEntry();
			}

			player->setCombatEventFlag(BRICK_FLAGS::Hit);
			player->setCombatEventFlag(BRICK_FLAGS::slotToFlag(action.HitLocalisation));
		}
	}

	// test defender damage shield
	if (_MeleeCombat && defender->getDamageShieldDamage() > 0)
	{
		if ( defender->getDamageShieldHpDrain() > 0 && !defender->isDead() )
		{
			defender->changeCurrentHp(defender->getDamageShieldHpDrain());
			PHRASE_UTILITIES::sendDamageShieldDamageMessages( actingEntity->getId(), defender->getId(),defender->getDamageShieldDamage(), defender->getDamageShieldHpDrain());
		}
		else
			PHRASE_UTILITIES::sendDamageShieldDamageMessages( actingEntity->getId(), defender->getId(),defender->getDamageShieldDamage(), 0);

		if ( actingEntity->changeCurrentHp( (-1) * defender->getDamageShieldDamage(), defender->getEntityRowId()) )
		{
			PHRASE_UTILITIES::sendDeathMessages(defender->getId(), actingEntity->getId());
		}
	}

	// test some racial auras
	if ( _MeleeCombat && damage > 0 )
	{
		// FIRE WALL
		const CSEffect * effect = defender->lookForActiveEffect( EFFECT_FAMILIES::PowerFireWall );
		if( effect )
		{
			sint32 damage = effect->getParamValue();
			if ( actingEntity->changeCurrentHp( (-1)*damage, defender->getEntityRowId() ) )
			{
				PHRASE_UTILITIES::sendHitMessages(defender->getId(),actingEntity->getId(), false, damage, damage, 0, 0);
				PHRASE_UTILITIES::sendDeathMessages( defender->getEntityRowId(), actingEntity->getEntityRowId() );
			}
			else
			{
				PHRASE_UTILITIES::sendHitMessages(defender->getId(),actingEntity->getId(), false, damage, damage, 0, 0);
			}
		}
		
	
		// THORN WALL
		effect = defender->lookForActiveEffect( EFFECT_FAMILIES::PowerThornWall );
		if( effect )
		{
			sint32 slowAttackFactor = effect->getParamValue();
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( HitArmsSlowDuration / CTickEventHandler::getGameTimeStep() );

			const CSEffectPtr attackerEffect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::CombatAttackSlow );
			if( attackerEffect )
			{
				CSimpleEffect * _attackerEffect = dynamic_cast<CSimpleEffect *> (& (*attackerEffect));
				_attackerEffect->setEndDate( CTickEventHandler::getGameCycle() + duration );
			}
			else				
			{
				CCombatActionSimpleEffect attackSlow( defender->getEntityRowId(), this, duration, EFFECT_FAMILIES::CombatAttackSlow, slowAttackFactor, (uint8)(abs(slowAttackFactor)) );
				attackSlow.applyOnEntity(actingEntity,1.0f);
			}		
		}
		
		// LIGHTNING WALL
		effect = defender->lookForActiveEffect( EFFECT_FAMILIES::PowerLightningWall );
		if( effect )
		{
			sint32 debuffCombatSkillValue = effect->getParamValue();
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( HitHandsDebuffDuration / CTickEventHandler::getGameTimeStep() );
			
			const CSEffectPtr attackerEffect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffCombatSkills );
			if( attackerEffect )
			{
				CSimpleEffect * _attackerEffect = dynamic_cast<CSimpleEffect *> (& (*attackerEffect));
				_attackerEffect->setEndDate( CTickEventHandler::getGameCycle() + duration );
			}
			else				
			{
				CCombatActionSimpleEffect debuffCombatSkills( defender->getEntityRowId(), this, duration, EFFECT_FAMILIES::CombatDebuffCombatSkills, debuffCombatSkillValue, (uint8)(abs(debuffCombatSkillValue)) );
				debuffCombatSkills.applyOnEntity(actingEntity,1.0f);
			}		
		}

		// Weapon enchantment/auras
		effect = actingEntity->lookForActiveEffect(EFFECT_FAMILIES::PowerEnchantWeapon);
		if (effect)
		{
			CEnchantWeaponEffect const* enchant = static_cast<CEnchantWeaponEffect const*>((CSEffect*)effect);
			
			CMagicActionBasicDamage::CTargetInfos targetInfos;
			targetInfos.RowId = defender->getEntityRowId();
			targetInfos.MainTarget = /*mainTarget*/true;
		//	targetInfos.DmgHp = (sint32)enchant->getDmgBonus();
		//	targetInfos.ResistFactor = 1.f;
		//	targetInfos.DmgFactor = 0.f;
			targetInfos.Immune = false;
		//	targetInfos.ReportAction = TReportAction;
			targetInfos.ReportAction.ActorRowId = actingEntity->getEntityRowId();
			targetInfos.ReportAction.TargetRowId = defender->getEntityRowId();
			targetInfos.ReportAction.ActionNature = ACTNATURE::OFFENSIVE_MAGIC;
			targetInfos.ReportAction.DeltaLvl = 0/*can be computed*/;
			targetInfos.ReportAction.Skill = weapon.Skill;
			targetInfos.ReportAction.factor = 1.f;
			targetInfos.ReportAction.Hp = 0;
			targetInfos.ReportAction.Sta = 0;
			targetInfos.ReportAction.Sap = 0;
			targetInfos.ReportAction.Focus = 0;
			
			// computeMagicResistance returns true if the spell has been resisted
			if (!CMagicActionBasicDamage::computeMagicResistance(defender, targetInfos, attackerLevel, enchant->getDmgType(), (sint32)enchant->getDmgBonus(), actingEntity, /*rangeFactor*/1.f, /*powerFactor*/0.f))
			{
				if (!CMagicActionBasicDamage::applyOnEntity(
					/*phrase*/NULL,
					actingEntity,
					defender,
					/*vamp*/0,
					/*vampRatio*/0,
					targetInfos,
					
					enchant->getDmgType(),
					(sint32)enchant->getDmgBonus(),
					/*_DmgSap*/0,
					/*_DmgSta*/0,
					/*_VampirismValue*/0))
				{
					if (targetInfos.ReportAction.Hp != 0 || targetInfos.ReportAction.Sap != 0 || targetInfos.ReportAction.Sta != 0 || targetInfos.ReportAction.Focus != 0)
					{
						PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport(targetInfos.ReportAction);
						PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(targetInfos.ReportAction);
					}
					_Behaviour.DeltaHP -= (sint16)targetInfos.DmgHp;
					if ( _Behaviour.Behaviour != MBEHAV::UNKNOWN_BEHAVIOUR )
						PHRASE_UTILITIES::sendUpdateBehaviour( actingEntity->getEntityRowId(), _Behaviour );
				}
			}
		}
	}
	
	// Procs
	if( damage > 0 )
	{
		std::vector<SItemSpecialEffect> effects;
		if ( _Attacker->getEntity()->getId().getType() == RYZOMID::player )
		{
			CCharacter* c = dynamic_cast<CCharacter*>(_Attacker->getEntity());
			effects = c->lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::ISE_FIGHT_VAMPIRISM);
		}
		if ( _Attacker->getEntity()->getId().getType() == RYZOMID::creature )
		{
			CGameItemPtr usedItem;
			CCreature* c = dynamic_cast<CCreature*>(_Attacker->getEntity());
			usedItem = rightHand?c->getRightHandItem():c->getLeftHandItem();
			if (usedItem!=NULL && usedItem->getStaticForm() && usedItem->getStaticForm()->ItemSpecialEffects && !usedItem->getStaticForm()->ItemSpecialEffects->Effects.empty())
				effects = usedItem->getStaticForm()->lookForEffects(ITEM_SPECIAL_EFFECT::ISE_FIGHT_VAMPIRISM);
		}
		std::vector<SItemSpecialEffect>::const_iterator it, itEnd;
		for (it=effects.begin(), itEnd=effects.end(); it!=itEnd; ++it)
		{
			float rnd = RandomGenerator.frand();
			if (rnd<it->EffectArgFloat[0])
			{
				actingEntity->changeCurrentHp(damage);
				PHRASE_UTILITIES::sendItemSpecialEffectProcMessage(ITEM_SPECIAL_EFFECT::ISE_FIGHT_VAMPIRISM, actingEntity, defender, damage);
			}
		}
	}

	// compute aggro
	computeAggro(_AiEventReport, defender, damage,lostStamina, lostSap);

} // applyAction //


//--------------------------------------------------------------
//					end()  
//--------------------------------------------------------------
void CCombatPhrase::end()
{
	H_AUTO(CCombatPhrase_end);
	
	if (!_Attacker) return;

	CEntityBase *attacker = _Attacker->getEntity();
	if (!attacker)
		return;

	_Attacker->unlockRightItem();
	
	// set the attacks flag of the acting entity
	attacker->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
	attacker->clearCurrentAction();

	// disengage if attacker main target is dead or invalid
	bool targetValid = false;
	CEntityBase *target = CEntityBaseManager::getEntityBasePtr(attacker->getTargetDataSetRow());
	if (target)
	{
		string errorCode;
		if ( !target->isDead() && PHRASE_UTILITIES::testOffensiveActionAllowed(attacker->getId(),target->getId(), errorCode, true) == true)
		{
			targetValid = true;
		}
	}

	if (_DisengageOnEnd || !targetValid)
	{
		CPhraseManager::getInstance().disengage( _Attacker->getEntityRowId(), false, true);
	}	
} // end //

//--------------------------------------------------------------
//					stop()  
//--------------------------------------------------------------
void CCombatPhrase::stop()
{
	H_AUTO(CCombatPhrase_stop);
	
	if (!_Attacker) return;

	// only set latency end date if it has been set
	if ( _LatencyEndDate > 0)
	{
		CCharacter *character = PlayerManager.getChar(_Attacker->getEntityRowId());
		if (character)
		{
			character->dateOfNextAllowedAction( _LatencyEndDate );
		}
	}

	end();
} // stop //


//--------------------------------------------------------------
//					computeDeltaLevel()  
//--------------------------------------------------------------
sint16 CCombatPhrase::computeDeltaLevel(CCombatDefenderPtr &combatDefender, bool rightHand)
{
	H_AUTO(CCombatPhrase_computeDeltaLevel);
	
	if (combatDefender.isNull()) return 0;
	
	CEntityBase *attacker = _Attacker->getEntity(); 
	if ( !attacker )
		return 0;
	
	CEntityBase *defender = combatDefender->getEntity(); 
	if ( !defender )
		return 0;

	sint32 attackerSkillBeforeMod = 0;
	const CCombatWeapon &weapon = (rightHand ? _RightWeapon : _LeftWeapon);

	attackerSkillBeforeMod = _Attacker->getSkillBaseValue(weapon.Skill);

	if (defender->getId().getType() != RYZOMID::player)
	{
		CCreature *creatureDefender = dynamic_cast<CCreature*> (defender);
		if (creatureDefender)
		{
			const CStaticCreatures* form = creatureDefender->getForm();
			if (form)
			{
				return sint16(attackerSkillBeforeMod - form->getXPLevel());
			}
		}
	}

	const sint32 defenseLevel = combatDefender->getBaseDefenseValue();

	return sint16 (attackerSkillBeforeMod - defenseLevel);

	/*const SKILLS::ESkills skill = combatDefender->getDefenseSkill();

	if (defender->getId().getType() == RYZOMID::player)
	{
		CCharacter *player = dynamic_cast<CCharacter*> (defender);
		if (player)
			return sint16(attackerSkillBeforeMod - player->getSkillEquivalentDodgeValue(skill));
	}

	return sint16(attackerSkillBeforeMod - combatDefender->getSkillBaseValue(skill));
	*/
} // computeDeltaLevel //

//--------------------------------------------------------------
//					testOpponentDefense()  
//--------------------------------------------------------------
float CCombatPhrase::testOpponentDefense(CCombatDefenderPtr &combatDefender, bool useRightHand, SLOT_EQUIPMENT::TSlotEquipment slot, sint16 deltaLevel, sint32	&nbParryDodgeFlyingTextRequired)
{
	H_AUTO(CCombatPhrase_testOpponentDefense);
	
	if (combatDefender.isNull()) return 1.0f;
	
	CEntityBase *attacker = _Attacker->getEntity(); 
	if ( !attacker )
		return 1.0f;

	CEntityBase *defender = combatDefender->getEntity(); 
	if ( !defender )
		return 1.0f;

	// if attacker == defender, return 0
	if (attacker == defender)
		return 0.0f;

	// test defender can defend
	if (!defender->canEntityDefend())
		return 0.0f;
	
	const TDataSetRow targetRowId = defender->getEntityRowId();
	const EGSPD::CPeople::TPeople targetRace = defender->getRace();
	const EGSPD::CPeople::TPeople attackerRace = attacker->getRace();

	// get weapon to use
	const CCombatWeapon &weapon = (useRightHand?_RightWeapon:_LeftWeapon);

	sint32 attackerLevel = 0;
	const sint32 attackerSkillBeforeMod = _Attacker->getSkillValue(weapon.Skill, targetRace);
	const sint32 defenderSkillBeforeMod = combatDefender->getDefenseValue(attackerRace);

	// get skill debuff effect on melee/range
	sint32 attackMod = 0;
	if (_MeleeCombat)
	{
		// do not use smart pointers for local use only
		const CSEffect *effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMelee );
		if ( effect )
			attackMod +=  effect->getParamValue(); // effect value must be <0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffCombatSkills );
		if ( effect )
			attackMod +=  effect->getParamValue(); // effect value must be <0 on a debuff		

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::OutpostCombat );
		if ( effect )
			attackMod += effect->getParamValue();

	}
	else
	{
		// do not use smart pointers for local use only
		const CSEffect *effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillRange );
		if ( effect )
			attackMod +=  effect->getParamValue(); // effect value must be <0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffCombatSkills );
		if ( effect )
			attackMod +=  effect->getParamValue(); // value must be < 0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::OutpostCombat );
		if ( effect )
			attackMod += effect->getParamValue();
		
	}
	
	attackerLevel = attackerSkillBeforeMod + attackMod;

	if (useRightHand)
		attackerLevel += _AttackSkillModifier.applyValue(_WeaponSabrinaValue);

	if (attackerLevel < 0)
		attackerLevel = 0;

	// compute melee  weapon reach bonus/malus to apply on attacker, only in melee
	if (_MeleeCombat)
	{
		// test if defender is engaged in a melee combat againt this attacker
		TDataSetRow engagedEntity = CPhraseManager::getInstance().getEntityEngagedMeleeBy(defender->getEntityRowId());
		if (engagedEntity == attacker->getEntityRowId())
		{
			sint32 reachDelta = 0;
			if (defender->getId().getType() == RYZOMID::player)
			{
				CCharacter *player = dynamic_cast<CCharacter*> (defender);
				if (!player)
				{
					nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", defender->getId().toString().c_str());
					reachDelta = 0;
				}
				else
				{
					CGameItemPtr item = player->getRightHandItem();
					if (item != NULL && item->getStaticForm() != NULL )
					{
						const CStaticItem *form = item->getStaticForm();
						if (form->MeleeWeapon)
							reachDelta = weapon.ReachValue - form->MeleeWeapon->ReachValue;
					}
					else
						reachDelta = weapon.ReachValue - HandToHandReachValue;
				}
			}
			else
			{
				CCreature *creature = dynamic_cast<CCreature*> (defender);
				if (!creature)
				{
					nlwarning("Entity %s type is creature but dynamic_cast in CCreature * returns NULL ?!", defender->getId().toString().c_str());
					reachDelta = 0;
				}
				else
				{
					if (creature->getForm() != NULL)
					{
						reachDelta = weapon.ReachValue - creature->getForm()->getMeleeReachValue();
					}
				}
			}

			// only consider malus
			if (reachDelta < 0)
				attackerLevel += reachDelta;
		}
	}

	// compute defender malus when facing several opponents
	const set<TDataSetRow> &aggressors = CPhraseManager::getInstance().getMeleeAggressors(targetRowId);
	sint32 defenseModifier = 0;
	uint32 nb = (uint32)aggressors.size();
	if (nb > NbOpponentsBeforeMalus)
	{
		defenseModifier = (nb - NbOpponentsBeforeMalus) * ModPerSupernumeraryOpponent;
	}
	
	// take protected slot into account for defense skills modifier
	defenseModifier += PHRASE_UTILITIES::getDefenseLocalisationModifier( slot, defender->protectedSlot() );

	sint32 defenderLevel = max( sint32((defenderSkillBeforeMod + defenseModifier )), (sint32)0);

	// apply equipment modifiers, dodge is mandatory when defending against range attacks
	if ( !_MeleeCombat || combatDefender->getEntity()->dodgeAsDefense())
	{
		defenderLevel += _Attacker->adversaryDodgeModifier();
//		defenderLevel += combatDefender->dodgeModifier();
	}
	else
	{
		defenderLevel += _Attacker->adversaryParryModifier();
//		defenderLevel += combatDefender->parryModifier();
	}
	
	if (defenderLevel < 0) 
		defenderLevel = 0;

	// oposition test
	uint8 chances = 0;
	float dodgeFactor = 0.0f;
	uint8 test = (uint8)RandomGenerator.rand(99);
	#if !FINAL_VERSION
		if(CombatForceDodgeParry)
			test= 0;
	#endif


	uint8 chanceBonus = 0;

	// check target nature to use right table
	const sint32 delta = defenderLevel - attackerLevel;
	if (defender->getId().getType() == RYZOMID::player)
	{
		chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::FightDefense, delta );
		dodgeFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::FightDefense, delta, test);
	}
	else
	{
		chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::FightDefenseAI, delta);
		dodgeFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::FightDefenseAI, delta, test);
	}

	if ( combatDefender->getEntity()->getId().getType() == RYZOMID::player )
	{
		// add spire effect ( dodge/parry )
		const CSEffect* pEffect = combatDefender->getEntity()->lookForActiveEffect( EFFECT_FAMILIES::TotemCombatPar );
		if ( pEffect != NULL )
		{
			chanceBonus += ( chances * pEffect->getParamValue() / 100 );
		} 
	}

	chances += chanceBonus;
	
	// XP LOG
	if (PlayerManager.logXPGain(attacker->getEntityRowId()))
	{
		if (dodgeFactor < 1.0f)
		{
			nlinfo("[XPLOG] Combat action Player %s, target %s, skill %s, delta %d, Dodge chances %u, tirage %u, damage factor = %f",attacker->getId().toString().c_str(), 
				defender->getId().toString().c_str(), SKILLS::toString(weapon.Skill).c_str(), defenderLevel - attackerLevel, chances,test,1.0f-dodgeFactor );
		}
		else
		{
			nlinfo("[XPLOG] Combat action Player %s, target %s, skill %s, delta %d, Dodge chances %u, tirage %u -> Attack Dodged ",attacker->getId().toString().c_str(), 
				defender->getId().toString().c_str(), SKILLS::toString(weapon.Skill).c_str(), defenderLevel - attackerLevel, chances,test);
		}		
	}

	if ( dodgeFactor >= 1.0f )
	{
		// avoid sending 2 flying text if right and left melee weapons failed
		bool	sendFlyingText= (nbParryDodgeFlyingTextRequired++)==0;
		// send an event at apply time
		if ( !combatDefender->getEntity()->dodgeAsDefense() )
			addDelayedEvent(targetRowId, EventParry, sendFlyingText);
		else
			addDelayedEvent(targetRowId, EventDodge, sendFlyingText);
	}

	return dodgeFactor;
} // testOpponentDefense //


//--------------------------------------------------------------
//					testShieldEfficiency()  
//--------------------------------------------------------------
bool CCombatPhrase::testShieldEfficiency(CCombatDefenderPtr &combatDefender, CCombatShield &shield)
{
	H_AUTO(CCombatPhrase_testShieldEfficiency);
	
	nlassert(_Attacker != NULL);

	if (combatDefender.isNull()) 
		return false;

	if (shield.ShieldType == SHIELDTYPE::NONE)
		return false;
	
	CEntityBase *attacker = _Attacker->getEntity(); 
	if ( !attacker )
		return true;

	CEntityBase *defender = combatDefender->getEntity(); 
	if ( !defender )
		return true;

	// test defender can defend
	if (!defender->canEntityDefend())
	{
		return false;
	}	

	TDataSetRow targetRowId = defender->getEntityRowId();
	EGSPD::CPeople::TPeople targetRace = defender->getRace();

	sint32 attackerLevel = 0;
	
	// get skill debuff effect on melee/range
	sint32 attackMod = 0;
	if (_MeleeCombat)
	{
		// do not use smart pointers for local use only
		const CSEffect *effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMelee );
		if ( effect )
			attackMod +=  effect->getParamValue(); // effect value must be <0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffCombatSkills );
		if ( effect )
			attackMod +=  effect->getParamValue(); // effect value must be <0 on a debuff		

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::OutpostCombat );
		if ( effect )
			attackMod += effect->getParamValue();
	}
	else
	{
		// do not use smart pointers for local use only
		const CSEffect *effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillRange );
		if ( effect )
			attackMod +=  effect->getParamValue(); // effect value must be <0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffCombatSkills );
		if ( effect )
			attackMod +=  effect->getParamValue(); // value must be < 0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::OutpostCombat );
		if ( effect )
			attackMod += effect->getParamValue();		
	}
	
	attackerLevel = _Attacker->getSkillValue(_RightWeapon.Skill, targetRace) + _AttackSkillModifier.applyValue(_WeaponSabrinaValue) + attackMod;

	if (attackerLevel<0)
		attackerLevel = 0;

	// compute shield level
	sint32 defenderShieldLevel;
	if (defender->getId().getType() == RYZOMID::player)
	{
		sint32 bestSkill = 0;
		CCharacter * c = dynamic_cast<CCharacter *>(defender);
		if (c)
		{
			bestSkill = c->getSkillEquivalentDodgeValue(c->getSkillUsedForDodge());
		}
		else
		{
			nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", defender->getId().toString().c_str());
		}
		defenderShieldLevel = (bestSkill >= (shield.Quality)) ? shield.Quality : bestSkill;
	}
	else
	{
		defenderShieldLevel = shield.Quality;
	}

	// oposition test
	uint8 chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::ShieldUse, defenderShieldLevel - attackerLevel );
	uint8 test = (uint8)RandomGenerator.rand(99);
	if ( test < chances)
	{
/*		TReportAction report;
		report.ActionNature = ACTNATURE::SHIELD_USE;
		report.ActorRowId = defender->getEntityRowId();
		report.TargetRowId = _Attacker->getEntity()->getEntityRowId();
		report.DeltaLvl = defenderLevel - attackerLevel;
		report.Skill = shield.Skill;
//		report.ItemUsed = shield.ItemRefPtr;
		if (defender->getId().getType() == RYZOMID::player)
		{
			CCharacter *character = dynamic_cast<CCharacter*> (defender);
			if (!character)
			{
				nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", defender->getId().toString().c_str());
				report.ItemUsed = NULL;
			}
			else
			{
				report.ItemUsed = character->getRightHandItem();
			}
		}
		else
		{
			report.ItemUsed = NULL;
		}
		
		report.factor = report.factor = CStaticSuccessTable::getSuccessFactor( SUCCESS_TABLE_TYPE::ShieldUse, chances, test );
		PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( report );
		PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(report);
*/
		return true;
	}

	return false;
} // testShieldEfficiency //


//--------------------------------------------------------------
//					testPhraseSuccess()  
//--------------------------------------------------------------
bool CCombatPhrase::testPhraseSuccess(bool useRightHand)
{
	H_AUTO(CCombatPhrase_testPhraseSuccess);
	
	_CriticalHit = false;
	
	CEntityBase *attacker = _Attacker->getEntity();
	if (!attacker)
		return false;

	// test forced failure
	if (PHRASE_UTILITIES::forceActionFailure(attacker) )
	{
		_PhraseSuccessDamageFactor = 0.0;
		return false;
	}

	uint32 nbPlayers = 1;
	EGSPD::CPeople::TPeople targetRace = EGSPD::CPeople::Unknown;
	if( !_Targets.empty() )
	{
		CEntityBase * primTarget = _Targets[0].Target->getEntity();
		if (primTarget)
		{
			targetRace = primTarget->getRace();
			if( primTarget->getId().getType() != RYZOMID::player )
			{
				const CStaticCreatures * creatureSheet = primTarget->getForm();
				if( creatureSheet != 0 )
				{
					nbPlayers = creatureSheet->getNbPlayers();
				}			
			}
		}
	}

	// get skill debuff effect on melee/range
	sint32 skillModifier = 0;
	if (_MeleeCombat)
	{
		// do not use smart pointers for local use only
		const CSEffect *effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMelee );
		if ( effect )
			skillModifier +=  effect->getParamValue(); // value must be < 0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffCombatSkills );
		if ( effect )
			skillModifier +=  effect->getParamValue(); // value must be < 0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::OutpostCombat );
		if ( effect )
			skillModifier += effect->getParamValue();
		
	}
	else
	{
		// do not use smart pointers for local use only
		const CSEffect * effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillRange );
		if ( effect )
			skillModifier +=  effect->getParamValue(); // value must be < 0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffCombatSkills );
		if ( effect )
			skillModifier +=  effect->getParamValue(); // value must be < 0 on a debuff

		effect = attacker->lookForActiveEffect( EFFECT_FAMILIES::OutpostCombat );
		if ( effect )
			skillModifier += effect->getParamValue();
	}
	
	// get skill value
	SKILLS::ESkills skillUsed;
	sint32 skillValue;
	sint32 effectiveSkillValue;
	if ( _RootSkill != SKILLS::unknown )
	{
		skillUsed = _RootSkill;
		skillValue = _Attacker->getSkillValue( _RootSkill, targetRace );
		effectiveSkillValue = skillValue;
	}
	else
	{
		const CCombatWeapon &weapon = ( useRightHand ? _RightWeapon : _LeftWeapon );
		skillUsed = weapon.Skill;
		skillValue = _Attacker->getSkillValue(weapon.Skill, targetRace);
		effectiveSkillValue = min( skillValue, (sint32)weapon.Quality );
	}

	// add fight skill bonus given by consumable effect
	if ( _Attacker->getEntity()->getId().getType() == RYZOMID::player )
	{
		CCharacter * c = dynamic_cast<CCharacter*>(_Attacker->getEntity());
		if( _MeleeCombat )	
		{
			skillValue += c->meleeSuccessModifier();
		}
		else
		{
			skillValue += c->rangeSuccessModifier();
		}
	}

	// compute relative level (actor against phrase difficulty)
	const uint16 cost = max(sabrinaCost(), (uint16)effectiveSkillValue);
	const sint16 relativeLevel = (sint16)( (skillValue * max( 1.0f, nbPlayers / 2.0f ) ) + (sabrinaCredit() - _BrickMaxSabrinaCost - cost));

	// oposition test
	uint8 chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::FightPhrase, relativeLevel);
	uint8 tirage = (uint8)RandomGenerator.rand(99);
	#if !FINAL_VERSION
		if(CombatForceMiss)
			tirage= 99;
		if(CombatForceCritical)
			tirage= 0;
	#endif
	// get succes factor
		
	_PhraseSuccessDamageFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::FightPhrase, relativeLevel, tirage);
	
	// XP LOG for range combat
	if (!_MeleeCombat && attacker->getId().getType() == RYZOMID::player && !_Targets.empty())
	{
		CEntityBase *target = _Targets[0].Target->getEntity();
		if (target)
		{
			if (PlayerManager.logXPGain(attacker->getEntityRowId()))
			{
				nlinfo("[XPLOG] Combat action Player %s, target %s, skill %s, delta %d, chances %u, tirage %u, factor = %f",attacker->getId().toString().c_str(), 
					target->getId().toString().c_str(), SKILLS::toString(skillUsed).c_str(), relativeLevel, chances,tirage,_PhraseSuccessDamageFactor );
			}
		}
	}
	
	// test critical hit
	if (tirage < chances )
	{
		uint8 criticalChances = (uint8) min((uint16)100, uint16(CriticalHitChances + _CriticalHitChancesModifier.applyValue(_WeaponSabrinaValue)) );

		// add spire effect ( critical hit )
		if ( _Attacker->getEntity()->getId().getType() == RYZOMID::player )
		{
			const CSEffect* pEffect = _Attacker->getEntity()->lookForActiveEffect( EFFECT_FAMILIES::TotemCombatCri );
			if ( pEffect != NULL )
				criticalChances += (uint8)pEffect->getParamValue();
		} 
		// Add item effect (only take into account the used item for the moment)
		{
			std::vector<SItemSpecialEffect> effects;
			if ( _Attacker->getEntity()->getId().getType() == RYZOMID::player )
			{
				CCharacter* c = dynamic_cast<CCharacter*>(_Attacker->getEntity());
				effects = c->lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::ISE_FIGHT_ADD_CRITICAL);
			}
			if ( _Attacker->getEntity()->getId().getType() == RYZOMID::creature )
			{
				CGameItemPtr usedItem;
				CCreature* c = dynamic_cast<CCreature*>(_Attacker->getEntity());
				usedItem = useRightHand?c->getRightHandItem():c->getLeftHandItem();
				if (usedItem!=NULL && usedItem->getStaticForm() && usedItem->getStaticForm()->ItemSpecialEffects && !usedItem->getStaticForm()->ItemSpecialEffects->Effects.empty())
					effects = usedItem->getStaticForm()->lookForEffects(ITEM_SPECIAL_EFFECT::ISE_FIGHT_ADD_CRITICAL);
			}
			std::vector<SItemSpecialEffect>::const_iterator it, itEnd;
			for (it=effects.begin(), itEnd=effects.end(); it!=itEnd; ++it)
				criticalChances += (uint8)(it->EffectArgFloat[0]*100.);
		}
		clamp( criticalChances, 0, 100 );

		if (tirage < criticalChances)
		{
			_CriticalHit = true;
		}
	}

	if (_PhraseSuccessDamageFactor == 0.0f)
	{
		// failed
		return false;
	}
	else if (_PhraseSuccessDamageFactor < 0.0f)
	{
		// fumble
		return false;
	}

	return true;
} // testPhraseSuccess //


//--------------------------------------------------------------
//					checkTargetValidity()  
//--------------------------------------------------------------
bool CCombatPhrase::checkTargetValidity( const TDataSetRow &targetRowId, string &errorCode )
{
	H_AUTO(CCombatPhrase_checkTargetValidity);
	
	// already tested before
	TDataSetRow attacker = _Attacker->getEntityRowId();
	if( TheDataset.isAccessible( attacker ) )
	{
		if ( TheDataset.getEntityId(_Attacker->getEntityRowId()).getType() != RYZOMID::player )
		{
			return true;
		}
	}
	else
	{
		return false;
	}

	if ( !TheDataset.isAccessible(targetRowId) )
	{
		errorCode = "INVALID_TARGET";
		return false;
	}
	
	if (targetRowId == _Attacker->getEntityRowId())
	{
		errorCode = "INVALID_TARGET";
		return false;
	}

	if ( ! PHRASE_UTILITIES::testOffensiveActionAllowed(_Attacker->getEntityRowId(), targetRowId, errorCode) )
	{
		return false;
	}

	return true;
} // checkTargetValidity //

	
//--------------------------------------------------------------
//					checkPhraseCost()
//--------------------------------------------------------------
bool CCombatPhrase::checkPhraseCost( string &errorCode )
{
	H_AUTO(CCombatPhrase_checkPhraseCost);
	
	if (!_Attacker || !_Attacker->getEntity()) return false;
	if (!TheDataset.isAccessible(_Attacker->getEntityRowId()) ) return false;

	// only check costs for players
	if ( TheDataset.getEntityId(_Attacker->getEntityRowId()).getType() != RYZOMID::player)
		return true;

	const SCharacteristicsAndScores &stamina = _Attacker->getEntity()->getScores()._PhysicalScores[SCORES::stamina];
	if ( stamina.Current < _TotalStaminaCost)
	{
		errorCode = "EGS_TOO_EXPENSIVE_STAMINA";
		return false;
	}

	const SCharacteristicsAndScores &hp = _Attacker->getEntity()->getScores()._PhysicalScores[SCORES::hit_points];
	if ( hp.Current <= _TotalHPCost)
	{
		errorCode = "EGS_TOO_EXPENSIVE_HP";
		return false;
	}

	return true;
} // checkPhraseCost //


//--------------------------------------------------------------
//					createDefender()  
//--------------------------------------------------------------
CCombatDefenderPtr CCombatPhrase::createDefender( const TDataSetRow &targetRowId )
{
	H_AUTO(CCombatPhrase_createDefender);
	
	if ( !TheDataset.isAccessible(targetRowId) )
		return NULL;

	// create defender structure
	if ( TheDataset.getEntityId(targetRowId).getType() == RYZOMID::player )
	{
		return new CCombatDefenderPlayer(targetRowId);
	}
	else
	{
		return new CCombatDefenderAI(targetRowId);
	}
} // createDefender //


//--------------------------------------------------------------
//					validateCombatActions()  
//--------------------------------------------------------------
bool CCombatPhrase::validateCombatActions( string &errorCode )
{
	H_AUTO(CCombatPhrase_validateCombatActions);

	const uint size = (uint)_CombatActions.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		if ( _CombatActions[i] != NULL)
		{
			if (!_CombatActions[i]->validate(this, errorCode))
				return false;
		}
	}

	return true;
} // validateCombatActions //

//--------------------------------------------------------------
//					applyCombatActions()  
//--------------------------------------------------------------
void CCombatPhrase::applyCombatActions()
{
	H_AUTO(CCombatPhrase_applyCombatActions);
	
//	if (!_Defender) return;

	const uint size = (uint)_CombatActions.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		if ( _CombatActions[i] != NULL)
		{
			_CombatActions[i]->apply(this);
		}
	}
} // applyCombatActions //

//--------------------------------------------------------------
//					getHitLocalisation()  
//--------------------------------------------------------------
PHRASE_UTILITIES::TPairSlotShield CCombatPhrase::getHitLocalisation(CCombatDefenderPtr &defender, const CCombatShield &shield, DMGTYPE::EDamageType dmgType)
{	
	H_AUTO(CCombatPhrase_getHitLocalisation);
	
	PHRASE_UTILITIES::TPairSlotShield localisation = make_pair(SLOT_EQUIPMENT::UNDEFINED,false);
	if (defender.isNull())
		return localisation;
	
	CEntityBase *defenderEntity = defender->getEntity();
	if (!defenderEntity)
		return localisation;

	// todo : use the right localisation table according to target type
	sint8 adjustment = PHRASE_UTILITIES::getLocalisationSizeAdjustement( _Attacker->getEntityRowId(), defender->getEntityRowId() );

	// if attacker is a player
	if ( dynamic_cast<CCombatAttackerPlayer*> (_Attacker) != NULL )
	{
		BODY::TBodyType bodyType;
		if ( _AimedSlot.BodyType != BODY::UnknownBodyType )
			bodyType = EGSPD::getBodyType(defenderEntity->getRace());
		else
			bodyType = BODY::UnknownBodyType;		

		// check if the aimed slot is hit (if any)(and thus bypass normal localisation)
		if ( _AimedSlot.Slot != SLOT_EQUIPMENT::UNDEFINED && _AimedSlot.BodyType == bodyType )
		{
			float factor = _AimedSlot.applyFactor(_WeaponSabrinaValue) * _PhraseSuccessDamageFactor;
			float randResult = RandomGenerator.frand(1.0f);
			if (randResult <= factor)
			{
				localisation = PHRASE_UTILITIES::getLocalisation( shield.ShieldType, adjustment, _AimedSlot.Slot );
				return localisation;
			}
		}
	}
	else if (_AiAimingType != AI_AIMING_TYPE::Random)
	{
		SLOT_EQUIPMENT::TSlotEquipment slot;
		switch(_AiAimingType)
		{
		case AI_AIMING_TYPE::LeastProtected:
			slot = defender->getLeastProtectedSlot(dmgType);
			break;
		case AI_AIMING_TYPE::AveragestProtected:
			slot = defender->getAveragestProtectedSlot(dmgType);
			break;
		case AI_AIMING_TYPE::MostProtected:
			slot = defender->getMostProtectedSlot(dmgType);
			break;
		default:
			slot = AI_AIMING_TYPE::toSlot(_AiAimingType);
		};
		
		localisation = PHRASE_UTILITIES::getLocalisation( shield.ShieldType, adjustment, slot );
		return localisation;
	}
	
	// default localisation
	localisation = PHRASE_UTILITIES::getLocalisation( shield.ShieldType, adjustment );
	
	return localisation;
} // getHitLocalisation //

//--------------------------------------------------------------
//					applyLocalisationSpecialEffect()  
//--------------------------------------------------------------
void CCombatPhrase::applyLocalisationSpecialEffect( CCombatDefenderPtr &defender, SLOT_EQUIPMENT::TSlotEquipment slot, sint32 damage, sint32 &lostStamina)
{
	H_AUTO(CCombatPhrase_applyLocalisationSpecialEffect);
	
	// for the moment, only apply such effects on critical success
	if (!_CriticalHit) return;

	if (defender.isNull()) return;

	CEntityBase *entity = defender->getEntity();

	if (!entity) return;

	switch(slot)
	{
	case SLOT_EQUIPMENT::HEAD:
		// stun
		{
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( HitHeadStunDuration / CTickEventHandler::getGameTimeStep() );
			CCombatActionStun stunAction( _Attacker->getEntityRowId(), this, duration);
			stunAction.applyOnEntity(entity,1.0f);
			DEBUGLOG("Entity %s, hit to head, stunned for %f seconds", entity->getId().toString().c_str(), HitHeadStunDuration.get());
		}
		break;
	case SLOT_EQUIPMENT::CHEST:
		// sta loss
		{
			lostStamina += (sint32) (damage * HitChestStaLossFactor);
			DEBUGLOG("Entity %s, hit to chest, lose %d stamina", entity->getId().toString().c_str(), lostStamina);
		}
		break;
	case SLOT_EQUIPMENT::ARMS:
		// slow attack
		{
			// CombatAttackSlow
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( HitArmsSlowDuration / CTickEventHandler::getGameTimeStep() );
			CCombatActionSimpleEffect attackSlow( _Attacker->getEntityRowId(), this, duration, EFFECT_FAMILIES::CombatAttackSlow, HitArmsSlowFactor, (uint8)(abs(HitArmsSlowFactor)) );
			attackSlow.applyOnEntity(entity,1.0f);

			DEBUGLOG("Entity %s, hit to arms, slow attack length = %f, slowing factor = %u", entity->getId().toString().c_str(), HitArmsSlowDuration.get(), HitArmsSlowFactor.get());
		}
		break;
	case SLOT_EQUIPMENT::HANDS:
		// debuff melee/range skills
		{
			// CombatDebuffCombatSkills
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( HitHandsDebuffDuration / CTickEventHandler::getGameTimeStep() );
			CCombatActionSimpleEffect debuffCombatSkills( _Attacker->getEntityRowId(), this, duration, EFFECT_FAMILIES::CombatDebuffCombatSkills, HitHandsDebuffValue, (uint8)(abs(HitHandsDebuffValue)));
			debuffCombatSkills.applyOnEntity(entity,1.0f);
			DEBUGLOG("Entity %s, hit to hands, debuff length = %f, debuff value = %d", entity->getId().toString().c_str(), HitHandsDebuffDuration.get(), HitHandsDebuffValue.get());
		}
		break;
	case SLOT_EQUIPMENT::LEGS:
		// slow move
		{
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( HitLegsSlowDuration / CTickEventHandler::getGameTimeStep() );
			CCombatActionSlowMove slowMove( _Attacker->getEntityRowId(), this, duration, HitLegsSlowFactor);
			slowMove.applyOnEntity(entity,1.0f);

			DEBUGLOG("Entity %s, hit to legs, slow move length = %f, factor = %d", entity->getId().toString().c_str(), HitLegsSlowDuration.get(), HitLegsSlowFactor.get());
		}
		break;
	case SLOT_EQUIPMENT::FEET:
		// debuff dodge skill
		{
			// CombatDebuffDodge
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( HitFeetDebuffDuration / CTickEventHandler::getGameTimeStep() );
			CCombatActionSimpleEffect debuffDodge( _Attacker->getEntityRowId(), this, duration, EFFECT_FAMILIES::CombatDebuffDodge, HitFeetDebuffValue, (uint8)(abs(HitFeetDebuffValue)));
			debuffDodge.applyOnEntity(entity,1.0f);
			DEBUGLOG("Entity %s, hit to feet, debuff dodge length = %f, debuff value = %d", entity->getId().toString().c_str(), HitFeetDebuffDuration.get(), HitFeetDebuffValue.get());
		}
		break;
	
		default:// unknown slot ?
		;
	};
} // applyLocalisationSpecialEffect //


//--------------------------------------------------------------
//					applyArmorProtections()  
//--------------------------------------------------------------
sint32 CCombatPhrase::applyArmorProtections(CCombatDefenderPtr &defender, sint32 damage, DMGTYPE::EDamageType dmgType, SLOT_EQUIPMENT::TSlotEquipment slot, bool shieldIsEffective)
{
	H_AUTO(CCombatPhrase_applyArmorProtections);
	
	if (defender.isNull())
		return 0;

	CEntityBase *entity = defender->getEntity();
	if (!entity)
		return 0;

	// test invulnerability auras and powers
	bool invulnerable = false;
	CSEffect *effect = entity->lookForActiveEffect( EFFECT_FAMILIES::PowerInvulnerability );
	if (effect)
	{
		invulnerable = true;
	}
	else
	{
		effect = entity->lookForActiveEffect( EFFECT_FAMILIES::Invincibility );
		if (effect)
			invulnerable = true;
		else
		{
			if (_MeleeCombat)
				effect = entity->lookForActiveEffect( EFFECT_FAMILIES::PowerProtection );
			else
				effect = entity->lookForActiveEffect( EFFECT_FAMILIES::PowerUmbrella );

			if (effect)
				invulnerable = true;
		}
	}

	if (invulnerable)
	{
		return 0;
	}

	//  get defender armor
	CCombatArmor armor;
	defender->getArmor( slot, armor);

	// get defender shield
	CCombatShield shield;
	defender->getShield(shield);

	// compute armor and shield protections
	sint32 defenderArmorLevel;
	sint32 defenderShieldLevel;
	if (entity->getId().getType() == RYZOMID::player)
	{
		sint32 bestArmorSkill = 0;
		sint32 bestShieldSkill = 0;
		CCharacter * c = dynamic_cast<CCharacter *>(entity);
		if (c)
		{
			bestArmorSkill = c->getSkillBaseValue(c->getBestSkill());
			bestShieldSkill = c->getSkillEquivalentDodgeValue(c->getSkillUsedForDodge());
		}
		else
		{
			nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", entity->getId().toString().c_str());
		}

		defenderArmorLevel = (bestArmorSkill >= (armor.Quality)) ? armor.Quality : bestArmorSkill;
		defenderShieldLevel = (bestShieldSkill >= (shield.Quality)) ? shield.Quality : bestShieldSkill;
	}
	else
	{
		defenderArmorLevel = armor.Quality;
		defenderShieldLevel = shield.Quality;
	}

	// get factor related to skill/required
	const float shieldFactor = (shield.Quality > 0) ? CWeaponDamageTable::getInstance().getRefenceDamage(shield.Quality, defenderShieldLevel) / CWeaponDamageTable::getInstance().getRefenceDamage(shield.Quality, shield.Quality) : 0.0f;
	const float armorFactor = (armor.Quality > 0) ? CWeaponDamageTable::getInstance().getRefenceDamage(armor.Quality, defenderArmorLevel) / CWeaponDamageTable::getInstance().getRefenceDamage(armor.Quality, armor.Quality) : 0.0f;

	uint32 dmgPreventedByShield = 0;
	uint32 dmgPreventedByArmor = 0;

	// get modifier according to armor type
	float shieldAbsorptionFactor = 1.0;
	sint32 shieldAbsorptionModifier = 0;
	float armorAbsorptionFactor = 1.0;
	sint32 armorAbsorptionModifier = 0;

	// test creature type
	if ( _ArmorAbsorptionFactor.CreatureType != EGSPD::CClassificationType::EndClassificationType && EGSPD::testClassificationType( entity->getRace(), _ArmorAbsorptionFactor.CreatureType) )
	{
		armorAbsorptionFactor = _ArmorAbsorptionFactor.applyValue(weaponSabrinaValue());
		shieldAbsorptionFactor = armorAbsorptionFactor;
	}
	// test armor type
	else if (_ArmorAbsorptionFactor.ArmorType != ARMORTYPE::UNKNOWN)
	{
		if ( _ArmorAbsorptionFactor.ArmorType == ARMORTYPE::ALL || armor.ArmorType == _ArmorAbsorptionFactor.ArmorType )
		{
			armorAbsorptionFactor = _ArmorAbsorptionFactor.applyValue(weaponSabrinaValue());
		}
		if ( _ArmorAbsorptionFactor.ArmorType == ARMORTYPE::ALL || shield.ArmorType == _ArmorAbsorptionFactor.ArmorType )
		{
			shieldAbsorptionFactor = _ArmorAbsorptionFactor.applyValue(weaponSabrinaValue());
		}
	}

	if (dmgType >= 0 && dmgType < DMGTYPE::NBTYPES)
	{
		if (shieldIsEffective)
		{
			const uint32 maxProtection = (uint32) ( shield.MaxProtection[dmgType] * shieldFactor);
			dmgPreventedByShield += (uint32)(maxProtection <  (uint32)(shield.ProtectionFactor[dmgType] * damage) ? maxProtection : shield.ProtectionFactor[dmgType] * damage );
			dmgPreventedByShield = max( sint32(0), sint32(dmgPreventedByShield * shieldAbsorptionFactor) + shieldAbsorptionModifier );
			damage -= dmgPreventedByShield;
		}
		const uint32 maxProtection = (uint32) ( armor.MaxProtection[dmgType] * armorFactor);
		dmgPreventedByArmor += (uint32)(maxProtection <  (uint32)(armor.ProtectionFactor[dmgType] * damage) ? maxProtection : armor.ProtectionFactor[dmgType] * damage );
		dmgPreventedByArmor = max( sint32(0), sint32(dmgPreventedByArmor * armorAbsorptionFactor) + armorAbsorptionModifier );

		// add spire effect ( armor )
		if ( defender->getEntity()->getId().getType() == RYZOMID::player )
		{
			const CSEffect* pEffect = defender->getEntity()->lookForActiveEffect( EFFECT_FAMILIES::TotemCombatArm );
			if ( pEffect != NULL )
			{
				dmgPreventedByArmor += ( dmgPreventedByArmor * pEffect->getParamValue() / 100 );
				if ( dmgPreventedByArmor < 0 )
					dmgPreventedByArmor = 0;
			} 
		}

		damage -= dmgPreventedByArmor;
	}


	if ( dmgPreventedByShield != 0)
	{
		INFOLOG("COMBAT : Entity %u (attacked by %u) Damage prevented by shield = %u", defender->getEntityRowId().getIndex(), _Attacker->getEntityRowId().getIndex(), dmgPreventedByShield);
		// remove shield hit points
//		defender->damageOnShield( dmgPreventedByShield );
	}

	if ( dmgPreventedByArmor != 0)
	{		
//		defender->damageOnArmor(slot, dmgPreventedByArmor );
		INFOLOG("COMBAT : Entity %u (attacked by %u) Damage prevented by armor = %u",defender->getEntityRowId().getIndex(), _Attacker->getEntityRowId().getIndex(), dmgPreventedByArmor);
	}

	if (damage < 0)
		damage = 0;

	return damage;
} // applyArmorProtections //

//--------------------------------------------------------------
//					checkOpening()  
//--------------------------------------------------------------
bool CCombatPhrase::checkOpening()
{
	H_AUTO(CCombatPhrase_checkOpening);
	
	if (!_Attacker) return false;

	CCharacter *player = PlayerManager.getChar(_Attacker->getEntityRowId());
	if (!player) return true;

	//const uint32 flags = player->getCombatEventFlags();
	BRICK_FLAGS::TBrickFlag flag = BRICK_FLAGS::UnknownFlag;
	bool success = true;

	for (uint i = 0 ; i < _OpeningNeededFlags.size() && success ; ++i)
	{
		flag  = _OpeningNeededFlags[i];
		//if ( !( flags & (1<<flag) ) )
		if( player->isCombatEventFlagActive(flag) == false )
		{
			//nlwarning("COMBAT : player %s tried to take an opening needing %s but failed", player->getId().toString().c_str(), BRICK_FLAGS::toString(flag).c_str());
			success = false;
		}
	}

	if (!success)
	{
		// either player trying to cheat or error because of lag
		// TODO : send message for other failure 
	}
	else
	{
		// clear used flags
		for (uint i = 0 ; i < _OpeningNeededFlags.size() ; ++i)
		{
			player->resetCombatEventFlag(_OpeningNeededFlags[i]);
		}
		
		// send message to indicate a success
		PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "EGS_OPENING_SUCCESS");
	}

	return success;
} // checkOpening //

//--------------------------------------------------------------
//					buildTargetList()
//--------------------------------------------------------------
void CCombatPhrase::buildTargetList(bool rightHand, bool isMad)
{
	H_AUTO(CCombatPhrase_buildTargetList);
	
	if (_Targets.empty())
		return;

	_Targets.resize(1);

	const TDataSetRow mainTargetRowId = _Targets[0].Target->getEntityRowId();

	_Targets[0].DamageFactor	= 1.0f;
	_Targets[0].Distance		= (float) PHRASE_UTILITIES::getDistance(mainTargetRowId, _Attacker->getEntityRowId());

	const CCombatWeapon & weapon = ( rightHand ? _RightWeapon : _LeftWeapon );

	// build affected target list
	if ( FightAreaEffectOn && weapon.Area )
	{
		// forbid use of multi target with an area effect weapon !!!!
		nlassert(_HitAllMeleeAggressors == false);

		if (DumpRangeAnalysis)
			AreaDebug.init(mainTargetRowId);

		CEntityRangeSelector areaSelector;
		areaSelector.buildTargetList(_Attacker->getEntityRowId(), mainTargetRowId, weapon.Area, ACTNATURE::FIGHT);
		areaSelector.clampTargetCount( MaxAreaTargetCount );

		_Targets.reserve(areaSelector.getEntities().size());

		string dummy;
		// skip main target so start to 1 !!
		uint targetIndex = 1;
		for ( uint i = 1; i <  areaSelector.getEntities().size() ; i++ )
		{
			if ( PHRASE_UTILITIES::testOffensiveActionAllowed(_Attacker->getEntityRowId(), areaSelector.getEntities()[i]->getEntityRowId(), dummy, false) )
			{
				CCombatDefenderPtr defender = createDefender(areaSelector.getEntities()[i]->getEntityRowId());
				if (defender != NULL)
				{
					nlassert( i < areaSelector.getDistances().size() );
					nlassert(targetIndex == _Targets.size());
					defender->setTargetIndex(targetIndex++);

					TTargetInfos targetInfos;
					targetInfos.Target			= defender;
					targetInfos.DamageFactor	= areaSelector.getFactor(i);
					targetInfos.Distance		= areaSelector.getDistances()[i];
					_Targets.push_back(targetInfos);
				}
			}
		}
	}

	if (rightHand && !isMad && _HitAllMeleeAggressors)
	{
		string dummy;

		const uint previousNbTargets = (uint)_Targets.size();
	
		const set<TDataSetRow> &aggressors = CPhraseManager::getInstance().getMeleeAggressors(_Attacker->getEntityRowId());
		set<TDataSetRow>::const_iterator it;
		set<TDataSetRow>::const_iterator itEnd = aggressors.end();

		uint targetIndex = previousNbTargets;
		for (it = aggressors.begin(); it != itEnd; ++it)
		{
			// skip main target as it's already a ... target 
			if ( (*it) == mainTargetRowId )
				continue;

			if ( PHRASE_UTILITIES::testOffensiveActionAllowed(_Attacker->getEntityRowId(), *it, dummy, false) )
			{
				CCombatDefenderPtr defender = createDefender(*it);
				if (defender != NULL)
				{
					nlassert(targetIndex == _Targets.size());
					defender->setTargetIndex(targetIndex++);

					TTargetInfos targetInfos;
					targetInfos.Target		= defender;
					targetInfos.Distance	= (float) PHRASE_UTILITIES::getDistance(defender->getEntityRowId(), _Attacker->getEntityRowId());
					_Targets.push_back(targetInfos);
				}
			}
		}

		const uint nbTargets = (uint)_Targets.size();
		const float damageFactor = min( 1.0f, _MultiTargetGlobalDamageFactor.applyValue(_WeaponSabrinaValue) / ( 1 + nbTargets - previousNbTargets) );

		// start to previousNbTarget as previous targets already processed
		for (uint i = previousNbTargets; i < nbTargets; ++i)
		{
			_Targets[i].DamageFactor = damageFactor;
		}
		// also set main target
		_Targets[0].DamageFactor	= damageFactor;
	}
}  // buildTargetList //


//--------------------------------------------------------------
//					applyDamageOnBodyguard()
//--------------------------------------------------------------
sint32 CCombatPhrase::applyDamageOnBodyguard(CEntityBase *actingEntity, TDataSetRow defenderRowId, sint32 attackerLevel, CSEffectPtr effect, sint32 damage, DMGTYPE::EDamageType dmgType)
{
	H_AUTO(CCombatPhrase_applyDamageOnBodyguard);
	
#ifdef NL_DEBUG
	nlassert(effect);
	nlassert(actingEntity);
	
	CShieldingEffect *seffect = dynamic_cast<CShieldingEffect *> (& (*effect));
	nlassert(seffect);
#else
	CShieldingEffect *seffect = dynamic_cast<CShieldingEffect *> (& (*effect));
	if (seffect == NULL)
	{
		nlwarning("This dynamic_cast should not return NULL");
		return damage;
	}
#endif
	// target is being protected by another entity, this entity takes some damage, same localisation as original target)
	CCombatDefenderPtr bodyguard = createDefender( effect->getCreatorRowId());
	if (!bodyguard || !bodyguard->getEntity())
	{
		// bodyguard gone or invalid, should remove effect
		// NO !
		//defender->removeSabrinaEffect(effect);
		return damage;
	}
	
	CEntityBase *entity = bodyguard->getEntity();
	
	// check distance between the bodyguard and the defender
	double distance = PHRASE_UTILITIES::getDistance(defenderRowId, entity->getEntityRowId());
	if (distance < ShieldingRadius)
	{
		float factor;
		uint16 max;
		seffect->getProtectionParams(bodyguard, factor, max);
		sint32 damageOnBodyguard = min( sint32(damage * factor), sint32(max) );
		
		damage -= damageOnBodyguard;

		const sint32 damageOnBodyguardBeforeArmor = damageOnBodyguard;
		
		BODY::TBodyPart hitBodyPart = BODY::getBodyPart(EGSPD::getBodyType(entity->getRace()), _HitLocalisation);
		
		CCombatShield shield;
		bodyguard->getShield(shield);
		
		bool shieldIsEffective = testShieldEfficiency(bodyguard, shield);
		damageOnBodyguard = applyArmorProtections(bodyguard, damageOnBodyguard, dmgType, _HitLocalisation, shieldIsEffective);
		
		if ( entity->changeCurrentHp( (-1)*damageOnBodyguard, actingEntity->getEntityRowId() ) )
		{
			PHRASE_UTILITIES::sendHitMessages(actingEntity->getId(),entity->getId(), false, damageOnBodyguard, damageOnBodyguardBeforeArmor, 0, 0, hitBodyPart);
			PHRASE_UTILITIES::sendDeathMessages( _Attacker->getEntityRowId(), entity->getEntityRowId() );
		}
		else
		{
			CPhraseManager::getInstance().breakCast(attackerLevel,actingEntity,entity);
			// send hit message
			PHRASE_UTILITIES::sendHitMessages(actingEntity->getId(),entity->getId(), false, damageOnBodyguard, damageOnBodyguardBeforeArmor, 0, 0, hitBodyPart);
		}
	}

	return damage;
} // applyDamageOnBodyguard //

//--------------------------------------------------------------
//					applyBounceEffect()
//--------------------------------------------------------------
void CCombatPhrase::applyBounceEffect(CEntityBase *actingEntity, sint32 attackerLevel, CSEffectPtr effect, sint32 damage, DMGTYPE::EDamageType dmgType)
{
	H_AUTO(CCombatPhrase_applyBounceEffect);
	
#ifdef NL_DEBUG
	nlassert(effect);
	nlassert(actingEntity);

	CBounceEffect *bounceEffect = dynamic_cast<CBounceEffect *> (& (*effect));
	nlassert(bounceEffect);
#else
	CBounceEffect *bounceEffect = dynamic_cast<CBounceEffect *> (& (*effect));
	if (bounceEffect == NULL)
	{
		nlwarning("This dynamic_cast should not return NULL");
		return;
	}
#endif

	CEntityBase *bounceTarget = bounceEffect->getTargetForBounce(actingEntity);
	if (!bounceTarget)
		return;

	CCombatDefenderPtr combatTarget = createDefender( bounceTarget->getEntityRowId() );
	if (!combatTarget)
		return;

	CCombatShield shield;
	combatTarget->getShield(shield);

	BODY::TBodyPart hitBodyPart = BODY::getBodyPart(EGSPD::getBodyType(bounceTarget->getRace()), _HitLocalisation);
	
	const sint32 damageBeforeArmor = damage;
	bool shieldIsEffective = testShieldEfficiency(combatTarget, shield);
	damage = applyArmorProtections(combatTarget, damage, dmgType, _HitLocalisation, shieldIsEffective);
	
	if ( bounceTarget->changeCurrentHp( (-1)*damage, actingEntity->getEntityRowId() ) )
	{
		PHRASE_UTILITIES::sendHitMessages(actingEntity->getId(),bounceTarget->getId(), false, damage, damageBeforeArmor, 0, 0, hitBodyPart);
		PHRASE_UTILITIES::sendDeathMessages( _Attacker->getEntityRowId(), bounceTarget->getEntityRowId() );
	}
	else
	{
		CPhraseManager::getInstance().breakCast(attackerLevel,actingEntity,bounceTarget);
		// send hit message
		PHRASE_UTILITIES::sendHitMessages(actingEntity->getId(),bounceTarget->getId(), false, damage, damageBeforeArmor, 0, 0, hitBodyPart);
	}
	
	// compute aggro
	CAiEventReport	aiEventReport;
	aiEventReport.Originator = actingEntity->getEntityRowId();
	aiEventReport.Target = bounceTarget->getEntityRowId();
	aiEventReport.Type = ACTNATURE::FIGHT;

	computeAggro(aiEventReport, bounceTarget,damage);
} // applyBounceEffect //


//--------------------------------------------------------------
//					computeAggro()
//--------------------------------------------------------------
void CCombatPhrase::computeAggro(CAiEventReport &aiEventReport, CEntityBase *target, sint32 hpDamage, sint32 staDamage, sint32 sapDamage )
{
	H_AUTO(CCombatPhrase_computeAggro);
	
#ifdef NL_DEBUG
	nlassert(target);
#endif

	// compute aggro
	float aggroHp = 0.0f;
	float aggroSap = 0.0f;
	float aggroSta = 0.0f;
	if (hpDamage > 0)
	{
		const sint32 maxPv = target->maxHp();
		if (maxPv)
		{
			aggroHp = min( 1.0f , float(hpDamage + _AggroModifier)/float(maxPv));
		}
		aiEventReport.addDelta(AI_EVENT_REPORT::HitPoints, -hpDamage);
	}
	if (sapDamage > 0)
	{
		const sint32 maxSap = target->getPhysScores()._PhysicalScores[SCORES::sap].Max;
		if (maxSap)
		{
			aggroSap = min( 1.0f, float(sapDamage)/float(maxSap));
		}
		aiEventReport.addDelta(AI_EVENT_REPORT::Sap, -sapDamage);
	}
	if (staDamage > 0)
	{
		const sint32 maxSta = target->getPhysScores()._PhysicalScores[SCORES::stamina].Max;
		if (maxSta)
		{
			aggroSta = min (1.0f, float(staDamage)/float(maxSta));
		}
		aiEventReport.addDelta(AI_EVENT_REPORT::Stamina, -staDamage);
	}
	
	const float aggro =  - min( 1.0f, _AggroMultiplier * (1.0f - (1.0f-aggroHp)*(1.0f-aggroSap)*(1.0f-aggroSta)) );
	
	// update the ai event report
	aiEventReport.AggroAdd = aggro;
	aiEventReport.Target = target->getEntityRowId();
	
	// TEMP FIX force aggro to be <0
	if (aiEventReport.AggroAdd == 0)
		aiEventReport.AggroAdd = -0.02f;
		
	CPhraseManager::getInstance().addAiEventReport(aiEventReport);
} // computeAggro //


//--------------------------------------------------------------
//					computeBaseDamage()
//--------------------------------------------------------------
void CCombatPhrase::computeBaseDamage(bool rightHand, EGSPD::CPeople::TPeople targetRace)
{
	H_AUTO(CCombatPhrase_computeBaseDamage);
	
#if !FINAL_VERSION
	nlassert(_Attacker);
#endif

	_BaseDamage = 0.0f;

	CEntityBase *actingEntity = _Attacker->getEntity();
	if (!actingEntity)
		return;

	const CCombatWeapon &weapon = (rightHand ? _RightWeapon : _LeftWeapon);

	// Compute damage
	sint32 attackerLevel = _Attacker->getSkillValue(weapon.Skill, targetRace);
	uint16 itemQuality = _MeleeCombat? weapon.Quality : std::min(_Ammo.Quality,weapon.Quality);
	
	float weaponDamage = _MeleeCombat? weapon.Damage : (_Ammo.Damage + weapon.Damage);

	/// Effects affecting the damage (modifiers)
	// check for enchant weapon effect
	const CSEffectPtr effect3 = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::PowerEnchantWeapon );
	if ( effect3 )
	{
//		CEnchantWeaponEffect* enchant = static_cast<CEnchantWeaponEffect*>((CSEffect*)effect3);
//		weaponDamage += enchant->getDpsBonus();
	}
	
	// if attacker is not a player, the _RightWeapon damage is direclty the value to use, otherwise we need the CWeaponDamageTable
	if (actingEntity->getId().getType() == RYZOMID::player)
	{
		// add fight skill bonus given by consumable effect
		CCharacter * c = dynamic_cast<CCharacter*>(actingEntity);
		if(c) 
		{
			if( _MeleeCombat )	
			{
				attackerLevel += c->meleeSuccessModifier();
			}
			else
			{
				attackerLevel += c->rangeSuccessModifier();
			}
		}

		weaponDamage *= CWeaponDamageTable::getInstance().getRefenceDamage(itemQuality, attackerLevel);
	}
	
	_NaturalDamage = (sint32)weaponDamage;

	float factor = _DamageFactor; 

	/// Effects affecting the damage factor
	// check for war cry aura
	const CSEffectPtr effect = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::PowerWarCry );
	if ( effect )
	{
		factor += effect->getParamValue() / 100.0f;
	}
	
	_BaseDamage = (weaponDamage * factor) + _DamageModifier;

	/// Effects affecting the damage (modifiers)
	// check for berserker effect
	const CSEffectPtr effect2 = actingEntity->lookForActiveEffect( EFFECT_FAMILIES::PowerBerserker );
	if ( effect2 )
	{
		_BaseDamage += effect2->getParamValue();
	}
	
	DEBUGLOG("COMBAT : (%s) Attacker : Entity %s, weaponDamage = %f (skill %s, quality %u), attackerLevel = %d, _DamageModifier= %d", 
		(rightHand ? "RightHand":"LeftHand"),
		_Attacker->getEntity()->getId().toString().c_str(),
		weaponDamage,
		SKILLS::toString(_RightWeapon.Skill).c_str(),
		itemQuality,
		attackerLevel,
		_DamageModifier
		);
} // computeBaseDamage //

//--------------------------------------------------------------
//					checkDistanceAndOrientation()
//--------------------------------------------------------------
bool CCombatPhrase::checkOrientation(  const CEntityBase *actor, const CEntityBase *target )
{
	return true;

	/*nlassert(actor);
	nlassert(target);

	const CEntityState &targetState = const_cast<CEntityBase*>(target)->getState();
	const CEntityState &actorState = const_cast<CEntityBase*>(actor)->getState();

	const sint32 x1 = actorState.X;
	const sint32 y1 = actorState.Y;
	const sint32 x2 = targetState.X;
	const sint32 y2 = targetState.Y;

	double angle;
	if ( x2 == x1 )
	{
		angle = NLMISC::Pi;
	}
	else
	{
		angle = asin( fabs(double(y2-y1)) / fabs(double(x2-x1)) );
	}

	if ( fabs(angle - actorState.Heading.getValue()) > MaxAngleForRangeCombat )
	{
		if (!_BadOrientationMsg)
		{
			_BadOrientationMsg = true;
			PHRASE_UTILITIES::sendSimpleMessage( actor->getId(), "BAD_ORIENTATION");
		}

		return false;
	}
	else
	{
		_BadOrientationMsg = false;
		return true;
	}
	*/
}

//--------------------------------------------------------------
//						applyEvents();
//--------------------------------------------------------------
void CCombatPhrase::addDelayedEvent(TDataSetRow defenderId, TDelayedEventType eventType, bool sendFlyingText)
{
	CDelayedEvent	ev;
	ev.DefenderRowId= defenderId;
	ev.EventType= eventType;
	ev.SendFlyingText= sendFlyingText;
	_DelayedEvents.push_back(ev);
}

//--------------------------------------------------------------
//						applyEvents();
//--------------------------------------------------------------
void CCombatPhrase::flushDelayedEvents()
{
	// apply the events
	if(_Attacker)
	{
		for(uint i=0;i<_DelayedEvents.size();i++)
		{
			CDelayedEvent	&ae= _DelayedEvents[i];
			switch(ae.EventType)
			{
			case EventEvade:
			case EventDodge:
			case EventParry:
				{
					// don't send a flying text if some damage is done (for instance on another entity or with the left weapon)
					// don't send a flying text if not wanted
					bool	sendFlyingText= _Behaviour.DeltaHP==0 && ae.SendFlyingText;
					// don't send a dodge/parry flying text if both Miss and DodgeParry happened (eg: can happen if right weapon attack miss, and left weapon attack is dodged)
					if(ae.EventType!=EventEvade && _MissFlyingTextTriggered)
						sendFlyingText= false;

					switch(ae.EventType)
					{
					case EventEvade: PHRASE_UTILITIES::sendCombatFailedMessages( _Attacker->getEntityRowId(), ae.DefenderRowId, 
						sendFlyingText?PHRASE_UTILITIES::FailMelee:PHRASE_UTILITIES::FailNoFlyingText); break;
					case EventDodge: PHRASE_UTILITIES::sendDodgeMessages( _Attacker->getEntityRowId(), ae.DefenderRowId, sendFlyingText); break;
					case EventParry: PHRASE_UTILITIES::sendParryMessages( _Attacker->getEntityRowId(), ae.DefenderRowId, sendFlyingText); break;
					default: break;
					}
				}
				break;
			case EventMeleeDodgeOpening:
			case EventMeleeParryOpening:
				{
					// retrieve the character
					CCharacter	*character= dynamic_cast<CCharacter*>(CEntityBaseManager::getEntityBasePtr(ae.DefenderRowId));

					// if still exist
					if(character)
					{
						// update player database
						if(ae.EventType==EventMeleeDodgeOpening)
						{
							character->setCombatEventFlag(BRICK_FLAGS::Dodge);
//							sint8 prop = sint8(character->_PropertyDatabase.getProp("FLAGS:DODGE") + 1);
							sint8 prop = CBankAccessor_PLR::getFLAGS().getDODGE(character->_PropertyDatabase)+1;
							if (prop == 0) prop++; // avoid 0 to bugfix flying text poping at startup
//							character->_PropertyDatabase.setProp("FLAGS:DODGE", prop);
							CBankAccessor_PLR::getFLAGS().setDODGE(character->_PropertyDatabase, prop);
						}
						else
						{
							character->setCombatEventFlag(BRICK_FLAGS::Parry);
//							sint8 prop = sint8(character->_PropertyDatabase.getProp("FLAGS:PARRY") + 1);
							sint8 prop = CBankAccessor_PLR::getFLAGS().getPARRY(character->_PropertyDatabase)+1;
							if (prop == 0) prop++; // avoid 0 to bugfix flying text poping at startup
//							character->_PropertyDatabase.setProp("FLAGS:PARRY", prop);
							CBankAccessor_PLR::getFLAGS().setPARRY(character->_PropertyDatabase, prop);
						}

						// Force update of the BRICK_FLAGS DB. important to make BRICK_FLAGS and DODGE synchronized
						character->updateBrickFlagsDBEntry();
					}
				}
				break;
			default:
				break;
			}
		}
	}

	// reset
	_DelayedEvents.clear();
}

