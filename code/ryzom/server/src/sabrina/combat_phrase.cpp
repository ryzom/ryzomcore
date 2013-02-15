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
#include "game_share/msg_combat_move_service.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/combat_state.h"
//#include "game_share/sheath.h"
#include "game_share/entity_structure/statistic.h"

#include "phrase_utilities_functions.h"
#include "entity_base.h"
#include "character.h"
#include "egs_mirror.h"
#include "phrase_manager.h"
#include "player_manager.h"
#include "s_phrase_factory.h"

#include "combat_action_stun.h"

//////////////
//	USING	//
//////////////
using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace RY_GAME_SHARE;


//////////////
//	EXTERN	//
//////////////
extern CRandom	RandomGenerator;
extern CPhraseManager *PhraseManager;
extern CPlayerManager PlayerManager;


uint16 HandToHandDamage = 20*260/11;
uint16 HandToHandSpeed = 30; // 3s = 30 ticks



DEFAULT_SPHRASE_FACTORY( CCombatPhrase, BRICK_TYPE::COMBAT );
/*
		New = 0,
		Evaluated,
		Validated,
		Idle,
		ExecutionInProgress,
		SecondValidated,
		WaitNextCycle,
		Latent,
		LatencyEnded,

		UnknownState,
*/

bool CCombatPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks )
{
	if (actorRowId.isValid() && TheDataset.isDataSetRowStillValid(actorRowId) )
	{
		// create attacker structure
		CEntityBase *attacker = PHRASE_UTILITIES::entityPtrFromId(actorRowId);
		if (attacker == NULL)
			return false;

		if (attacker->getId().getType() == RYZOMID::player )
		{
			_Attacker = new CCombatAttackerPlayer(actorRowId);
			if (!_Attacker)
				return false;
		}
		else
		{
			_Attacker = new CCombatAttackerAI(actorRowId);
			if (!_Attacker)
				return false;
		}
	}

	// create defender structure -> wait first validation

	// add bricks
	for (uint i = 0; i < bricks.size(); i++)
	{
		addBrick( *bricks[i] );
	}

	return true;
}



//--------------------------------------------------------------
//					destructor  
//--------------------------------------------------------------
CCombatPhrase::~CCombatPhrase()
{
	if (_Attacker != NULL)
		delete _Attacker;
	if (_Defender != NULL)
		delete _Defender;
	
	const uint size = _CombatActions.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		if (_CombatActions[i] != NULL)
			delete _CombatActions[i];
	}
};


//--------------------------------------------------------------
//					constructor  
//--------------------------------------------------------------
CCombatPhrase::CCombatPhrase(const CStaticBrick &rootBrick)
{
	init();
	//_RootSkill = rootBrick.Skill;

	addBrick(rootBrick);
} // constructor //

//--------------------------------------------------------------
//					init()  
//--------------------------------------------------------------
void CCombatPhrase::init()
{
	_State							= CSPhrase::New;
	_ExecutionBehaviour				= MBEHAV::UNKNOWN_BEHAVIOUR;
	_SuccessBehaviour				= MBEHAV::UNKNOWN_BEHAVIOUR;
	_CriticalSuccessBehaviour		= MBEHAV::UNKNOWN_BEHAVIOUR;
	_FailureBehaviour				= MBEHAV::UNKNOWN_BEHAVIOUR;
	_FumbleBehaviour				= MBEHAV::UNKNOWN_BEHAVIOUR;
	_EndBehaviour					= MBEHAV::UNKNOWN_BEHAVIOUR;
	_StopBehaviour					= MBEHAV::UNKNOWN_BEHAVIOUR;

	_ForcedLocalisation				= SLOT_EQUIPMENT::UNDEFINED;

	_Attacker						= NULL;
	_Defender						= NULL;

	_RootSkill						= SKILLS::unknown;
	_AttackSkill					= SKILLS::unknown;

	_CyclicPhrase					= false;
	_Idle							= false;
	_TargetTooFarMsg				= false;
	_NotEnoughStaminaMsg			= false;
	_NotEnoughHpMsg					= false;
	_CurrentTargetIsValid			= false;
	_MeleeCombat					= true;

	_ExecutionEndDate				= 0;
	_LatencyEndDate					= 0;
	_NbWaitingRequests				= 0;

	_SabrinaCost					= 0;
	_SabrinaCredit					= 0;

	_HPCost							= 0;
	_StaminaCost					= 0;

	_AttackSkillModifier			= 0;
	_ExecutionLengthModifier		= 0;
	_HitRateModifier				= 0;
	_DamageModifier					= 0;
	_DeltaLevel						= 0;

	_PhraseSuccessDamageFactor		= 1.0f;
	
	_LightArmorAbsorptionMultiplier = 1.0f;
	_LightArmorWearMultiplier		= 1.0f;
	_MediumArmorAbsorptionMultiplier= 1.0f;
	_MediumArmorWearMultiplier		= 1.0f;
	_HeavyArmorAbsorptionMultiplier	= 1.0f;
	_HeavyArmorWearMultiplier		= 1.0f;

	_LightArmorAbsorptionModifier	= 0;
	_MediumArmorAbsorptionModifier	= 0;
	_HeavyArmorAbsorptionModifier	= 0;

	_AggroMultiplier				= 1.0f;
	_AggroModifier					= 0;

	_DamageFactor					= 1.0f;
	_StaminaLossFactor				= 0.0f;
	_StaminaLossModifier			= 0;
	_SapLossFactor					= 0.0f;
	_SapLossModifier				= 0;

	_DamagePointBlank				= 1.0f;
	_DamageShortRange				= 1.0f;
	_DamageMediumRange				= 1.0f;
	_DamageLongRange				= 1.0f;
} // init //

