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
#include "magic_action_ai_effect.h"
#include "phrase_manager/magic_phrase.h"
#include "phrase_manager/effect_factory.h"
#include "phrase_manager/phrase_manager.h"
#include "entity_structure/statistic.h"
#include "ai_share/ai_event_report.h"
#include "player_manager/character.h"
#include "entity_manager/entity_manager.h"

using namespace NLMISC;
using namespace std;

//--------------------------------------------------------------
//					initFromAiAction  
//--------------------------------------------------------------
bool CMagicAiActionEffect::initFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase )
{
#ifdef NL_DEBUG
	nlassert(phrase);
	nlassert(aiAction);
#endif
	
	if (aiAction->getType() != AI_ACTION::EffectSpell )
		return false;
	
	// read parameters
	const CEffectSpellParams &data = aiAction->getData().EffectSpell;
	
	_EffectFamily = AI_ACTION::toEffectFamily(data.EffectFamily, aiAction->getType());
	_EffectDuration = data.Duration;
	_EffectDurationType = data.EffectDurationType;
	_ParamValue = (sint32)data.SpellParamValue;
	
	phrase->setMagicFxType(MAGICFX::toMagicFx( _EffectFamily), 3);
	
	return true;
} // initFromAiAction //

//--------------------------------------------------------------
//					launch
//--------------------------------------------------------------
void CMagicAiActionEffect::launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
								   const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
								   const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport )
{
	H_AUTO(CMagicAiActionEffect_launch);

	if (!phrase || successFactor <= 0.0f)
		return;

	CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
	if (!actor)
		return;
	
	const CSEffectPtr debuff = actor->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMagic );
	sint skillValue = 0;
	if ( actor->getId().getType() == RYZOMID::player )
	{
		CCharacter * pC = (CCharacter *) actor;
		skillValue = pC->getSkillValue( _Skill );
	}
	else
	{
		const CStaticCreatures * form = actor->getForm();
		if ( !form )
		{
			nlwarning( "<MAGIC>invalid creature form %s in entity %s", actor->getType().toString().c_str(), actor->getId().toString().c_str() );
			return;
		}	
		skillValue = form->getAttackLevel();
	}
	//get SpellLevel : 
	//if -1 : the spell cannot be resisted by target, whatever resist values the target has
	//if 0 : default behaviour, the spell level is not used 
	//if >0 : the spell level replaces the caster skill value
	sint32 spellLvl = phrase->getSpellLevel();
	if (spellLvl > 0)
		skillValue = spellLvl;

	if ( debuff )
		skillValue -= debuff->getParamValue();
	
	const vector<CSpellTarget> &targets = phrase->getTargets();
	const uint nbTargets = (uint)targets.size();
	for (uint i = 0 ; i < nbTargets ; ++i)
	{
		if (!TheDataset.isAccessible(targets[i].getId()))
			continue;

		// check target validity
		string errorCode;
		if ( !isMad && !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),targets[i].getId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, i==0 ) )
		{
			// skip target as it's invalid
			affectedTargets.clear(i);
			continue;
		}

		// get target entity
		CEntityBase *target = CEntityBaseManager::getEntityBasePtr(targets[i].getId());
		if (!target)
			continue;

		affectedTargets.set(i);
		resists.clear(i);

		CTargetInfos targetInfos;
		targetInfos.RowId			= target->getEntityRowId();
		targetInfos.MainTarget		= (i == 0);
		targetInfos.ResistFactor	= 1.0f;
		targetInfos.Immune			= false;

		// test if target really affected
		if ( invulnerabilityAll[i] || invulnerabilityOffensive[i] )
		{
			targetInfos.ResistFactor = 0.0f;
			resists.set(i);
			targetInfos.Immune = true;
		}
		else if (phrase->getNature() == ACTNATURE::OFFENSIVE_MAGIC && !EntitiesNoResist)
		{
			// test resistance
			uint32 resistValue = target->getMagicResistance(_EffectFamily);
			if (spellLvl == -1)
				resistValue = 0;

			const sint32 delta = skillValue - resistValue;
		
			// get the result
			const uint8 roll = (uint8)RandomGenerator.rand( 99 );
			targetInfos.ResistFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::MagicResistDirect, delta, roll);

			// increase resists modifier for next resists test
			if (targetInfos.ResistFactor > 0.0f)
				target->incResistModifier(_EffectFamily, targetInfos.ResistFactor);
			
			if (resistValue == CCreatureResists::ImmuneScore || targetInfos.ResistFactor <= 0.0f)
			{
				resists.set(i);
				targetInfos.Immune = true;
			}
		}

		_ApplyTargets.push_back(targetInfos);
	}	
} // launch //

