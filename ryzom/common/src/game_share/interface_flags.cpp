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
#include "interface_flags.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace INTERFACE_FLAGS
{
	// The conversion table
	const CStringConversion<TInterfaceFlag>::CPair stringTable [] =
	{
		{ "Magic", Magic },
		{ "Combat", Combat },
		{ "Special", Special },
		{ "Commerce", Commerce },
		{ "FaberCreate", FaberCreate },
		{ "FaberRefine", FaberRefine },
		{ "FaberRepair", FaberRepair },
		{ "Tracking", Tracking},

		{ "Unknown", Unknown },
	};

	CStringConversion<TInterfaceFlag> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  Unknown);



// convert type id to type name string
const std::string& toString( TInterfaceFlag type )
{
	return conversion.toString(type);
}

// convert type name to type enum value
TInterfaceFlag toInterfaceFlag( const std::string& str )
{
	return conversion.fromString(str);
}

// convert a skill to an interface flag
TInterfaceFlag toInterfaceFlag( SKILLS::ESkills skill )
{
	return conversion.fromString( SKILLS::toString(skill) );
}

// convert an interface flag to a skill
SKILLS::ESkills toSkill( TInterfaceFlag flag )
{
	return SKILLS::toSkill(conversion.toString(flag));
}

} // INTERFACE_FLAGS
