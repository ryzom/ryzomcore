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



#ifndef RY_PACT_H
#define RY_PACT_H

/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"

#include <string>

// Name space People in game share
namespace GSPACT
{
	// All different people
	enum EPactNature
	{
		Unknown = 0,
		Kamique,
		Caravane,

		NUM_PACT_NATURE = Caravane // 2
	};

	enum EPactType
	{
		Type1 = 0,
		Type2,
		Type3,
		Type4,
		Type5,
		Type6,
		UnknownType,

		NUM_PACT_TYPE = UnknownType // 6
	};


/// Return a string according to the enum parameter
const std::string& toString (EPactNature pactNature);

/// Return a string according to the enum parameter
const std::string& toString (EPactType pactType);

};// namespace GSPACT





#endif // RY_PACT_H

/* End of pact.h */
