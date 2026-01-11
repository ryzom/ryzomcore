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



#ifndef RY_ECOSYSTEM_H
#define RY_ECOSYSTEM_H

#include "nel/misc/types_nl.h"

namespace ECOSYSTEM
{
	// Please do not change the ordering of these (otherwise change arrays [ECOSYSTEM::NUM_ECOSYSTEM])
	enum EECosystem
	{
		common_ecosystem = 0,
		desert,
		forest,
		lacustre,
		jungle,
		goo,
		primary_root,

		unknown,
		NUM_ECOSYSTEM = unknown
	};


	/**
	 * get the right ecosystem enum from the input string (case-unsensitive comparison)
	 * \param str the input string
	 * \return the EECosystem associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	EECosystem stringToEcosystem(const std::string &str);

	/**
	 * get the ecosystem type as a string
	 * \param cat the EECosystem cat
	 * \return the string associated to this EECosystem
	 */
	const std::string & toString (EECosystem e);

}; // namespace ECOSYSTEM


// Currently, ecotypes are mapped to ecosystems
typedef ECOSYSTEM::EECosystem TEcotype;


#endif // RY_ECOSYSTEM_H
/* End of ecosystem.h */
