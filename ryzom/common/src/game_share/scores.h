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



#ifndef RY_SCORES_H
#define RY_SCORES_H

#include "nel/misc/types_nl.h"

#include <string>

namespace SCORES
{
	enum TScores
	{
		hit_points = 0,
		stamina,
		sap,
		focus,

		NUM_SCORES,
		unknown = NUM_SCORES
	};

	/**
	 * get the right score enum from the input string
	 * \param str the input string
	 * \return the TScores associated to this string (Unknown if the string cannot be interpreted)
	 */
	TScores toScore( const std::string &str );

	/**
	 * get the right score string from the gived enum
	 * \param s is the enum number
	 * \return the string associated to this enum number (Unknown if the enum number not exist)
	 */
	const std::string& toString( TScores s );
	const std::string& toString( uint s );

}; // SCORES

#endif // RY_SCORES_H
/* End of scores.h */
