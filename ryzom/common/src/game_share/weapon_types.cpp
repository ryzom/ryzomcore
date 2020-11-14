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

#include "nel/misc/debug.h"
#include "weapon_types.h"


namespace WEAPONTYPE
{
	EWeaponType stringToWeaponType(const std::string &str)
	{
		if (str == "LIGHT" || str == "light" )
			return LIGHT;

		if (str == "MEDIUM" || str == "medium" )
			return MEDIUM;

		if (str == "HEAVY" || str == "heavy")
			return HEAVY;

		if (str == "HANDS" || str == "hands" )
			return HANDS;

		if (str == "LIGHT_GUN" || str == "light gun")
			return LIGHT_GUN;

		if (str == "MEDIUM_GUN" || str == "medium gun")
			return MEDIUM_GUN;

		if (str == "HEAVY_GUN" || str == "heavy gun")
			return HEAVY_GUN;

		if (str == "UNKNOWN" || str == "unknown")
			return UNKNOWN;

		nlwarning("<stringToWeaponType> Unknown type %s", str.c_str() );
		return UNKNOWN;
	}



	std::string toString(EWeaponType type)
	{
		if (type == LIGHT )
			return "LIGHT";

		if (type == MEDIUM )
			return "MEDIUM";

		if (type == HEAVY )
			return "HEAVY";

		if (type == HANDS )
			return "HANDS";

		if (type == LIGHT_GUN )
			return "LIGHT_GUN";

		if (type == MEDIUM_GUN )
			return "MEDIUM_GUN";

		if (type == HEAVY_GUN )
			return "HEAVY_GUN";

		if (type == UNKNOWN )
			return "UNKNOWN";

		nlwarning("<toString> Unknown type %d", type );
		return "UNKNOWN";
	}
}; // WEAPONTYPE
