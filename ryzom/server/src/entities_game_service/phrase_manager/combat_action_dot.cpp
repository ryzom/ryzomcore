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
#include "combat_action_dot.h"
#include "combat_phrase.h"
#include "nolink_dot_effect.h"
#include "game_share/mode_and_behaviour.h"
#include "player_manager/player.h"

using namespace NLMISC;
using namespace std;

extern SKILLS::ESkills		BarehandCombatSkill;

//--------------------------------------------------------------
//					initFromAiAction  
//--------------------------------------------------------------
bool CCombatActionDoT::initFromAiAction( const CStaticAiAction *aiAction, CCombatPhrase *phrase )
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
	
	_EffectDuration = data.EffectTime;
	_TotalDamageValue = (sint32)data.EffectValue;
	_UpdateFrequency = data.EffectUpdateFrequency;
	_AffectedScore = data.EffectAffectedScore;
	_DamageType = data.EffectDamageType;
	
	_EffectFamily = EFFECT_FAMILIES::getCombatDoTEffect(_DamageType);
	
	if (_EffectFamily == EFFECT_FAMILIES::Unknown)
		return false;
	
	return true;
} // initFromAiAction //

//--------------------------------------------------------------
//					apply  
//--------------------------------------------------------------
void CCombatActionDoT::apply( CCombatPhrase *phrase )
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif
	H_AUTO(CCombatActionDoT_apply);
	
	const TGameCycle endDate = CTickEventHandler::getGameCycle() + _EffectDuration;

	const vector<CCombatPhrase::TTargetInfos> &targets = phrase->getTargets();
	const uint nbTargets = (uint)targets.size();
	for (uint i = 0 ; i < nbTargets ; ++i)
	{
//		if ( !phrase->hasTargetDodged(i) )
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
void CCombatActionDoT::applyOnTarget(uint8 targetIndex, CCombatPhrase *phrase)
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif
	
	const CCombatDefenderPtr &targetDefender = phrase->getTarget(targetIndex);
	if(!targetDefender) return;
	
	CEntityBase *entity = targetDefender->getEntity();
	if (!entity)
	{
		nlwarning("COMBAT : <CCombatActionDoT::applyOnTarget> Cannot find the target entity, cancel");
		return;
	}
	
	//applyOnEntity(entity, phrase->getPhraseSuccessDamageFactor());
	applyOnEntity(entity, 1.0f-phrase->getTargetDodgeFactor(targetIndex));
	
} // applyOnTarget //


//--------------------------------------------------------------
//					applyOnEntity()  
//--------------------------------------------------------------
void CCombatActionDoT::applyOnEntity( CEntityBase *entity, float successFactor )
{
	if (!entity || !_EffectDuration) return;
	
	// if entity is already dead, return
	if (entity->isDead())
		return;
	
	const TGameCycle endDate = _EffectDuration + CTickEventHandler::getGameCycle();

	float cycleDamage = float(_TotalDamageValue) * successFactor * float(_UpdateFrequency) / float(_EffectDuration);

	CNoLinkDOTEffect *effect = new CNoLinkDOTEffect( _ActorRowId, entity->getEntityRowId(), _EffectFamily, _TotalDamageValue, endDate, 
		_UpdateFrequency, _AffectedScore, cycleDamage, _DamageType
		);

	if (effect)
	{
		entity->addSabrinaEffect(effect);
	}
		
} // applyOnEntity //


CCombatAIActionTFactory<CCombatActionDoT> *CCombatAiDoTFactoryInstance = new CCombatAIActionTFactory<CCombatActionDoT>(AI_ACTION::Dot);
