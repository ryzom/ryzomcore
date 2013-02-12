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

#include "magic_action_attack.h"

#include "phrase_manager/magic_phrase.h"
#include "creature_manager/creature.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "game_share/magic_fx.h"
#include "phrase_manager/s_effect.h"
#include "phrase_manager/phrase_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "creature_manager/creature_manager.h"
#include "egs_sheets/egs_static_ai_action.h"
#include "bounce_effect.h"
#include "progression/progression_pvp.h"

using namespace NLNET;
using namespace NLMISC;
using namespace std;

extern CPlayerManager PlayerManager;
extern CCreatureManager CreatureManager;

vector< pair< string , IMagicActionFactory* > >* IMagicActionFactory::Factories = NULL;
vector< pair< AI_ACTION::TAiActionType , IMagicAiActionFactory* > >* IMagicAiActionFactory::Factories = NULL;
vector< pair< AI_ACTION::TAiEffectType , IMagicAiActionFactory* > >* IMagicAiActionFactory::SpecializedActionFactories = NULL;


BEGIN_MAGIC_ACTION_FACTORY(CMagicActionBasicDamage)
	ADD_MAGIC_ACTION_TYPE( "moet" )	
END_MAGIC_ACTION_FACTORY(CMagicActionBasicDamage)


CMagicActionBasicDamage::CMagicActionBasicDamage()
: _DmgHp(0)
, _DmgSap(0)
, _DmgSta(0)
, _VampirismValue(0)
, _DmgType(DMGTYPE::UNDEFINED)
{
}

bool CMagicActionBasicDamage::initFromAiAction(CStaticAiAction const* aiAction, CMagicPhrase* phrase)
{
#ifdef NL_DEBUG
	nlassert(phrase);
	nlassert(aiAction);
#endif
	if (aiAction->getType() != AI_ACTION::DamageSpell ) return false;
	
	sint32 damageBonus = 0;
	CCreature *creature = CreatureManager.getCreature( phrase->getActor() );
	if (creature && creature->getForm())
		damageBonus = sint32(aiAction->getData().Spell.SpellPowerFactor * creature->getForm()->getAttackLevel());
	
	switch(aiAction->getData().Spell.AffectedScore)
	{
	case SCORES::sap:
		_DmgSap = sint32(aiAction->getData().Spell.SpellParamValue + damageBonus);
		break;
	case SCORES::stamina:
		_DmgSta = sint32(aiAction->getData().Spell.SpellParamValue + damageBonus);
		break;
	case SCORES::hit_points:
		_DmgHp = sint32(aiAction->getData().Spell.SpellParamValue + damageBonus);
		break;
	default:
		return false;
	};
	
	_VampirismValue = (sint32)aiAction->getData().Spell.SpellParamValue2;
	
	_DmgType = aiAction->getData().Spell.DamageType;
	phrase->setMagicFxType(MAGICFX::toMagicFx( _DmgType, false), 1);
	return true;
}

bool CMagicActionBasicDamage::addBrick(CStaticBrick const& brick, CMagicPhrase* phrase, bool& effectEnd, CBuildParameters& buildParams)
{
	for ( uint i=0 ; i<brick.Params.size() ; ++i)
	{
		const TBrickParam::IId* param = brick.Params[i];
		
		switch(param->id())
		{
		case TBrickParam::MA_END:
			INFOLOG("MA_END Found: end of effect");
			effectEnd = true;
			return true;
			
		case TBrickParam::MA_DMG_TYPE:
			INFOLOG("MA_DMG_TYPE: %s",((CSBrickParamMagicDmgType *)param)->DmgType.c_str());
			_DmgType = DMGTYPE::stringToDamageType( ((CSBrickParamMagicDmgType *)param)->DmgType );
			if ( _DmgType == DMGTYPE::UNDEFINED )
			{
				nlwarning("<CMagicActionBasicDamage addBrick> invalid dmg type %s", ((CSBrickParamMagicDmgType *)param)->DmgType.c_str());
				return false;
			}
			// set the spellId of the main phrase
			phrase->setMagicFxType(MAGICFX::toMagicFx( _DmgType, false), brick.SabrinaValue);				
			break;
			
		case TBrickParam::MA_DMG:
			INFOLOG("MA_DMG: %u %u %u",((CSBrickParamMagicDmg *)param)->Hp,((CSBrickParamMagicDmg *)param)->Sap,((CSBrickParamMagicDmg *)param)->Sta);
			_DmgHp = ((CSBrickParamMagicDmg *)param)->Hp;
			_DmgSap = ((CSBrickParamMagicDmg *)param)->Sap;
			_DmgSta = ((CSBrickParamMagicDmg *)param)->Sta;
			break;
		default:
			// unused param, can be useful in the phrase
			phrase->applyBrickParam( param, brick, buildParams );
			break;
		}
	}
	///\todo nico: check if everything is set
	return true;
}