//--------------------------------------------------------------
//					addBrick()  
//--------------------------------------------------------------
void CCombatPhrase::addBrick( const CStaticBrick &brick )
{
	if ( brick.ForcedLocalisation != SLOT_EQUIPMENT::UNDEFINED )
	{
		if ( _ForcedLocalisation == SLOT_EQUIPMENT::UNDEFINED )
		{
			_ForcedLocalisation = brick.ForcedLocalisation;
		}
		else if (brick.ForcedLocalisation != _ForcedLocalisation)
		{
			nlwarning("<CCombatPhrase::addBrick> Phrase as a forced localisation to %s, but the new brick (name %s) has localisation %s",SLOT_EQUIPMENT::toString(_ForcedLocalisation).c_str(), brick.Name.c_str(), SLOT_EQUIPMENT::toString(brick.ForcedLocalisation).c_str());
		}
	}

	brick.SabrinaValue < 0 ? _SabrinaCredit += abs(brick.SabrinaValue) : _SabrinaCost += brick.SabrinaValue;

	// process params
	unsigned i;
	for (i=0 ; i<brick.Params.size() ; ++i)
	{
		switch(brick.Params[i]->id())
		{
		case TBrickParam::HP:
//			INFOLOG("HP: %i",((CSBrickParamHp *)brick.Params[i])->Hp);
			_HPCost += ((CSBrickParamHp *)brick.Params[i])->Hp;
			break;

		case TBrickParam::SAP:
			//printf("SAP: %i\n",((CSBrickParamSap *)brick.Params[i])->Sap);
			break;

		case TBrickParam::STA:
//			INFOLOG("STA: %i",((CSBrickParamSta *)brick.Params[i])->Sta);
			_StaminaCost += ((CSBrickParamSta *)brick.Params[i])->Sta;
			break;

		case TBrickParam::EXECUTION_LENGTH:
//			INFOLOG("EXECUTION_LENGTH: %i",((CSBrickParamExecutionLength *)brick.Params[i])->ExecutionLength);
			_ExecutionLengthModifier += ((CSBrickParamExecutionLength *)brick.Params[i])->ExecutionLength;
			break;

		case TBrickParam::LATENCY_LENGTH:
//			INFOLOG("LATENCY_LENGTH: %i",((CSBrickParamLatencyLength *)brick.Params[i])->LatencyLength);
			_HitRateModifier += ((CSBrickParamLatencyLength *)brick.Params[i])->LatencyLength;
			break;

		case TBrickParam::DMG_MOD:
//			INFOLOG("DMG_MOD: %i",((CSBrickParamDamageModifier *)brick.Params[i])->DamageModifier);
			_DamageModifier += ((CSBrickParamDamageModifier *)brick.Params[i])->DamageModifier;
			break;

		case TBrickParam::DMG_MUL:
//			INFOLOG("DMG_MUL: %i",((CSBrickParamDamageFactor *)brick.Params[i])->DamageFactor);
			_DamageFactor += ((CSBrickParamDamageFactor *)brick.Params[i])->DamageFactor - 1.0f;
			break;

		case TBrickParam::AGGRO:
//			INFOLOG("AGGRO: factor %f mod %d",((CSBrickParamAggro *)brick.Params[i])->AggroFactor, ((CSBrickParamAggro *)brick.Params[i])->AggroModifier);
			_AggroMultiplier += ((CSBrickParamAggro *)brick.Params[i])->AggroFactor - 1.0f;
			_AggroModifier += ((CSBrickParamAggro *)brick.Params[i])->AggroModifier;
			break;
			
		case TBrickParam::STA_LOSS:
//			INFOLOG("STA_LOSS: %f, %u",((CSBrickParamStaLossFactor *)brick.Params[i])->StaLossFactor, ((CSBrickParamStaLossFactor *)brick.Params[i])->StaLossModifier);
			_StaminaLossFactor += ((CSBrickParamStaLossFactor *)brick.Params[i])->StaLossFactor;
			_StaminaLossModifier += ((CSBrickParamStaLossFactor *)brick.Params[i])->StaLossModifier;
			break;

		case TBrickParam::SAP_LOSS:
//			INFOLOG("SAP_LOSS: %f, %u",((CSBrickParamSapLossFactor *)brick.Params[i])->SapLossFactor, ((CSBrickParamSapLossFactor *)brick.Params[i])->SapLossModifier);
			_SapLossFactor += ((CSBrickParamSapLossFactor *)brick.Params[i])->SapLossFactor;
			_SapLossModifier += ((CSBrickParamSapLossFactor *)brick.Params[i])->SapLossModifier;
			break;

		case TBrickParam::ATT_SKILL_MOD:
//			INFOLOG("ATT_SKILL_MOD: %f",((CSBrickParamAttackSkillModifier *)brick.Params[i])->AttackSkillModifier);
			_AttackSkillModifier += ((CSBrickParamAttackSkillModifier *)brick.Params[i])->AttackSkillModifier;
			break;

		case TBrickParam::AIM:
//			INFOLOG("AIMED SLOT: %s",((CSBrickParamAim *)brick.Params[i])->AimedSlot.c_str());
			_ForcedLocalisation = SLOT_EQUIPMENT::stringToSlotEquipment( ((CSBrickParamAim *)brick.Params[i])->AimedSlot );
			break;

		case TBrickParam::OPENING:
			INFOLOG("OPENING : %s",((CSBrickParamOpening *)brick.Params[i])->OpeningType.c_str());
			_Opening = ((CSBrickParamOpening *)brick.Params[i])->OpeningType;
			break;

		case TBrickParam::BREAK_CAST:
//			nlinfo("TODO TODO BREAK_CAST : modifier on damage for break cast test %d",((CSBrickParamBreakCast *)brick.Params[i])->DamageModifier);
			// $*STRUCT CSBrickParamBreakCast: public TBrickParam::CId <TBrickParam::BREAK_CAST>
			// $*-i sint32 DamageModifier = 0 // damage modifier only for break cast test
			break;

		case TBrickParam::RANGES:
			/*printf("RANGES: %f",	((CSBrickParamRanges *)brick.Params[i])->ShortRange);
			printf(" : %f",			((CSBrickParamRanges *)brick.Params[i])->MediumRange);
			printf(" : %f\n",		((CSBrickParamRanges *)brick.Params[i])->LongRange);
			*/
			break;

		case TBrickParam::COMBAT_STUN:
			{
		// $*STRUCT CSBrickParamCombatStun: public TBrickParam::CId <TBrickParam::COMBAT_STUN>
		// $*-f float Duration	// duration of the stun in seconds
		// $*-f float DurationResisted	// duration of the stun in seconds when resisted
		// $*-i uint16 Power // stun power (to oppose to target resistance)
//			nlinfo("COMBAT_STUN : duration %f, duration resisted %f, power %u",((CSBrickParamCombatStun *)brick.Params[i])->Duration, ((CSBrickParamCombatStun *)brick.Params[i])->DurationResisted, ((CSBrickParamCombatStun *)brick.Params[i])->Power);
			NLMISC::TGameCycle duration = NLMISC::TGameCycle( ((CSBrickParamCombatStun *)brick.Params[i])->Duration / CTickEventHandler::getGameTimeStep() );
			NLMISC::TGameCycle durationResisted = NLMISC::TGameCycle( ((CSBrickParamCombatStun *)brick.Params[i])->DurationResisted / CTickEventHandler::getGameTimeStep() );
			uint16 power = ((CSBrickParamCombatStun *)brick.Params[i])->Power;
			_CombatActions.push_back( new CCombatActionStun( _Attacker->getEntityRowId(), this, duration, durationResisted, power));
			}
			break;

		case TBrickParam::LARMOR_MOD:
//			INFOLOG("LARMOR_MOD: %f, %u",((CSBrickParamLightArmorMod *)brick.Params[i])->AborptionFactor, ((CSBrickParamLightArmorMod *)brick.Params[i])->AborptionModifier);
			_LightArmorAbsorptionMultiplier += ((CSBrickParamLightArmorMod *)brick.Params[i])->AborptionFactor;
			_LightArmorAbsorptionModifier += ((CSBrickParamLightArmorMod *)brick.Params[i])->AborptionModifier;
			break;

		case TBrickParam::MARMOR_MOD:
//			INFOLOG("MARMOR_MOD: %f, %u",((CSBrickParamMediumArmorMod *)brick.Params[i])->AborptionFactor, ((CSBrickParamMediumArmorMod *)brick.Params[i])->AborptionModifier);
			_MediumArmorAbsorptionMultiplier += ((CSBrickParamMediumArmorMod *)brick.Params[i])->AborptionFactor;
			_MediumArmorAbsorptionModifier += ((CSBrickParamMediumArmorMod *)brick.Params[i])->AborptionModifier;
			break;

		case TBrickParam::HARMOR_MOD:
//			INFOLOG("HARMOR_MOD: %f, %u",((CSBrickParamHeavyArmorMod *)brick.Params[i])->AborptionFactor, ((CSBrickParamHeavyArmorMod *)brick.Params[i])->AborptionModifier);
			_HeavyArmorAbsorptionMultiplier += ((CSBrickParamHeavyArmorMod *)brick.Params[i])->AborptionFactor;
			_HeavyArmorAbsorptionModifier += ((CSBrickParamHeavyArmorMod *)brick.Params[i])->AborptionModifier;
			break;

		default:
			break;
		}
	}
} // addBrick //


