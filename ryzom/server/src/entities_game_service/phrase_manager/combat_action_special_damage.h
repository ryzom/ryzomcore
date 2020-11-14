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



#ifndef RY_COMBAT_ACTION_SPECIAL_DAMAGE_H
#define RY_COMBAT_ACTION_SPECIAL_DAMAGE_H

#include "combat_action.h"
#include "game_share/damage_types.h"

/**
 * Class managing special damage (fire, cold...) for combat
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatActionSpecialDamage : public CCombatDynamicAction
{
public:
	/// Constructor
	CCombatActionSpecialDamage(): CCombatDynamicAction()
	{
	}

	/// Constructor
	CCombatActionSpecialDamage(const TDataSetRow & actorRowId, CCombatPhrase *phrase, DMGTYPE::EDamageType dmgType) : _DamageType(dmgType)
	{
		_CombatPhrase = phrase;
		
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CCombatActionSpecialDamage> invalid data set row passed as actor");
		}
	}

	/// validate the combat action
	virtual bool validate(CCombatPhrase *phrase, std::string &errorCode) { return true; }

	/// apply combat action effects
	virtual void apply(CCombatPhrase *phrase);

private:
	/// apply combat action effects on target
	void applyOnTarget( uint8 targetIndex, CCombatPhrase *phrase);

	// type of damage
	DMGTYPE::EDamageType	_DamageType;
};


#endif // RY_COMBAT_ACTION_SPECIAL_DAMAGE_H

/* End of combat_action_special_damage.h */
