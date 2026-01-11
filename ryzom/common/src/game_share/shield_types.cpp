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
#include "shield_types.h"


namespace SHIELDTYPE
{

// ***************************************************************************
EShieldType stringToShieldType(const std::string &str)
{
	if (str == "SMALL_SHIELD" || str == "small shield" )
		return SMALL_SHIELD;

	if (str == "LARGE_SHIELD" || str == "large shield" )
		return LARGE_SHIELD;

	if (str == "NONE" || str == "none")
		return NONE;

	nlwarning("<stringToShieldType> Unknown type %s", str.c_str() );
	return NONE;
}

// ***************************************************************************
static const std::string StringArray[NUM_SHIELD_TYPE]=
{
	"NONE",
	"SMALL_SHIELD",
	"LARGE_SHIELD",
};

// ***************************************************************************
const std::string &toString(EShieldType e)
{
	nlassert(e<NUM_SHIELD_TYPE);
	return StringArray[e];
}

// ***************************************************************************
/*SKILLS::ESkills		shieldTypeToSkill(EShieldType e)
{
	nlctassert( (sizeof(ShieldToSkill)/sizeof(ShieldToSkill[0])) == NUM_SHIELD_TYPE );

	if(e>=NUM_SHIELD_TYPE)
		return SKILLS::unknown;

	return ShieldToSkill[e];
}
*/

}; // SHIELDTYPE