//--------------------------------------------------------------
//					evaluate()  
//--------------------------------------------------------------
bool CCombatPhrase::evaluate(CEvalReturnInfos *msg)
{
	_State = CSPhrase::Evaluated;
	
	_Idle = false;
	_PhraseSuccessDamageFactor = 0.0f;
	_AttackSkill = SKILLS::unknown;
	_Validated = false;
	_TargetTooFarMsg = false;
	_NotEnoughStaminaMsg = false;
	_NotEnoughHpMsg	= false;
	_DisengageOnEnd = false;
	//_CurrentTargetIsValid = false;
	//_MeleeCombat = true;
	return true;
} // evaluate //


//--------------------------------------------------------------
//					validate()  
//--------------------------------------------------------------
bool CCombatPhrase::validate()
{
	if (_Attacker == NULL )
	{
		nlwarning("<CCombatPhrase::validate> Found NULL attacker.");
		return false;
	}


	CEntityBase *actingEntity = _Attacker->getEntity();
	if (actingEntity == NULL)
	{
		nlwarning("<CCombatPhrase::validate> Cannot find entity ptr for acting entity %u", _Attacker->getEntityRowId().getIndex());
		_BeingProcessed = false;
		return false;
	}
	
	// set the attack flag to 0, will be set to 1 at the end of method if phrase is valid
	actingEntity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );

	_BeingProcessed = true;

	//// TEMP
	_ExecutionBehaviour			= MBEHAV::DEFAULT_ATTACK;

	_Idle = false;

	if (_State == Evaluated)
		_State = Validated;
	else if (_State == ExecutionInProgress)
		_State = SecondValidated;
	else
		nlwarning("Validate phrase while in state %u", _State );

	bool engaged = false;
	string errorCode;
	CEntityBase *defender = NULL;

	// get target
	TDataSetRow targetRowId = TheDataset.getDataSetRow(actingEntity->getTarget()); 
	if ( !_Defender || targetRowId != _Defender->getEntityRowId() )
	{
		createDefender(targetRowId);
	}

	if (!_Defender)
	{
		_CurrentTargetIsValid = false;
		errorCode = "INVALID_TARGET";
	}
	else
	{
		// check the player has engaged a target if not auto engage the target
		TDataSetRow entityRowId = PhraseManager->getEntityEngagedMeleeBy( _Attacker->getEntityRowId() );
		if (entityRowId.isValid() && TheDataset.isDataSetRowStillValid(entityRowId) )
		{
			CEntityId entityId = TheDataset.getEntityId(entityRowId);
			if ( _Defender->getEntityRowId() == entityRowId )
			{
				engaged = true;
			}
			else
			{
				engaged = false;
			}		
		}

		defender = _Defender->getEntity();
		if (defender == NULL)
		{
			_CurrentTargetIsValid = false;
			errorCode = "INVALID_TARGET";

			//nlwarning("<CCombatPhrase::validate> Cannot find entity ptr for defending entity %u", _Defender->getEntityRowId().getIndex());
			//_BeingProcessed = false;
			//if  (!engaged)
			//	PHRASE_UTILITIES::sendEngageFailedMessage( TheDataset.getEntityId(_Attacker->getEntityRowId()) );

			//return false;
		}
		else
		{
			// check opening
			if ( !_Opening.empty() && !_Validated)
			{
				// get combat initiated by current defender
				CCombat *combat = PhraseManager->getCombatInitiatedBy( _Defender->getEntityRowId() );
				
				COMBAT_HISTORY::TCombatHistory event = COMBAT_HISTORY::Unknown;

				if (combat)
					COMBAT_HISTORY::TCombatHistory event = combat->historyEvent(0);

				if (_Opening == string("Parry") && event != COMBAT_HISTORY::Parry)
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "EGS_OPENING_PARRY_FAILED");
					return false;
				}
				if (_Opening == string("Dodge") && event != COMBAT_HISTORY::Dodge)
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "EGS_OPENING_DODGE_FAILED");
					return false;
				}

				PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "EGS_OPENING_SUCCESS");		
			}
			
			_CurrentTargetIsValid = checkTargetValidity( _Defender->getEntityRowId(), errorCode);
		}
	}

	if (!_CurrentTargetIsValid)
	{
//		if (!engaged)
//			PHRASE_UTILITIES::sendEngageFailedMessage(TheDataset.getEntityId(_Attacker->getEntityRowId()));

		PHRASE_UTILITIES::sendSimpleMessage(_Attacker->getEntityRowId(), errorCode);
		idle( true );
	}
	
	// get the weapon used by the acting entity and check it
	CCombatWeapon weapon;
	CCombatWeapon ammo;
	if ( _Attacker->getItem( CCombatAttacker::RightHandItem, weapon) )
	{
		if (weapon.Family == ITEMFAMILY::MELEE_WEAPON )
		{
			_MeleeCombat = true;
		}
		else if (weapon.Family == ITEMFAMILY::RANGE_WEAPON )
		{
			_MeleeCombat = false;
			// test ammo			
			if ( _Attacker->getItem( CCombatAttacker::Ammo, ammo) )
			{
				// check ammo qty
				if ( !_Attacker->checkAmmoAmount() )
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "EGS_NOT_ENOUGH_AMMO");
					_BeingProcessed = false;
					return false;
				}

				// lock ammos
				_Attacker->lockAmmos();
			}
			else
			{
				PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_NO_AMMO");
				_BeingProcessed = false;
				return false;
			}
		}
		else
		{
			DEBUGLOG("<CCombatPhrase::validate> Entity %u, item in right hand is not a weapon", _Attacker->getEntityRowId().getIndex() );
			// ERROR -> Not a weapon
			PHRASE_UTILITIES::sendSimpleMessage(_Attacker->getEntityRowId(), "BS_ITEM_INCOMPATIBLE");
//			if  (!engaged)
//				PHRASE_UTILITIES::sendEngageFailedMessage( TheDataset.getEntityId(_Attacker->getEntityRowId()) );

			_BeingProcessed = false;
			return false;
		}
	}
	else
	{
		_MeleeCombat = true; // hand to hand
	}

	if ( _CurrentTargetIsValid && defender != 0 && TheDataset.getEntityId(_Attacker->getEntityRowId()).getType() == RYZOMID::player)
	{
		// check target is still alive
		// test the targeted entity is still alive
		const sint32 hp = defender->getScores()._PhysicalScores[ SCORES::hit_points ].Current;
		if (hp <= 0 )
		{
			nlwarning("<CCombatPhrase::validate> Entity %s is dead", TheDataset.getEntityId(_Defender->getEntityRowId()).toString().c_str());
			//errorCode = "BS_TARGET_DEAD";
			PHRASE_UTILITIES::sendSimpleMessage(_Attacker->getEntityRowId(), "BS_TARGET_DEAD");
//			if  (!engaged)
//				PHRASE_UTILITIES::sendEngageFailedMessage( TheDataset.getEntityId(_Attacker->getEntityRowId()) );

//			_BeingProcessed = false;
//			return false;
			idle(true);
			_CurrentTargetIsValid = false;
		}

		if(_MeleeCombat)
		{
			// check combat float mode
			CCharacter *character = PlayerManager.getChar(_Attacker->getEntityRowId());
			if (character && !character->getCombatFloatMode())
			{
				if (!_TargetTooFarMsg)
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_TARGET_TOO_FAR_OR");
					_TargetTooFarMsg = true;
				}

				idle( true );
				INFOLOG("<CCombatPhrase::validate> Entity %s is isn't engaged IDLE mode",TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str());
			}
		}
		else
		{
			// test range
			double range = weapon.Range + ammo.Range;
			double distance = PHRASE_UTILITIES::getDistance(_Attacker->getEntityRowId(), defender->getEntityRowId());
			if ( distance > range)
			{
				if (!_TargetTooFarMsg)
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_TARGET_TOO_FAR");
					_TargetTooFarMsg = true;
				}

				idle( true );
				INFOLOG("<CCombatPhrase::validate> Entity %s is too far from it's target %s",TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str(), defender->getId().toString().c_str());
			}			
		}
	}
	
	// check the player has engaged a target if not auto engage the target
	if  (!engaged && _CurrentTargetIsValid)
	{
		// Test entity can engage combat right now
	/*	if ( ! PhraseManager->canEntityEngageCombat(TheDataset.getEntityId(_Attacker->getEntityRowId())) )
		{
			//errorCode = "EGS_CANNOT_ENGAGE_COMBAT_YET";
			nlwarning("<CCombatPhrase::validate> Entity %s cannot engage combat yet", TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str() );
			return false;
		}
*/
		// engage the target and continue
	/*	const double d = PHRASE_UTILITIES::getDistance( TheDataset.getEntityId(_Attacker->getEntityRowId()), TheDataset.getEntityId(_Defender->getEntityRowId()) );
		if (d >= MaxEngageMeleeDistance )
		{
			PHRASE_UTILITIES::sendEngageFailedMessage(TheDataset.getEntityId(_Attacker->getEntityRowId()));
			//errorCode = "BS_TARGET_TOO_FAR";
			return false;
		}
	*/	
		if (_MeleeCombat)
		{
			if ( ! PHRASE_UTILITIES::engageTargetInMelee( TheDataset.getEntityId(_Attacker->getEntityRowId()), TheDataset.getEntityId(_Defender->getEntityRowId()) ) )
			{
				_BeingProcessed = false;
				DEBUGLOG("<CCombatPhrase::validate> Entity %u Failed to engage its target in melee combat", _Attacker->getEntityRowId().getIndex() );
				return false;
			}
		}
		else
		{
			if ( ! PHRASE_UTILITIES::engageTargetRange( TheDataset.getEntityId(_Attacker->getEntityRowId()), TheDataset.getEntityId(_Defender->getEntityRowId()) ) )
			{
				DEBUGLOG("<CCombatPhrase::validate> Entity %u Failed to engage its target in range combat", _Attacker->getEntityRowId().getIndex() );
				_BeingProcessed = false;
				return false;
			}
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
		_Idle = true;
	}

	if (!validateCombatActions(errorCode))
	{
		PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), errorCode );
		DEBUGLOG("<CCombatPhrase::validate> Entity %u Failed to validate combat actions, error = %s", _Attacker->getEntityRowId().getIndex(), errorCode.c_str() );
		return false;
	}
	

	// set the attacks flag of the acting entity
	actingEntity->setActionFlag( RYZOMACTIONFLAGS::Attacks, true );

	_Validated = true;
	_BeingProcessed = false;

	return true;
} // validate //

