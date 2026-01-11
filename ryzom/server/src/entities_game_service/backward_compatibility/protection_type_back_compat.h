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



#ifndef RY_PROTECTION_TYPE_BACK_COMPAT_H
#define RY_PROTECTION_TYPE_BACK_COMPAT_H

#include "nel/misc/types_nl.h"

namespace BACK_COMPAT
{
namespace OLD_PROTECTION_TYPE
{

enum TOldProtectionType
{
	Cold = 0,
	Acid,
	Rot,
	Fire,			// Fyros speciality
	Shockwave,		// Tryker speciality
	Poison,			// Matis speciality
	Electricity,	// Zorai speciality
	Madness,
	Slow,
	Snare,
	Sleep,
	Stun,
	Root,
	Blind,
	Fear,
	
	None,
	
	NB_OLD_PROTECTION_TYPE = None
};

TOldProtectionType fromString(const std::string & str);
const std::string & toString(TOldProtectionType oldType);

} // namespace OLD_PROTECTION_TYPE
} // namespace BACK_COMPAT


#endif // RY_PROTECTION_TYPE_BACK_COMPAT_H

