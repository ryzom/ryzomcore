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



#ifndef RY_GENDER_H
#define RY_GENDER_H

/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"

#include <string>

// Namespace People in game share
namespace GSGENDER
{
	// All different gender
	enum EGender
	{
		male = 0,
		female,
		neutral,
		unknown
	};

	const std::string StringMale("Male");
	const std::string StringFemale("Female");
	const std::string StringNeutral("Neutral");
	const std::string StringUnknown("Unknown");

	/// Return an enum according to the string parameter.
	inline EGender stringToEnum(const std::string &str)
	{
		if( str == StringMale )
		{
			return male;
		}
		else if( str == StringFemale )
		{
			return female;
		}
		else if( str == StringNeutral )
		{
			return neutral;
		}
		return unknown;
	}

	/// Return a string according to the enum parameter
	inline const std::string& toString( EGender gender )
	{
		switch( gender )
		{
			case male:
				return StringMale;
			case female:
				return StringFemale;
			case neutral:
				return StringNeutral;
			default:
				return StringUnknown;
		}
	}
};// namespace GSGENDER


#endif // RY_GENDER_H

/* End of gender.h */