//--------------------------------------------------------------
//					update()  
//--------------------------------------------------------------
bool  CCombatPhrase::update()
{
	_BeingProcessed = true;

	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();

	// if the sentence execution delay time has ended, apply sentence effects
	if ( _State == SecondValidated && _ExecutionEndDate <= time && _NbWaitingRequests == 0 && !_Idle)
	{
		apply();
	}
	else if ( _State == Validated || _State == SecondValidated || _State == ExecutionInProgress )
	{
		_Idle = false;

		CEntityBase *actor = _Attacker->getEntity();
		if (!actor)
		{
			_BeingProcessed = false;
			// cannot reset the action flag, the actor cannot be found...
			return false;
		}

		// check is actor is stunned
		if (actor->isStunned())
		{
			_Idle = true;
		}

		actor->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
		
		// check target validity		
		TDataSetRow target = TheDataset.getDataSetRow(actor->getTarget());
		if (!_Defender || !TheDataset.isDataSetRowStillValid(_Defender->getEntityRowId()) || target != _Defender->getEntityRowId())
		{
			createDefender(target);

			if (!_Defender)
			{
				_CurrentTargetIsValid =false;
				_Idle = true;
			}
			else
			{			
				string errorCode;
				_CurrentTargetIsValid = checkTargetValidity( _Defender->getEntityRowId(), errorCode);
				if ( !_CurrentTargetIsValid )
				{
					PHRASE_UTILITIES::sendSimpleMessage(_Attacker->getEntityRowId(), errorCode);
					_Idle = true;
				}
				else
				{
					if (_MeleeCombat)
					{
						if ( ! PHRASE_UTILITIES::engageTargetInMelee( TheDataset.getEntityId(_Attacker->getEntityRowId()), TheDataset.getEntityId(_Defender->getEntityRowId()) ) )
						{
							DEBUGLOG("<CCombatPhrase::validate> Entity %u Failed to engage its target in melee combat", _Attacker->getEntityRowId().getIndex() );
							_BeingProcessed = false;
							return false;
						}
					}
					else
					{
						if ( ! PHRASE_UTILITIES::engageTargetRange( TheDataset.getEntityId(_Attacker->getEntityRowId()), TheDataset.getEntityId(_Defender->getEntityRowId()) ) )
						{
							DEBUGLOG("<CCombatPhrase::validate> Entity %u Failed to engage its target in range combat", _Attacker->getEntityRowId().getIndex() );
							_BeingProcessed = false;
							return false;
						}
					}
				}
			}
		}
		else
		{
			if (!_CurrentTargetIsValid)
				_Idle = true;
		}

		// check costs
		string errorCode;
		if(!checkPhraseCost(errorCode))
		{
			if (!_NotEnoughStaminaMsg && errorCode == "EGS_TOO_EXPENSIVE_STAMINA" && !_Idle )
			{
				_NotEnoughStaminaMsg = true;
				PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), errorCode );
			}
			else if (!_NotEnoughHpMsg && errorCode == "EGS_TOO_EXPENSIVE_HP" && !_Idle )
			{
				_NotEnoughHpMsg = true;
				PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), errorCode );
			}
			_Idle = true;
		}


		// check combat float mode
		if ( _CurrentTargetIsValid && TheDataset.getEntityId(_Attacker->getEntityRowId()).getType() == RYZOMID::player)
		{
			if (_MeleeCombat )
			{
				CCharacter *character = dynamic_cast<CCharacter *> (actor);
				if (character && !character->getCombatFloatMode())
				{
					if (!_TargetTooFarMsg && !_Idle)
					{
						PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_TARGET_TOO_FAR_OR");
						_TargetTooFarMsg = true;
					}

					_Idle = true;
				}
			}
			else
			{
				CCombatWeapon weapon;
				CCombatWeapon ammo;
				_Attacker->getItem( CCombatAttacker::RightHandItem, weapon);
				_Attacker->getItem( CCombatAttacker::Ammo, ammo);
				
				// test range
				double range = weapon.Range + ammo.Range;
				double distance = PHRASE_UTILITIES::getDistance(_Attacker->getEntityRowId(), _Defender->getEntityRowId());
				if ( distance > range)
				{
					if (!_TargetTooFarMsg)
					{
						PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_TARGET_TOO_FAR");
						_TargetTooFarMsg = true;
					}

					idle( true );
					INFOLOG("<CCombatPhrase::validate> Entity %s is too far from it's target %s",TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str(), _Defender->getEntity()->getId().toString().c_str());
				}	
			}
		}

		actor->setActionFlag( RYZOMACTIONFLAGS::Attacks, true );
	}
	else if ( _State == Latent && _LatencyEndDate <= time )
	{
		INFOLOG("Latency ended");
		end();
	}

	_BeingProcessed = false;

	return true;
} // update //