//--------------------------------------------------------------
//					apply  
//--------------------------------------------------------------
void CMagicAiActionEffect::apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
								  const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
								  const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
								  sint32 vamp, float vampRatio, bool reportXp )
{
	H_AUTO(CMagicAiActionEffect_apply);

	CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
	if (!actor)
		return;
	
	TGameCycle endDate;
	switch(_EffectDurationType) 
	{
	case DURATION_TYPE::Permanent:
	case DURATION_TYPE::UntilCasterDeath:
		endDate = ~0u;
		break;
		
	case DURATION_TYPE::Normal:
	default:
		endDate = CTickEventHandler::getGameCycle() + _EffectDuration;
	}

	const uint nbTargets = (uint)_ApplyTargets.size();
	for (uint i = 0 ; i < nbTargets ; ++i)
	{
		if (!TheDataset.isAccessible(_ApplyTargets[i].RowId))
			continue;

		// check target validity
		string errorCode;
		if ( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),_ApplyTargets[i].RowId,ACTNATURE::OFFENSIVE_MAGIC, errorCode, _ApplyTargets[i].MainTarget ) )
		{
			continue;
		}

		// get target entity
		CEntityBase *target = CEntityBaseManager::getEntityBasePtr(_ApplyTargets[i].RowId);
		if (!target)
			continue;

		TGameCycle endDateLocal = endDate;
		sint32 paramValueLocal = _ParamValue;

		const float & resistFactor = _ApplyTargets[i].ResistFactor;

		CAiEventReport report;
		report.Originator = phrase->getActor();
		report.Target = _ApplyTargets[i].RowId;
		report.Type = ACTNATURE::OFFENSIVE_MAGIC;
		report.AggroAdd = -0.02f;
		CPhraseManager::getInstance().addAiEventReport(report);

		if (_ApplyTargets[i].Immune)
		{
			PHRASE_UTILITIES::sendSpellResistMessages( actor->getEntityRowId(), target->getEntityRowId());
			continue;
		}

		// partial resist, change effect strength or duration
		if (resistFactor < 1.0f)
		{
			if (_EffectDurationType == DURATION_TYPE::Normal)
			{
				endDateLocal = TGameCycle (endDate * resistFactor);
			}
			else
			{
				paramValueLocal = sint32(_ParamValue * resistFactor);
			}
		}

		CSTimedEffect *effect = IEffectFactory::buildEffect(_EffectFamily);
		if (effect)
		{
			effect->setCreatorRowId(phrase->getActor());
			effect->setFamily(_EffectFamily);
			effect->setTargetRowId(_ApplyTargets[i].RowId);
			effect->setParamValue(paramValueLocal);
			effect->setEndDate(endDateLocal);

			target->addSabrinaEffect(effect);
		}
	}	
} // apply //

CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicActionAiEffectBlindFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::Blind);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicActionAiEffectRootFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::Root);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicActionAiEffectStunFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::Stun);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicActionAiEffectHatredFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::Hatred);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicActionAiEffectFearFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::Fear);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiMezzFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::Mezz);

CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiSnareFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::SlowMove);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiSlowMagicFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::SlowMagic);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiSlowRangeFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::SlowRange);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiSlowMeleeFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::SlowMelee);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiMeleeMadnessFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::MeleeMadness);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiRangeMadnessFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::RangeMadness);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiMagicMadnessFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::MagicMadness);


CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffMeleeFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::SkillDebufMelee );
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffRangeFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::SkillDebufRange );
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffMagicFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::SkillDebufMagic);

CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffAcidResistFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::ResistDebufAcid);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffColdResistFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::ResistDebufCold);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffRotResistFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::ResistDebufRot);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffFireResistFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::ResistDebufFire);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffElectricyResistFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::ResistDebufElectric);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffPoisonResistFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::ResistDebufPoison);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiDebuffShockResistFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::ResistDebufShockwave);

CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiBounceFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::Bounce);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiReflectDamageFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::ReflectDamage);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiInverseDamageFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::ReverseDamage);
CMagicAiSpecializedActionTFactory<CMagicAiActionEffect> *CMagicAiRedirectAttacksFactory = new CMagicAiSpecializedActionTFactory<CMagicAiActionEffect>(AI_ACTION::RedirectAttacks);


