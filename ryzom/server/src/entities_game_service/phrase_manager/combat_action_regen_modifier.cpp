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
// net
#include "nel/net/message.h"
//
#include "combat_action_regen_modifier.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "regen_modifier_effect.h"
#include "combat_phrase.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CCombatActionRegenModifier::apply(CCombatPhrase *phrase)
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif
	H_AUTO(CCombatActionRegenModifier_apply);

	_CombatPhrase = phrase;

	if (_ApplyOnTargets)
	{
		const std::vector<CCombatPhrase::TTargetInfos> &targets = phrase->getTargets();
		for (uint i = 0; i < targets.size() ; ++i)
		{
//			if ( !phrase->hasTargetDodged(i) )
			if ( phrase->getTargetDodgeFactor(i) == 0.0f )
			{
				if (targets[i].Target != NULL)
				{
					//applyOnEntity(targets[i].Target->getEntity(), phrase->getPhraseSuccessDamageFactor());
					applyOnEntity(targets[i].Target->getEntity(), 1.0f-phrase->getTargetDodgeFactor(i));
				}
			}
		}
	}
	else
	{
		CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_ActorRowId);
		if (!entity)
		{
			nlwarning("COMBAT : <CCombatActionRegenModifier::apply> Cannot find the target entity, cancel");
			return;
		}
//		applyOnEntity(entity, phrase->getPhraseSuccessDamageFactor());
		applyOnEntity(entity, 1.0f);
	}
} // apply //

//--------------------------------------------------------------
//					applyOnEntity()  
//--------------------------------------------------------------
void CCombatActionRegenModifier::applyOnEntity( CEntityBase *entity, float successFactor )
{
	if (!entity || !_CombatPhrase) return;

	// if entity is already dead, return
	if (entity->isDead())
		return;
	
	TGameCycle	endDate = TGameCycle(_Duration*successFactor) + CTickEventHandler::getGameCycle();

	float factor = getApplyValue(_CombatPhrase->weaponSabrinaValue());

	CRegenModifierEffect *effect = new CRegenModifierEffect(_ActorRowId, entity->getEntityRowId(), _Family, endDate, (uint8)(factor), factor, _AffectedScore);
	if (!effect)
	{
		nlwarning("COMBAT : <CCombatActionRegenModifier::apply> Failed to allocate new CSimpleEffect object !");
		return;
	}

	effect->effectName(_EffectName);

	entity->addSabrinaEffect(effect);

	// check an effect name has been set
	if (_EffectName.empty())
	{
		nlwarning("COMBAT : Effect name not set for effect family %s", EFFECT_FAMILIES::toString(_Family).c_str() );
		return;
	}

	// send messages
	PHRASE_UTILITIES::sendEffectStandardBeginMessages(_ActorRowId, entity->getEntityRowId(), _EffectName);

} // applyOnEntity //
