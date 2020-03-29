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

#include "stdpch.h"

#include "nel/misc/debug.h"
#include "place_type.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace PLACE_TYPE
{
	const CStringConversion<TPlaceType>::CPair stringTable [] =
	{
		{ "Capital", Capital },
		{ "Village", Village },
		{ "Outpost", Outpost },
		{ "Locality", Place },
		{ "Street", Street },
		{ "Undefined", Undefined },
	};		
	CStringConversion<TPlaceType> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]), Undefined);


	//-----------------------------------------------
	// fromString :
	//-----------------------------------------------
	TPlaceType fromString(const std::string &str)
	{
		return conversion.fromString(str);
	}


	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const std::string& toString(TPlaceType place_type)
	{
		return conversion.toString(place_type);
	}
};
