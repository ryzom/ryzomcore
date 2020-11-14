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



#ifndef RYZOM_COMBAT_H
#define RYZOM_COMBAT_H

#include "nel/misc/types_nl.h"

namespace COMBAT_HISTORY
{
	enum TCombatHistory
	{
		BeginMiss = 0,
			Miss = BeginMiss,
			Fumble,
			Parry,
			Dodge,
		EndMiss = Dodge,

		BeginHit,
			HitHead = BeginHit,
			HitChest,
			HitArms,
			HitHands,
			HitLegs,
			HitFeet,

		BeginCriticalHit,
			CriticalHitHead = BeginCriticalHit,
			CriticalHitChest,
			CriticalHitArms,
			CriticalHitHands,
			CriticalHitLegs,
			CriticalHitFeet,
		EndCriticalHit = CriticalHitFeet,
		EndHit = EndCriticalHit,

		NbHistoryEvent,
		Unknown = NbHistoryEvent, 
	};

	/// convert an history entry to a string
	const std::string &toString(TCombatHistory event);
}; // COMBAT_HISTORY //

/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombat
{
public:

	/// Constructor
	CCombat()
	{
		_History.resize(_MaxHistoryDepth, COMBAT_HISTORY::Unknown);
		_CurrentIndex = 0;
		_Melee = true;
	}

	/// Constructor
	CCombat( const TDataSetRow &attackerRowId, const TDataSetRow &targetRowId, bool melee = true ) : _AttackerRowId(attackerRowId), _TargetRowId(targetRowId), _Melee(melee)
	{
		_History.resize(_MaxHistoryDepth, COMBAT_HISTORY::Unknown);
		_CurrentIndex = 0;
	}

	/// get attacker row id
	inline const TDataSetRow &attacker() const { return _AttackerRowId; }
	/// get target row id
	inline const TDataSetRow &target() const { return _TargetRowId; }
	/// get combat history
	inline const std::vector<COMBAT_HISTORY::TCombatHistory> &history() const { return _History; }

	/// get history event of given depth
	inline COMBAT_HISTORY::TCombatHistory historyEvent(uint8 depth) const 
	{
		nlassert( _MaxHistoryDepth == _History.size() );

		if(depth < _MaxHistoryDepth )
			return _History[ (_MaxHistoryDepth + _CurrentIndex - depth) % _MaxHistoryDepth ];
		else
		{
			nlwarning("<CCombat::historyEvent> asked for event depth %u, but max history size is %u", depth, _MaxHistoryDepth);
			return COMBAT_HISTORY::Unknown;
		}
	}

	/// push an event in the history
	inline void pushEvent(COMBAT_HISTORY::TCombatHistory event)
	{
		const uint8 size = _History.size();
		nlassert( _MaxHistoryDepth == size );

		_CurrentIndex = (_CurrentIndex+1)%_MaxHistoryDepth;
		_History[_CurrentIndex] = event;
	}

	/// static : max history depth
	static uint8 maxHistoryDepth() { return _MaxHistoryDepth; }

private:
	/// attacker
	TDataSetRow		_AttackerRowId;

	/// target
	TDataSetRow		_TargetRowId;

	/// combat History
	std::vector<COMBAT_HISTORY::TCombatHistory> _History;

	/// current index in History
	uint8			_CurrentIndex;

	/// range or melee combat
	uint8			_Melee;

	static uint8	_MaxHistoryDepth;
};


#endif // RYZOM_COMBAT_H

/* End of combat.h */