bool CMagicActionBasicDamage::validate(CMagicPhrase* phrase, std::string& errorCode)
{
#if !FINAL_VERSION
	nlassert(phrase);
	nlassert(!phrase->getTargets().empty());
#endif
	const TDataSetRow mainTarget = phrase->getTargets()[0].getId();
	if( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),mainTarget,ACTNATURE::OFFENSIVE_MAGIC, errorCode, true) )
	{
		//			PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
		return false;
	}
	return true;
}

bool CMagicActionBasicDamage::launchOnEntity(
	CMagicPhrase*		phrase,
	CEntityBase*		actor,
	CEntityBase*		target,
	bool				mainTarget,
	uint32				casterSkillvalue,
	uint32				casterSkillBaseValue,
	MBEHAV::CBehaviour&	behav,
	bool				isMad,
	float				rangeFactor,
	CTargetInfos&		targetInfos)
{
	// for main target only, check reflect damage effects
	if (mainTarget)
	{
		const CSEffectPtr effect = target->lookForActiveEffect(EFFECT_FAMILIES::ReflectDamage);
		if ( effect )
		{
			target = actor;
			isMad = true;
		}
	}

	// change caster skill according to target race (for special items that gives a skill bonus against Kitins for exemple)
	casterSkillvalue += actor->getSkillModifierForRace(target->getRace());

	// init target infos
	targetInfos.RowId			= target->getEntityRowId();
	targetInfos.MainTarget		= mainTarget;
	targetInfos.DmgHp			= 0;
	targetInfos.ResistFactor	= 1.0f;
	targetInfos.DmgFactor		= 1.0f;
	targetInfos.Immune			= false;

	targetInfos.ReportAction.TargetRowId = target->getEntityRowId();

	if( target->getId().getType() != RYZOMID::player )
	{
		const CStaticCreatures * creatureSheet = target->getForm();
		if( creatureSheet != 0 )
		{
			targetInfos.ReportAction.DeltaLvl = casterSkillBaseValue - creatureSheet->getXPLevel();
		}
	}

	
	// test resistance
	float resistFactor = 1.0f;
	//if entity invincible, it aways resists the effect
	const CSEffectPtr effect = target->lookForActiveEffect(EFFECT_FAMILIES::Invincibility);
	if ( effect )
	{
		targetInfos.Immune = true;
		resistFactor = 0.0f;
	}
	else
	{
		if (!EntitiesNoResist)
		{
			uint32 resistValue = target->getMagicResistance(_DmgType);

			if (resistValue == CCreatureResists::ImmuneScore )
			{
				targetInfos.Immune = true;
				resistFactor = 0.0f;
			}
			else
			{
				if ( actor->getId().getType() == RYZOMID::player )
				{
					// boost magic skill for low level chars
					uint32 sb = MagicSkillStartValue.get();
					casterSkillvalue = max( sb, casterSkillvalue );

					// add magic boost from consumable
					CCharacter * pC = dynamic_cast<CCharacter *>(actor);
					if(pC)
						casterSkillvalue += pC->magicSuccessModifier();
				}

				// get the chances
				const uint8 roll = (uint8)RandomGenerator.rand( 99 );
				resistFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::MagicResistDirect, casterSkillvalue - resistValue, roll);

				// increase resists modifier for next resists test
				if (resistFactor > 0.0f)
					target->incResistModifier(_DmgType, resistFactor);

				if ( resistFactor <= 0.0f )
				{
					// Xp gain log
					if (PlayerManager.logXPGain(actor->getEntityRowId()))
					{
						const uint8 chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::MagicResistDirect, casterSkillvalue - resistValue ); 
						nlinfo("[XPLOG] Magic attack Actor %s on %s, delta = %d, target has Resisted (chances %u, tirage %u)", actor->getId().toString().c_str(), target->getId().toString().c_str(),
							targetInfos.ReportAction.DeltaLvl, chances, roll);
					}
				}
			}
		}
		else
		{
			resistFactor = 1.0f;
		}
	}
	if ( resistFactor > 0.0f )
	{
		if ( resistFactor > 1.0f )
			resistFactor = 1.0f;
		
		float dmgFactor = resistFactor * rangeFactor * (1.0f + phrase->getUsedItemStats().getPowerFactor(_Skill, phrase->getBrickMaxSabrinaCost()));
		
		// if PVP, apply PVP magic damage factor
		if( actor != target && actor->getId().getType() == RYZOMID::player && target->getId().getType() == RYZOMID::player)
		{
			dmgFactor *= PVPMagicDamageFactor.get();
		}

		// add spire effect ( magic offense )
		if ( actor->getId().getType() == RYZOMID::player )
		{
			CCharacter* charac = (CCharacter*)actor;
			const CSEffect* pEffect = charac->lookForActiveEffect( EFFECT_FAMILIES::TotemCombatMagOff );
			if ( pEffect != NULL )
			{
				dmgFactor *= ( 1.0f + pEffect->getParamValue() / 100.0f );
			}
		}
		
		sint32 maxDmgHp =  sint32 ( _DmgHp * dmgFactor );
		sint32 realDmgHp = target->applyDamageOnArmor( _DmgType, maxDmgHp );
		
		// update behaviour for lost Hp
		CCreature * npc = dynamic_cast<CCreature*>(target);
		CCharacter * c = dynamic_cast<CCharacter*>(actor);
		if(npc && c)
		{
			if(PHRASE_UTILITIES::testRange(*actor, *target, (uint32)npc->getMaxHitRangeForPC()*1000))
			{
				behav.DeltaHP -= (sint16)realDmgHp;
			}
			else
			{
				behav.DeltaHP = 0;
			}
		}
		
		// update target infos
		targetInfos.DmgHp			= maxDmgHp;
		targetInfos.ResistFactor	= resistFactor;
		targetInfos.DmgFactor		= dmgFactor;
		
		return false;
	}
	else
	{
		// don't tell the player now, but wait to send message when the missile hit the player (at apply() time)
		targetInfos.ResistFactor	= 0.f;
		
		CAiEventReport report;
		report.Originator = phrase->getActor();
		report.Target = target->getEntityRowId();
		report.Type = ACTNATURE::OFFENSIVE_MAGIC;
		//////////////////////////////////////////////////////////////////////////
		// TEMPORARY : set a small value for resist aggro
		//////////////////////////////////////////////////////////////////////////					
		report.AggroAdd = -0.02f;
		CCreature * npc = dynamic_cast<CCreature*>(target);
		CCharacter * c = dynamic_cast<CCharacter*>(actor);
		if(npc && c && PHRASE_UTILITIES::testRange(*actor, *target, (uint32)npc->getMaxHitRangeForPC()*1000) )
			CPhraseManager::getInstance().addAiEventReport(report);
		return true;
	}
}