//--------------------------------------------------------------
//					execute()  
//--------------------------------------------------------------
void  CCombatPhrase::execute()
{
	if( _Idle || _NbWaitingRequests != 0)
		return;

	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();

	_State = CSPhrase::ExecutionInProgress;
	
	_ExecutionEndDate  = time + _ExecutionLengthModifier ;// + uint32(sentenceLatency / CTickEventHandler::getGameTimeStep()) ;

	if (_Attacker)
	{
		CCharacter* player = dynamic_cast<CCharacter*> (_Attacker->getEntity());
		if (player)
			player->setCurrentAction(CLIENT_ACTION_TYPE::Combat,_ExecutionEndDate);
	}
} // execute //

//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CCombatPhrase::apply()
{
	if ( !_Attacker || !_Defender) 
	{
		nlwarning("<CCombatPhrase::validate> Found NULL attacker or defender.");
		return;
	}

	// spend stamina, hp
	CEntityBase* actingEntity = PHRASE_UTILITIES::entityPtrFromId( _Attacker->getEntityRowId() );
	if (actingEntity == NULL)
	{
		nlwarning("<CCombatPhrase::apply> Invalid entity Id %s", TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str() );
		_BeingProcessed = false;
		return;
	}

	actingEntity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );

	CEntityBase *defender = _Defender->getEntity();
	if (!defender)
	{
		nlwarning("<CCombatPhrase::apply> Cannot get entity base pointer, error");
		return;
	}	

	// get combat object
	CCombat *combat = PhraseManager->getCombatInitiatedBy( _Attacker->getEntityRowId() );
	if (!combat)
		return;

	_BeingProcessed = true;
	_TargetTooFarMsg = false;

	_State = CSPhrase::Latent;
	_ExecutionBehaviour.Data = 0;	

	_AiEventReport.init();

	_AiEventReport.Originator = _Attacker->getEntityRowId();
	_AiEventReport.Target = _Defender->getEntityRowId();
	_AiEventReport.Type = ACTNATURE::OFFENSIVE;


	if (_StaminaCost != 0)
	{
		SCharacteristicsAndScores &stamina = actingEntity->getScores()._PhysicalScores[SCORES::stamina];
		if ( stamina.Current != 0 )
		{
//			nlinfo("Stamina current = %d, cost %d", stamina.Current.getValue(), _StaminaCost);
			stamina.Current = stamina.Current - _StaminaCost;
			if (stamina.Current < 0)
				stamina.Current = 0;
		}
	}

	if ( _HPCost != 0)
	{
		actingEntity->changeCurrentHp( (_HPCost) * (-1) );		
	}

	CCombatWeapon weapon;
	CCombatWeapon ammo;
	if ( _Attacker->getItem( CCombatAttacker::RightHandItem, weapon) )
	{
		if (weapon.Family == ITEMFAMILY::MELEE_WEAPON )
		{
			_MeleeCombat = true;
		}
		else if (weapon.Family == ITEMFAMILY::RANGE_WEAPON )
		{
			if ( _Attacker->getItem( CCombatAttacker::Ammo, ammo) )
			{
				// check ammo qty
				if ( !_Attacker->checkAmmoAmount() )
				{
					PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "EGS_NOT_ENOUGH_AMMO");
					_BeingProcessed = false;
					return;
				}

				// lock ammos
				_Attacker->lockAmmos();
			}
			else
			{
				PHRASE_UTILITIES::sendSimpleMessage( _Attacker->getEntityRowId(), "BS_NO_AMMO");
				_BeingProcessed = false;
				return;
			}  
		}
		else
		{
			// ERROR -> Not a weapon
			PHRASE_UTILITIES::sendSimpleMessage(_Attacker->getEntityRowId(), "BS_ITEM_INCOMPATIBLE");
			_BeingProcessed = false;
			return;
		}

		weapon.SkillValue = _Attacker->getSkillValue(weapon.Skill);
	}
	else
	{
		weapon.Damage = HandToHandDamage;
		weapon.DmgType = DMGTYPE::BLUNT;
		weapon.SpeedInTicks = HandToHandSpeed;

// TODO skill BareHandCombat missing 		
//		weapon.Quality = (uint16)PHRASE_UTILITIES::getEntityLevel( _Attacker->getEntityRowId(), SKILLS::BareHandCombat);
		weapon.Quality = (uint16)PHRASE_UTILITIES::getEntityLevel( _Attacker->getEntityRowId(), SKILLS::SFM1H);
		
// TODO
		//weapon.SkillValue = 10;//_Attacker->getEntity()->getSkills()._Skills[ SKILLS::BareHandCombat ].Current;//(uint16)PHRASE_UTILITIES::getEntityLevel( _Attacker->getEntityRowId(), SKILLS::BareHandCombat);
		weapon.SkillValue = _Attacker->getEntity()->getSkills()._Skills[ SKILLS::SFM1H ].Current;
		weapon.Skill = SKILLS::SFM1H;//SKILLS::BareHandCombat;
	}
	
	
// compute latency end date
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	_LatencyEndDate = time + _HitRateModifier + weapon.SpeedInTicks + ammo.SpeedInTicks ;	

