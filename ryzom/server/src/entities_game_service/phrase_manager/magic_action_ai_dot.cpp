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
#include "magic_action_ai_dot.h"
#include "phrase_manager/magic_phrase.h"
#include "nolink_dot_effect.h"
#include "phrase_manager/phrase_manager.h"
#include "creature_manager/creature_manager.h"
#include "entity_structure/statistic.h"
#include "ai_share/ai_event_report.h"
#include "player_manager/character.h"

using namespace NLMISC;
using namespace std;

extern CCreatureManager CreatureManager;

//--------------------------------------------------------------
//					initFromAiAction  
//--------------------------------------------------------------
bool CMagicAiActionDoT::initFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase )
{
#ifdef NL_DEBUG
	nlassert(phrase);
	nlassert(aiAction);
#endif
	
	if (aiAction->getType() != AI_ACTION::DoTSpell )
		return false;
	
	// read parameters
	const COTSpellParams &data = aiAction->getData().OTSpell;

	CCreature *creature = CreatureManager.getCreature( phrase->getActor() );
	if (creature && creature->getForm())
		_TotalDamageValue = (uint16)(data.SpellParamValue + data.SpellPowerFactor * creature->getForm()->getAttackLevel());
	else
		_TotalDamageValue = (uint16)data.SpellParamValue;
	
	_EffectDuration = data.Duration;
	_UpdateFrequency = data.UpdateFrequency;
	_AffectedScore = data.AffectedScore;
	_DamageType = data.DamageType;
	_Stackable = data.Stackable;
	_DurationType = data.EffectDurationType;
		
	phrase->setMagicFxType(MAGICFX::toMagicFx( _DamageType, true), 3);
	
	return true;
} // initFromAiAction //


void CMagicAiActionDoT::launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
								const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
								const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport )
{
	H_AUTO(CMagicAiActionDoT_launch);

	if ( !phrase || successFactor <= 0.0f || ( !_EffectDuration && _DurationType == DURATION_TYPE::Normal ) )
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

		// test if target really affected
		if (invulnerabilityAll[i] || invulnerabilityOffensive[i])
		{
			resists.set(i);
			continue;
		}

		CTargetInfos targetInfos;
		targetInfos.RowId			= target->getEntityRowId();
		targetInfos.MainTarget		= (i == 0);
		targetInfos.ResistFactor	= 1.0f;
		targetInfos.Immune			= false;

		// test resistance
		if ( ! EntitiesNoResist )
		{
			uint32 resistValue = target->getMagicResistance(_DamageType);

			const sint32 delta = skillValue - resistValue;

			// get the chances
			const uint8 roll = (uint8)RandomGenerator.rand( 99 );
			targetInfos.ResistFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::MagicResistDirect, delta, roll);

			// increase resists modifier for next resists test
			if (targetInfos.ResistFactor > 0.0f)
				target->incResistModifier(_DamageType, targetInfos.ResistFactor);
			
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
void CMagicAiActionDoT::apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
							   const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
							   const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
							   sint32 vamp, float vampRatio, bool reportXp )
{
	H_AUTO(CMagicAiActionDoT_apply);

	TGameCycle endDate;
	float cycleDamage;

	switch(_DurationType) 
	{
	case DURATION_TYPE::Permanent:
	case DURATION_TYPE::UntilCasterDeath:
		endDate = ~0u;
		cycleDamage = float(_TotalDamageValue);
		break;
		
	case DURATION_TYPE::Normal:
	default:
		endDate = CTickEventHandler::getGameCycle() + _EffectDuration;
		cycleDamage = float(_TotalDamageValue) * float(_UpdateFrequency) / float(_EffectDuration);
	}

	CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
	if (!actor)
		return;
	
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

		if (_ApplyTargets[i].Immune)
		{
			PHRASE_UTILITIES::sendSpellResistMessages( actor->getEntityRowId(), target->getEntityRowId());
			
			CAiEventReport report;
			report.Originator = phrase->getActor();
			report.Target = _ApplyTargets[i].RowId;
			report.Type = ACTNATURE::OFFENSIVE_MAGIC;
			report.AggroAdd = -0.02f;
			CPhraseManager::getInstance().addAiEventReport(report);
			
			continue;
		}

		const float & resistFactor = _ApplyTargets[i].ResistFactor;
		TGameCycle endDateLocal = endDate;
		sint32 totalDamage = _TotalDamageValue;

		// partial resist, change effect strength or duration
		if (resistFactor < 1.0f)
		{
			if (_DurationType == DURATION_TYPE::Normal)
			{
				endDateLocal = TGameCycle (endDate * resistFactor);
			}
			else
			{
				cycleDamage *= resistFactor;
				totalDamage = sint32(_TotalDamageValue * resistFactor);
			}
		}

		EFFECT_FAMILIES::TEffectFamily family = _Stackable ? EFFECT_FAMILIES::DoTStackable : EFFECT_FAMILIES::Dot;
		
		CNoLinkDOTEffect *effect = new CNoLinkDOTEffect( phrase->getActor(), target->getEntityRowId(), family, totalDamage, endDateLocal, 
													_UpdateFrequency, _AffectedScore, cycleDamage, _DamageType
													);
		if (effect)
		{
			if (_DurationType == DURATION_TYPE::UntilCasterDeath)
			{
				effect->endsAtCasterDeath(true);
			}

			effect->stackable(_Stackable);
			target->addSabrinaEffect(effect);
		}
	}	
} // apply //


CMagicAiActionTFactory<CMagicAiActionDoT> *CMagicActionDotAiFactoryInstance = new CMagicAiActionTFactory<CMagicAiActionDoT>(AI_ACTION::DoTSpell);
