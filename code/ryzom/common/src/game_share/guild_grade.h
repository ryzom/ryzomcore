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


#ifndef GUILD_GRADE_H
#define GUILD_GRADE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <vector>
#include <map>

// User #includes

namespace EGSPD
{

//
// Forward declarations
//



//
// Typedefs & Enums
//

/** TGuildGrade
 * defined at game_share/pd_scripts/guild_grade.pds:6
 */
class CGuildGrade
{

public:

	/// \name Enum values
	// @{

	enum TGuildGrade
	{
		Leader = 0,
		HighOfficer = 1,
		Officer = 2,
		Member = 3,
		___TGuildGrade_useSize = 4,
		Unknown = 4,
		EndGuildGrade = 4,
	};

	// @}


public:

	/// \name Conversion methods
	// @{

	/**
	 * Use these methods to convert from enum value to string (and vice versa)
	 */

	static const std::string&		toString(TGuildGrade v);
	static CGuildGrade::TGuildGrade	fromString(const std::string& v);

	// @}


private:

	/// \name Enum initialisation
	// @{

	static void						init();
	static bool						_Initialised;
	static std::string				_UnknownString;
	static std::vector<std::string>	_StrTable;
	static std::map<std::string, TGuildGrade>	_ValueMap;

	// @}

};


} // End of EGSPD


//
// Inline implementations
//

#include "guild_grade_inline.h"

#endif
