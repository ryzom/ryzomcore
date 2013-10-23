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



#ifndef RY_COMBAT_ACTION_BLEED_H
#define RY_COMBAT_ACTION_BLEED_H

#include "combat_action.h"
#include "bleed_effect.h"


/**
 * Class managing the creation of bleed effects for combat (nb: combat bleed effects stacks)
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatActionBleed : public CCombatDynamicAction
{
public:
	/// Constructor
	CCombatActionBleed(): CCombatDynamicAction(), _BleedDuration(0), _BleedCycleDuration(0)
	{
	}

	/// Constructor
	CCombatActionBleed(const TDataSetRow & actorRowId, CCombatPhrase *phrase, NLMISC::TGameCycle totalDuration, NLMISC::TGameCycle cycleDuration)
	: _BleedDuration(totalDuration), _BleedCycleDuration(cycleDuration)
	{
		_CombatPhrase = phrase;
		
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CCombatActionBleed> invalid data set row passed as actor");
		}
	}

	/// validate the combat action
	virtual bool validate(CCombatPhrase *phrase, std::string &errorCode) { return true; }

	/// apply combat action effects
	virtual void apply(CCombatPhrase *phrase);

	/// apply on entity
	virtual void applyOnEntity( CEntityBase *entity, float successFactor );

private:
	/// apply combat action effects on target
	void applyOnTarget( uint8 targetIndex, CCombatPhrase *phrase);

	/// bleed total duration (in ticks)
	NLMISC::TGameCycle		_BleedDuration;

	/// time between two hp loss (in ticks)
	NLMISC::TGameCycle		_BleedCycleDuration;

	// temp vars
	mutable sint32			_TotalBleedDamage;
	mutable float			_CycleDamage;
};


#endif // RY_COMBAT_ACTION_BLEED_H

/* End of combat_action_bled.h */