// test phrase success
	sint32 damage = 0;
	if (!testPhraseSuccess())
	{
		// Total failure
		// send miss message		
		_ExecutionBehaviour.Combat.DamageType = (ammo.Damage != 0) ? ammo.DmgType : weapon.DmgType;
		_ExecutionBehaviour.Combat.AttackIntensity = PHRASE_UTILITIES::getAttackIntensity(_SabrinaCost / 10);
		_ExecutionBehaviour.Combat.ImpactIntensity = 0;
		_ExecutionBehaviour.Combat.KillingBlow = 0;
		
		if (_PhraseSuccessDamageFactor == 0.0f)
		{
			// failed
			combat->pushEvent( COMBAT_HISTORY::Miss );			
			PHRASE_UTILITIES::sendCombatResistMessages( _Attacker->getEntityRowId(), _Defender->getEntityRowId() );
		}
		else if (_PhraseSuccessDamageFactor < 0.0f)
		{
			// fumble
			combat->pushEvent( COMBAT_HISTORY::Fumble );
			PHRASE_UTILITIES::sendFumbleMessage( _Attacker->getEntityRowId(), _Defender->getEntityRowId() );
			// double latency !
			_LatencyEndDate = (_LatencyEndDate-time) * 2 + time;
		}		
	}
	else
	{
		// get shield 
		CCombatShield shield;
		_Defender->getShield(shield);
		
		// determine localisation	
		PHRASE_UTILITIES::TPairSlotShield localisation;
		sint8 adjustment = PHRASE_UTILITIES::getLocalisationSizeAdjustement( _Attacker->getEntityRowId(), _Defender->getEntityRowId() );
		if ( _ForcedLocalisation != SLOT_EQUIPMENT::UNDEFINED )
			localisation = PHRASE_UTILITIES::getLocalisation( shield.ShieldType, adjustment, _ForcedLocalisation );
		else
			localisation = PHRASE_UTILITIES::getLocalisation( shield.ShieldType, adjustment );

		// test against opponent defense
		if ( testOpponentDefense( _Defender->getEntityRowId(), localisation ) )
		{
			// opponent dodged the attack, send messages
			DEBUGLOG("<CCombatPhrase::execute> Actor %u, target %u has dodged", _Attacker->getEntityRowId().getIndex(), _Defender->getEntityRowId().getIndex() );

			// update the actor behaviour
			_ExecutionBehaviour.Combat.DamageType = (ammo.Damage != 0) ? ammo.DmgType : weapon.DmgType;
			_ExecutionBehaviour.Combat.AttackIntensity = PHRASE_UTILITIES::getAttackIntensity(_SabrinaCost / 10);
			_ExecutionBehaviour.Combat.ImpactIntensity = 0;
			_ExecutionBehaviour.Combat.KillingBlow = 0;

			if (defender->dodgeAsDefense())
				combat->pushEvent( COMBAT_HISTORY::Dodge );
			else
				combat->pushEvent( COMBAT_HISTORY::Parry );
		}
		else
		{
			if (_PhraseSuccessDamageFactor > 1.0f)
			{
				// critical strike
				PHRASE_UTILITIES::sendCriticalHitMessage( _Attacker->getEntityRowId(), _Defender->getEntityRowId() );
			}

			// Compute damage
			sint32 attackerLevel;

			uint16 itemQuality = _MeleeCombat ? weapon.Quality : ammo.Quality;
			if ( (weapon.SkillValue/10) >= itemQuality )
			{
				// attacker level is the same or higher, compute the mean between attacker and weapon level
				attackerLevel = (weapon.SkillValue + itemQuality*10)/2;
			}
			else
			{
				// attacker level is lower, give him a +10 bonus to skill
				attackerLevel = 10 + weapon.SkillValue;
			}

			sint32 weaponDamage = _MeleeCombat ? weapon.Damage : ammo.Damage;			
			damage = sint32( ( weaponDamage * ( 10.0f + float(attackerLevel)) / 260.0f + _DamageModifier ) * _PhraseSuccessDamageFactor );
			sint32 damageBeforeArmor = damage;

			// get damage amplifier on target
			const CSEffect * effect = defender->lookForSEffect( EFFECT_FAMILIES::MeleeDmgAmpli );
			if ( effect )
				damage *=  effect->getParamValue() / 100;

			//  armor
			CCombatArmor armor;
			_Defender->getArmor( localisation.first, armor);
			
			// compute armor and shield protections
			sint32 defenderArmorLevel = ((armor.SkillValue/10) >= armor.Quality) ? ((weapon.SkillValue + armor.Quality)*10)/2 : 10+armor.SkillValue;
			sint32 defenderShieldLevel = ((shield.SkillValue/10) >= shield.Quality) ? ((shield.SkillValue + shield.Quality)*10)/2 : 10+shield.SkillValue;

			DMGTYPE::EDamageType dmgType = (ammo.Damage != 0) ? ammo.DmgType : weapon.DmgType;
			
			uint32 dmgPreventedByShield = 0;
			uint32 dmgPreventedByArmor = 0;

			// get modifier according to armor type
			float shieldAbsorptionFactor = 1.0;
			sint32 shieldAbsorptionModifier = 0;
			float armorAbsorptionFactor = 1.0;
			sint32 armorAbsorptionModifier = 0;

			switch(armor.ArmorType)
			{
			case ARMORTYPE::LIGHT:
				armorAbsorptionFactor = _LightArmorAbsorptionMultiplier;
				armorAbsorptionModifier = _LightArmorAbsorptionModifier;
				break;
			case ARMORTYPE::MEDIUM:
				armorAbsorptionFactor = _MediumArmorAbsorptionMultiplier;
				armorAbsorptionModifier = _MediumArmorAbsorptionModifier;
				break;
			case ARMORTYPE::HEAVY:
				armorAbsorptionFactor = _HeavyArmorAbsorptionMultiplier;
				armorAbsorptionModifier = _HeavyArmorAbsorptionModifier;
				break;
			default:;
			};

			switch(shield.ArmorType)
			{
			case ARMORTYPE::LIGHT:
				shieldAbsorptionFactor = _LightArmorAbsorptionMultiplier;
				shieldAbsorptionModifier = _LightArmorAbsorptionModifier;
				break;
			case ARMORTYPE::MEDIUM:
				shieldAbsorptionFactor = _MediumArmorAbsorptionMultiplier;
				shieldAbsorptionModifier = _MediumArmorAbsorptionModifier;
				break;
			case ARMORTYPE::HEAVY:
				shieldAbsorptionFactor = _HeavyArmorAbsorptionMultiplier;
				shieldAbsorptionModifier = _HeavyArmorAbsorptionModifier;
				break;
			default:;
			};

			switch( dmgType)
			{
			case DMGTYPE::BLUNT:
				if (localisation.second)
				{
					dmgPreventedByShield += (uint32)(shield.MaxBluntProtection <  (uint32)(0.01 * shield.BluntProtectionFactor * damage) ? shield.MaxBluntProtection : 0.01 * shield.BluntProtectionFactor * damage );					
					dmgPreventedByShield = (uint32) ((10.0f+ float(defenderShieldLevel))/260.0f * dmgPreventedByShield);
					dmgPreventedByShield = max( sint32(0), sint32(dmgPreventedByShield * shieldAbsorptionFactor) + shieldAbsorptionModifier );
					damage -= dmgPreventedByShield;
				}
				dmgPreventedByArmor += (uint32)(armor.MaxBluntProtection <  (uint32)(0.01 * armor.BluntProtectionFactor * damage) ? armor.MaxBluntProtection : 0.01 * armor.BluntProtectionFactor * damage );
				dmgPreventedByArmor = (uint32) ((10.0f+ float(defenderArmorLevel))/260.0f * dmgPreventedByArmor);
				dmgPreventedByArmor = max( sint32(0), sint32(dmgPreventedByArmor * armorAbsorptionFactor) + armorAbsorptionModifier );
				damage -= dmgPreventedByArmor;
				break;
			case DMGTYPE::SLASHING:
				if (localisation.second)
				{
					dmgPreventedByShield += (uint32)(shield.MaxSlashingProtection <  (uint32)(0.01 * shield.SlashingProtectionFactor * damage) ? shield.MaxSlashingProtection : 0.01 * shield.SlashingProtectionFactor * damage );
					dmgPreventedByShield = (uint32) ((10.0f+ float(defenderShieldLevel))/260.0f * dmgPreventedByShield);
					dmgPreventedByShield = max( sint32(0), sint32(dmgPreventedByShield * shieldAbsorptionFactor) + shieldAbsorptionModifier );
					damage -= dmgPreventedByShield;
				}
				dmgPreventedByArmor += (uint32)(armor.MaxSlashingProtection <  (uint32)(0.01 * armor.SlashingProtectionFactor * damage) ? armor.MaxSlashingProtection : 0.01 * armor.SlashingProtectionFactor * damage );
				dmgPreventedByArmor = (uint32) ((10.0f+ float(defenderArmorLevel))/260.0f * dmgPreventedByArmor);
				dmgPreventedByArmor = max( sint32(0), sint32(dmgPreventedByArmor * armorAbsorptionFactor) + armorAbsorptionModifier );
				damage -= dmgPreventedByArmor;
				break;
			case DMGTYPE::PIERCING:
				if (localisation.second)
				{
					dmgPreventedByShield += (uint32)(shield.MaxPiercingProtection <  (uint32)(0.01 * shield.PiercingProtectionFactor * damage) ? shield.MaxPiercingProtection : 0.01 * shield.PiercingProtectionFactor * damage );
					dmgPreventedByShield = (uint32) ((10.0f+ float(defenderShieldLevel))/260.0f * dmgPreventedByShield);
										dmgPreventedByShield = max( sint32(0), sint32(dmgPreventedByShield * shieldAbsorptionFactor) + shieldAbsorptionModifier );
					damage -= dmgPreventedByShield;
				}
				dmgPreventedByArmor += (uint32)(armor.MaxPiercingProtection <  (uint32)(0.01 * armor.PiercingProtectionFactor * damage) ? armor.MaxPiercingProtection : 0.01 * armor.PiercingProtectionFactor * damage );				
				dmgPreventedByArmor = (uint32) ((10.0f+ float(defenderArmorLevel))/260.0f * dmgPreventedByArmor);
				dmgPreventedByArmor = max( sint32(0), sint32(dmgPreventedByArmor * armorAbsorptionFactor) + armorAbsorptionModifier );
				damage -= dmgPreventedByArmor;
				break;
			default:
				break;
			};

			if ( dmgPreventedByShield != 0)
			{
				INFOLOG("Entity %u (attacked by %u) Damage prevented by the shield = %u", _Defender->getEntityRowId().getIndex(), _Attacker->getEntityRowId().getIndex(), dmgPreventedByShield);
				// remove shield hit points
				// compute the real _DamageAmount of hp removed  for the shield
				_Defender->damageOnShield( dmgPreventedByShield );
				//_ShieldLostHp[targetIndex] += (uint32) (dmgPreventedByShield * ( 1 + shieldDeltaLevel * 0.1));
				//sentence->LogReportStructure.ShieldAbsorption += dmgPreventedByShield;
			}

			if ( dmgPreventedByArmor != 0)
			{		
				_Defender->damageOnArmor(localisation.first, dmgPreventedByArmor );
				INFOLOG("Entity %u (attacked by %u) Damage prevented by the armor = %u",_Defender->getEntityRowId().getIndex(), _Attacker->getEntityRowId().getIndex(), dmgPreventedByArmor);
			}

			_ExecutionBehaviour.Combat.DamageType = dmgType;
			_ExecutionBehaviour.Combat.AttackIntensity = PHRASE_UTILITIES::getAttackIntensity(_SabrinaCost / 10);
			_ExecutionBehaviour.Combat.ImpactIntensity = PHRASE_UTILITIES::getImpactIntensity( damage, _Defender->getEntityRowId() );

			if (damage == 0)
			{
				// MAY BE AN BUG -> LOG MANY INFOS FOR DEBUG
				nlwarning("COMBAT : Entity %s hits entity %s but does 0 damage.", actingEntity->getId().toString().c_str(), defender->getId().toString().c_str() );
				nlwarning("Attacker weapon infos : %s", weapon.toString());
				nlwarning("attackerLevel = %d, weaponDamage : %d", attackerLevel, weaponDamage);
				nlwarning("");
				nlwarning("Defender armor infos : %s",	armor.toString());
				nlwarning("Damage before effects and armor : %d",	damageBeforeArmor);
				nlwarning("Damage prevented by armor : %d",	dmgPreventedByArmor);
				nlwarning("Damage prevented by shield : %d",	dmgPreventedByShield);
			}

		// apply damage
			if (damage > 0)
			{
				applyCombatActions();

				_ExecutionBehaviour.DeltaHP = (sint16)((-1)*damage);
					
				if ( defender->changeCurrentHp( (-1)*damage ) == true)
				{
					// entity has been killed, change the bahaviour of the attacker to set the flag
					_ExecutionBehaviour.Combat.KillingBlow = 1;

					// send mission event
					if ( actingEntity->getId().getType()== RYZOMID::player )
					{
						CMissionEventKill event ( _Defender->getEntityRowId() );
						((CCharacter*) actingEntity)->processMissionEvent( event );
					}
				}
				else
				{
					CPhraseManager::getInstance()->breakCast(attackerLevel,actingEntity,defender);
				}

			// add modifier to sentence AI event reports
				// TODO 

			// test spell break
				//PHRASE_UTILITIES::testSpellBreakOnDamage( _Defender->getEntityRowId(), _Attacker->getEntityRowId(), damage, damageType);
				// TODO

				sint32 lostStamina = (sint32)( damage * _StaminaLossFactor + _StaminaLossModifier);
				sint32 lostSap = (sint32)(damage * _SapLossFactor + _SapLossModifier);

				if ( lostStamina != 0 )
				{
					INFOLOG("entity %s lose %d stamina", TheDataset.getEntityId(_Defender->getEntityRowId()).toString().c_str(),lostStamina );
					defender->getPhysScores()._PhysicalScores[SCORES::stamina].Current = defender->getPhysScores()._PhysicalScores[SCORES::stamina].Current - lostStamina;

					//TEMPFIX
					// clip score to 0
					if ( defender->getPhysScores()._PhysicalScores[SCORES::stamina].Current < 0 )
						defender->getPhysScores()._PhysicalScores[SCORES::stamina].Current = 0;

					// add modifier to sentence AI event reports
					_AiEventReport.addDelta(AI_EVENT_REPORT::Stamina, (-1)*lostStamina);
				}
				if ( lostSap != 0 )
				{
					//value = toString( _BaseSapAbsorption[i] );
					INFOLOG("entity %s lose %d sap", TheDataset.getEntityId(_Defender->getEntityRowId()).toString().c_str(), lostSap );
					defender->getPhysScores()._PhysicalScores[SCORES::sap].Current = defender->getPhysScores()._PhysicalScores[SCORES::sap].Current - lostSap;

					//TEMPFIX
					// clip score to 0
					if ( defender->getPhysScores()._PhysicalScores[SCORES::sap].Current < 0 )
						defender->getPhysScores()._PhysicalScores[SCORES::sap].Current = 0;

					// add modifier to sentence AI event reports
					_AiEventReport.addDelta(AI_EVENT_REPORT::Sap, (-1)*lostSap);
				}

			// send chat messages
				PHRASE_UTILITIES::sendHitMessages(_Attacker->getEntityRowId(),_Defender->getEntityRowId(), damage, lostStamina, lostSap);
					
				if ( _ExecutionBehaviour.Combat.KillingBlow == 1 )
					PHRASE_UTILITIES::sendDeathMessages( _Attacker->getEntityRowId(), _Defender->getEntityRowId() );

				// send report for xp gain if actor is a player
				if (_Attacker->getEntity()->getId().getType() == RYZOMID::player )
					((CCharacter*)(_Attacker->getEntity()))->actionReport( _Defender->getEntity(), _DeltaLevel, ACTNATURE::OFFENSIVE, SKILLS::toString(_AttackSkill) );

			}
			else
				PHRASE_UTILITIES::sendHitNullMessages( _Attacker->getEntityRowId(), _Defender->getEntityRowId() );

			INFOLOG("Entity %s hits entity %s does %u damage", TheDataset.getEntityId(_Attacker->getEntityRowId()).toString().c_str(), TheDataset.getEntityId(_Defender->getEntityRowId()).toString().c_str(), damage );


// --- TODO : INSERT THE REAL HIT LOCATION + CRITICAL HIT MANAGEMENT
			if ( !_ExecutionBehaviour.Combat.KillingBlow ) 
				combat->pushEvent( COMBAT_HISTORY::HitChest );
		}
	}


	// update the actor behaviour
	if ( _ExecutionBehaviour.Behaviour != MBEHAV::UNKNOWN_BEHAVIOUR )
		PHRASE_UTILITIES::sendUpdateBehaviour( _Attacker->getEntityRowId(), _ExecutionBehaviour );

	// compute aggro
	sint32 aggro = 0;
	const sint32 maxPv = defender->getPhysScores()._PhysicalScores[SCORES::hit_points].Max;
	if (maxPv)
	{
		aggro = (-1) * sint32((100.0 * double(damage + _AggroModifier))/double(maxPv) * _AggroMultiplier) ;
	}

	// update the repor
	_AiEventReport.AggroMul = 1.0f;
	_AiEventReport.AggroAdd = aggro;
	_AiEventReport.addDelta(AI_EVENT_REPORT::HitPoints, (-1)*damage);

	PhraseManager->addAiEventReport(_AiEventReport);

	CCharacter* player = dynamic_cast<CCharacter*> (_Attacker->getEntity());
	if (player)
		player->setCurrentAction(CLIENT_ACTION_TYPE::Combat,_LatencyEndDate);

	actingEntity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );

	_BeingProcessed = false;
} // apply //