/// Returns true if the spell was resisted
bool CMagicActionBasicDamage::computeMagicResistance(
	CEntityBase* const target,
	CTargetInfos& targetInfos,
	uint32 const casterSkillvalue,
	DMGTYPE::EDamageType const _DmgType,
	sint32 _DmgHp,
	CEntityBase const* const actor,
	float rangeFactor,
	float powerFactor)
{
	// test resistance
	float resistFactor = 1.0f;
	//if entity invincible, it aways resists the effect
	const CSEffectPtr effect = target->lookForActiveEffect(EFFECT_FAMILIES::Invincibility);
	if ( effect )
	{
		targetInfos.Immune = true;
		resistFactor = 0.0f;
	}
	else
	{
		if (!EntitiesNoResist)
		{
			uint32 resistValue = target->getMagicResistance(_DmgType);
			
			if (resistValue == CCreatureResists::ImmuneScore )
			{
				targetInfos.Immune = true;
				resistFactor = 0.0f;
			}
			else
			{
				uint32 localCasterSkillValue = casterSkillvalue;

				if ( actor->getId().getType() == RYZOMID::player )
				{
					// boost magic skill for low level chars
					uint32 sb = (uint32)MagicSkillStartValue.get();
					localCasterSkillValue = max( sb, localCasterSkillValue );

					// add magic boost from consumable
					CCharacter * pC = (CCharacter *)actor;
					localCasterSkillValue += pC->magicSuccessModifier();
				}

				// get the chances
				const uint8 roll = (uint8)RandomGenerator.rand( 99 );
				resistFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::MagicResistDirect, localCasterSkillValue - resistValue, roll);
				
				// increase resists modifier for next resists test
				if (resistFactor > 0.0f)
					target->incResistModifier(_DmgType, resistFactor);
				
				if ( resistFactor <= 0.0f )
				{
					// Xp gain log
					if (PlayerManager.logXPGain(actor->getEntityRowId()))
					{
						const uint8 chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::MagicResistDirect, localCasterSkillValue - resistValue ); 
						nlinfo("[XPLOG] Magic attack Actor %s on %s, delta = %d, target has Resisted (chances %u, tirage %u)", actor->getId().toString().c_str(), target->getId().toString().c_str(),
							targetInfos.ReportAction.DeltaLvl, chances, roll);
					}
				}
			}
		}
		else
		{
			resistFactor = 1.0f;
		}
	}
	if ( resistFactor > 0.0f )
	{
		if ( resistFactor > 1.0f )
			resistFactor = 1.0f;
		
		float dmgFactor = resistFactor * rangeFactor * (1.0f + powerFactor);
		
		// if PVP, apply PVP magic damage factor
		if( actor != target && actor->getId().getType() == RYZOMID::player && target->getId().getType() == RYZOMID::player)
		{
			dmgFactor *= PVPMagicDamageFactor.get();
		}

		// add spire effect ( magic offense )
		if ( actor->getId().getType() == RYZOMID::player )
		{
			CCharacter* charac = (CCharacter*)actor;
			const CSEffect* pEffect = charac->lookForActiveEffect( EFFECT_FAMILIES::TotemCombatMagOff );
			if ( pEffect != NULL )
			{
				dmgFactor *= ( 1.0f + pEffect->getParamValue() / 100.0f );
			}
		}
		
		sint32 realDmgHp =  sint32 ( _DmgHp * dmgFactor );
		
		// update target infos
		targetInfos.DmgHp			= realDmgHp;
		targetInfos.ResistFactor	= resistFactor;
		targetInfos.DmgFactor		= dmgFactor;
		
		return false;
	}
	else
	{
		return true;
	}
}

