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




#ifndef RY_TARGET_H
#define RY_TARGET_H

#include <string>

namespace TARGET
{

enum EAggro
{
	Friendly = 0,
	AlmostFriendly,
	Neutral,
	AlmostAggressive,
	Agressive,
	NUM_ENUM_AGGRO
};

enum EForceRatio
{
	ThreeLevelBelowOrLess = 0,
	TwoLevelBelow,
	OneLevelBelow,
	SameLevel,
	OneLevelAbove,
	TwoLevelAbove,
	ThreeLevelAboveOrMore,
	NUM_ENUM_FORCE_RATIO
};

	// Enum for the target's restrictions.
	enum TTargetRestriction
	{
		EveryBody = 0,
		SelfOnly,
		TargetRestrictionCount	
	};
	/**
	 * Get the Enum Value for a given target's restriction in string.
	 * \param str : the input string
	 * \return TTargetRestriction : the target's restriction value (for the enum).
	 */
	TTargetRestriction stringToTargetRestriction(const std::string &str);
	/**
	 * Get the string for a given target's restriction.
	 * \param targetRestriction : target's restriction
	 * \return string : the target's restriction as a string.
	 */
	const std::string & targetRestrictionToString(TTargetRestriction targetRestriction);


} // TARGET

#endif