//--------------------------------------------------------------
//					end()  
//--------------------------------------------------------------
void CCombatPhrase::end()
{
	if (!_Attacker) return;

	_BeingProcessed = true;

	_Attacker->unlockRightItem();
	
	// set the attacks flag of the acting entity
	CEntityBase *entityPtr = PHRASE_UTILITIES::entityPtrFromId(_Attacker->getEntityRowId());
	if (entityPtr)
		entityPtr->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );

	_State = CSPhrase::LatencyEnded;

	if (_DisengageOnEnd)
	{
		PhraseManager->disengage( _Attacker->getEntityRowId(), false, true);
	}

	_BeingProcessed = false;

	CCharacter* player = dynamic_cast<CCharacter*> (entityPtr);
	if (player)
		player->clearCurrentAction();
} // end //




//--------------------------------------------------------------
//					testOpponentDefense()  
//--------------------------------------------------------------
bool CCombatPhrase::testOpponentDefense(const TDataSetRow &targetRowId, const PHRASE_UTILITIES::TPairSlotShield &localisation)
{
	sint32 attackerLevel = 0;
	CCombatWeapon weapon;
	if (_Attacker->getItem( CCombatAttacker::RightHandItem, weapon) )
	{
		_AttackSkill = weapon.Skill;
		attackerLevel = _AttackSkillModifier + ((weapon.SkillValue/10) >= weapon.Quality) ? ((weapon.SkillValue + weapon.Quality)*10)/2 : 10 + weapon.SkillValue;
		attackerLevel /= 10;
	}
	else
	{
		if ( _RootSkill == SKILLS::unknown )
//TODO barehandedcombat
			_AttackSkill = SKILLS::SFM1H;//SKILLS::BareHandCombat;
		else
			_AttackSkill = _RootSkill;

		attackerLevel = PHRASE_UTILITIES::getEntityLevel(_Attacker->getEntityRowId(), _AttackSkill,_AttackSkillModifier);
	}

	// defender has a malus of one level per attacker beyond the first one
	const set<TDataSetRow> &aggressors = PhraseManager->getMeleeAggressors(targetRowId);
	sint32 malus = 0;
	sint32 nb = aggressors.size();
	if (nb)
		malus = (1-nb)*10;
	
	sint32 valueTemp = _Defender->getDefenseValue();
	sint32 defenderLevel = max( sint32((valueTemp + malus ) / 10), (sint32)0);

	_DeltaLevel = attackerLevel - defenderLevel;

	// oposition test
	uint8 chances = PHRASE_UTILITIES::getSuccessChance( defenderLevel - attackerLevel );

// TEMP HACK : limit dodge chances for the moment, wait new rules (more 'fun')
chances /= 3;


	float result = PHRASE_UTILITIES::getSucessFactor(chances, (uint8)RandomGenerator.rand(99) );

	if (result>=1.0f)
	{
		if ( !_Defender->getEntity()->dodgeAsDefense() )
		{
			PHRASE_UTILITIES::sendMessage( _Attacker->getEntityRowId(), string("EGS_ACTOR_COMBAT_PARRY_E"), targetRowId);
			PHRASE_UTILITIES::sendMessage( targetRowId, string("EGS_TARGET_COMBAT_PARRY_E"), _Attacker->getEntityRowId());
		}
		else
		{
			PHRASE_UTILITIES::sendMessage( _Attacker->getEntityRowId(), string("EGS_ACTOR_COMBAT_DODGE_E"), targetRowId);
			PHRASE_UTILITIES::sendMessage( targetRowId, string("EGS_TARGET_COMBAT_DODGE_E"), _Attacker->getEntityRowId());
		}

		return true;
	}

	return false;
} // testOpponentDefense //


