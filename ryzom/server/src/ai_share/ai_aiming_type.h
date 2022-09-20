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



#ifndef RY_AI_AIMING_TYPE_H
#define RY_AI_AIMING_TYPE_H

#include "nel/misc/types_nl.h"
#include "game_share/slot_equipment.h"

namespace AI_AIMING_TYPE
{
	enum TAiAimingType
	{
		Random = 0,
		Head,
		Chest,
		Arms,
		Hands,
		Legs,
		Feet,
		
		LeastProtected,
		AveragestProtected,
		MostProtected,
		
		Unknown,
	};

	/// convert a flag to a string
	const std::string &toString(TAiAimingType type);

	/// convert a string to a flag
	TAiAimingType toAimingType( const std::string &str);

	/// convert a slot to a combat flag
	SLOT_EQUIPMENT::TSlotEquipment toSlot(TAiAimingType type);

}; // AI_AIMING_TYPE

#endif // RY_AI_AIMING_TYPE_H
/* End of ai_aiming_type.h */
