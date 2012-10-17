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



#ifndef RY_COMBAT_ACTION_STUN_H
#define RY_COMBAT_ACTION_STUN_H

#include "nel/misc/types_nl.h"
#include "combat_action.h"
#include "combat_stun_effect.h"


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatActionStun : public CCombatAction
{
public:
	/// Constructor
	CCombatActionStun() : _StunDuration(0), _StunDurationResisted(0), _StunEffect(NULL)
	{}

	/// Constructor
	CCombatActionStun(const TDataSetRow & actorRowId, CCombatPhrase *phrase, NLMISC::TGameCycle duration, NLMISC::TGameCycle durationResisted, uint16 level)
	: _StunDuration(duration), _StunDurationResisted(durationResisted), _StunEffect(NULL), _StunLevel(level)
	{
		_CombatPhrase = phrase;
		if (actorRowId.isValid() )
			_ActorRowId = actorRowId;
	}

	/// build
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, uint &brickIndex, CCombatPhrase * phrase );

	/// validate the combat action
	virtual bool validate(CCombatPhrase *phrase, std::string &errorCode) { return true; }

	/// apply combat action effects
	virtual void apply(CCombatPhrase *phrase);

private:
	/// stun duration if no resist (in ticks)
	NLMISC::TGameCycle		_StunDuration;

	/// stun duration if resisted (in ticks)
	NLMISC::TGameCycle		_StunDurationResisted;

	/// stun relative power (used to test target resistance)
	uint16					_StunLevel;

	/// the stun effect
	CCombatStunEffect		*_StunEffect;
};


#endif // RY_COMBAT_ACTION_STUN_H

/* End of combat_action_stun.h */
