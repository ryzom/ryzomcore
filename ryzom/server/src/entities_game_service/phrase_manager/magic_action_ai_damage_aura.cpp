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
#include "magic_action_ai_damage_aura.h"
#include "phrase_manager/magic_phrase.h"
#include "phrase_manager/effect_factory.h"
#include "phrase_manager/phrase_manager.h"
#include "creature_manager/creature_manager.h"
#include "damage_aura_effect.h"
#include "entity_structure/statistic.h"
#include "ai_share/ai_event_report.h"

using namespace NLMISC;
using namespace std;

extern CCreatureManager CreatureManager;

//--------------------------------------------------------------
//					initFromAiAction  
//--------------------------------------------------------------
bool CMagicAiActionDamageAura::initFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase )
{
#ifdef NL_DEBUG
	nlassert(phrase);
	nlassert(aiAction);
#endif
	
	if (aiAction->getType() != AI_ACTION::EoTSpell )
		return false;
	
	// read parameters
	const COTEffectSpellParams &data = aiAction->getData().OTEffectSpell;

	CCreature *creature = CreatureManager.getCreature( phrase->getActor() );
	if (creature && creature->getForm())
		_DamagePerUpdate = (uint16)(data.SpellParamValue2 + data.SpellPowerFactor * creature->getForm()->getAttackLevel());
	else
		_DamagePerUpdate = (uint16)data.SpellParamValue2;
	
	_EffectFamily = AI_ACTION::toEffectFamily(data.EffectFamily, aiAction->getType());
	_EffectDuration = data.Duration;
	_Range = data.SpellParamValue;
	_CycleLength = data.UpdateFrequency;
	
	phrase->setMagicFxType(MAGICFX::toMagicFx( _EffectFamily), 3);
		
	return true;
} // initFromAiAction //

//--------------------------------------------------------------
//					launch
//--------------------------------------------------------------
void CMagicAiActionDamageAura::launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
										const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
										const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport )
{
	H_AUTO(CMagicAiActionDamageAura_launch);

	if (!phrase || successFactor <= 0.0f)
		return;

	const vector<CSpellTarget> &targets = phrase->getTargets();
	const uint nbTargets = (uint)targets.size();
	for (uint i = 0 ; i < nbTargets ; ++i)
	{
		if (!TheDataset.isAccessible(targets[i].getId()))
			continue;

		// get target entity
		CEntityBase *target = CEntityBaseManager::getEntityBasePtr(targets[i].getId());
		if (!target)
			continue;

		// check target validity
		string errorCode;
		if ( !isMad && !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),target->getEntityRowId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, i==0 ) )
		{
			// skip target as it's invalid
			affectedTargets.clear(i);
			continue;
		}

		affectedTargets.set(i);

		CTargetInfos targetInfos;
		targetInfos.RowId		= target->getEntityRowId();
		targetInfos.MainTarget	= (i == 0);

		_ApplyTargets.push_back(targetInfos);
	}
} // launch //

//--------------------------------------------------------------
//					apply  
//--------------------------------------------------------------
void CMagicAiActionDamageAura::apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
									  const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
									  const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
									  sint32 vamp, float vampRatio, bool reportXp )
{
	H_AUTO(CMagicAiActionDamageAura_apply);

	const TGameCycle endDate = CTickEventHandler::getGameCycle() + _EffectDuration;

	const uint nbTargets = (uint)_ApplyTargets.size();
	for (uint i = 0 ; i < nbTargets ; ++i)
	{
		if (!TheDataset.isAccessible(_ApplyTargets[i].RowId))
			continue;

		// get target entity
		CEntityBase *target = CEntityBaseManager::getEntityBasePtr(_ApplyTargets[i].RowId);
		if (!target)
			continue;

		// check target validity
		string errorCode;
		if ( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),target->getEntityRowId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, _ApplyTargets[i].MainTarget ) )
		{
			continue;
		}

		// create effect
		CDamageAuraEffect *effect = new CDamageAuraEffect(phrase->getActor(), target->getEntityRowId(), _EffectFamily,0, endDate, _CycleLength, _DamagePerUpdate, _Range);
		if (!effect)
		{
			nlwarning("EFFECT failed to create new CDamageAuraEffect, memory full ?");
			return;
		}
		
		effect->affectPlayers(true);
		effect->affectAttackableEntities(false);
		
		target->addSabrinaEffect(effect);
	}
} // apply //


CMagicAiSpecializedActionTFactory<CMagicAiActionDamageAura> *CMagicActionAiStenchFactoryInstance = new CMagicAiSpecializedActionTFactory<CMagicAiActionDamageAura>(AI_ACTION::Stench);
