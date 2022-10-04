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
#include "combat_action_ai_effect.h"
#include "combat_phrase.h"
#include "phrase_manager/effect_factory.h"
#include "game_share/mode_and_behaviour.h"
#include "player_manager/player.h"

using namespace NLMISC;
using namespace std;

extern SKILLS::ESkills		BarehandCombatSkill;

vector< pair< AI_ACTION::TAiEffectType, CCombatAIActionFactory* > >* CCombatAIActionFactory::Factories;

//--------------------------------------------------------------
//					apply  
//--------------------------------------------------------------
bool CCombatAiActionEffect::initFromAiAction( const CStaticAiAction *aiAction, CCombatPhrase *phrase )
{
#ifdef NL_DEBUG
	nlassert(phrase);
	nlassert(aiAction);
#endif
	
	if (aiAction->getType() != AI_ACTION::Range && aiAction->getType() != AI_ACTION::Melee)
		return false;
	
	_ActorRowId = phrase->getAttacker()->getEntityRowId();
	
	// read parameters
	const CCombatParams &data = aiAction->getData().Combat;
	
	_EffectFamily = AI_ACTION::toEffectFamily(data.EffectFamily, aiAction->getType());
	_EffectDuration = data.EffectTime;
	_ParamValue = (sint32)data.EffectValue;
	
	return true;
} // initFromAiAction //

//--------------------------------------------------------------
//					apply  
//--------------------------------------------------------------
void CCombatAiActionEffect::apply( CCombatPhrase *phrase )
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif
	
	H_AUTO(CCombatAiActionEffect_apply);
		
	const vector<CCombatPhrase::TTargetInfos> & targets = phrase->getTargets();
	const uint nbTargets = (uint)targets.size();
	for (uint i = 0 ; i < nbTargets ; ++i)
	{
		//if ( !phrase->hasTargetDodged(i) )
		if ( phrase->getTargetDodgeFactor(i) == 0.0f )
			applyOnTarget(i,phrase);
	}

	// change behaviour
	MBEHAV::CBehaviour &behav = phrase->getExecutionBehaviour();
	if ( behav.isCreatureAttack() )
	{
		// get creature level
		const CCombatAttacker *attacker = phrase->getAttacker();
		if (attacker)
			behav.CreatureAttack.MagicImpactIntensity = 1 + attacker->getSkillValue(BarehandCombatSkill, EGSPD::CPeople::Unknown) / 250;
	}
} // apply //

//--------------------------------------------------------------
//					applyOnTarget()  
//--------------------------------------------------------------
void CCombatAiActionEffect::applyOnTarget(uint8 targetIndex, CCombatPhrase *phrase)
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif
	
	const CCombatDefenderPtr &targetDefender = phrase->getTarget(targetIndex);
	if(!targetDefender) return;
	
	CEntityBase *entity = targetDefender->getEntity();
	if (!entity)
	{
		nlwarning("COMBAT : <CCombatAiActionEffect::applyOnTarget> Cannot find the target entity, cancel");
		return;
	}
	
//	applyOnEntity(entity, phrase->getPhraseSuccessDamageFactor());
	applyOnEntity(entity, 1.0f-phrase->getTargetDodgeFactor(targetIndex));
	
} // applyOnTarget //


//--------------------------------------------------------------
//					applyOnEntity()  
//--------------------------------------------------------------
void CCombatAiActionEffect::applyOnEntity( CEntityBase *entity, float successFactor )
{
	if (!entity || !_EffectDuration) return;
	
	// if entity is already dead, return
	if (entity->isDead())
		return;
	
	const TGameCycle endDate = TGameCycle(_EffectDuration * successFactor) + CTickEventHandler::getGameCycle();

	CSTimedEffect *effect = IEffectFactory::buildEffect(_EffectFamily);
	if (effect)
	{
		effect->setCreatorRowId(_ActorRowId);
		effect->setFamily(_EffectFamily);
		effect->setTargetRowId(entity->getEntityRowId());
		effect->setParamValue(_ParamValue);
		effect->setPower((uint32)_ParamValue);
		effect->setEndDate(endDate);
		effect->isStackable(false);
		
		entity->addSabrinaEffect(effect);
	}
	
} // applyOnEntity //


CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiBlindFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::Blind);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiFearFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::Fear);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiMezzFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::Mezz);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiRootFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::Root);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiSnareFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::SlowMove);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiSlowMagicFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::SlowMagic);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiSlowRangeFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::SlowRange);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiSlowMeleeFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::SlowMelee);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiMeleeMadnessFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::MeleeMadness);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiRangeMadnessFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::RangeMadness);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiMagicMadnessFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::MagicMadness);

CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffMeleeFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::SkillDebufMelee );
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffRangeFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::SkillDebufRange );
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffMagicFactoryInstance = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::SkillDebufMagic);

CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffAcidResistFactory = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::ResistDebufAcid);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffColdResistFactory = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::ResistDebufCold);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffRotResistFactory = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::ResistDebufRot);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffFireResistFactory = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::ResistDebufFire);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffElectricyResistFactory = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::ResistDebufElectric);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffPoisonResistFactory = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::ResistDebufPoison);
CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiDebuffShockResistFactory = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::ResistDebufShockwave);

CCombatAIActionTFactory<CCombatAiActionEffect> *CCombatAiReverseDamageFactory = new CCombatAIActionTFactory<CCombatAiActionEffect>(AI_ACTION::ReverseDamage);