// This method was tuenre into a static function to be used outside of the
// class, coz building a magic phrase ane applying a magic action should be
// distinguished but wasn't. (vuarand)
bool CMagicActionBasicDamage::applyOnEntity(
	CMagicPhrase*	phrase,
	CEntityBase*	actor,
	CEntityBase*	target,
	sint32			vamp,
	float			vampRatio,
	CTargetInfos&	targetInfos,
	
	DMGTYPE::EDamageType const	_DmgType,
	sint32 const				_DmgHp,
	sint32 const				_DmgSap,
	sint32 const				_DmgSta,
	sint32 const				_VampirismValue)
{
	// If target is immune to magic tell player and return
	if (targetInfos.Immune)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].setEIdAIAlias(target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()));
		PHRASE_UTILITIES::sendDynamicSystemMessage(actor->getEntityRowId(), "MAGIC_TARGET_IMMUNE", params);
		
		return true;
	}
	
	// If target automatically resist to magic, tell player and return
	if (targetInfos.ResistFactor <= 0.0f)
	{
		PHRASE_UTILITIES::sendSpellResistMessages(actor->getEntityRowId(), target->getEntityRowId());
		return true;
	}
	
	bool sendAggro = true;
	CCreature * npc = dynamic_cast<CCreature*>(target);
	CCharacter * c = dynamic_cast<CCharacter*>(actor);
	if(npc && c)
	{
		if(!PHRASE_UTILITIES::testRange(*actor, *target, (uint32)npc->getMaxHitRangeForPC()*1000))
		{
			const_cast<sint32&>(_DmgHp) = 0;
			const_cast<sint32&>(_DmgSap) = 0;
			const_cast<sint32&>(_DmgSta) = 0;
			const_cast<sint32&>(_VampirismValue) = 0;
			targetInfos.DmgHp = 0;
			sendAggro = false;
			c->sendDynamicSystemMessage(c->getId(), "UNEFFICENT_RANGE");
		}
	}

	sint32 realDmgHp = targetInfos.DmgHp;
	realDmgHp = sint32( target->applyDamageOnArmor( _DmgType, realDmgHp ) );
	if (realDmgHp > target->currentHp())
		realDmgHp = target->currentHp();
	
	targetInfos.ReportAction.Hp += realDmgHp;
	
	// apply vampirism
	vamp += _VampirismValue;
	if (vamp && actor->getId().getType() == RYZOMID::player)
	{
		sint32 vampirise = (sint32) (realDmgHp * vampRatio);
		if (vampirise > vamp)
			vampirise = vamp;
		
		actor->changeCurrentHp(vampirise);
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer);
		params[0].setEIdAIAlias(target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()));
		params[1].Int = vampirise;
		CCharacter::sendDynamicSystemMessage(actor->getId(),"EGS_ACTOR_VAMPIRISE_EI", params);
	}
	
	// apply Sap damage
	sint32 realDmgSap = 0;
	{
		SCharacteristicsAndScores& score = target->getScores()._PhysicalScores[SCORES::sap];
		sint32 maxRealDmgSap = (sint32)( _DmgSap * targetInfos.DmgFactor );
		realDmgSap = target->applyDamageOnArmor(_DmgType, maxRealDmgSap);
		if (realDmgSap != 0)
		{
			PHRASE_UTILITIES::sendScoreModifierSpellMessage(actor->getId(), target->getId(), -realDmgSap, -maxRealDmgSap, SCORES::sap, ACTNATURE::OFFENSIVE_MAGIC);
			
			if (realDmgSap > score.Current)
				realDmgSap = score.Current;
			score.Current = score.Current - realDmgSap;
			
			targetInfos.ReportAction.Sap += realDmgSap;
		}
	}
	
	// apply Stamina damage
	sint32 realDmgSta = 0;
	{
		SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::stamina];
		sint32 maxRealDmgSta =  sint32( _DmgSta * targetInfos.DmgFactor );
		realDmgSta = target->applyDamageOnArmor( _DmgType, maxRealDmgSta );
		if (realDmgSta != 0)
		{
			PHRASE_UTILITIES::sendScoreModifierSpellMessage(actor->getId(), target->getId(), -realDmgSta, -maxRealDmgSta, SCORES::stamina, ACTNATURE::OFFENSIVE_MAGIC);
			
			if (realDmgSta > score.Current)
				realDmgSta = score.Current;
			score.Current = score.Current - realDmgSta;
			
			targetInfos.ReportAction.Sta += realDmgSta;
		}
	}
	
	targetInfos.ReportAction.ActionNature = ACTNATURE::OFFENSIVE_MAGIC;
	
	// compute aggro
	sint32 max = target->maxHp();
	
	float aggroHp = 0.0f;
	float aggroSap = 0.0f;
	float aggroSta = 0.0f;
	
	CAiEventReport report;
	if (phrase)
		report.Originator = phrase->getActor();
	else
		report.Originator = actor->getEntityRowId();
	report.Target = target->getEntityRowId();
	report.Type = ACTNATURE::OFFENSIVE_MAGIC;
	if (max && _DmgHp != 0)
	{
		aggroHp = min(1.0f, float(realDmgHp)/float(max) );
		report.addDelta(AI_EVENT_REPORT::HitPoints, (-1)*realDmgHp);
	}
	
	max = target->getPhysScores()._PhysicalScores[SCORES::sap].Max;
	if (max && _DmgSap != 0)
	{
		aggroSap = min(1.0f, float(realDmgSap)/float(max) );
		report.addDelta(AI_EVENT_REPORT::Sap, (-1)*realDmgSap);
	}
	
	max = target->getPhysScores()._PhysicalScores[SCORES::stamina].Max;
	if (max && _DmgSta != 0)
	{
		aggroSta = min(1.0f,float(realDmgSta)/float(max));
		report.addDelta(AI_EVENT_REPORT::Stamina, (-1)*realDmgSta);
	}
	
	// send report
	report.AggroAdd = - min(1.0f, 1.0f - (1.0f-aggroHp)*(1.0f-aggroSap)*(1.0f-aggroSta) );
	
	if(sendAggro)
		CPhraseManager::getInstance().addAiEventReport(report);
	
	// apply Hp damage
	if (realDmgHp > 0)
	{
		if ( target->changeCurrentHp( -realDmgHp, actor->getEntityRowId()) )
		{
			PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor->getId(), target->getId(), -realDmgHp, -targetInfos.DmgHp ,SCORES::hit_points , ACTNATURE::OFFENSIVE_MAGIC);
			PHRASE_UTILITIES::sendDeathMessages( actor->getEntityRowId(), target->getEntityRowId() );
		}
		else
		{
			PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor->getId(), target->getId(), -realDmgHp, -targetInfos.DmgHp ,SCORES::hit_points , ACTNATURE::OFFENSIVE_MAGIC);
		}
	}
	
	return !sendAggro;
}

