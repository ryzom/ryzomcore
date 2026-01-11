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
#include "combat_action_slow_move.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "combat_phrase.h"
#include "change_move_speed_effect.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CCombatActionSlowMove::apply(CCombatPhrase *phrase)
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif

	H_AUTO(CCombatActionSlowMove_apply);

	_CombatPhrase = phrase;

	if (_ApplyOnTargets)
	{
		const std::vector<CCombatPhrase::TTargetInfos> &targets = phrase->getTargets();
		for (uint i = 0; i < targets.size() ; ++i)
		{
			if ( phrase->getTargetDodgeFactor(i) == 0.0f )
			{
				if (targets[i].Target != NULL)
				{
					applyOnEntity(targets[i].Target->getEntity(), 1.0f - phrase->getTargetDodgeFactor(i));
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
		applyOnEntity(entity, 1.0f);
	}
} // apply //


//--------------------------------------------------------------
//					applyOnEntity()  
//--------------------------------------------------------------
void CCombatActionSlowMove::applyOnEntity( CEntityBase *entity, float successFactor )
{
	H_AUTO(CCombatActionSlowMove_applyOnEntity);

	if (!entity || !_CombatPhrase) return;
	
	// if entity is already dead, return
	if (entity->isDead())
		return;
	
	TGameCycle endDate = TGameCycle(_Duration*successFactor) + CTickEventHandler::getGameCycle();

	CChangeMoveSpeedEffect *effect = new CChangeMoveSpeedEffect( _ActorRowId, entity->getEntityRowId(), EFFECT_FAMILIES::CombatMvtSlow, _SpeedModifier, endDate, entity);
	if (!effect)
	{
		nlwarning("COMBAT : <CCombatActionSlowMove::apply> Failed to allocate new CCombatSlowMoveEffect object !");
		return;
	}

	entity->addSabrinaEffect(effect);
} // applyOnEntity //

