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




#ifndef RY_ROLES_H
#define RY_ROLES_H

#include <string>

namespace ROLES
{

enum ERole
{
	unknown = 0,
	role_unknown = unknown,

	fighter,
	caster,
	crafter,
	harvester,

	NB_ROLES,
};

/**
  * get the role sheet string from the given enum
  * \param  R is the enum number
  * \return the string associated to this enum number (Unknown if the enum number not exist)
  */
const std::string& toSheetString( ERole r );

/**
  * get the right role string from the given enum
  * \param  R is the enum number
  * \return the string associated to this enum number (Unknown if the enum number not exist)
  */
const std::string& toString( ERole r );

/**
  * get the right role ID from its string
  * \param str the input string
  * \return the RoleId associated to this string (unknown if the string cannot be interpreted)
  */
ERole toRoleId( const std::string& Role );

// Return the Translated name of the Job
const ucstring &roleToUCString (ERole r);

} // ROLES

#endif