void CMagicActionBasicDamage::launch(
	CMagicPhrase*				phrase,
	sint						deltaLevel,
	sint						skillLevel,
	float						successFactor,
	MBEHAV::CBehaviour&			behav,
	std::vector<float> const&	powerFactors,
	NLMISC::CBitSet&			affectedTargets,
	NLMISC::CBitSet const&		invulnerabilityOffensive,
	NLMISC::CBitSet const&		invulnerabilityAll,
	bool						isMad,
	NLMISC::CBitSet&			resists,
	TReportAction const&		actionReport)
{
	H_AUTO(CMagicActionBasicDamage_launch);
	
	///\todo nico:
	//		- location
	//		- armor + shield
	//		- player damages + on armor
	//		- behaviour + chat messages
	//		- aggro

	if ( successFactor <= 0.0f )
		return;

	CEntityBase * actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
	if (!actor)
		return;

	const std::vector< CSpellTarget > & targets = phrase->getTargets();

	sint skillValue = 0;
	sint skillBaseValue = 0;
	if ( actor->getId().getType() == RYZOMID::player )
	{
		CCharacter * pC = safe_cast<CCharacter *> (actor);
		skillValue = pC->getSkillValue( _Skill );
		skillBaseValue = pC->getSkillBaseValue( _Skill );
	}
	else
	{
		const CStaticCreatures * form = actor->getForm();
		if ( !form )
		{
			nlwarning( "<MAGIC>invalid creature form %s in entity %s", actor->getType().toString().c_str(), actor->getId().toString().c_str() );
			return;
		}	
		skillBaseValue = skillValue = form->getAttackLevel();
	}
	
	const CSEffect * debuff = actor->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMagic );
	if ( debuff )
		skillValue -= debuff->getParamValue();
	const CSEffect * outPostBuff = actor->lookForActiveEffect( EFFECT_FAMILIES::OutpostMagic );
	if ( outPostBuff )
		skillValue += outPostBuff->getParamValue();

	bool targetChanged = false;

	const uint nbTargets = (uint)targets.size();

	for ( uint i = 0; i < nbTargets ; ++i )
	{
		TReportAction reportAction = actionReport;
		reportAction.Skill = _Skill;

		// check target
		CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i].getId() );
		if (!target)
			continue;

		string errorCode;
		if ( !isMad && !PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, (i==0) ) )
		{
			continue;
		}
		affectedTargets.set(i);

		// for main target only, check reflect damage effects
		if (i == 0)
		{
			const CSEffectPtr effect = target->lookForActiveEffect(EFFECT_FAMILIES::ReflectDamage);
			if ( effect )
			{
				target = actor;
				isMad = true;
				targetChanged = true;
				affectedTargets.clear(i);
			}
		}

		// for main target only, check bounce effects
		if (i == 0)
		{
			const CSEffectPtr effect = target->lookForActiveEffect(EFFECT_FAMILIES::Bounce);
			if ( effect )
			{
				CBounceEffect *bounceEffect = safe_cast<CBounceEffect *> (& (*effect));
				bool bounce = false;
				CEntityBase *bounceTarget = bounceEffect->getTargetForBounce(actor);
				if (bounceTarget)
				{
					bounce = true;
					// anti magic shield prevents offensive magic
					CSEffect *effect = NULL;
					// invulnerability power prevents ALL magic (offensive AND curative)
					effect = bounceTarget->lookForActiveEffect( EFFECT_FAMILIES::PowerInvulnerability);
					if (effect)
					{
						bounce = false;
					}
					else
					{
						effect = bounceTarget->lookForActiveEffect( EFFECT_FAMILIES::PowerAntiMagicShield);
						if (effect)
						{
							bounce = false;
						}
					}
				}

				if (bounce)
				{
					CTargetInfos targetInfos;
					targetInfos.ReportAction = actionReport;
					targetInfos.ReportAction.DeltaLvl = deltaLevel;
					targetInfos.ReportAction.Skill = _Skill;
					launchOnEntity(phrase, actor, bounceTarget,false, skillValue, skillBaseValue, behav, isMad, 1.0f, targetInfos);

					if ( _DmgSap || _DmgSta || targetInfos.needsApply() )
					{
						_ApplyTargets.push_back(targetInfos);
					}
				}
			}
		}
		
		bool invulnerable = false;
		// test if target really affected
		if (targetChanged)
		{
			// anti magic shield prevents offensive magic
			CSEffect *effect = NULL;
			// invulnerability power prevents ALL magic (offensive AND curative)
			effect = target->lookForActiveEffect( EFFECT_FAMILIES::PowerInvulnerability);
			if (effect)
			{
				invulnerable = true;
			}
			else
			{
				effect = target->lookForActiveEffect( EFFECT_FAMILIES::PowerAntiMagicShield);
				if (effect)
				{
					invulnerable = true;
				}
			}
		}
		else if (invulnerabilityAll[i] || invulnerabilityOffensive[i])
		{
			invulnerable = true;
		}

		if (invulnerable)
		{
			resists.set(i);
			CAiEventReport report;
			report.Originator = phrase->getActor();
			report.Target = target->getEntityRowId();
			report.Type = ACTNATURE::OFFENSIVE_MAGIC;
			report.AggroAdd = -0.02f;
			CPhraseManager::getInstance().addAiEventReport(report);
			continue;
		}

		reportAction = actionReport;
		reportAction.Skill = _Skill;

		CTargetInfos targetInfos;
		targetInfos.ReportAction = actionReport;
		targetInfos.ReportAction.DeltaLvl = deltaLevel;
		targetInfos.ReportAction.Skill = _Skill;
		if (launchOnEntity(phrase, actor, target, (i==0), skillValue, skillBaseValue, behav, isMad, powerFactors[i] * phrase->getAreaSpellPowerFactor(), targetInfos))
		{
			resists.set(i);
		}
		else
		{
			resists.clear(i);
		}

		if ( _DmgSap || _DmgSta || targetInfos.needsApply() )
		{
			// insert the main target at the first position because there may already be a bounce target
			if (i == 0)
			{
				_ApplyTargets.insert(_ApplyTargets.begin(), targetInfos);
			}
			else
			{
				_ApplyTargets.push_back(targetInfos);
			}
		}
	}
}

