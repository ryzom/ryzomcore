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
#include "combat_action.h"

NL_INSTANCE_COUNTER_IMPL(CCombatAction);
NL_INSTANCE_COUNTER_IMPL(CCombatAIActionFactory);

CCombatAction * CCombatAIActionFactory::buildAiAction(const CStaticAiAction *aiAction, CCombatPhrase * phrase )
{
#ifdef NL_DEBUG
	nlassert(aiAction);
	nlassert(phrase);
#endif
	const AI_ACTION::TAiActionType actionType = aiAction->getType();
	if (actionType != AI_ACTION::Melee && actionType != AI_ACTION::Range) return NULL;

	AI_ACTION::TAiEffectType effectType = aiAction->getData().Combat.EffectFamily;
	
	//get appropriate factory
	for ( uint i = 0; i < Factories->size(); i++ )
	{
		if ( (*Factories)[i].first == effectType )
		{
			return (*Factories)[i].second->build(aiAction,phrase);
		}
	}
	nlwarning( "<CCombatActionFactory buildAction> the AIEffect type %s has no corresponding combat action class", AI_ACTION::toString(effectType).c_str() );
	return NULL;
}
