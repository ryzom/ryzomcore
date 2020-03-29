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
#include "magic_action_ai_hot.h"
#include "phrase_manager/magic_phrase.h"
#include "nolink_hot_effect.h"
#include "creature_manager/creature_manager.h"

using namespace NLMISC;
using namespace std;

extern CCreatureManager CreatureManager;

//--------------------------------------------------------------
//					initFromAiAction
//--------------------------------------------------------------
bool CMagicAiActionHoT::initFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase )
{
#ifdef NL_DEBUG
	nlassert(phrase);
	nlassert(aiAction);
#endif
	
	if (aiAction->getType() != AI_ACTION::EffectSpell )
		return false;
	
	// read parameters
	const COTSpellParams &data = aiAction->getData().OTSpell;

	CCreature *creature = CreatureManager.getCreature( phrase->getActor() );
	if (creature && creature->getForm())
		_TotalHealValue = (uint16)(data.SpellParamValue + data.SpellPowerFactor * creature->getForm()->getAttackLevel());
	else
		_TotalHealValue = (uint16)data.SpellParamValue;
	
	_EffectDuration = data.Duration;
	_EffectDurationType = data.EffectDurationType;
	_TotalHealValue = (sint32)data.SpellParamValue;
	_UpdateFrequency = data.UpdateFrequency;
	_AffectedScore = data.AffectedScore;
	
	switch(_AffectedScore)
	{
	case SCORES::hit_points:
		phrase->setMagicFxType( MAGICFX::healtoMagicFx( 5,0,0,true ), 5);
		break;
	case SCORES::sap:
		phrase->setMagicFxType( MAGICFX::healtoMagicFx( 0,5,0,true ), 5);
		break;
	case SCORES::stamina:
		phrase->setMagicFxType( MAGICFX::healtoMagicFx( 0,0,5,true ), 5);
		break;
	};
	
	return true;
} // initFromAiAction //

//--------------------------------------------------------------
//					launch
//--------------------------------------------------------------
void CMagicAiActionHoT::launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
								const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
								const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport )
{
	if (!phrase || successFactor <= 0.0f || !_EffectDuration)
		return;

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
			continue;
		}

		affectedTargets.set(i);

		// get target entity
		CEntityBase *target = CEntityBaseManager::getEntityBasePtr(targets[i].getId());
		if (!target)
			continue;

		// if target has resisted, do not affect it
		if (resists.get(i) == true)
			continue;

		// test if target really affected
		if (invulnerabilityAll[i])
		{
			resists.set(i);
			continue;
		}

		CTargetInfos targetInfos;
		targetInfos.RowId		= target->getEntityRowId();
		targetInfos.MainTarget	= (i == 0);

		_ApplyTargets.push_back(targetInfos);
	}
} // launch //

//--------------------------------------------------------------
//					apply  
//--------------------------------------------------------------
void CMagicAiActionHoT::apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
							   const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
							   const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
							   sint32 vamp, float vampRatio, bool reportXp )
{
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
			// skip target as it's invalid
			continue;
		}

		// get target entity
		CEntityBase *target = CEntityBaseManager::getEntityBasePtr(_ApplyTargets[i].RowId);
		if (!target)
			continue;

		float cycleHeal= float(_TotalHealValue) * float(_UpdateFrequency) / float(_EffectDuration);

		CNoLinkHoTEffect *effect = new CNoLinkHoTEffect( phrase->getActor(), target->getEntityRowId(), EFFECT_FAMILIES::Dot, _TotalHealValue, endDate, 
													_UpdateFrequency, _AffectedScore, cycleHeal
													);
		if (effect)
		{
			target->addSabrinaEffect(effect);
		}
	}
} // apply //


CMagicAiActionTFactory<CMagicAiActionHoT> *CMagicActionHotAiFactoryInstance = new CMagicAiActionTFactory<CMagicAiActionHoT>(AI_ACTION::HoTSpell);