//--------------------------------------------------------------
//					testPhraseSuccess()  
//--------------------------------------------------------------
bool CCombatPhrase::testPhraseSuccess()
{
	sint32 skillValue;
	if ( _RootSkill != SKILLS::unknown )
		skillValue = _Attacker->getSkillValue( _RootSkill );
	else
	{
//TODO skill barehanded combat
		CCombatWeapon weapon;
		if (_Attacker->getItem( CCombatAttacker::RightHandItem, weapon) )
		{
			skillValue = weapon.SkillValue;
		}
		else
		{
			//skillValue = _Attacker->getSkillValue( SKILLS::BareHandCombat );
			skillValue = _Attacker->getSkillValue( SKILLS::SFM1H );
		}
	}

	const sint16 relativeLevel = (skillValue + _SabrinaCredit - 2*_SabrinaCost) / 10;

	// oposition test
	uint8 chances = PHRASE_UTILITIES::getSuccessChance( relativeLevel );

	_PhraseSuccessDamageFactor = PHRASE_UTILITIES::getSucessFactor(chances, (uint8)RandomGenerator.rand(99) );
//	nlinfo("_PhraseSuccessDamageFactor = %f, (skill+bonus = %d, difficulty = %d)", _PhraseSuccessDamageFactor, (skillValue + _SabrinaCredit - _SabrinaCost), _SabrinaCost);

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
	if ( !targetRowId.isValid() || !TheDataset.isDataSetRowStillValid(targetRowId) )
	{
		errorCode = "BS_INVALID_TARGET";
		return false;
	}

	if (targetRowId == _Attacker->getEntityRowId() && TheDataset.getEntityId(_Attacker->getEntityRowId()).getType() == RYZOMID::player)
	{
		errorCode = "BS_INVALID_TARGET";
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
	if (!_Attacker || !_Attacker->getEntity()) return false;

	// only check costs for players
	if ( TheDataset.getEntityId(_Attacker->getEntityRowId()).getType() != RYZOMID::player)
		return true;

	const SCharacteristicsAndScores &stamina = _Attacker->getEntity()->getScores()._PhysicalScores[SCORES::stamina];
	if ( stamina.Current < _StaminaCost)
	{
		errorCode = "EGS_TOO_EXPENSIVE_STAMINA";
		return false;
	}

	const SCharacteristicsAndScores &hp = _Attacker->getEntity()->getScores()._PhysicalScores[SCORES::hit_points];
	if ( hp.Current < _HPCost)
	{
		errorCode = "EGS_TOO_EXPENSIVE_HP";
		return false;
	}

	return true;
} // checkPhraseCost //


//--------------------------------------------------------------
//					createDefender()  
//--------------------------------------------------------------
void CCombatPhrase::createDefender( const TDataSetRow &targetRowId )
{
	if (_Defender != NULL)
	{
		delete _Defender;
		_Defender = NULL;
	}

	if ( !targetRowId.isValid() || !TheDataset.isDataSetRowStillValid(targetRowId) )
		return;

// create defender structure
	if ( TheDataset.getEntityId(targetRowId).getType() == RYZOMID::player )
	{
		_Defender = new CCombatDefenderPlayer(targetRowId);
	}
	else
	{
		_Defender = new CCombatDefenderAI(targetRowId);
	}
} // createDefender //


//--------------------------------------------------------------
//					validateCombatActions()  
//--------------------------------------------------------------
bool CCombatPhrase::validateCombatActions( string &errorCode )
{
	const uint size = _CombatActions.size();
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
	if (!_Defender) return;

	const uint size = _CombatActions.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		if ( _CombatActions[i] != NULL)
		{
			_CombatActions[i]->setTarget( _Defender->getEntityRowId() );
			_CombatActions[i]->apply(this);
		}
	}
} // validateCombatActions //

