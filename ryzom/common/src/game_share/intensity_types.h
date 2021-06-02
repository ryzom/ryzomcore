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



#ifndef RY_INTENSITY_TYPES_H
#define RY_INTENSITY_TYPES_H

#include "nel/misc/types_nl.h"

namespace INTENSITY_TYPE
{
	// Attack intensity
	enum TAttackIntensity
	{
		NONE = 0,
		WEAK,
		STANDARD,
		STRONG,
		CRITICAL_HIT,

		WEAK_COMBO,
		STRONG_COMBO,

		UNDEFINED_ATTACK	// = Nb intensity too.
	};


	/**
	 * get the right attack intensity from the input string
	 * \param str the input string
	 * \return the TAttackIntensity associated to this string (UNDEFINED_ATTACK if the string cannot be interpreted)
	 */
	TAttackIntensity stringToAttackIntensity(const std::string &str);

	/**
	 * get the TAttackIntensity type as a string
	 * \param type the TAttackIntensity size
	 * \return the string associated to this TAttackIntensity
	 */
	const std::string & attackIntensityToString(TAttackIntensity type);


	// Impact intensity
	enum TImpactIntensity
	{
		IMPACT_NONE = 0,
		IMPACT_INSIGNIFICANT,	// < 2% Hp Max
		IMPACT_VERY_WEAK,		// < 5%
		IMPACT_WEAK,			// < 10%
		IMPACT_AVERAGE,			// < 20%
		IMPACT_STRONG,			// < 30%
		//IMPACT_VERY_STRONG,		// < 40%
		//IMPACT_HUGE,			// > 40%

		NB_IMPACT,
		IMPACT_UNDEFINED = NB_IMPACT,		// = Nb Impact too.
	};

	/**
	 * get the right impact intensity from the input string
	 * \param str the input string
	 * \return the TImpactIntensity associated to this string (IMPACT_UNDEFINED if the string cannot be interpreted)
	 */
	TImpactIntensity stringToImpactIntensity(const std::string &str);


	/**
	 * get the TImpactIntensity type as a string
	 * \param type the TImpactIntensity size
	 * \return the string associated to this TImpactIntensity
	 */
	const std::string & impactIntensityToString(TImpactIntensity type);

}; // INTENSITY_TYPE

#endif // RY_INTENSITY_TYPES_H
/* End of intensity_types.h */
