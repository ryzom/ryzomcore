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



#ifndef RY_MEMORIZATION_SET_TYPE_H
#define RY_MEMORIZATION_SET_TYPE_H

#include "nel/misc/types_nl.h"
#include "skills.h"

namespace MEM_SET_TYPES
{
	// New System. Only 10 memories are allowed
	enum
	{
		NumMemories= 10,
	};

	// Deprecated
/*	enum TMemorizationSetType
	{
		EmptyHand = 0,

		Autolaunch,
		Axe,
		Bowpistol,
		Bowrifle,
		Dagger,
		Grenade,
		Harpoon,
		Launcher,
		Mace,
		Pike,
		Pistol,
		Rifle,
		Spear,
		Staff,
		Sword,

		TwoHandedAxe,
		TwoHandedMace,
		TwoHandedSword,

		Craft,
		Harvest,

		NbMemSetTypes,
		Unknown = NbMemSetTypes,
	};

	// Table MemSetType to Skill.
	const	SKILLS::ESkills		MemToSkill[]=
	{
		SKILLS::SFMCH,

		SKILLS::SFR2A,
		SKILLS::SFM1SA,
		SKILLS::SFR1B,
		SKILLS::SFR2B,
		SKILLS::SFMCD, // dagger
		SKILLS::SFR1P, //grenade skill missing
		SKILLS::SFR1P, //harpoon missing
		SKILLS::SFR2L,
		SKILLS::SFM1BM,
		SKILLS::SFM2PP,
		SKILLS::SFR1P,
		SKILLS::SFR2R,
		SKILLS::SFM1PS,
		SKILLS::SFM1BS,
		SKILLS::SFM1SS,

		SKILLS::SFM2SA,
		SKILLS::SFM2BM,
		SKILLS::SFM2SS,

		SKILLS::SC,
		SKILLS::SH,
	};
*/
	/**
	 * get the right mem set type from the input string
	 * \param str the input string
	 * \return the TMemorizationSetType associated to this string (Unknown if the string cannot be interpreted)
	 */
/*	TMemorizationSetType toMemSetType(const std::string &str);

	/// convert to a string
	const std::string &toString(TMemorizationSetType type);

	/// convert an memSet to the associated skill
	SKILLS::ESkills memSetTypeToSkill(TMemorizationSetType memType);

	/// convert an itemSkill to the associated memorization set type
	TMemorizationSetType skillToMemSetType(SKILLS::ESkills skill);
*/
}; // MEM_SET_TYPES

#endif // RY_MEMORIZATION_SET_TYPE_H
/* End of memorization_set_types.h */
