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




#ifndef RY_INTERFACE_FLAGS_H
#define RY_INTERFACE_FLAGS_H

#include "skills.h"

namespace INTERFACE_FLAGS
{
	enum TInterfaceFlag
	{
		Magic = 0,
		Combat,
		Special,
		Commerce,
		FaberCreate,
		FaberRefine,
		FaberRepair,
		Tracking,

		NBFLAGS, // 8

		Unknown,
	};

	/**
	  * get the right string from the given enum value
	  * \param jop the TInterfaceFlag value to convert
	  * \return the string associated to this enum number (Unknown if the enum number not exist)
	  */
	const std::string& toString( TInterfaceFlag type );

	/**
	  * get the right TInterfaceFlag from its string
	  * \param str the input string
	  * \return the TInterfaceFlag associated to this string (unknown if the string cannot be interpreted)
	  */
	TInterfaceFlag toInterfaceFlag( const std::string& str );

	/**
	 * convert a skill to an interfaceFlag
	 * \param skill the skill to convert
	 * \return the related interface flag
	 */
	TInterfaceFlag toInterfaceFlag( SKILLS::ESkills skill );

	/**
	 * convert an interfaceFlag to the related skill if any
	 * \param flag the flag to convert
	 * \return the related skill
	 */
	SKILLS::ESkills toSkill( TInterfaceFlag flag );

} // INTERFACE_FLAGS

#endif // RY_INTERFACE_FLAGS_H //

