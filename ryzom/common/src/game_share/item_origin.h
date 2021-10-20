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



#ifndef RY_ITEM_ORIGIN_H
#define RY_ITEM_ORIGIN_H

#include "nel/misc/types_nl.h"

#include "people.h"

namespace ITEM_ORIGIN
{
	enum EItemOrigin
	{
		COMMON = 0,
		FYROS,
		MATIS,
		TRYKER,
		ZORAI,
		REFUGEE,
		TRIBE,
		KAMI,
		KARAVAN,

		UNKNOWN,
		NUM_ITEM_ORIGIN = UNKNOWN
	};


	/**
	 * get the right EItemOrigin enum from the input string
	 * \param str the input string
	 * \return the EItemOrigin associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	EItemOrigin stringToEnum(const std::string &str);

	/**
	 * get the EItemOrigin type as a string
	 * \param e the EItemOrigin enum
	 * \return the string associated to this EItemOrigin
	 */
	const std::string & enumToString (EItemOrigin e);

	/**
	 * return people enum corresponding to item origin string
	 * \param e the EItemOrigin enum
	 * \return the string associated to this EItemOrigin
	 */
	EGSPD::CPeople::TPeople itemOriginStringToPeopleEnum( const std::string &str );
}; // namespace ITEM_ORIGIN

#endif // RY_ITEM_ORIGIN_H