void CMagicActionBasicDamage::apply(
	CMagicPhrase*				phrase,
	sint						deltaLevel,
	sint						skillLevel,
	float						successFactor,
	MBEHAV::CBehaviour&			behav,
	std::vector<float> const&	powerFactors,
	NLMISC::CBitSet&			affectedTargets,
	NLMISC::CBitSet const&		invulnerabilityOffensive,
	NLMISC::CBitSet const&		invulnerabilityAll,
	bool						isMad,
	NLMISC::CBitSet&			resists,
	TReportAction const&		actionReport,
	sint32						vamp,
	float						vampRatio,
	bool						reportXp)
{
	H_AUTO(CMagicActionBasicDamage_apply);
	
	CEntityBase* actor = CEntityBaseManager::getEntityBasePtr(phrase->getActor());
	if (!actor)
		return;
	
	uint const nbTargets = (uint)_ApplyTargets.size();
	
	std::vector<TReportAction> actionReports;
	actionReports.reserve(nbTargets);
	
	for (uint i=0; i<nbTargets; ++i)
	{
		CEntityBase* target = CEntityBaseManager::getEntityBasePtr(_ApplyTargets[i].RowId);
		if (!target)
			continue;
		
		// validate the target again
		string errorCode;
		if (!PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(), target->getEntityRowId(), ACTNATURE::OFFENSIVE_MAGIC, errorCode, _ApplyTargets[i].MainTarget))
			continue;
		
		bool reportXpForEntity = true;
		// true if target has resisted or is immune
		if (applyOnEntity(phrase, actor, target, vamp, vampRatio, _ApplyTargets[i], _DmgType, _DmgHp, _DmgSap, _DmgSta, _VampirismValue))
			reportXpForEntity = false;
		
		if (reportXpForEntity)
			actionReports.push_back(_ApplyTargets[i].ReportAction);
	}
	
	// send all reports, even if reportXp == false because damage must be registered
	for (uint i=0; i<actionReports.size(); ++i)
	{
		if (!reportXp)
		{
			actionReports[i].Skill = SKILLS::unknown;						// no xp gain but damage must be registered
			actionReports[i].SkillLevel = phrase->getBrickMaxSabrinaCost();	// use the real level of the enchantment
		}
		
		// only send report if damage have been made
		if (actionReports[i].Hp != 0 || actionReports[i].Sap != 0 || actionReports[i].Sta != 0 || actionReports[i].Focus != 0)
		{
			PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport(actionReports[i], (i==0));
			PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(actionReports[i]);
		}
	}
}

CMagicAiActionTFactory<CMagicActionBasicDamage>* CMagicActionBasicDamageAiFactoryInstance = new CMagicAiActionTFactory<CMagicActionBasicDamage>(AI_ACTION::DamageSpell);
